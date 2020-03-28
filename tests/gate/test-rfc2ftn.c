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

Ensure(rfcsender_parses_no_realname)
{
    char *src = "john.doe@example.com";
    char *user = "john.doe";
    char *addr = "example.com";
    char *real = "";

    RFCAddr res;

    res = _rfc_sender(src, NULL);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

Ensure(rfcsender_parses_comment)
{
    char *src = "john.doe@example.com (John Doe)";
    char *user = "john.doe";
    char *addr = "example.com";
    char *real = "John Doe";

    RFCAddr res;

    res = _rfc_sender(src, NULL);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

Ensure(rfcsender_parses_no_quotes)
{
    char *src = "John Doe <john.doe@example.com>";
    char *user = "john.doe";
    char *addr = "example.com";
    char *real = "John Doe";

    RFCAddr res;

    res = _rfc_sender(src, NULL);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

Ensure(rfcsender_parses_quotes)
{
    char *src = "\"John Doe\" <john.doe@example.com>";
    char *user = "john.doe";
    char *addr = "example.com";
    char *real = "John Doe";

    RFCAddr res;

    res = _rfc_sender(src, NULL);

    assert_that(res.user, is_equal_to_string(user));
    assert_that(res.addr, is_equal_to_string(addr));
    assert_that(res.real, is_equal_to_string(real));
}

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

Ensure(rfcparse_converts_useronly)
{
    RFCAddr src = {
        .real = "",
        .user = "user",
        .addr = "example.com",
    };
    char res[NAMEBUFSIZE];
    char *exp = "User";

    rfc_parse(&src, res, sizeof(res), NULL, FALSE);

    assert_that(res, is_equal_to_string(exp));
}

Ensure(rfcparse_doesnot_convert_real)
{
    RFCAddr src = {
        .real = "user",
        .user = "user",
        .addr = "example.com",
    };
    char res[NAMEBUFSIZE];
    char *exp = "user";

    rfc_parse(&src, res, sizeof(res), NULL, FALSE);

    assert_that(res, is_equal_to_string(exp));
}

static TestSuite *create_my_suite(void)
{
    TestSuite *suite = create_named_test_suite("rfc2ftn suite");

    add_test(suite, mailsender_converts_user_dot);
    add_test(suite, mailsender_converts_user_underscore);
    add_test(suite, mailsender_does_not_capitalizes_real);
    add_test(suite, mailsender_does_not_amend_dot_underscore_in_real);

    add_test(suite, rfcsender_parses_no_realname);
    add_test(suite, rfcsender_parses_comment);
    add_test(suite, rfcsender_parses_no_quotes);
    add_test(suite, rfcsender_parses_quotes);

    add_test(suite, rfcparse_converts_useronly);
    add_test(suite, rfcparse_doesnot_convert_real);

    return suite;
}

int main(int argc, char **argv)
{
    TestSuite *suite = create_my_suite();

    if (argc > 1)
	return run_single_test(suite, argv[1], create_text_reporter());

    return run_test_suite(suite, create_text_reporter());
}
