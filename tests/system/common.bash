
fail() {
    {
        echo ""
        if (( $# > 0 )); then
            echo "$@"
        else
            cat -
        fi
    } >&2
    return 1
}

msim_tests_setup() {
    MSIM="$( dirname "$BATS_TEST_FILENAME" )/../../msim"
    # If we are running a prehistoric version of BATS, we need to setup
    # the temporary directory ourselves to be on the safe side.
    if [ "$( ( echo "$BATS_VERSION"; echo 1.3.0 ) | sort -V | head -n 1 || true )" = "1.3.0" ]; then
        MSIM_TEST_TMPDIR="$BATS_TEST_TMPDIR"
    else
        MSIM_TEST_TMPDIR="$( mktemp -d -p "$BATS_RUN_TMPDIR" )"
    fi
}

setup() {
    msim_tests_setup
}

deindent() {
    python3 -c 'import sys,textwrap; print(textwrap.dedent(sys.stdin.read()).strip())'
}

msim_command_check() {
    local expected="$( echo "$expected" | deindent )"
    local expected_exit_code_is_zero="${exit_success:-true}"
    echo "$config" | deindent >"$MSIM_TEST_TMPDIR/msim.conf"
    echo "quit" >>"$MSIM_TEST_TMPDIR/msim.conf"

    {
        echo
        echo "# MSIM configuration msim.conf"
        sed 's:.*:#  | &:' "$MSIM_TEST_TMPDIR/msim.conf"
    } >&2

    run bash -c "cd '$MSIM_TEST_TMPDIR' && '$MSIM'"
    {
        echo
        echo "# MSIM output (stdout and stderr interleaved)"
        echo "$output" | sed 's:.*:#  | &:'
    } >&2

    if $expected_exit_code_is_zero; then
        if [ "$status" -ne 0 ]; then
            fail "MSIM failed with exit code $status."
        fi
    else
        if [ "$status" -eq 0 ]; then
            fail "MSIM terminated with exit code 0 but expecting failure."
        fi
    fi

    if [ "$output" != "$expected" ]; then
        {
            echo "Failure: unexpected output."
            echo "-- Expected --"
            echo "$expected"
            echo "-- Actual --"
            echo "$output"
            echo "--"
        } | fail
    fi
}
