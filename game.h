#include <stdbool.h>
#include <stdint.h>

#ifndef GAME_DEFINITION

#define GAME_DEFINITION
#define MAX_PLAYERS 4
#define MAX_CRATES 50
#define MAX_BOMBS 4
#define MAP_H 50
#define MAP_W 50
#define MAX_PLAYER_HEALTH 100
#define CRATE_MASS_KG 5
#define MAX_BOMBS_HELD 10
#define PLAYER_SPEED 3.0f
#define TICKS_TO_DETONATE 200 
#define BOMB_BLAST_RADIUS 20
#define BOMB_FORCE 120
#define PLAYER_MASS 20
#define BOMB_DAMAGE 20

#define BUTTON_MOVE_UP 10000000
#define BUTTON_MOVE_DOWN 01000000
#define BUTTON_MOVE_LEFT 00100000
#define BUTTON_MOVE_RIGHT 00010000
#define BUTTON_PLACE_BOMB 00001000


typedef struct {
  uint8_t id;
  float x,y;
  float velocity_x, velocity_y;
  uint8_t bomb_held;
  uint8_t health;
  bool is_alive;
} Player;

typedef struct {
  float x, y;
  uint8_t owner_id;
  uint16_t time_to_detonate;
  bool is_alive;
} Bomb; 

typedef struct {
  float x, y;
  float velocity_x, velocity_y;
  bool is_alive;
  uint8_t mass;
} Crate;

typedef struct {
  Player players[MAX_PLAYERS];
  Bomb bombs[MAX_BOMBS];
  Crate crates[MAX_CRATES];
  uint32_t tick;
  uint8_t map[MAP_H][MAP_W];
} GameState;

#include "protocol.h"

void game_init(GameState *gs);

void game_process_input(GameState *gs, uint8_t player_id, ClientInput *input);

void game_tick(GameState *gs);

void game_player_leave(GameState *gs, uint8_t player_id);

void game_player_join(GameState *gs, uint8_t player_id);

#endif
