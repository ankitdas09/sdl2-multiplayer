#include <stdio.h>
#include <string.h>
#include "game.h"
#include "protocol.h"

void game_init(GameState *gs) {
  memset(gs, 0, sizeof(*gs));

  for(int i = 0; i < MAP_H; i++){
    gs->map[i][0] = 1;
    gs->map[i][MAP_W - 1] = 1;
  }

  for(int i = 0; i < MAP_W; i++){
    gs->map[0][i] = 1;
    gs->map[MAP_H - 1][i] = 1;
  }

  int crate_positions[5][2] = {
    {2,3},
    {4,5},
    {7,8},
    {9,10},
    {5,9},
  };

  for(int i = 0; i < 5; i++){
    gs->crates[i].x = crate_positions[i][0];
    gs->crates[i].y = crate_positions[i][1];
    gs->crates[i].is_alive = true;
    gs->crates[i].mass = CRATE_MASS_KG;
  }

  float spawn_x[MAX_PLAYERS] = {2, MAP_W-3, 2,        MAP_W-3};
  float spawn_y[MAX_PLAYERS] = {2, 2,       MAP_H-3,  MAP_H-3};

  for (int i = 0; i < MAX_PLAYERS; i++) {
    gs->players[i].x = spawn_x[i];
    gs->players[i].y = spawn_y[i];
  }

}

void game_player_leave(GameState *gs, uint8_t player_id){
  if(player_id >= MAX_PLAYERS) return;
  gs->players[player_id].health = 0;
  gs->players[player_id].is_alive = false;
}

void game_player_join(GameState *gs, uint8_t player_id){
  if(player_id >= MAX_PLAYERS) return;
  gs->players[player_id].health = MAX_PLAYER_HEALTH;
  gs->players[player_id].is_alive = true;
  gs->players[player_id].bomb_held = MAX_BOMBS_HELD;
}


void game_process_input(GameState *gs, uint8_t player_id, ClientInput *input) {
}

void game_tick(GameState *gs) {
}
