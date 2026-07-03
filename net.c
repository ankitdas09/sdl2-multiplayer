#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "net.h"
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>

static NetClient clients[MAX_PLAYERS];

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

  memset(clients, 0, sizeof(clients));

  return sockfd;
}

long long now_us(){
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

int find_or_register_client(struct sockaddr_in *addr, GameState *gs, bool *is_new){
  for(int i = 0; i < MAX_PLAYERS; i++){
    if(clients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr && clients[i].addr.sin_port == addr->sin_port){
      *is_new = false;
      return i;
    }
    if(!clients[i].is_active){
      *is_new = true;
      clients[i].addr = *addr;
      clients[i].is_active = true;
      clients[i].id = i;
      clients[i].last_seen = time(NULL);
      game_player_join(gs, i);
      return i;
    }
  }

  return -1;
}

void net_broadcast_snapshot(int sock, GameState *gs){
  ServerSnapshot snapshot = {0};

  snapshot.type = PACKET_SNAPSHOT;
  snapshot.tick = gs->tick;
  
  for (int i = 0; i < MAX_PLAYERS; i++) {
    snapshot.players[i].is_alive = gs->players[i].is_alive;
    snapshot.players[i].x = gs->players[i].x;
    snapshot.players[i].y = gs->players[i].y;
    snapshot.players[i].health = gs->players[i].health;
  }
 
  for (int i = 0; i < MAX_CRATES; i++) {
    snapshot.crates[i].is_alive = gs->crates[i].is_alive;
    snapshot.crates[i].x = gs->crates[i].x;
    snapshot.crates[i].y = gs->crates[i].y;
  }
 
  for (int i = 0; i < MAX_BOMBS; i++) {
    snapshot.bombs[i].is_alive = gs->bombs[i].is_alive;
    snapshot.bombs[i].x = gs->bombs[i].x;
    snapshot.bombs[i].y = gs->bombs[i].y;
    snapshot.bombs[i].time_to_detonate = gs->bombs[i].time_to_detonate;
  }

  for(int i = 0; i < MAX_CLIENT; i++){
    if(!clients[i].is_active) continue;
    sendto(sock, &snapshot, sizeof(snapshot), 0, (struct sockaddr *)&clients[i].addr, sizeof(clients[i].addr));
  }
}

void net_send_welcome(int sock, int player_id, struct sockaddr_in *addr, GameState *gs) {
  ServerWelcomePacket welcome_packet = {0};
  welcome_packet.player_id = player_id;
  welcome_packet.health = MAX_PLAYER_HEALTH;
  welcome_packet.current_server_tick = gs->tick;
  welcome_packet.type = PACKET_WELCOME;
  welcome_packet.x = gs->players[player_id].x;
  welcome_packet.y = gs->players[player_id].y;

  sendto(sock, &welcome_packet, sizeof(welcome_packet), 0, (struct sockaddr *)addr, sizeof(*addr)); 
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
          bool is_new = false;
          int slot = find_or_register_client(&client, gs, &is_new); 
          if(slot < 0){
            printf("server full\n");
            continue;
          }
          if(is_new) {

          }
          clients[slot].last_seen = time(NULL);
          ClientInput client_input;
          if((size_t)bytes >= sizeof(ClientInput)){
            memcpy(&client_input, buffer, sizeof(ClientInput));
            if(client_input.type == PACKET_INPUT){
              game_process_input(gs, (uint8_t)slot, &client_input);
            }
          }
        }
      }
    }

    long long _now_us = now_us();

    if(_now_us >= next_tick_time){
      printf("Tick %u\n", tick);
      next_tick_time += TICK_US;
      game_tick(gs) ;
      net_broadcast_snapshot(sock, gs);
      tick++;
    }
  }
}
