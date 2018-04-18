#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef struct tag_udpData
{
        struct sockaddr_in si_other;
        int slen;
        struct sockaddr_in si_me;
        int s;
        int nread;
        int set;
        int port;
} udpData, *pudpData;

int udp_server_open(int, pudpData);
int udp_server_receive(void*, int, pudpData);
int udp_server_receive_nb(void*, int, pudpData);
int udp_server_receive_heartbeat(pudpData);
int udp_server_send(void*, int, pudpData);
int udp_server_send2address(void*, int, int, pudpData);
int udp_server_send_all(void*, int, pudpData);
int udp_server_close(pudpData);

int udp_client_open(char*, int, pudpData);
int udp_client_send(void*, int, pudpData);
int udp_client_receive(void*, int, pudpData);
int udp_client_close(pudpData);

