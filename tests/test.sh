#!/bin/sh

run_netcat_server() {
    nc -l -x -p 1337 -w 2 -o /tmp/ncout < tests/$1-in-nc.bin > /dev/null
}

run_testclient() {
    sleep 0.5
    bin/testclient < tests/$1-in-client > /tmp/testclientout
}

run_test() {
    run_netcat_server $1 & run_testclient $1
    wait

    diff /tmp/ncout tests/$1-out-nc.hex &&
    diff /tmp/testclientout tests/$1-out-client

    if [ $? == 0 ]
    then
        echo "test $1 succeeded"
    else
        echo "test $1 FAILED"
    fi
}

run_test 1

