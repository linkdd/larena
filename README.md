# Little Arena

![tests](https://img.shields.io/github/actions/workflow/status/linkdd/larena/tests.yml?style=flat-square&logo=github&label=tests)
![license](https://img.shields.io/github/license/linkdd/larena?style=flat-square&color=blue)
![version](https://img.shields.io/github/v/release/linkdd/larena?style=flat-square&color=red)

This project is a C/C++ header only library providing an Arena implementation.

## Installation

Copy the `larena.h` file in your source tree, then in a single `.c` file:

```c
#define LARENA_IMPLEMENTATION
#include "larena.h"
```

## Usage

To create an arena, you first need an allocator:

```c
lallocator myallocator = {0};
lallocator_init_stdlib(&myallocator);
```

This will initialize (on the stack) an allocator using `stdlib.h`'s `malloc`,
`realloc` and `free` functions.

To create a custom allocator:

```c
void *myallocator_alloc(size_t sz, void *ctx) {
  // ...
}

void *myallocator_realloc(void *ptr, size_t oldsz, size_t newsz, void *ctx) {
  // ...
}

void myallocator_dealloc(void *ptr, size_t sz, void *ctx) {
  // ...
}
```

```c
lallocator myallocator = {
  .alloc   = myallocator_alloc,
  .realloc = myallocator_realloc,
  .dealloc = myallocator_dealloc,
  .ctx     = NULL
};
```

Once your allocator is set-up, you will be able to create an arena:

```c
larena myarena = {0};
larena_init(&myarena, &myallocator);
```

You can then allocate objects with your arena:

```c
lobject myobj = {0};

if (larena_alloc(&myarena, sizeof(int), &myobj) != 0) {
  // allocation failed
}
```

The possible error codes are:

| Code | Description |
| --- | --- |
| `0` | The allocation was successful |
| `EOVERFLOW` | The size of the allocation would produce an integer overflow |
| `ENOMEM` | The heap allocation, to extend the arena, failed |

> **NB:** Error codes are found in the header `errno.h`

An object is a simple structure containing a reference to the arena used to
allocate said object, and a pointer relative to the base of the arena.

If the arena does not have enough capacity, the allocator's `realloc` method
will be called to increase the available memory. Because `realloc` can move the
memory, old pointers would be invalidated. This is why we store in the object
a **relative** pointer.

> **NB:** The returned memory has been zero'd.

To access the allocated memory, we need to dereference the object:

```c
int *mymem = lobject_deref(&myobj);
```

The **absolute** pointer will be resolved by summing the arena's base with the
**relative** pointer contained in the object. This way, there is no pointer
invalidation when `realloc` moves the memory.

Once you're done with your arena, you can free all the memory at once:

```c
larena_free(&myarena);
// all lobjects are now invalid
```

But if you wish to reuse the arena, for example each frame of a game loop, you
can simply clear it instead:

```c
larena_clear(&myarena);
```

The arena's offset is reset to 0, but the allocated memory has not been free'd.
This is especially useful for loops so that the arena grows to its maximum
capacity only once:

```c
larena myarena = {0};
larena_init(&myarena);

while (true) {
  // do work, and some allocations

  larena_clear(&myarena);
}

larena_free(&myarena);
```

## License

This library is distributed under the terms of the [MIT License](./LICENSE.txt).
