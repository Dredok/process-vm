# process-vm
simple proof of concept reading memory from an unrelated process using process-vm-readv syscall

* needs linux kernel 5.6 or higher

simple instructions to run it:
```
mkdir build
cd build && cmake .. && make
sudo setcap CAP_SYS_PTRACE=ep ./server
./server &
./client
```
