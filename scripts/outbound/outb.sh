#!/bin/sh
#
# $Id: outb.sh,v 4.1 2001/01/28 15:53:16 mj Exp $
#
# Pager wrapper for out-ls
#
exec <BINDIR>/out-ls | ${PAGER-less}

