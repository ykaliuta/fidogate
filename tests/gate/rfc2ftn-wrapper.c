/*
 * This is a test wrapper of rfc2ftn
 *
 * It's purpose is to remove main() function
 * and export needed APIs
 */

#define main test_main
#include <rfc2ftn.c>
#undef main

