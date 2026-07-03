#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include "net.h"
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>

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

long long now_us(){
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

void net_run(int sock, GameState *gs){ 
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);
  
  fd_set readfds;
  char buffer[1024];

  long long next_tick_time = now_us();
  uint32_t tick = 0;

  while(1){
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = TICK_US; 
    select(sock + 1, &readfds, NULL, NULL, &tv);

    if(FD_ISSET(sock, &readfds)){
      while(1){
        int bytes = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client, &client_len);
        if(bytes < 0) {
          if(errno == EAGAIN || errno == EWOULDBLOCK) break;
          perror("recvfrom");
          break;
        } else {
          buffer[bytes] = '\0';
          printf("Message: %s\n", buffer);
        }
      }
    }

    long long _now_us = now_us();

    if(_now_us >= next_tick_time){
      printf("Tick %u\n", tick);
      next_tick_time += TICK_US;
      game_tick(gs);
      tick++;
    }
  }
}
