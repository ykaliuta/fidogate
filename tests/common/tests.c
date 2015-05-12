/* */

#include <cgreen/cgreen.h>

TestSuite *create_log_suite(void);

int main(int argc, char **argv) {
    TestSuite *suite = create_log_suite();
    return run_test_suite(suite, create_text_reporter());
}

