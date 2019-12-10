#!/bin/sh

WORKDIR=$(dirname $(realpath $0))

. $WORKDIR/test-common.sh

FAIL=""
CMP="$WORKDIR/../cmp-pkt"
COMMAND="$WORKDIR/../../src/gate/rfc2ftn -n"
TEST_DIR=$WORKDIR/tests_rfc2ftn
RESULT=$FIDOGATE_OUTPKT_NEWS/00000001.pkt

run_one()
{
    local command="$1"
    local dir="$2"
    local input="$WORKDIR/$dir/input"
    local expected="$WORKDIR/$dir/expected"

    cat $input | $command

    if $CMP $expected $RESULT; then
        echo "Test $dir PASSED"
    else
        echo "Test $dir FAILED"
        return 1
    fi
}

# run_dir is in test-common.sh

run_dir "$COMMAND" $TEST_DIR
[ -z $FAIL ]
