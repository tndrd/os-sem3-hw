echo "Testing Fifo..."
./Fifo 100 1mb.in 2> /dev/null > /dev/null
(md5sum 1mb.in; md5sum out) | python3 ../../3-ipc-comparison/Tests/CompareMd5.py
rm -rf out

echo "\nTesting Shm..."
./Shm 100 1mb.in 2> /dev/null > /dev/null
(md5sum 1mb.in; md5sum out) | python3 ../../3-ipc-comparison/Tests/CompareMd5.py
rm -rf out

echo "\nTesting Msg..."
./Msg 100 1mb.in 2> /dev/null > /dev/null
(md5sum 1mb.in; md5sum out) | python3 ../../3-ipc-comparison/Tests/CompareMd5.py
rm -rf out