#include <stdint.h>
#include <stdbool.h>

#ifndef GAME_PROTOCOL
#define GAME_PROTOCOL
#define PACKET_INPUT    0x01
#define PACKET_SNAPSHOT 0x02
#define PACKET_WELCOME  0x03
#define BTN_PLACE_BOMB  (1 << 0)
#define BTN_KICK_CRATE  (1 << 1)
#define BTN_TAUNT       (1 << 2)

typedef struct __attribute__((packed)) {
  uint8_t type; 
  uint32_t tick;
  int8_t move_x;
  int8_t move_y;
  uint8_t buttons;
} ClientInput;

typedef struct __attribute__((packed)) {
  float x, y;
  bool is_alive;
  uint8_t health;
} ServerPlayer;

typedef struct __attribute__((packed)) {
  float x, y;
  uint16_t time_to_detonate;
  bool is_alive;
} ServerBomb;

typedef struct __attribute__((packed)) {
  float x, y;
  bool is_alive;
} ServerCrate;

typedef struct __attribute__((packed)) {
  uint8_t type;
  uint32_t current_server_tick;
  uint8_t player_id;
  float    x, y;
  uint8_t  health;
} ServerWelcomePacket;

#include "game.h"

typedef struct __attribute__((packed)) {
  uint8_t type;
  uint32_t tick;
  ServerPlayer players[MAX_PLAYERS];
  ServerBomb bombs[MAX_BOMBS];
  ServerCrate crates[MAX_CRATES];
} ServerSnapshot;

#endif 
