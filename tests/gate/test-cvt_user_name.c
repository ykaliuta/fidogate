/*
 */

#include "rfc2ftn.h"
#include <cgreen/cgreen.h>

Ensure(cvt_accepts_empty_line)
{
    char str[] = "";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string(""));
}

Ensure(cvt_capitalizes_one_word)
{
    char str[] = "word";
    
    cvt_user_name(str);

    assert_that(str, is_equal_to_string("Word"));
}

Ensure(cvt_converts_the_only_percent)
{
    char str[] = "word%second";

    cvt_user_name(str);

    assert_that(str[4], is_equal_to('@'));
}

Ensure(cvt_converts_only_last_percent)
{
    char str[] = "averyverylongword";

    /* insert several % in random positions */
    str[3] = '%';
    str[5] = '%';
    str[6] = '%';
    str[10] = '%';

    cvt_user_name(str);

    assert_that(str[3], is_equal_to('%'));
    assert_that(str[5], is_equal_to('%'));
    assert_that(str[6], is_equal_to('%'));
    assert_that(str[10], is_equal_to('@'));
}    

Ensure(cvt_converts_dot_to_space)
{
    char str[] = "User.Name";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("User Name"));
}

Ensure(cvt_converts_all_dots_to_space)
{
    char str[] = "User.Name.Some.Other";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("User Name Some Other"));
}

Ensure(cvt_converts_underscore_to_space)
{
    char str[] = "User_Name";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("User Name"));
}

Ensure(cvt_converts_all_underscores_to_space)
{
    char str[] = "User_Name_Some_Other";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("User Name Some Other"));
}

Ensure(cvt_converts_dots_and_underscores)
{
    char str[] = "user.name_some_other.word";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("User.Name Some Other.Word"));
}

Ensure(cvt_converts_dot_email)
{
    char str[] = "user.name@p.f.n.z.fidonet.org";

    cvt_user_name(str);

    assert_that(str,
		is_equal_to_string("User.Name@p.F.N.Z.Fidonet.Org"));
}

Ensure(cvt_converts_underscore_email)
{
    char str[] = "user_name@p.f.n.z.fidonet.org";

    cvt_user_name(str);

    assert_that(str,
		is_equal_to_string("User Name@p.F.N.Z.Fidonet.Org"));
}

Ensure(cvt_converts_underscore_and_dot_email)
{
    char str[] = "user_u.name@p.f.n.z.fidonet.org";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("User U.Name@p.F.N.Z.Fidonet.Org"));
}

Ensure(cvt_converts_dot_email_with_percent)
{
    char str[] = "user.name%host%p.f.n.z.fidonet.org";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("user.name%host@p.f.n.z.fidonet.org"));
}

Ensure(cvt_converts_underscore_email_with_percent)
{
    char str[] = "user_name%host%p.f.n.z.fidonet.org";

    cvt_user_name(str);

    assert_that(str, is_equal_to_string("user_name%host@p.f.n.z.fidonet.org"));
}

TestSuite *create_cvt_user_name_suite(void)
{
    TestSuite *suite = create_named_test_suite("cvt_user_name suite");

    add_test(suite, cvt_accepts_empty_line);
    add_test(suite, cvt_capitalizes_one_word);
    add_test(suite, cvt_converts_the_only_percent);
    add_test(suite, cvt_converts_only_last_percent);
    add_test(suite, cvt_converts_dot_to_space);
    add_test(suite, cvt_converts_all_dots_to_space);
    add_test(suite, cvt_converts_underscore_to_space);
    add_test(suite, cvt_converts_all_underscores_to_space);
    add_test(suite, cvt_converts_dots_and_underscores);
    add_test(suite, cvt_converts_dot_email);
    add_test(suite, cvt_converts_underscore_email);
    add_test(suite, cvt_converts_underscore_and_dot_email);
    add_test(suite, cvt_converts_dot_email_with_percent);
    add_test(suite, cvt_converts_underscore_email_with_percent);

    return suite;
}
