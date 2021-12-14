#!/bin/sh
#
# $Id: out-rmbsy.sh,v 4.2 1999/03/06 17:51:24 mj Exp $
#
# Remove BSY files in outbound, older than 4 hours
#
# Needs GNU find!
#

OUT="<BTBASEDIR>/out*"

find $OUT -follow -name '*.bsy' -mmin +240 -print -exec rm -f {} \; 2>/dev/null
