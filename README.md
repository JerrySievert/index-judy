# index-judy

An experimental building block of a judy array index, accessible as an embedded
C library.

Currently, it builds, is useful to link to, but no guarantees are made about
anything.

## Building

Building relies on you local build environment.  It should build under most
Unix variants, including Linux, and OS X.

```
$ gcc -c src/index.c src/judy64nb.c
```

### Testing

There is a test program available, `test.c`.

```
$ gcc test.c index.o judy64nb.o -o test
$ ./test
```

This tests functionality of the index, and can serve as a terse reference to
the library itself.

## Interface

Uses of the library require that `index.h` be included:

```
#include "src/index.h"
```
