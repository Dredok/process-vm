# process-vm
simple proof of concept reading memory from an unrelated process using process-vm-readv syscall


This proof of concept is to demonstrate how an arbitrary process (with CAP_SYS_PTRACE capability) can read or write directly from the memory of an unrelated process. 
The "server" process will create a unix domain socket, which will use to listen for connections. Then the "client" process will establish the connection; passing a single message with address to be read and the size. The server reads the message, calls process_vm_readv, shows the "client" memory (a simple string in this POC) and closes the connection. The client simply waits 1 second in order to give time to the server to read the memory and exits.

Take into account this works even crossing docker boundaries, since the call getsockopt with SO_PEERCREED translates PIDs number to the current namespace. (At least it works for the server being at the host and the client running in a dockerized env, I don't think it would work the other way around).


* needs linux kernel 5.6 or higher

simple instructions to run it:
```
mkdir build
cd build && cmake .. && make
sudo setcap CAP_SYS_PTRACE=ep ./server
./server &
./client
```
