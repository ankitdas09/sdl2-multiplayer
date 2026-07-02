#include "net.h"
#include <stdio.h>

int main() {
  int sock = net_init();
  if(sock < 0){
    fprintf(stderr, "Failed to init network\n");
    return 1;
  }

  net_run(sock);

  return 0;
}
