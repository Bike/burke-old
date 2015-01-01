Burke was a murderer who with his accomplice Hare killed several people by suffocation. The pair then sold the bodies to Dr. Robert Knox, who used them for medical demonstrations.

Burke is a computer programming language based on [Kernel](http://web.cs.wpi.edu/~jshutt/kernel.html) and [Maru](http://piumarta.com/software/maru/). It is an amateur project.

Build
-----

The runtime is called liblisp.so. If you have the Boehm garbage collector ("libgc"), you can use `make boehm' to produce it. If you don't, `make malloc' will produce something using libc's malloc and no freeing, so it'll burn through memory quick.

`make repl' will build an example program, a read-eval-print-loop, linked against a liblisp.so in the same directory.

The default make target 'all' is just boehm + repl.

Run
---

The example repl needs to know where liblisp is. You can do this with an invocation like `LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH ./repl', assuming liblisp.so and repl are in the working directory. Of course, you could also install liblisp in /usr/local/lib or something.

Goals
-----

* Produce ELFs linkable from other environments
* Allow distribution of compile-time customizations with libraries (e.g. optimization routines specific to library functions)
* First class functions (closures) in a systems programming language
* Runtime manipulation of compiled and uncompiled code responsibly
* Good compile-time manipulation of code (e.g. macros, custom optimizations)
