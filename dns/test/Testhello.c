#include "hello.h"
#include "unity.h"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_f_hello_1(void)
{
    TEST_ASSERT_EQUAL_STRING("Hello, World!", hello());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_f_hello_1);
    return UNITY_END();
}
