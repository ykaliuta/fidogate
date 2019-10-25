#!/bin/sh

ROOT=$PWD/testroot

export FIDOGATE_CONFIGDIR=$ROOT/etc
export FIDOGATE_OUTPKT_NEWS=$ROOT/outpkt_news
export FIDOGATE_OUTPKT_MAIL=$ROOT/outpkt_mail
export FIDOGATE_OUTRFC_NEWS=$ROOT/outrfc_news
export FIDOGATE_OUTRFC_MAIL=$ROOT/outrfc_mail
export FIDOGATE_LOGFILE=$ROOT/log/log
export FIDOGATE_SEQ_PKT=$ROOT/seq/pkt
export FIDOGATE_SEQ_MAIL=$ROOT/seq/mail
export FIDOGATE_SEQ_NEWS=$ROOT/seq/news
export FIDOGATE_LIBEXECDIR=$ROOT/lib
export FIDOGATE_INBOUND=$ROOT/in


DIRS="$FIDOGATE_OUTPKT_NEWS $FIDOGATE_OUTPKT_MAIL \
      $FIDOGATE_OUTRFC_NEWS $FIDOGATE_OUTRFC_MAIL \
      $ROOT/log $ROOT/seq \
      $FIDOGATE_INBOUND"

create_dirs()
{
    for d in $DIRS; do
        [ -d $d ] || mkdir -p $d
    done
}

clear_pool()
{
    rm -f $FIDOGATE_OUTPKT_NEWS/*.pkt
    rm -f $FIDOGATE_OUTPKT_MAIL/*.pkt
    rm -f $FIDOGATE_OUTRFC_NEWS/*.rfc
    rm -f $FIDOGATE_OUTRFC_MAIL/*.rfc
}

reset_seq()
{
    echo 0 > $FIDOGATE_SEQ_PKT
    echo 0 > $FIDOGATE_SEQ_MAIL
    echo 0 > $FIDOGATE_SEQ_NEWS
}

clear_log()
{
    rm -f $FIDOGATE_LOGFILE
}

setup()
{
    create_dirs
    reset_seq
    clear_pool
    clear_log
}

# requires run_one from the main file
# since now way of calling is different
run_dir()
{
    local command="$1"
    local dir="$2"

    for t in $(ls -1d $dir/test*); do
        if [ -d "$PWD/$t/etc" ]; then
            export FIDOGATE_CONFIGDIR="$PWD/$t/etc"
        fi

        setup

        run_one "$command" "$t" || FAIL=true

        export FIDOGATE_CONFIGDIR=$ROOT/etc
    done
}
