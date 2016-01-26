/*
 * This is a test wrapper of rfc2ftn
 *
 * It's purpose is to remove main() function
 * and export needed APIs
 */

#include <stdio.h>

FILE *rfc2ftn_stdin;

#define main rfc2ftn_main
#ifdef stdin
#undef stdin
#define stdin rfc2ftn_stdin
#endif

#include "../../src/gate/rfc2ftn.c"

#undef main
#undef stdin

