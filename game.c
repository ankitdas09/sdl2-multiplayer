#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "protocol.h"
#include <math.h>

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
  if(!gs->players[player_id].is_alive) return;

  gs->players[player_id].velocity_x = input->move_x * PLAYER_SPEED;
  gs->players[player_id].velocity_y = input->move_y * PLAYER_SPEED;

  if(input->buttons & BTN_PLACE_BOMB && gs->players[player_id].bomb_held > 0){
    for(int i = 0; i < MAX_BOMBS; i++){
      if(!gs->bombs[i].is_alive){
        gs->bombs[i].is_alive = true;
        gs->bombs[i].owner_id = player_id;
        gs->bombs[i].x = gs->players[player_id].x;
        gs->bombs[i].y = gs->players[player_id].y;
        gs->bombs[i].time_to_detonate = TICKS_TO_DETONATE;
        break;
      }
    }
  }
}

void game_explode_bomb(GameState *gs, int bomb_index){
  float bomb_x = gs->bombs[bomb_index].x;
  float bomb_y = gs->bombs[bomb_index].y;

  for(int i = 0; i < MAX_CRATES; i++){
    if(!gs->crates[i].is_alive) continue;

    float crate_x = gs->crates[i].x; 
    float crate_y = gs->crates[i].y; 

    float dx = crate_x - bomb_x;
    float dy = crate_y - bomb_y;
    float dist_from_bomb = sqrtf((dx*dx) + (dy*dy));

    if(dist_from_bomb <= BOMB_BLAST_RADIUS && dist_from_bomb > 0){
      float dxn_vec_x = dx / dist_from_bomb;
      float dxn_vec_y = dy / dist_from_bomb;

      float fall_off = 1.0f - (dist_from_bomb / BOMB_BLAST_RADIUS);
      float force = BOMB_FORCE * fall_off;

      gs->crates[i].velocity_x += dxn_vec_x * (force / gs->crates[i].mass);
      gs->crates[i].velocity_y += dxn_vec_y * (force / gs->crates[i].mass);
    }
  }


  for(int i = 0; i < MAX_PLAYERS; i++){
    if(!gs->players[i].is_alive) continue;

    float player_x = gs->players[i].x; 
    float player_y = gs->players[i].y; 

    float dx = player_x - bomb_x;
    float dy = player_y - bomb_y;
    float dist_from_bomb = sqrtf((dx*dx) + (dy*dy));

    if(dist_from_bomb <= BOMB_BLAST_RADIUS && dist_from_bomb > 0){
      float dxn_vec_x = dx / dist_from_bomb;
      float dxn_vec_y = dy / dist_from_bomb;

      float fall_off = 1.0f - (dist_from_bomb / BOMB_BLAST_RADIUS);
      float force = BOMB_FORCE * fall_off;

      gs->players[i].velocity_x += dxn_vec_x * (force / PLAYER_MASS);
      gs->players[i].velocity_y += dxn_vec_y * (force / PLAYER_MASS);

      uint8_t explosion_damage = (uint8_t)(BOMB_DAMAGE * fall_off);
      if(explosion_damage >= gs->players[i].health){
        gs->players[i].health = 0;
        game_player_leave(gs, i);
      } else {
        gs->players[i].health -= explosion_damage;
      }
    }
  }
  
  uint8_t owner = gs->bombs[bomb_index].owner_id;
  if(gs->players[owner].is_alive && gs->players[owner].bomb_held < MAX_BOMBS_HELD && owner < MAX_PLAYERS){
    gs->players[owner].bomb_held++;
  }

  gs->bombs[bomb_index].is_alive = false;
}

void game_tick(GameState *gs) {
  for(int i = 0; i < MAX_PLAYERS; i++){
    if(gs->players[i].is_alive){
      float estimated_x = gs->players[i].x + gs->players[i].velocity_x;
      float estimated_y = gs->players[i].y + gs->players[i].velocity_y;

      int tile_x = (int)estimated_x;
      int tile_y = (int)estimated_y;

      if(gs->map[tile_y][tile_x] == 0){
        gs->players[i].x = estimated_x;
        gs->players[i].y = estimated_y;
      }

      gs->players[i].velocity_x = 0;
      gs->players[i].velocity_y = 0;
    }
  }

  for(int i = 0; i < MAX_BOMBS; i++){
    if(gs->bombs[i].is_alive){
      gs->bombs[i].time_to_detonate -= 1;
      if(gs->bombs[i].time_to_detonate == 0){
        game_explode_bomb(gs, i);       
      }
    }
  }

 for(int i = 0; i < MAX_CRATES; i++){
   if(!gs->crates[i].is_alive) continue;

   float estimated_x = gs->crates[i].x + gs->crates[i].velocity_x;
   float estimated_y = gs->crates[i].y + gs->crates[i].velocity_y;

   int tile_x = (int)estimated_x;
   int tile_y = (int)estimated_y;

   if(gs->map[tile_y][tile_x] == 0){
      gs->crates[i].x = estimated_x;
      gs->crates[i].y = estimated_y;
   }

   gs->crates[i].velocity_x *= 0.84f;
   gs->crates[i].velocity_y *= 0.84f;
 }

  gs->tick++;
}
