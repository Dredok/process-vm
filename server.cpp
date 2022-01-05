#include <asm-generic/socket.h>
#include <cstdint>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/param.h>




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include <sys/uio.h>
#include <string>
#include <iostream>

int main() {
  auto fd = socket(AF_UNIX, SOCK_STREAM, 0);
  int val = 1;

  union {
    char buf[CMSG_SPACE(sizeof(struct ucred))]; /* Space large enough to hold a 'ucred' structure */
    cmsghdr align;
  } controlMsg;
  

  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path)-1);
  remove(kSocketPath);
  if(bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind error");
    exit(1);
  }  
  
  if (listen(fd, 10) < 0) {
    perror("listen error");
    exit(1);
  }
  
  while (true) {
    auto cl = accept(fd, nullptr, nullptr);
    if (cl < 0) {
      perror("accept error");
      continue;
    }    
    if (setsockopt (cl, SOL_SOCKET, SO_PASSCRED, &val, sizeof (val)) < 0) {
      perror("unable to set local per credentials");
      exit(1);
    }

    socklen_t len = sizeof(ucred);
    struct ucred ucred;
    if (getsockopt(cl, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1) {
      perror("getsockopt error");
      continue;
    }

    printf("SERVER (PID:%ld) ===> Credentials from SO_PEERCRED (client credentials): pid=%ld, euid=%ld, egid=%ld\n",
    (long)getpid(), (long) ucred.pid, (long) ucred.uid, (long) ucred.gid);
    struct msg {
      uint64_t address;
      uint64_t size;
    } msg;

    read(cl, &msg, sizeof(msg));
    std::string serverString;
    serverString.resize(msg.size);
    iovec local;
    iovec remote;
    local.iov_base = &serverString[0];
    local.iov_len = msg.size;

    remote.iov_base = (void*)msg.address;
    remote.iov_len = msg.size;
    if(process_vm_readv(ucred.pid, &local, 1, &remote, 1, 0) != msg.size) {
      perror("process_vm_readv");
      exit(1);
    };
    
    std::cout << "Client string was: " << serverString << std::endl;
    close(cl);
  }
}
