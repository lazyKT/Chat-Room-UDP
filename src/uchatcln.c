#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>


#define BUFF_SIZE 512
#define PORT 12345
#define STDIN 0


void assign_args (int argc, char** argv, char* uname, char* ip); /* take credentials from cmd args */
int get_idx (int, char**, char*); /* get idx of the element from the array */
int handshake (int, struct sockaddr_in, char*); /* perform handshake and establish connection with server */
int send_msg (int in, int out, struct sockaddr_in);
int recv_msg (int fd, struct sockaddr_in);

int main (int argc, char* argv[])
{
  int sock_fd, s, r;
  char uname[20], ip[16];
  struct sockaddr_in server, client;
  struct hostent *h;
  struct in_addr server_addr;
  fd_set test_set, ready_set;

  assign_args (argc, argv, uname, ip); /* assign credentials from cmd args */
  printf("usrname: %s, ip: %s\n", uname, ip);
  inet_aton(ip, &server_addr); /* convert string to IP Addr */
  h = gethostbyaddr(&server_addr, sizeof(server_addr), AF_INET);
  if (h == NULL)
  {
    perror("unknown host");
    exit(EXIT_FAILURE);
  }

  if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) /* UDP Socket */
  {
    perror("Creating Socket");
    exit(EXIT_FAILURE);
  }
  
  memset((char*)&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  memcpy((char *)&server.sin_addr, h->h_addr, h->h_length);

  if (handshake(sock_fd, server, uname) != 0) /* initial handshake */
  {
    perror("Initial-handshake");
    exit(EXIT_FAILURE);
  }

  FD_ZERO(&test_set);
  FD_SET(sock_fd, &test_set);
  FD_SET(STDIN, &test_set);


  while(1)
  {
    /* copy the contents of test_set into ready_set */  
    memcpy(&ready_set, &test_set, sizeof(test_set)); 

    if (select(FD_SETSIZE, &ready_set, NULL, NULL, NULL) < 0)
    {
      perror("select");
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(sock_fd, &ready_set))
    {
      /* receive message */
      if (recv_msg(sock_fd, server) == -1)
         break;
    }

    if (FD_ISSET(STDIN, &ready_set))
    {

      /* send message */
      if (send_msg(STDIN, sock_fd, server) == -1)
        break;
    }
  }

  return 0;
}


/* search element in array */
int get_idx (int argc, char** argv, char* val)
{
  int i;

  for (i = 0; i < argc; i++)
  {
    if (strcmp(val, argv[i]) == 0)
      return i;
  }

  return -1;
}


/* take credentials from cmd args and assign */
void assign_args (int argc, char** argv, char* uname, char* ip)
{
  int i;
  if (argc < 4)
  {
    fprintf(stderr, "Usage; %s <ipv4-address> -u username\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  i = get_idx (argc, argv, "-u");
  if (i < 0 || i > 2)
  {
    fprintf(stderr, "Usage: %s <ipv4-address> -u username\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  strcpy (uname, argv[i+1]);
  if (i == 1) strcpy (ip, argv[i+2]);
  else  strcpy (ip, argv[i-1]);
}


/* initial handshake between server and client */
int handshake (int fd, struct sockaddr_in server, char* uname)
{
  int r, s, s_len = sizeof(server);
  
  if ((s = sendto(fd, uname, strlen(uname) + 1, 0, 
      (struct sockaddr*)&server, s_len)) < 0)
  {
    perror("sendto");
  }

  if (s == 0) /* server is not running or unreachable */
  {
    perror("Connection refused: Server is unreachable or not running.\n");
    exit(EXIT_FAILURE);
  }

  char msg[BUFF_SIZE];
  memset(msg, 0, BUFF_SIZE);
  if ((r = recvfrom(fd, msg, BUFF_SIZE, 0, 
      (struct sockaddr*)&server, &s_len)) < 0)
  {
    perror("recvfrom");
  } 
  if (r < BUFF_SIZE) msg[r] = '\0';
  printf("%s", msg);

  return strcmp(msg, "Connected\n");
}


/* send message */
int send_msg (int in, int out, struct sockaddr_in server)
{
  char buffer[BUFF_SIZE];
  memset(buffer, 0, BUFF_SIZE);
  int cnt, s, s_len = sizeof(server);
  
  if ((cnt = read(in, buffer, BUFF_SIZE)) < 0)
  {
    perror("stdin");
    return -1;
  }
  
  if (strcmp(buffer, "./exit\n") == 0) 
  {
    /* disconnect from the server and exit */
    sendto (out, buffer, cnt, 0, (struct sockaddr*)&server, s_len);
    return -1;
  }

  if ((s = sendto (out, buffer, cnt, 0, (struct sockaddr*)&server, s_len)) < 0)
    perror ("sendto");
  
  
  return 0;
}


/* receive message */
int recv_msg (int fd, struct sockaddr_in server)
{
  char buffer[BUFF_SIZE];
  memset(buffer, 0, BUFF_SIZE);
  int cnt, s_len = sizeof(server);
  
  if ((cnt = recvfrom(fd, buffer, BUFF_SIZE, 0,
      (struct sockaddr*)&server, &s_len)) < 0)
  {
    perror("recvfrom");
    return -1;
  }
  printf("%s", buffer); 
  return 0;
}

