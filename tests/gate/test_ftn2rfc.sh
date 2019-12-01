#!/bin/sh

. $PWD/test-common.sh

FAIL=""
CMP="../cmp-rfc.sh"
COMMAND="$PWD/../../src/gate/ftn2rfc -1 -n"
TEST_DIR=tests_ftn2rfc
RESULT=$FIDOGATE_OUTRFC_NEWS/00000001.rfc

run_one()
{
    local command="$1"
    local dir="$2"
    local input="$PWD/$dir/input"
    local expected="$PWD/$dir/expected"

    $command $input

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
