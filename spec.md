Introduction
============

As it says in the README, Kernel plus Maru. In more detail, some design goals:

+ homoiconicity
+ compilers and other code processors should not be black boxes: almost anything an implementation does should be exportable as part of a standard module.
+ things should be really extremely customizable: things like lisp-1/N evaluation or calling conventions are modifiable.
+ targets conventional machines, is retargetable
+ can use (be compiled to and link) normal code formats, e.g. ELF
+ modular: modules can be loaded, unloaded, have dependencies, etc. the standard library consists of multiple modules
+ talk is cheap

Check the example code in any of the .l files. standard.l has things I think are standardsworthy in addition to what is described here.

Evaluation
==========

The complex guts of evaluation are mostly described in "Combination" and "Environments". Eval passes conses to the former and symbols to the latter, and everything else is (without customization) self-evaluating.

_Generic function_ **eval**

**eval** form env

+ form -- a form (T)
+ env -- an environment

Evaluates _form_ in _env_ and returns the result.

Methods are on _form_. Three methods are standard:

+ eval symbol env = lookup env symbol
+ eval cons env = combine (car cons) (cdr cons) env
+ eval default env = default

Combination
===========

"combination" is how Burke evaluates conses: a thing (the "combiner") does something with respect to another thing (the "combinand"). It's purposefully very broad. Combination subsumes special forms, function calls, and macros.

Terminology for combiners first: There are "operators" and "applicatives", explained below. A "macro" is a kind of operator with its own module. A "function" is an applicative with an underlying combiner that is an operator that ignores its environment argument (complex, huh), which basically corresponds to an intuitive "function". A "generic function" is a function that dispatches based on the types of its arguments, which is something I haven't decided on the mechanics of yet.

_Generic function_ **combine**

**combine** combiner combinand env

+ combiner -- a combiner
+ combinand -- an object (T)
+ env -- an environment

Methods are on _combiner_.

There are two kinds of essential combiner: _operators_ and _applicatives_.

_Operator_ **fexpr**

**fexpr** params eparam env form

_env_ is evaluated, the others are not.

Creates a basic operator.

The standard method on **combine** with respect to an operator does, essentially, `(eval form new-env)`, where _new-env_ is a child of the original _env_ passed to this operator, augmented with the binding of _params_ to the combinand, and _eparam_ to the environment of the combination. E.g.,

    (combine (fexpr x #ignore (make-environment) x) y e) => y, e is ignored
    (combine (fexpr #ignore e (make-enviroment) e) x f) => f, x is ignored

Basic lambda calculus, with two hitches: environments and evaluation.

An operator receives the environment it was called in as a special parameter. It may alter or use this environment in quite a lot of ways. This is useful because it allows an operator to explicitly do whatever evaluation it likes on its unevaluated arguments, but makes ahead-of-time compilation much more difficult.

To mitigate this, I imposed the condition that the child environment made during fexpr combination does not allow the creation of new bindings, to prevent shadowing. It simplifies analysis very much and defining local variables like that is silly anyway. See also the environments module.

An operator does not perform any other evaluation, so that e.g.

    ((fexpr x #ignore (make-environment) x) a b c) => (A B C)

\#ignore is a special object meaning that that thing is ignored.

I haven't decided on how params is bound. Kernel allows arbitrary destructuring trees. Currently I use Scheme-style arguments, i.e. a param is either a symbol/ignore or a possibly dotted list of symbol/ignores.

_Operator_ **vau**

**vau** params eparam form

(vau params eparam form) is equivalent to (fexpr params eparam (get-current-environment) form), and so makes pretty normal closures.

I define _vau_ as a macro in terms of _fexpr_. It would be possible to do it the opposite way, or a different way entirely, but I'd like to at least keep the basic idea of having both a "normal" closure-y lambda, and a more explicit version. I think this will be helpful for compilation.

Now, for applicatives:

_Function_ **wrap**

**wrap** combiner

Makes an applicative out of a combiner.

_Function_ **unwrap**

**unwrap** applicative

Returns the underlying combiner of an applicative.

The important thing about applicatives is the method on combine. When an applicative is combined with an applicative, the unwrapped applicative is combined with the combiner, evaluated as a list of arguments. That is

    (combine app arg e) = (combine (unwrap app) (map (lambda (f) (eval f e)) arg) e)

and so applicatives are essential for basic evaluation.

_Operator_ **lambda**

**lambda** params form

Makes a function. Can be implemented as a macro expanding to (wrap (vau params #ignore form)).

Macros
======

A restricted form of operator, for which `(op ...)` is equivalent to some other form. For example, `(let binds form)` is equivalent to `((lambda bind-names form) bind-values)`.

_Function_ **make-macro**

**make-macro** combiner

Makes a macro.

_Function_ **macro-combiner**

Returns the combiner a macro was made with.

The method on **combine** for macros has this equivalence:

    (combine macro c e) = (eval (combine (macro-combiner macro) c e) e)

That is, the combiner receives the combinand and the environment, and returns a form to be evaluated.

macroexpand, etc etc

Types (not yet implemented)
=====

The type system is, like CL's, a kind of naïve set theory which is in general uncomputable. Unlike CL's, it is very customizable, and types are themselves first-class objects.

A type is something with a method on one operator:

_Generic function_ **of-type?**

**of-type?** object type

Methods are on _type_. Returns #t if object is of type _type_, or else #f. Set theory wise, this is set membership.

To be useful, there will probably also need to be methods on:

_Complex generic function_ **subtype?**

**subtype?** type1 type2

Determines whether type1 is a subtype (set theory wise, subset) of type2. In general this is not computable, so this operator's return value either indicates "certainly is", "certainly is not", or "unknown". I haven't decided on the actual values for that yet.

Methods are on both arguments. Furthermore, some kind of method combination mechanism might be helpful to combine results, so that with subclassed type-objects all methods are called, and only one has to return a certain result.

Bla bla I've written about this before and it's not implemented anyway so I'll go into detail once I have something working.

fexpr arg\* (earg|#ignore) env form
if condition consequent alternative
bind name value env ; ADDS, does not SET, a binding
defstruct name slot\*

Structs
=======

Structs are product types. No inheritance or nothing.

Haven't decided how to access.

_Special form_ **struct**

**struct** slot\*

slot -- symbol, unevaluated

Each slot is just a name (for now).

Returns an object, the struct. It's a type, and works with of-type?

_Function_ **make-struct**

make-struct struct-type slot-value*

+ struct-type -- a result from **struct**
+ slot-value -- a value (T)

Returns a new instance of a struct.

Macros
======

Take parameters and an environment. Work like macros.
The environment passed in is possibly compile time, so certain operations are undefined (getting values, e.g.)

macros should maybe be done internally, rather than exposing c-c.

    (defstruct macro macro-function)

    (defmethod combine macro (combiner combinand env)
      (eval (call (macro-function combiner) combinand env) env))

    (defmethod compile-combination macro (combiner combinand env)
      (compile (call (macro-function combiner) combinand env) env))

Environments
==========

_Class_ **environment**

Abstractly, it's something that works when eval is called with it. Less abstractly, it's a map from symbols to values, arranged in a tree structure (the "parents"). Lookup is depth-first.

It should have methods defined for **locally-bound?**, **local-lookup**, **environment-parents** at least. You may also override **lookup** and **bound?** if you don't like their behavior.

_Generic function_ **lookup**

**lookup** env symbol

Returns an option of the value bound to SYMBOL in ENV, or nothing if it's not bound.

Methods are on ENV. One method is standard:

    (defmethod lookup t (environment symbol)
      (cond ((locally-bound? environment symbol)
             (just (local-lookup environment symbol)))
      	    (t (map (lambda (parent)
                      (let ((value (lookup parent symbol)))
	       	        (if (just? value)
	       	    	    (return-from lookup value)
		   	    #inert)))
	            (environment-parents environment))
      	       (nothing))))
