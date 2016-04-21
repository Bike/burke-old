;;; a simple partial evaluator that does no optimization

(defun compile (form env)
  (cond ((symbol? form)
	 (if (bound? env form)
	     (eval form env)
	     form))
	((cons? form)
	 (compile-combination (compile (car form) env)
			      (cdr form) env))
	(#t form)))

(define *combiner-compilers* ())

(defun compile-combination (combiner combinand env)
  (let ((compiler (assoc combiner *combiner-compilers*)))
    (if (null? compiler)
	(cond ((applicative? combiner)
	       (let ((args (map-combinand (rpartial compile env)
				combinand)))
		 (cons combiner args)))
	      ((macro? combiner)
	       (compile (combine (macro-combiner combiner)
				 combinand env)
			env))
	      ;; can't do anything for an operator
	      (#t (cons combiner combinand)))
	(call compiler combiner combinand env))))

(defun map-combinand (function combinand)
  ;; since forms like (list . a) are ok, we can't just use map.
  (cond ((null? combinand) ())
	((cons? combinand) (cons (call function (car combinand))
				 (map-combinand function
						(cdr combinand))))
	(#t (call function combinand))))

;; unused
(defun constant? (form)
  (not (or? (symbol? form) (cons? form))))

(defun compile-if (combiner combinand env)
  (let ((condition (compile (car combinand) env))
	(consequent (compile (car (cdr combinand)) env))
	(alternative (compile (car (cdr (cdr combinand))) env)))
    (list combiner condition consequent alternative)))
(set *combiner-compilers*
     (cons (cons if compile-if) *combiner-compilers*))

(defun compile-fexpr (combiner combinand env)
  (let ((params (car combinand))
	(eparam (car (cdr combinand)))
	(close (car (cdr (cdr combinand))))
	(form (car (cdr (cdr (cdr combinand))))))
    (let ((close (compile close env)))
      ;; KLUDGE
      (if (or (symbol? close) (cons? close))
	  (list combiner params eparam close form)
	  (list combiner params eparam
		close (compile form close))))))
(set *combiner-compilers*
     (cons (cons fexpr compile-fexpr) *combiner-compilers*))

(defun compile-set-in (combiner combinand env)
  (let ((name (car combinand))
	(value (car (cdr combinand)))
	(in (car (cdr (cdr combinand)))))
    (list combiner name (compile value env) (compile in env))))
(set *combiner-compilers*
     (cons (cons set-in compile-set-in) *combiner-compilers*))

(defun compile-seq (combiner combinand env)
  (cons combiner
	;; (seq . 4) is illegal
	(map (rpartial compile env) combinand)))
(set *combiner-compilers*
     (cons (cons seq compile-seq) *combiner-compilers*))

(defun compile-tagbody (combiner combinand env)
  (cons combiner
	;; (tagbody . 4) is illegal
	(map (lambda (form)
	       (if (symbol? form) ; tag
		   form
		   (compile form env)))
	     combinand)))
(set *combiner-compilers*
     (cons (cons tagbody compile-tagbody) *combiner-compilers*))
