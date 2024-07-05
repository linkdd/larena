#include <errno.h>

#include "utest.h"

#define LARENA_IMPLEMENTATION
#include "../include/larena.h"

UTEST_MAIN();

UTEST(main, readme) {
  lallocator allocator = {0};
  lallocator_init_stdlib(&allocator);

  larena arena = {0};
  larena_init(&arena, &allocator);

  lobject obj = {0};
  EXPECT_EQ(larena_alloc(&arena, sizeof(int), &obj), 0);

  int *ptr = (int *)lobject_deref(&obj);
  *ptr = 42;

  larena_clear(&arena);

  EXPECT_EQ(larena_alloc(&arena, sizeof(int), &obj), 0);
  EXPECT_EQ(*(int *)lobject_deref(&obj), 0);

  larena_free(&arena);
}

UTEST(main, zero_sized_alloc) {
  lallocator allocator = {0};
  lallocator_init_stdlib(&allocator);

  larena arena = {0};
  larena_init(&arena, &allocator);

  lobject obj_a = {0};
  lobject obj_b = {0};
  lobject obj_c = {0};
  EXPECT_EQ(larena_alloc(&arena, sizeof(int), &obj_a), 0);
  EXPECT_EQ(larena_alloc(&arena, 0, &obj_b), 0);
  EXPECT_EQ(larena_alloc(&arena, sizeof(int), &obj_c), 0);

  EXPECT_EQ(obj_b.ptr, obj_c.ptr);

  larena_free(&arena);
}

UTEST(ub, alignment) {
  lallocator allocator = {0};
  lallocator_init_stdlib(&allocator);

  larena arena = {0};
  larena_init(&arena, &allocator);

  lobject obj_a = {0};
  lobject obj_b = {0};
  EXPECT_EQ(larena_alloc(&arena, sizeof(short), &obj_a), 0);
  EXPECT_EQ(larena_alloc(&arena, sizeof(int), &obj_b), 0);

  short *ptr_a = (short *)lobject_deref(&obj_a);
  int *ptr_b   = (int *)lobject_deref(&obj_b);

  *ptr_a = 42;
  *ptr_b = 42;

  larena_free(&arena);
}

UTEST(int_overflow, large_alloc) {
  lallocator allocator = {0};
  lallocator_init_stdlib(&allocator);

  larena arena = {0};
  larena_init(&arena, &allocator);

  lobject obj = {0};
  EXPECT_EQ(larena_alloc(&arena, SIZE_MAX - 100, &obj), EOVERFLOW);

  larena_free(&arena);
}
