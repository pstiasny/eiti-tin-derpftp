#!/bin/bash

FAILED_ASSERTIONS=0

transaction() {
    rm -f /tmp/OUT
    bin/derpftpd 2>&1 > /tmp/dout &
    DPID=$!
    bin/testclient 2>&1 > /tmp/cout
    kill -SIGTERM $DPID 2>&1 > /dev/null
}

assert_file_eq() {
    diff -w $2 -
    if [ $? != 0 ]; then
        echo "** FAIL **: $1"
        FAILED_ASSERTIONS=$(( $FAILED_ASSERTIONS + 1 ))
    fi
}


###############################################################################
echo "TEST opening a simple file"
transaction <<TERM
open tests/FILE1
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_close_server returned 0
TERM


###############################################################################
echo "TEST downloading a simple file"
transaction <<TERM
open tests/FILE1
read /tmp/OUT 2048
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_read returned 32
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT < tests/FILE1


###############################################################################
echo "TEST downloading a part of simple file"
transaction <<TERM
open tests/FILE1
read /tmp/OUT 10
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_read returned 10
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT <<TERM
to dane te
TERM


###############################################################################
echo "TEST downloading inner part of simple file"
transaction <<TERM
open tests/FILE1
lseek 11 0
lseek -1 1
read /tmp/OUT 5
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_lseek returned 11
fs_lseek returned 10
fs_read returned 5
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT <<TERM
stowe
TERM


###############################################################################
echo "TEST uploading a simple file"
transaction <<TERM
open /tmp/OUT
write tests/FILE1 1024
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_write returned 32
fs_close_server returned 0
TERM

assert_file_eq "uploaded file" /tmp/OUT < tests/FILE1


###############################################################################
echo "TEST opening a file with plain text"
transaction <<TERM
open tests/FILE2
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_close_server returned 0
TERM


###############################################################################
echo "TEST downloading a file with plain text"
transaction <<TERM
open tests/FILE2
read /tmp/OUT 4096
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_read returned 32
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT < tests/FILE2


###############################################################################
echo "TEST downloading a part of file with plain text"
transaction <<TERM
open tests/FILE2
read /tmp/OUT 100
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_read returned 10
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT <<TERM
Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore
TERM


###############################################################################
echo "TEST downloading inner part of file with plain text"
transaction <<TERM
open tests/FILE2
lseek 50 0
lseek -1 10
read /tmp/OUT 5
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_lseek returned 11
fs_lseek returned 10
fs_read returned 5
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT <<TERM
qwewqreetafsgadg
TERM


###############################################################################
echo "TEST uploading a file with plain text"
transaction <<TERM
open /tmp/OUT
write tests/FILE2 1024
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_write returned 32
fs_close_server returned 0
TERM

assert_file_eq "uploaded file" /tmp/OUT < tests/FILE2


###############################################################################
echo "TEST opening a file with bad test case"
transaction <<TERM
open tests/FILE3
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_close_server returned 0
TERM


###############################################################################
echo "TEST downloading a file with bad test case"
transaction <<TERM
open tests/FILE3
read /tmp/OUT 4096
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_read returned 32
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT < tests/FILE1


###############################################################################
echo "TEST downloading a part of file with bad test case"
transaction <<TERM
open tests/FILE3
read /tmp/OUT 100
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_read returned 10
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT <<TERM
to dane te
TERM


###############################################################################
echo "TEST downloading inner part of file with bad test case"
transaction <<TERM
open tests/FILE3
lseek 50 0
lseek -1 10
read /tmp/OUT 5
close
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_lseek returned 11
fs_lseek returned 10
fs_read returned 5
fs_close returned 0
fs_close_server returned 0
TERM

assert_file_eq "downloaded file" /tmp/OUT <<TERM
stowe
TERM


###############################################################################
echo "TEST uploading a file with bad test case"
transaction <<TERM
open /tmp/OUT
write tests/FILE3 1024
TERM

assert_file_eq "client messages" /tmp/cout <<TERM
fs_open_server returned 0
fs_open returned 3
fs_write returned 32
fs_close_server returned 0
TERM

assert_file_eq "uploaded file" /tmp/OUT < tests/FILE1


###############################################################################

if [ $FAILED_ASSERTIONS -gt 0 ]; then
    echo "$FAILED_ASSERTIONS assertions failed" >&2
    exit 1
fi
