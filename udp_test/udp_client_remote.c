#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int udp_port = 1500;

void main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: %s <remote IP>\n", argv[0]);
		exit(0);
	}
	
	
	char *remote_ip = argv[1];
	int sockfd;
	struct collisionDetect{
		double agent1;
		double agent2;
		double aggressor;
	} collision;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	socklen_t addr_size;

	sockfd=socket(PF_INET, SOCK_DGRAM, 0);
	memset(&serverAddr, '\0', sizeof(serverAddr));

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(udp_port);
	serverAddr.sin_addr.s_addr=inet_addr(remote_ip);

	while(1){
		collision.agent1 = 1;
		collision.agent2 = 2;
		collision.aggressor = 1;
		memcpy(buffer, &collision, sizeof(collision));
		sendto(sockfd, buffer, sizeof(collision), MSG_DONTWAIT, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		printf("[+]Collision between: [%lf],[%lf] | Caused by: [%lf] \n", collision.agent1, collision.agent2, collision.aggressor);
	}
}
