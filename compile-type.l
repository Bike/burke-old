;;; the most obvious issue with that is that if the combiner of
;;;  a combination is non-constant, it gives up.
;;; Especially: (let (...) forms) => ((lambda (...) ...) forms)
;;; The compiler doesn't even look at the forms, since it doesn't
;;;  realize that (wrap (fexpr ...)) is an applicative.

;;; I think the best solution to this is a type system.
;;; This is a short demo using too-simple types,
;;;  represented varyingly

(define *standard-compile-env* (make-standard-environment))
(define *combiner-compilers* ())

(defun map-combinand (function combinand)
  ;; since forms like (list . a) are ok, we can't just use map.
  (cond ((null? combinand) ())
	((cons? combinand) (cons (call function (car combinand))
				 (map-combinand function
						(cdr combinand))))
	(#t (call function combinand))))

;;; valid types are (eql object), #t, #f, (wrap type), (op type)

(defun eql-type? (object)
  (and? (cons? object) (eql? (car object) (quote eql))
	(cons? (cdr object)) (null? (cdr (cdr object)))))
(defun eql-type-object (eql-type)
  (car (cdr eql-type)))
(defun make-eql-type (object)
  (list (quote eql) object))

(defun custom-compiler-type? (object)
  (applicative? object))

(defun op-type? (object)
  (and? (cons? object) (eql? (car object) (quote op))
	(cons? (cdr object)) (null? (cdr (cdr object)))))
(defun op-type-returns (op-type)
  (car (cdr op-type)))
(defun make-op-type (type)
  (list (quote op) type))

(defun wrap-type? (object)
  (and? (cons? object) (eql? (car object) (quote wrap))
	(cons? (cdr object)) (null? (cdr (cdr object)))))
(defun wrap-type-unwrap (wrap-type)
  (car (cdr wrap-type)))
(defun make-wrap-type (type)
  (list (quote wrap) type))

;; returns (cons result inferred-type)
(defun compile (form env)
  (cond ((symbol? form)
	 ;; could allow for unknown variables by checking bound?
	 (let ((e (eval form env)))
	   (cons (if (eql-type? e) (eql-type-object e) form) e)))
	((cons? form)
	 (compile-combination (compile (car form) env) (cdr form)
			      env))
	(#t (compile-constant form))))

(defun compile-constant (object)
  (cons object (make-eql-type object)))

(defun compile-combination (combiner combinand env)
  (let ((comb-form (car combiner))
	(comb-type (cdr combiner)))
    (cond ((eql-type? comb-type)
	   (let* ((obj (eql-type-object comb-type))
		  (c (assoc obj *combiner-compilers*)))
	     (if (null? c)
		 (cond ((macro? obj)
			(compile
			 (combine
			  (macro-combiner obj) combinand env)
			 env))
		       ((applicative? obj)
			(compile-application
			 obj combinand env
			 (find-return-type (unwrap obj))))
		       (#t (cons obj combinand env)))
		 (call (cdr c) comb-form combinand env))))
	  ((wrap-type? comb-type)
	   (compile-application comb-form combinand env
				(wrap-type-unwrap comb-type)))
	  ((op-type? comb-type)
	   (cons (cons comb-form combinand)
		 (op-type-returns comb-type)))
	  (#t (cons (cons comb-form combinand) #t)))))

(defun find-return-type (combiner)
  ;; i don't care.
  (if (eql? combiner (unwrap wrap))
      (make-op-type (make-wrap-type #t))
      #t))

(defun compile-application (app combinand env type)
  (let ((combinand
	 (map-combinand (compose car (rpartial compile env))
			combinand)))
    (cons (cons app combinand)
	  (if (op-type? type)
	      (op-type-returns type)
	      ;; this means that literal apps have worse inference
	      ;;  then variable ones, right now. ha ha.
	      #t))))

(defun compile-if (combiner combinand env)
  (let ((condition (compile (car combinand) env))
	(consequent (compile (car (cdr combinand)) env))
	(alternative (compile (car (cdr (cdr combinand))) env)))
    (cons (list combiner (car condition)
		(car consequent) (car alternative))
	  ;; disjunction is hard.
	  #t)))
(set *combiner-compilers*
     (acons if compile-if *combiner-compilers*))

;; necessary due to how i made fexpr work... oops
;;  could just put a compiler on vau of course
(defun compile-get-current-environment (combiner combinand env)
  (cons (list combiner) (make-eql-type env)))
(set *combiner-compilers*
     (acons get-current-environment
	    compile-get-current-environment
	    *combiner-compilers*))
(defun compile-make-standard-environment (combiner combinand env)
  ;; sort of isn't actually eql since it's fresh,
  ;;  but types don't need that distinction
  (cons (list combiner) (make-eql-type *standard-compile-env*)))
(set *combiner-compilers*
     (acons make-standard-environment
	    compile-make-standard-environment
	    *combiner-compilers*))

(defun compile-fexpr (combiner combinand env)
  (let ((params (car combinand))
	(eparam (car (cdr combinand)))
	(close (car (cdr (cdr combinand))))
	(form (car (cdr (cdr (cdr combinand))))))
    (let ((close (compile close env)))
      ;; if the environment isn't more or less constant we can't
      ;;  compile the form. hence the above compile--environments.
      (if (eql-type? (cdr close))
	  (let ((new (make-environment
		      (eql-type-object (cdr close)))))
	    (map-combinand ; bit of an abuse
	     (lambda (param)
	       (cond ((ignore? param) #inert)
		     ((symbol? param)
		      ;; establish a binding (to top type)
		      ((wrap set-in) param #t new))
		     (#t (error "bad parameter"))))
	     params)
	    ((wrap set-in) eparam #t new) ; could be env type
	    (let ((form (compile form new)))
	      (cons (list combiner params eparam
			  (car close) (car form))
		    (make-op-type (cdr form)))))
	  (cons (list combiner params eparam (car close) form)
		(make-op-type #t))))))
(set *combiner-compilers*
     (acons fexpr compile-fexpr *combiner-compilers*))

(defun compile-set-in (combiner combinand env)
  (let ((name (car combinand))
	(value (compile (car (cdr combinand)) env))
	(in (compile (car (cdr (cdr combinand))) env)))
    (cons (list combiner name (car value) (car in))
	  (make-eql-type #inert))))
(set *combiner-compilers*
     (acons set-in compile-set-in *combiner-compilers*))

(defun compile-seq (combiner combinand env)
  (if (null? combinand)
      (compile-constant #inert)
      (let ((forms (map (rpartial compile env) combinand)))
	(cons (cons combiner (map car forms))
	      ;; the inefficiency..........
	      (car (reverse (map cdr forms)))))))
(set *combiner-compilers*
     (acons seq compile-seq *combiner-compilers*))

(defun compile-tagbody (combiner combinand env)
  (cons (cons combiner
	      (map (lambda (thing)
		     (if (symbol? thing) ; tag
			 thing
			 (car (compile thing env))))
		   combinand))
	(make-eql-type #inert)))
(set *combiner-compilers*
     (acons tagbody compile-tagbody *combiner-compilers*))

(defun compile-go (combiner combinand env)
  (cons (cons combiner combinand) #f))
(set *combiner-compilers*
     (acons go compile-go *combiner-compilers*))

(defun compile-quote (combiner combinand env)
  (cons (cons combiner combinand) (make-eql-type (car combinand))))
(set *combiner-compilers*
     (acons quote compile-quote *combiner-compilers*))

(defmacro define-as-eql (name) env
  (list set-in name (list quote (make-eql-type (eval name env)))
	(quote *standard-compile-env*)))

(defmacro defines-eql names #ignore
  (cons seq (map (lambda (name) (list define-as-eql name)) names)))

(defines-eql if fexpr set-in seq tagbody go quote eql? cons? null? cons car cdr inert? ignore? symbol? gensym =? 1+ 1- wrap unwrap vector? vref vlength eval combine applicative? operator? macro? bound? error make-environment make-macro macro-combiner load)
(defines-eql get-current-environment list macro define set vau list* defmacro lambda defun call apply map let let* not and? or? boolean? when cond case partial rpartial identity compose assoc acons reverse append make-standard-environment load-module)
