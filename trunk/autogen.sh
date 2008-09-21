#!/bin/sh

aclocal
autoheader
automake --copy --add-missing --force-missing
autoconf --force
