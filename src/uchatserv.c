#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "netutils.h"

#define BUFF_SIZE 512
#define PORT 12345
#define STDIN 0

typedef struct client_struct client_t;
typedef struct packet_struct packet_t;

client_t *head = NULL;
client_t *tail = NULL;
int num_clients = 0;

/* initial handshake for clients */
void process_packets (int, struct sockaddr_in);

int main (int argc, char** argv)
{
  char username[BUFF_SIZE];
  int sock_fd, client_fd, client_len;
  struct sockaddr_in server, client;
  fd_set test_set, ready_set;

  memset((char*)&server, 0, sizeof(server));
  memset((char*)&client, 0, sizeof(client));

  printf("Creating UDP Socket..\n");
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0)
  {
    perror("Creating UDP Socket");
    exit(EXIT_FAILURE);
  }
  printf("UDP Socket Created!\n");
  
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock_fd, (struct sockaddr*)&server, sizeof(server)) < 0)
  {
    perror("Binding Socket");
    exit(EXIT_FAILURE);
  }

  /* initialise FD_SETs */
  FD_ZERO(&test_set);
  FD_SET(sock_fd, &test_set);
  FD_SET(STDIN, &test_set);

  printf("%s listening on port:%d\n", argv[0], PORT);

  while(1)
  {
    /* copy test_set into ready_set */
    memcpy(&ready_set, &test_set, sizeof(test_set));
    if (select(sock_fd+1, &ready_set, NULL, NULL, NULL) < 0)
    {
      perror("select");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET (sock_fd, &ready_set))
    {
      /* receive messages from clients */
      process_packets(sock_fd, client);
    }

    if (FD_ISSET (STDIN, &ready_set))
    {
      /* server logging */
      if (get_server_command (STDIN, head) == -1)
        break;
    }
  }

  return 0;
}

/* process received messages from clients */
void process_packets (int fd, struct sockaddr_in client)
{
  char message[BUFF_SIZE];
  memset(message, 0, BUFF_SIZE);
  int c_len = sizeof(client);
  int r, s;

  if ((r = recvfrom(fd, message, BUFF_SIZE, 0,
      (struct sockaddr*)&client, &c_len)) < 0)
  {
    perror("recvfrom");
    exit(EXIT_FAILURE);
  }

  if (r < BUFF_SIZE) message[r-1] = '\0';
  
  int cc = -1;
  if (strcmp (message, "./exit") != 0)
    cc = assign_client (&client, message, &head, &tail); /* assign client to client list */ 

  if (cc > 0)
  {
    printf("%s connected\n", message);
    
    char server_hello[10] = "Connected\n";
    if ((s = sendto(fd, server_hello, 10, 0, 
        (struct sockaddr*)&client, c_len)) < 0)
    {
      perror("sendto");
    }
    num_clients ++; /* increment num of connected clients */
  }
  else
  {
    int s;
    char username[20];
    memset(username, '\0', 20);
    strcpy(username, get_username (&client, head));

    if (strcmp(username, "") == 0)
      strcpy(username, "Unkown User");

    if (strcmp(message, "./exit") == 0)
    {
      disconnect_client (&client, &head, &tail);
      strcpy (message, "disconnected");
    }

    if (strcmp (message, "./sh cli") == 0) /* client requests for connected list */
    {
      sh_cli_to_client (fd, &client, head);
      return;
    }

    send_to_all (fd, &client, username, message, head);
  }
}


