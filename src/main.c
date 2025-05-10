#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/node.h"

int main(int argc, char **argv) 
{
  int profile = (argc > 1) ? atoi(argv[1]) : 1;
  Node node = {0};
  node.listen_port = (profile == 2) ? "27016" : "27015";
  node.peer_port   = (profile == 2) ? "27015" : "27016";
  for (int i = 0; i < MAX_CONNECTIONS; ++i) node.conns[i] = -1;

  node.listen_sock = create_listener(node.listen_port);
  if (node.listen_sock == -1) return 1;
  int peerfd = connect_peer("127.0.0.1", node.peer_port);
  if (peerfd != -1) add_conn(&node, peerfd);

  printf("Node on %s ready\n", node.listen_port);
  event_loop(&node);
  return 0;

}