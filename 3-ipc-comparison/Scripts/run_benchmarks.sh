msgMax=$( cat /proc/sys/kernel/msgmax )

srcFile=1gb.in
nRepeats=1

bufSizesFifo="200 4096 1048576"
bufSizesShm="200 4096 1048576"
bufSizesMsg="200 4096 $msgMax"

echo "Fifo ${bufSizesFifo}:"
./FifoBenchmark $srcFile $nRepeats $bufSizesFifo

echo "Shm ${bufSizesShm}:"
./ShmBenchmark $srcFile $nRepeats $bufSizesShm

echo "Msg ${bufSizesMsg}:"
./MsgBenchmark $srcFile $nRepeats $bufSizesMsg
