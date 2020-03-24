/*
 *  test-rfc2ftn.c
 */

#include "rfc2ftn.h"
#include "rfc2ftn-wrapper.c"

#include <cgreen/cgreen.h>
#include "fidogate.h"

/* rfc2ftn: */
/* From: john.doe@example.com -> John Doe */
/* From: john_doe@example.com -> John Doe */
/* From: john.doe@example.com (John Doe)-> John Doe */
/* From: john.doe@example.com (john doe)-> john doe */
/* From: John Doe <john.doe@example.com> -> John Doe */
/* From: john doe <john.doe@example.com> -> john doe */
/* From: "John Doe" <john.doe@example.com> -> John Doe */
/* From: "john doe" <john.doe@example.com> -> john doe */


Ensure(mailsender_converts_user_dot)
{
    Node node;
    RFCAddr rfc = {
        .user = "john.doe",
        .addr = "example.com",
        .real = "",
    };
    char res[NAMEBUFSIZE];
    char *exp = "John Doe";

    mail_sender(&rfc, &node, res, sizeof(res));

    assert_that(res, is_equal_to_string(exp));
}

Ensure(mailsender_converts_user_underscore)
{
    Node node;
    RFCAddr rfc = {
        .user = "john_doe",
        .addr = "example.com",
        .real = "",
    };
    char res[NAMEBUFSIZE];
    char *exp = "John Doe";

    mail_sender(&rfc, &node, res, sizeof(res));

    assert_that(res, is_equal_to_string(exp));
}

Ensure(mailsender_does_not_capitalizes_real)
{
    Node node;
    RFCAddr rfc = {
        .user = "john_doe",
        .addr = "example.com",
        .real = "john doe",
    };
    char res[NAMEBUFSIZE];
    char *exp = "john doe";

    mail_sender(&rfc, &node, res, sizeof(res));

    assert_that(res, is_equal_to_string(exp));
}

Ensure(mailsender_does_not_amend_dot_underscore_in_real)
{
    Node node;
    RFCAddr rfc = {
        .user = "john_doe",
        .addr = "example.com",
        .real = "john_doe.foo",
    };
    char res[NAMEBUFSIZE];
    char *exp = "john_doe.foo";

    mail_sender(&rfc, &node, res, sizeof(res));

    assert_that(res, is_equal_to_string(exp));
}

static TestSuite *create_my_suite(void)
{
    TestSuite *suite = create_named_test_suite("rfc2ftn suite");

    add_test(suite, mailsender_converts_user_dot);
    add_test(suite, mailsender_converts_user_underscore);
    add_test(suite, mailsender_does_not_capitalizes_real);
    add_test(suite, mailsender_does_not_amend_dot_underscore_in_real);

    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_my_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
