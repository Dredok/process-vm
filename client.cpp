#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "constants.h"
#include <string>
#include <thread>


int main() {
  auto fd = socket(AF_UNIX, SOCK_STREAM, 0);

  std::string str = "I'm the client.";
  sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, kSocketPath, sizeof(addr.sun_path)-1);
  int val = 1;
  if (setsockopt (fd, SOL_SOCKET, SO_PASSCRED, &val, sizeof (val)) < 0) {
    perror("unable to set local per credentials");
    exit(1);
  }
  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("connect error");
    exit(1);
  }
  ucred ucred;
  socklen_t len = sizeof(ucred);
  if (getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == -1) {
    perror("getsockopt error");
    exit(1);
  }

  printf("CLIENT (PID: %ld) ===> Credentials from SO_PEERCRED (server credentials): pid=%ld, euid=%ld, egid=%ld\n",
  (long)getpid(), (long) ucred.pid, (long) ucred.uid, (long) ucred.gid);
  //sending the address of the string
  struct msg {
    uint64_t address;
    uint64_t size;
  } msg;
  msg.address =  (uint64_t)str.data();
  msg.size = str.size();
  write(fd, &msg, sizeof(msg));
  close(fd);
  printf("Waiting a bit till the server ends\n");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  printf("Ending the client\n");
}