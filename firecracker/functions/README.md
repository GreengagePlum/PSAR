# Functions

These are the user defined "functions" to be executed by the microVMs upon request. Any executable file here
(recursively) is
considered a standalone function and is copied inside the microVM rootfs to be available for execution by the Flask web
server inside (see [management programs](../programs/README.md)).

The function executable is expected to write its result on `stdout` for it to be captured and sent back to the user. For
now only character output (and not binary) is supported for the result captured from the `stdout` of user functions.

For now only functions written in POSIX shell, C or Python are supported for execution inside the microVMs.

Couple notes:

* Make sure your compiled languages like C are compiled statically!
* Make sure that your function is not super long running and is stateless.
* `/tmp` directory is where you can write temporary files stored in RAM (assuming they are not huge)
* You have very little resources (down as low as 1 vcore and 128Mb or RAM), so be wise
* Internet normally is accessible
* There is a very simple `Makefile` to compile your basic C programs
