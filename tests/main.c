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
  ASSERT_TRUE(larena_alloc(&arena, sizeof(int), &obj));

  int *ptr = (int *)lobject_deref(&obj);
  *ptr = 42;

  larena_clear(&arena);

  ASSERT_TRUE(larena_alloc(&arena, sizeof(int), &obj));
  EXPECT_EQ(0, *(int *)lobject_deref(&obj));

  larena_free(&arena);
}
