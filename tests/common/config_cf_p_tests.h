#ifndef _CONFIG_CF_P_TESTS_H
#define _CONFIG_CF_P_TESTS_H

#define GEN_CF_P_TESTS(name,dflt,cf_option)	\
    Ensure(cf_p_ ## name ## _default) \
    {				    \
	char *exp = dflt;	    \
	char *ret;		    \
				    \
	ret = cf_p_ ## name();	    \
						\
	assert_that(ret, is_equal_to_string(exp));	\
    }							\
							\
    Ensure(cf_p_ ## name ## _returns_config)			\
    {								\
	char *exp = "MY " dflt;					\
	char *ret;						\
	void *to_free;						\
								\
	to_free = cf_add_string(cf_option, exp);		\
								\
	ret = cf_p_ ## name();					\
								\
	assert_that(ret, is_equal_to_string(exp));		\
								\
	free(to_free);						\
    }

#define ADD_CF_P_TESTS(suite, name)		\
    add_test(suite, cf_p_ ## name ## _default); \
    add_test(suite, cf_p_ ## name ## _returns_config) ;

#endif /* _CONFIG_CF_P_TESTS_H */
