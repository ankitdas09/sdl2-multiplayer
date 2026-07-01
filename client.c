#include <stdio.h>
#include <SDL.h>
#include <stdbool.h>

#define WINDOW_W 800
#define WINDOW_H 800
#define TICK_RATE 30 
#define MS_PER_TICK (1000.0f / TICK_RATE)

typedef struct {
  SDL_Window *win;
  SDL_Renderer *ren;
  bool running;
} App;

typedef struct {
  float x, y;
  float w, h;
  float speed;
} Player;

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
  SDL_Rect rect = {(int)player->x, (int)player->y, (int)player->w, (int)player->h};
  SDL_RenderFillRect(ren, &rect);

  SDL_RenderPresent(ren);
}

void update(Player *player, float dt){
  const Uint8 *keys = SDL_GetKeyboardState(NULL);
  if(keys[SDL_SCANCODE_W]) player->y -= player->speed * dt;
  if(keys[SDL_SCANCODE_S]) player->y += player->speed * dt;
  if(keys[SDL_SCANCODE_A]) player->x -= player->speed * dt;
  if(keys[SDL_SCANCODE_D]) player->x += player->speed * dt;
}

int main(){
  App app;
  if(!init_app(&app)) return 1;

  SDL_Event event;

  float x = 360, y = 260;
  float speed = 200.0f;
  
  Player player;
  player.x = x, player.y = y, player.h = 80, player.w = 80, player.speed = speed;

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
