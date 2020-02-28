#!/bin/sh

ONETEST="$1"

WORKDIR=$(dirname $(realpath $0))

. $WORKDIR/test-common.sh

FAIL=""
CMP="$WORKDIR/../cmp-rfc.sh"
COMMAND="$WORKDIR/../../src/gate/ftn2rfc -1 -n"
TEST_DIR=$WORKDIR/tests_ftn2rfc
RESULT=$FIDOGATE_OUTRFC_NEWS/00000001.rfc

run_one()
{
    local command="$1"
    local dir="$2"
    local input="$WORKDIR/$dir/input"
    local expected="$WORKDIR/$dir/expected"

    $command $input

    if $CMP $expected $RESULT; then
        echo "Test $dir PASSED"
    else
        echo "Test $dir FAILED"
        return 1
    fi
}

# run_dir is in test-common.sh

run_dir "$COMMAND" $TEST_DIR "$ONETEST"
[ -z $FAIL ]
