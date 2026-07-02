#ifndef SERVER_NET_CODE 
#define SERVER_NET_CODE 

#include <stdint.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_CLIENT 4
#define PORT 7777
#define TIMEOUT_SEC 5
#define TICK_RATE 60
#define TICK_US (1000000 / TICK_RATE)

typedef struct {
  struct sockaddr_in addr;
  int is_active;
  time_t last_seen;
  uint8_t id;
} NetClient;

int net_init(void);
void net_run(int sock);

#endif
