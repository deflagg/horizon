#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/server.h"
#include "../include/client.h"

int main(int argc, char **argv) 
{
  if(argc > 1 && strcmp(argv[1], "client") == 0)
  {
    client("127.0.0.1");
  }
  else
  {
    server();
  }

  return 0;
}