#include <stdio.h>
#include "net.h"
#include <fcntl.h>
#include <string.h>

int net_init() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) {
    perror("socket");
    return -1;
  }
  
  int flags = fcntl(sockfd, F_GETFL);
  if(flags < 0){
    perror("fcntl get");
    return -1;
  }

  if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0){
    perror("fcntl set");
    return -1;
  }

  struct sockaddr_in server_addr;

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    perror("bind");
    return -1;
  }

  return sockfd;
}
