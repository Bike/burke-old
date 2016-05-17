;;;; Rewriting this thing again and again is stupid.
;;;; Stop. You know a new one will work just as well as this one.
;;;; Which is to say, not perfectly, because perfection is protean.
;;;; But good enough for you.
;;;; And you know how this one works. eval, combine. Call #'repl.

(defpackage #:burke
  (:use #:cl #:alexandria)
  (:export #:repl)
  (:shadow #:eval))

(in-package #:burke)

(defvar *standard-library* "/home/aeshtaer/src/burke/standard.l")

;;; Runtime objects
;; vectors, symbols, numbers are just CL
;; so are conses
;; combiners, booleans, environments are novel
;; NIL is an empty list, so uh, don't use it as a symbol

;; named objects for debugging
;; Definitely only for debugging. Unlike CL, there's no global
;;  environment taking precedent over everything, so two Burke
;;  objects might print the same by being named the same thing
;;  in two different environments.
;; Plus renaming/etc. objects is more common...
(defclass named ()
  ((name :initarg :name :accessor name))
  (:documentation "Objects named by SET-IN. See that fsubr."))

(declaim (inline nameable? named?))
(defun nameable? (o) (typep o 'named))
(defun named? (o)
  (and (nameable? o)
       (slot-boundp o 'name)))
(defgeneric maybe-set-name (name object)
  (:documentation "Set the name of object and subsidiaries, but only if they don't have names already.")
  (:method (name object)
    (when (and (nameable? object) (not (named? object)))
      (setf (name object) name))))

;; eval
(defclass combiner (named) ())
(defclass fsubr (combiner)
  ((cl-function :initarg :fun :accessor fsubr-function)))
(defclass subr (combiner)
  ((parameters :initarg :params :accessor subr-parameters)
   (environment-parameter :initarg :eparam :accessor subr-eparam)
   (closure-environment :initarg :env :accessor subr-env)
   (body :initarg :body :accessor subr-body)))
(defclass applicative (combiner)
  ((underlying :initarg :underlying
	       :accessor applicative-underlying)))
(defclass macro (combiner)
  ((macro-combiner :initarg :combiner :accessor macro-combiner)))

(defmethod maybe-set-name :before (name (object applicative))
  (maybe-set-name name (applicative-underlying object)))
(defmethod maybe-set-name :before (name (object macro))
  (maybe-set-name name (macro-combiner object)))

;; singletons
(defclass singleton ()
  ((id :initarg :id :accessor singleton-id)
   (text :initarg :text :accessor singleton-text)))
(defvar *singleton-last-id* 0)
(defun make-singleton (text)
  (prog1 (make-instance 'singleton
			:text text :id *singleton-last-id*)
    (incf *singleton-last-id*)))

(defvar *true* (make-singleton "#t"))
(defvar *false* (make-singleton "#f"))
(defvar *inert* (make-singleton "#inert"))
(defvar *ignore* (make-singleton "#ignore"))

;; environments
(defclass basic-environment ()
  ((parents :initarg :parents :accessor environment-parents)
   (bindings :initarg :binds :accessor environment-bindings)
   (mutate? :initarg :mutate? :accessor can-mutate-binding?)
   (add? :initarg :add? :accessor can-add-binding?)))
(defclass alist-environment (basic-environment) ())
(defclass hash-environment (basic-environment) ())
(defvar *empty-environment*
  (make-singleton "#<empty environment>"))
(defmethod can-mutate-binding? ((env (eql *empty-environment*)))
  nil)
(defmethod can-add-binding? ((env (eql *empty-environment*)))
  nil)

;;; Read
;; use CL's, even though it could be considered wrong,
;;  because parsing is annoying to write.
(defvar *burke-readtable* (copy-readtable nil))

(defun \#i-reader (stream sub num)
  (declare (ignore sub))
  (assert (null num)) ; #4i not allowed.
  (let ((rest (make-string 4)))
    (assert (= (read-sequence rest stream) 4))
    (cond ((string= rest "nert") *inert*)
	  ((string= rest "gnor")
	   (assert (char= (read-char stream) #\e))
	   *ignore*)
	  (t (error "bad #i")))))

(defun \#t-reader (stream sub num)
  (declare (ignore stream sub))
  (assert (null num))
  *true*)

(defun \#f-reader (stream sub num)
  (declare (ignore stream sub))
  (assert (null num))
  *false*)

(set-dispatch-macro-character #\# #\i
			      #'\#i-reader *burke-readtable*)
(set-dispatch-macro-character #\# #\t
			      #'\#t-reader *burke-readtable*)
(set-dispatch-macro-character #\# #\f
			      #'\#f-reader *burke-readtable*)

(defun burke-read (&optional (stream *standard-input*)
		     (eof-error-p t) eof-value recursive-p)
  (let ((*package* (find-package "BURKE"))
	(*readtable* *burke-readtable*))
    (read stream eof-error-p eof-value recursive-p)))

;;; Write

(defmethod print-object ((object singleton) stream)
  (write-string (singleton-text object) stream))

(defmethod print-object ((object combiner) stream)
  (if (named? object)
      (print-unreadable-object (object stream :type t)
	(write (name object) :stream stream))
      (print-unreadable-object
	  (object stream :type t :identity t))))

(defmethod print-object ((object basic-environment) stream)
  (print-unreadable-object (object stream :type t :identity t)))

;;; Evaluation

(defgeneric eval (form env))
(defgeneric combine (combiner combinand env))

(defmethod eval (form env) (declare (ignore env)) form)
(defmethod eval ((form null) env)
  (declare (ignore env))
  form)
(defmethod eval ((form symbol) env)
  (multiple-value-bind (value bound?)
      (lookup env form)
    (if bound?
	value
	(error "Unbound: ~a" form))))
(defmethod eval ((form cons) env)
  (combine (eval (car form) env) (cdr form) env))

(defmethod combine ((combiner fsubr) combinand env)
  (funcall (fsubr-function combiner) combinand env))
(defmethod combine ((combiner subr) combinand env)
  (let ((new (make-subr-env
	      (subr-env combiner)
	      (subr-eparam combiner) env
	      (subr-parameters combiner) combinand)))
    (eval (subr-body combiner) new)))
(defmethod combine ((combiner applicative) combinand env)
  (combine (applicative-underlying combiner)
	   (evlis combinand env)
	   env))

(defmethod combine ((combiner macro) combinand env)
  (eval
   (combine (macro-combiner combiner) combinand env)
   env))

(defun evlis (forms env)
  (mapcar (lambda (form) (eval form env)) forms))

;; could generalize this to have different kinds of param list
;; right now it's scheme style
(defun make-subr-env (parent eparam env params args)
  (let* ((binds
	  (if (and params (symbolp params))
	      (list (cons params args))
	      (loop for (param . restp) on params
		 ;; parameter list is checked valid at creation
		 when (symbolp param)
		 collect (cons param (car args)) into binds
		 when (and restp (symbolp restp)) ; for nil
		 collect (cons restp (cdr args)) into binds
		 and do (loop-finish)
		 do (setf args (cdr args))
		 finally (return binds))))
	 (binds (if (symbolp eparam)
		    (acons eparam env binds)
		    binds)))
    (make-instance 'alist-environment
		   :mutate? t
		   :add? nil
		   :parents (list parent)
		   :binds binds)))

;;; Environments

(defgeneric lookup (env name))
(defgeneric (setf lookup) (nv env name)
  (:argument-precedence-order env name nv))
(defgeneric bound? (env name))
(defgeneric local-lookup (env name))
(defgeneric (setf local-lookup) (nv env name)
  (:argument-precedence-order env name nv))
(defgeneric local-bound? (env name))
(defgeneric dump-environment (env)) ; for debug

(defmethod lookup (env name)
  (cond ((local-bound? env name) (local-lookup env name))
	(t (map nil (lambda (env)
		      (multiple-value-bind (value bound?)
			  (lookup env name)
			(when bound?
			  (return-from lookup
			    (values value bound?)))))
		(environment-parents env))
	   (values nil nil))))
;; semantics: tries to do everything at the low level first.
;; this means, importantly, that if E can add bindings and has a
;;  parent F that can add bindings, a (setf lookup) on E or its
;;  children will never add to F.
(defmethod (setf lookup) (nv env name)
  (cond ((or (can-add-binding? env)
	     (and (can-mutate-binding? env)
		  (local-bound? env name)))
	 (setf (local-lookup env name) nv))
	(t (map nil (lambda (env)
		      (multiple-value-bind (value bound?)
			  (setf (lookup env name) nv)
			(when bound?
			  (return-from lookup
			    (values value bound?)))))
		(environment-parents env))
	   (values nil nil))))
(defmethod (setf lookup) (nv (env (eql *empty-environment*)) name)
  (values nil nil))
(defmethod lookup ((env (eql *empty-environment*)) name)
  (values nil nil))

(defmethod bound? (env name)
  (or (local-bound? env name)
      (some (lambda (env) (bound? env name))
	    (environment-parents env))))

;; NOTE: the setf local- methods don't check mutability constraints
(defmethod local-lookup ((env (eql *empty-environment*)) name)
  (values nil nil))
(defmethod (setf local-lookup)
    (nv (env (eql *empty-environment*)) name)
  (values nil nil))
(defmethod local-lookup ((env alist-environment) name)
  (assoc-value (environment-bindings env) name))
(defmethod (setf local-lookup) (nv (env alist-environment) name)
  (values (setf (assoc-value (environment-bindings env) name)
		nv)
	  t))
(defmethod local-lookup ((env hash-environment) name)
  (gethash name (environment-bindings env)))
(defmethod (setf local-lookup) (nv (env hash-environment) name)
  (values (setf (gethash name (environment-bindings env)) nv) t))

(defmethod local-bound? ((env (eql *empty-environment*)) name) nil)
(defmethod local-bound? ((env alist-environment) name)
  (find name (environment-bindings env) :key #'car))
(defmethod local-bound? ((env hash-environment) name)
  (nth-value 1 (gethash name (environment-bindings env))))

(defmethod environment-parents ((env (eql *empty-environment*)))
  nil)

(defmethod dump-environment ((env alist-environment))
  (format t "~{~a: ~a~%~}" (environment-bindings env))
  (values))
(defmethod dump-environment ((env hash-environment))
  (maphash (lambda (k v) (format t "~a: ~a~%" k v))
	   (environment-bindings env))
  (values))
(defmethod dump-environment ((env (eql *empty-environment*)))
  (values))

;;; Ground environment and definitions

(defparameter *ground-environment*
  (make-instance 'hash-environment
		 :mutate? t
		 :add? t
		 :parents nil
		 :binds (make-hash-table)))

(defun set-ground (name value)
  (setf (gethash name (environment-bindings *ground-environment*))
	value))

(defun fsubr (fun name)
  (make-instance 'fsubr :fun fun :name name))
(defun wrap (op)
  ;; note this is called by burke's wrap, not just us.
  ;; hypothetically this name business could go in a
  ;;  shared-initialize :after method for applicatives, etc.
  (if (named? op)
      (make-instance 'applicative :underlying op :name (name op))
      (make-instance 'applicative :underlying op)))

(defmacro define-fsubr (name params eparam wrap &body body)
  (let* ((params (if (symbolp params)
		     ;; i'm not totally sure this is legal
		     ;; (that (&rest x) binding 4 is legal, e.g.)
		     ;; It does work on SBCL and CCL atm.
		     `(&rest ,params)
		     ;; dotted lists work in d-b, so
		     params))
	 (fname (alexandria:symbolicate 'fsubr- name))
	 (combinand (gensym "COMBINAND"))
	 (eparam (or eparam (gensym "ENV")))
	 (value (if wrap
		    `(wrap (fsubr #',fname ',name))
		    `(fsubr #',fname ',name))))
    `(progn
       (defun ,fname (,combinand ,eparam)
	 (declare (ignorable ,eparam))
	 (destructuring-bind ,params ,combinand
	   ,@body))
       (set-ground ',name ,value))))

;;; REPL

;; TODO: put this somewhere nicer, or define in burke, or whatever
(defun burke-load (filename env)
  (with-open-file (f filename)
    (let ((eof (gensym "EOF")))
      (loop (let ((form (burke-read f nil eof)))
	      (if (eq form eof)
		  (return)
		  (eval form env))))))
  (values))

(defparameter *repl-environment*
  (make-instance 'hash-environment
		 :add? t :mutate? t
		 :binds (make-hash-table)
		 :parents (list *ground-environment*)))

(defun repl ()
  (with-simple-restart (abort "Quit Burke.")
    (loop (with-simple-restart (abort "Return to Burke REPL.")
	    (fresh-line)
	    (write-string "> ")
	    (let ((*package* (find-package "BURKE")))
	      ;; (to avoid package prefixes when printing)
	      (prin1
	       (eval (burke-read) *repl-environment*)))))))

(defun clear-to-ground ()
  (setf *repl-environment*
	(make-instance 'hash-environment
		       :add? t :mutate? t
		       :binds (make-hash-table)
		       :parents (list *ground-environment*))))

;;; Some operators

(define-fsubr if (condition consequent alternative) env nil
  (let ((c (eval condition env)))
    (cond ((eql c *true*) (eval consequent env))
	  ((eql c *false*) (eval alternative env))
	  (t (error "if on a non-boolean: ~a [form = ~a]"
		    c condition)))))

(defun validate-parameters (params)
  (flet ((param? (param)
	   (or (eql param *ignore*)
	       (and param (symbolp param)))))
    (if (listp params)
	(loop for (p . rest) on params
	   do (assert (param? p))
	   when (param? rest) do (return t)
	   else unless (listp rest) do (error "dotted params"))
	(assert (param? params)))))

(define-fsubr fexpr (params eparam close form) env nil
  (let ((close (eval close env)))
    (validate-parameters params)
    (unless (and eparam
		 (or (symbolp eparam) (eql eparam *ignore*)))
      (error "fexpr eparam ~a is invalid" eparam))
    (make-instance 'subr :params params :eparam eparam
		   :env close :body form)))

(define-fsubr set-in (name value in) env nil
  (let ((value (eval value env))
	(in (eval in env)))
    ;; set name of object for debugging
    (maybe-set-name name value)
    ;; actually set binding
    (if (nth-value 1 (setf (lookup in name) value))
	*inert*
	(error "unable to set binding for ~a" name))))

(define-fsubr seq forms env nil
  (assert (listp forms))
  (cond ((null forms) *inert*)
	((null (rest forms)) (eval (first forms) env))
	(t (eval (first forms) env)
	   (fsubr-seq (rest forms) env))))

;; this is what it takes to do it correctly, i think.
;; i have no particular reason to do so, but why the fuck not.
;; The existence of all this does suggest yet more generalization
;;  for environments. TODO.
(defclass tagbody-env (basic-environment)
  ((catch-tag :initarg :catch :accessor tagbody-env-catch)
   (labels :initarg :labels :accessor tagbody-env-labels)))
(defmethod local-lookup ((env tagbody-env) name) (values nil nil))
(defmethod local-bound? ((env tagbody-env) name) nil)

(defun parse-tagbody (forms-and-labels)
  ;; doesn't allow numeric labels like CL does
  (let ((blocks (list (list nil))))
    (dolist (thing forms-and-labels
	     (let ((nr (reverse blocks)))
	       (values (nreverse (cdr (first nr)))
		       (mapcar (lambda (b)
				 (cons (car b) (nreverse (cdr b))))
			       (rest nr)))))
      (cond ((symbolp thing)
	     (push `(go ,thing) (cdr (first blocks)))
	     (push (list thing) blocks))
	    (t (push thing (cdr (first blocks))))))))
(define-fsubr tagbody combinand env nil
  (multiple-value-bind (start blocks) (parse-tagbody combinand)
    (let* ((tag (gensym "TAGBODY"))
	   (env (make-instance 'tagbody-env
			       :add? nil :mutate? nil
			       :labels (mapcar #'first blocks)
			       :catch tag :parents (list env)))
	   (evaluating start))
      (loop
	 (let* ((label (catch tag
			 (return (fsubr-seq evaluating env))))
		(block (assoc label blocks)))
	   (if block
	       (setf evaluating (cdr block))
	       (error "BURKE BUG: GO to nonexistent label ~a"
		      label))))
      *inert*)))

(defun find-tagbody-tag (env label)
  (cond ((eql env *empty-environment*) nil)
	((and (typep env 'tagbody-env)
	      (find label (tagbody-env-labels env)))
	 (tagbody-env-catch env))
	(t (some (lambda (e) (find-tagbody-tag e label))
		 (environment-parents env)))))

(define-fsubr go (label) env nil
  (throw (or (find-tagbody-tag env label)
	     (error "GO to nonexistent label ~a" label))
    label))

(define-fsubr quote (thing) nil nil thing)

;;; Some applicatives

(defun boolean-cl->burke (bool)
  (if bool *true* *false*))

(define-fsubr eql? (e1 e2) nil t (boolean-cl->burke (eql e1 e2)))

(defmacro define-type?-fsubr (name cl-type)
  `(define-fsubr ,name (object) nil t
     (boolean-cl->burke (typep object ',cl-type))))

(define-type?-fsubr cons? cons)
(define-type?-fsubr null? null)
(define-fsubr cons (car cdr) nil t (cons car cdr))
(define-fsubr car (cons) nil t
  (assert (consp cons)) ; nil
  (car cons))
(define-fsubr cdr (cons) nil t
  (assert (consp cons))
  (cdr cons))

(define-fsubr inert? (object) nil t
  (boolean-cl->burke (eql object *inert*)))
(define-fsubr ignore? (object) nil t
  (boolean-cl->burke (eql object *ignore*)))

(define-type?-fsubr symbol? (and symbol (not null)))
(define-fsubr gensym () nil t (gensym))

(define-fsubr =? (n1 n2) nil t (boolean-cl->burke (= n1 n2)))
(define-fsubr 1+ (n) nil t (1+ n))
(define-fsubr 1- (n) nil t (1- n))

(define-fsubr wrap (combiner) nil t (wrap combiner))
(define-fsubr unwrap (app) nil t (applicative-underlying app))

(define-type?-fsubr vector? vector)
(define-fsubr vref (vector index) nil t (aref vector index))
(define-fsubr vlength (vector) nil t (length vector))

(define-fsubr eval (form env) nil t (eval form env))
(define-fsubr combine (combiner combinand env) nil t
  (combine combiner combinand env))

(define-type?-fsubr applicative? applicative)
(define-type?-fsubr operator? (or fsubr subr))
(define-type?-fsubr macro? macro)

;;(define-fsubr lookup (env symbol) nil t (lookup env symbol))
(define-fsubr bound? (env symbol) nil t
  (boolean-cl->burke (bound? env symbol)))

(define-fsubr error (string) nil t (error string))

(define-fsubr make-environment (parents add? mutate? . binds)
    nil t
  (make-instance 'hash-environment ; parameterize?
		 :add? add? :mutate? mutate?
		 :binds (alexandria:alist-hash-table binds)
		 :parents (copy-list parents)))

(define-fsubr make-macro (combiner) nil t
  (make-instance 'macro :combiner combiner))
(define-fsubr macro-combiner (macro) nil t (macro-combiner macro))

(define-fsubr load (filename env) nil t
  (burke-load filename env)
  *inert*)

;; further definitions in burke
(burke-load *standard-library* *ground-environment*)
