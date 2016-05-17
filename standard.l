;;;; Burke standard library definitions
;;;; Everything here should be in the ground environment.

;;; Stuff we need to make defining the rest of the
;;;  standard library tolerable

(set-in empty-environment (make-environment () #f #f)
	((fexpr () e (make-environment () #f #f) e)))

(set-in get-current-environment (fexpr () e empty-environment e)
	((fexpr () e empty-environment e)))

(set-in list (wrap (fexpr x #ignore empty-environment x))
	(get-current-environment))

(set-in macro
	(make-macro
	 (fexpr (params eparam . body) #ignore
		(get-current-environment)
	   (list make-macro
		 (list fexpr params eparam
		       (list get-current-environment)
		       (cons seq body)))))
	(get-current-environment))

(set-in define (macro (name value) #ignore
		 (list set-in name value
		       (list get-current-environment)))
	(get-current-environment))

(define set define) ; TODO: setf-like mechanism

(define vau
  (macro (params eparam . body) #ignore
    (list fexpr params eparam (list get-current-environment)
	  (cons seq body))))

(define list*
  (wrap (vau (arg . more-args) #ignore
	  (if (null? more-args)
	      arg
	      ;; apply
	      (cons arg
		    (combine (unwrap list*)
			     more-args empty-environment))))))

(define defmacro
  (macro (name params eparam . body) #ignore
    (list define name (list* macro params eparam body))))

(defmacro lambda (params . body) #ignore
  (list wrap (list* vau params #ignore body)))

(defmacro defun (name params . body) #ignore
  (list define name (list* lambda params body)))

;; This seems pointless, but serves an important function:
;;  (call f ...) is an assertion that f is an applicative,
;;  and one whose underlying combiner doesn't need an environment.
;; It's a "normal function".
;; Good for optimization, and for avoiding one of the most quoted
;;  problems with fexprs.
(defun call (app . args)
  (combine (unwrap app) args empty-environment))

(defun apply (app args)
  (combine (unwrap app) args empty-environment))

(defun map (function list)
  (if (null? list)
      ()
      (cons (call function (car list))
	    (map function (cdr list)))))

;;; Control flow

(defmacro let (bindings . body) #ignore
  (cons (list* lambda (map car bindings) body)
	(map (lambda (cons) (car (cdr cons))) bindings)))

(defmacro let* (bindings . body) #ignore
  (if (null? bindings)
      (cons seq body)
      (list let (list (car bindings))
	    (list* let* (cdr bindings) body))))

(defun not (boolean) (if boolean #f #t))

(defmacro and? forms #ignore
  (if (null? forms)
      #t
      (list if (car forms)
	    (cons and? (cdr forms))
	    #f)))

(defmacro or? forms #ignore
  (if (null? forms)
      #f
      (list if (car forms)
	    #t
	    (cons or? (cdr forms)))))

(defun boolean? (object) (or? (eql? object #t) (eql? object #f)))

(defmacro when (condition . forms) #ignore
  (list if condition (cons seq forms) #inert))

(defmacro cond clauses #ignore
  (if (null? clauses)
      #inert
      (list if
	    (car (car clauses))
	    (cons seq (cdr (car clauses)))
	    (cons cond (cdr clauses)))))

(defmacro case (keyform . cases) #ignore
  (let* ((key (gensym))
	 (eqlfy (lambda (thing)
		  (list eql? key (list quote thing))))
	 (conditionify (lambda (case)
			 (cons
			  (if (eql? (car case) #t)
			      #t
			      (cons or? (map eqlfy (car case))))
			  (cdr case)))))
    (list let (list (list key keyform))
	  (cons cond (map conditionify cases)))))

;;; Functions

(defun partial (function . partial-args)
  ;; sometimes called "currying"
  (lambda args
    (apply function (append partial-args args))))

(defun rpartial (function . partial-args)
  (lambda args
    (apply function (append args partial-args))))

(defun identity (x) x)

(defun compose functions
  (cond ((null? functions) identity)
	((null? (cdr functions)) (car functions))
	(#t (let ((f (car functions))
		  (rest (apply compose (cdr functions))))
	      (lambda (object)
		(call f (call rest object)))))))

;;; Lists

(defun assoc (key alist)
  (cond ((null? alist) ())
	((eql? (car (car alist)) key) (car alist))
	(#t (assoc key (cdr alist)))))

(defun acons (key value alist)
  (cons (cons key value) alist))

;; FIXME: should not be in this environment
(defun reverse-aux (list result)
  (if (null? list)
      result
      (reverse-aux (cdr list) (cons (car list) result))))

(defun reverse (list)
  (reverse-aux list ()))

(defun append lists
  ;; hyper inefficient
  (cond ((null? lists) ())
	((null? (cdr lists)) (car lists))
	((null? (car lists)) (apply append (cdr lists)))
	(#t (cons (car (car lists))
		  (apply append
			 (cons (cdr (car lists)) (cdr lists)))))))

;;; Modules

(defun make-standard-environment ()
  ;; Definitions in this file are in the ground environment.
  ;; So, lexically, the environment in this function
  ;;  is (a child of) ground. With no other definitions, it's
  ;;  then standard.
  ;; make-environment is to insert something to eat shadowings,
  ;;  since lambda environments don't allow shadowing bla bla bla.
  (make-environment (list (get-current-environment)) #t #t))

(defun load-module (filename . depends)
  (let ((module (make-environment
		 (cons (make-standard-environment)
		       depends) #t #t)))
    (load filename module)
    module))
