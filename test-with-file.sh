#!/usr/bin/env bash

if [ -z $1 ]
then
    echo "usage: $0 <test-file-to-send>"
    echo "example: $0 test/binary"
    exit $E_MISSING_POS_PARAM
fi

make
test_port=`shuf -i 30000-50000 -n 1`
echo "Testing with port $test_port"
echo "running ./kvServer $test_port &"
./kvServer $test_port &

echo put a $1 get a testAfterReceived quit | sed -r 's/\s+/\n/g' | ./kvClient localhost $test_port

echo "Done, now diff $1 testAfterReceived"
diff $1 testAfterReceived
