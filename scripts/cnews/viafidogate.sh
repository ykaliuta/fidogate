#!/bin/sh
# Submit news batch to FIDOGATE's rfc2ftn.
#
# The 'exec' cuts down the number of processes active for this simple case.
exec <LIBDIR>/rfc2ftn -b -n
