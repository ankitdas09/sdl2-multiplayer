#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "game.h"
#include "protocol.h"

#define WINDOW_W 800
#define WINDOW_H 800
#define TICK_RATE 30 
#define MS_PER_TICK (1000.0f / TICK_RATE)

typedef struct {
  SDL_Window *win;
  SDL_Renderer *ren;
  bool running;
} App;

const Uint8 PLAYER_H = 80;
const Uint8 PLAYER_W = 80;

int client_init_socket(struct sockaddr_in *server_addr){
  server_addr->sin_port = htons(7777);
  char *server_ip = "127.0.0.1";
  server_addr->sin_family = AF_INET;

  if(inet_pton(AF_INET, server_ip, &server_addr->sin_addr.s_addr) < 0){
    perror("inet_pton");
    return -1;
  }

  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd < 0){
    perror("socket");
    return -1;
  }

  int flags = fcntl(sock_fd, F_GETFL);
  if(flags < 0){
    perror("fcntl get");
    return -1;
  }
 
  if(fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) < 0){
    perror("fcntl set");
    return -1;
  }

  return sock_fd;
}

void client_send_input(int sock, struct sockaddr_in *server_addr, int tick){
  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  int move_x = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
  int move_y = keys[SDL_SCANCODE_S] - keys[SDL_SCANCODE_W];
  
  ClientInput client_input;
  memset(&client_input, 0, sizeof(ClientInput));

  client_input.type = PACKET_INPUT; 
  client_input.tick = tick;
  client_input.move_x = move_x;
  client_input.move_y = move_y;
  Uint8 buttons = 0;
  if(keys[SDL_SCANCODE_SPACE]){
    buttons |= BTN_PLACE_BOMB;
  }
  client_input.buttons = buttons;

  sendto(sock, &client_input, sizeof(client_input), 0, (struct sockaddr*)server_addr, sizeof(*server_addr));
}

void client_receive_packets(int sock) {
  char *buffer[1024] = {0};
  int bytes = recvfrom(sock, void *, size_t, , struct sockaddr *restrict, socklen_t *restrict);

}

bool init_app(App *a) {
  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
    return false;
  }

  SDL_Window *win = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
  if(!win){
    fprintf(stderr, "SDL Window error: %s\n", SDL_GetError());
    SDL_Quit();
    return false;
  }

  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(!ren){
    fprintf(stderr, "SDL Renderer error: %s\n", SDL_GetError());
    SDL_DestroyWindow(win);
    SDL_Quit();
    return false;
  }

  a->win = win;
  a->ren = ren;
  a->running = true;

  return true;
}

void render(SDL_Renderer *ren, Player *player){
  SDL_SetRenderDrawColor(ren, 15, 15, 30, 255);
  SDL_RenderClear(ren);

  SDL_SetRenderDrawColor(ren, 123, 123, 123, 255);
  SDL_Rect rect = {(int)player->x, (int)player->y, (int)PLAYER_W, (int)PLAYER_H};
  SDL_RenderFillRect(ren, &rect);

  SDL_RenderPresent(ren);
}

void update(Player *player, float dt){
  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  if(keys[SDL_SCANCODE_W]) player->y -= PLAYER_SPEED * dt;
  if(keys[SDL_SCANCODE_S]) player->y += PLAYER_SPEED * dt;
  if(keys[SDL_SCANCODE_A]) player->x -= PLAYER_SPEED * dt;
  if(keys[SDL_SCANCODE_D]) player->x += PLAYER_SPEED * dt;
}

int main(){
  App app;
  if(!init_app(&app)) return 1;

  SDL_Event event;

  float x = 360, y = 260;
  float speed = 200.0f;
  
  Player player;
  player.x = x, player.y = y;

  Uint32 last = SDL_GetTicks();
  double accumulator = 0.0;

  while(app.running){
    Uint32 now = SDL_GetTicks();
    Uint32 frame = now - last;
    if(frame > 250) frame = 250; // clamp to avoid spiral of death on stalls
    accumulator += frame;
    last = now;

    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT) app.running = false;
      if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) app.running = false;
    }
    
    while(accumulator >= MS_PER_TICK){
      update(&player, (MS_PER_TICK / 1000.0f));
      accumulator -= MS_PER_TICK;
    }

    render(app.ren, &player);
  }

  SDL_DestroyRenderer(app.ren);
  SDL_DestroyWindow(app.win);
  SDL_Quit();

  return 0;
}
