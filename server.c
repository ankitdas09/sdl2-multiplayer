#include "net.h"
#include <stdio.h>
#include "game.h"

int main() {
  int sock = net_init();
  if(sock < 0){
    fprintf(stderr, "Failed to init network\n");
    return 1;
  }

  GameState gs;
  game_init(&gs);

  net_run(sock, &gs);

  return 0;
}
