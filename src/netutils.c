#include "netutils.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


typedef struct client_struct client_t;
typedef struct packet_struct packet_t;

#define BUF_SIZE 256

char data[BUF_SIZE];

/*
client handling
*/

/* check whether the connected client is already in the client_list */
int is_connected (struct sockaddr_in *s, client_t *head)
{
  /*
  if the client is already connected, this function will return 1
  if not, return 0
  */
  client_t *walker = head;
  while (walker != NULL)
  {
    if (is_equal(s, &(walker->sock_addr)) == 1)
      return 1;
    walker = walker->next;
  }

  return 0;
}

/* compare client's sockaddr_in */
int is_equal (struct sockaddr_in *s1, struct sockaddr_in *s2)
{
  /*
  if the two sockets are identical, this function returns 1. Otherwise, 0.
  */
  
  /* get ip address from socket */
  struct in_addr s_addr1 = s1->sin_addr;
  struct in_addr s_addr2 = s2->sin_addr;
  char addr1 [INET_ADDRSTRLEN];
  char addr2 [INET_ADDRSTRLEN];
  
  memset(addr1, 0, INET_ADDRSTRLEN);
  memset(addr2, 0, INET_ADDRSTRLEN);

  inet_ntop (AF_INET, &s_addr1, addr1, INET_ADDRSTRLEN);
  inet_ntop (AF_INET, &s_addr2, addr2, INET_ADDRSTRLEN);
  
  if (strcmp(addr1, addr2) != 0)
    return 0;

  int p1 = htons (s1->sin_port);
  int p2 = htons (s2->sin_port);

  return (p1 == p2);
}

/* add new client to client lists if not exist */
int assign_client (struct sockaddr_in *c_sock, char *buffer, client_t** head, client_t** tail)
{
  /*
  Client List is basically the singly linked list here.
  */
  if (*head == NULL)
  {
    client_t *c = (client_t*) malloc(sizeof(client_t));
    memset (c, 0, sizeof(client_t));
    strcpy (c->name, buffer);
    memcpy (&(c->sock_addr), c_sock, sizeof(*c_sock));
    c->next = NULL;
    *head = c;
    *tail = c;
    return 1;
  }
  else if (is_connected(c_sock, *head) == 0)
  {
    client_t *c = (client_t*) malloc(sizeof(client_t));
    memset (c, 0, sizeof(client_t));
    strcpy (c->name, buffer);
    memcpy (&(c->sock_addr), c_sock, sizeof(*c_sock));
    c->next = NULL;
    (*tail)->next = c;
    *tail = c;
    return 1;
  }
  return 0;
}


/*
data(packet) handling
*/

/* construct data packet */
packet_t* construct_packet (struct sockaddr_in *s, char* uname, char* buffer)
{
  /*
  this function contruct the packet using sender address and message received.
  If the message size is larger than 512 bytes, return NULL. Otherwise, packet_t pointer
  */
  if (sizeof(buffer) > BUF_SIZE)
  {
    perror("System Error: Message too long.\n");
    return NULL;
  }
  packet_t *pk = (packet_t *) malloc(sizeof(packet_t));
  memset (pk, 0, sizeof(packet_t));
  memcpy (&(pk->sock_addr), s, sizeof(*s));
  strcpy (pk->sender, uname);
  strcpy (pk->msg, buffer);

  return pk;
}

/* send to all clients except the sender */
void send_to_all (int sock_fd, struct sockaddr_in *sender_sock, char* uname, char* buffer, client_t *head)
{
  int c_len;
  char data[BUF_SIZE+20];
  packet_t *p = construct_packet(sender_sock, uname, buffer);
  if (p == NULL)
  {
    return; /* message length too long */
  }
  client_t *walker = head;
  while (walker != NULL)
  {
    if (is_equal (&(walker->sock_addr), &(p->sock_addr)) == 0)
    { 
      /* if sock_addr(s) aren't equal, send messge */
      sprintf(data, "%s: %s\n", p->sender, buffer);
      c_len = sizeof(walker->sock_addr);
      if (sendto (sock_fd, data, strlen(data), 0,
          (struct sockaddr*) &(walker->sock_addr), c_len) < 0) 
      {
        perror("send_to_all");
      }
    }
    walker = walker->next;
  }
}


/*
server logging and command sets
*/

/* get username of given socket address */
char* get_username (struct sockaddr_in *c, client_t* head)
{
  client_t *walker = head;
  while (walker != NULL)
  {
    if (is_equal (c, &(walker->sock_addr)) == 1)
      return walker->name;
    walker = walker->next;
  }

  return "";
}

/* get server commands */
int get_server_command (int in_fd)
{
  char buffer[BUF_SIZE];
  memset(buffer, 0, BUF_SIZE);
  int cnt;

  if ((cnt = read(in_fd, buffer, BUF_SIZE)) < 0)
    perror("stdin");
  if (cnt < BUF_SIZE) 
    buffer[cnt] = '\0';
  if (strcmp(buffer,"./exit\n") == 0)
    return -1;
  
  return 0;
}



