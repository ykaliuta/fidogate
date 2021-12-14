#!/bin/sh
#
# $Id: out-rm0.sh,v 4.1 1999/03/06 17:51:23 mj Exp $
#
# Remove empty ArcMail archives in outbound
#
# Needs GNU find!
#

OUT="<BTBASEDIR>/out*"

find $OUTBOUND -type f -size 0c -name '*.[mtwfs][ouehrs][0-9]' -exec rm -f {} \; -print
