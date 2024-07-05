/*
 * larena.h - A simple arena allocator.
 * Version 0.5.0
 *
 * Copyright (c) 2024, David Delassus <david.jose.delassus@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __LARENA_H
#define __LARENA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define LARENA_ALIGNMENT 4096

typedef struct lallocator lallocator;
struct lallocator {
  void *(*alloc)(size_t size, void *ctx);
  void *(*realloc)(void *ptr, size_t old_size, size_t new_size, void *ctx);
  void (*dealloc)(void *ptr, size_t size, void *ctx);

  void *ctx;
};

typedef struct larena larena;
struct larena {
  lallocator *allocator;

  uintptr_t offset;
  size_t capacity;
  void *data;
};

typedef struct lobject lobject;
struct lobject {
  larena *allocator;
  uintptr_t ptr;
};

void lallocator_init_stdlib(lallocator *self);

void larena_init(larena *self, lallocator *allocator);
int larena_alloc(larena *self, size_t size, lobject *obj);
void larena_clear(larena *self);
void larena_free(larena *self);

void *lobject_deref(const lobject *self);

#ifdef __cplusplus
}
#endif

#ifdef LARENA_IMPLEMENTATION

#include <assert.h>
#include <errno.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

static inline size_t align_size_forward(size_t size, size_t alignment) {
  if (alignment == 0) return size;
  size_t remainder = size % alignment;
  if (remainder == 0) return size;
  if (size > SIZE_MAX - (alignment - remainder)) return 0;
  return size + alignment - remainder;
}

static void *stdlib_alloc(size_t size, void *ctx) {
  (void)ctx;
  return malloc(size);
}

static void *stdlib_realloc(void *ptr, size_t old_size, size_t new_size, void *ctx) {
  (void)old_size;
  (void)ctx;
  return realloc(ptr, new_size);
}

static void stdlib_dealloc(void *ptr, size_t size, void *ctx) {
  (void)size;
  (void)ctx;
  free(ptr);
}

void lallocator_init_stdlib(lallocator *self) {
  assert(self != NULL);

  self->alloc   = stdlib_alloc;
  self->realloc = stdlib_realloc;
  self->dealloc = stdlib_dealloc;
  self->ctx     = NULL;
}

void larena_init(larena *self, lallocator *allocator) {
  assert(self != NULL);
  assert(allocator != NULL);

  self->allocator = allocator;
  self->offset    = 0;
  self->capacity  = 0;
  self->data      = NULL;
}

int larena_alloc(larena *self, size_t size, lobject *obj) {
  assert(self != NULL);
  assert(self->allocator != NULL);
  assert(obj != NULL);

  if (size == 0) {
    obj->allocator = self;
    obj->ptr       = self->offset;
    return 0;
  }

  size = align_size_forward(size, alignof(max_align_t));
  if ((size == 0) || (self->offset > SIZE_MAX - size)) return EOVERFLOW;

  if (self->offset + size > self->capacity) {
    size_t allocsz = align_size_forward(
      (self->offset + size) - self->capacity,
      LARENA_ALIGNMENT
    );

    if (allocsz == 0) {
      return EOVERFLOW;
    }

    void *newdata = self->allocator->realloc(
      self->data,
      self->capacity,
      self->capacity + allocsz,
      self->allocator->ctx
    );
    if (newdata == NULL) {
      return ENOMEM;
    }

    self->capacity += allocsz;
    self->data      = newdata;
  }

  memset((uint8_t *)self->data + self->offset, 0, size);
  obj->allocator = self;
  obj->ptr       = self->offset;
  self->offset  += size;

  return 0;
}

void larena_clear(larena *self) {
  assert(self != NULL);

  self->offset = 0;
}

void larena_free(larena *self) {
  assert(self != NULL);
  assert(self->allocator != NULL);

  if (self->data != NULL) {
    self->allocator->dealloc(self->data, self->capacity, self->allocator->ctx);
    self->data = NULL;
  }
}

void *lobject_deref(const lobject *self) {
  assert(self != NULL);
  assert(self->allocator != NULL);

  return (void*)((uintptr_t)self->allocator->data + self->ptr);
}

#endif // LARENA_IMPLEMENTATION

#endif // __LARENA_H
