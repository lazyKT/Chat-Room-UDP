#ifndef netutils_h
#define netutils_h

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct client_struct
{
  char name[20];
  struct sockaddr_in sock_addr;
  struct client_struct *next;
} client_t;

typedef struct packet_struct /* data packet structure */
{
  char sender[20];
  char msg[256];
  struct sockaddr_in sock_addr;
} packet_t;

int is_connected (struct sockaddr_in*, client_t*); /* check whether the client is already connected */
int is_equal (struct sockaddr_in*, struct sockaddr_in*); /* compare two clients */
int assign_client (struct sockaddr_in*, char*, client_t**, client_t**); /* add new client to client list */
char* get_username (struct sockaddr_in*, client_t*);

packet_t* construct_packet (struct sockaddr_in*, char*, char*);
int get_msg_len (char* uname, char* msg);
void send_to_all (int, struct sockaddr_in*, char*, char*, client_t*);

void do_server_command (char*);
void print_clients (client_t**, char*); /* print all clients */
int get_server_command (int);

#endif
