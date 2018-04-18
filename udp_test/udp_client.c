#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

void main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		exit(0);
	}
	
	int port = atoi(argv[1]);
	int sockfd;
	struct hedgePacket{
		double x;
		double y;
		double z;
	} drone_position;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	socklen_t addr_size;

	sockfd=socket(PF_INET, SOCK_DGRAM, 0);
	memset(&serverAddr, '\0', sizeof(serverAddr));

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port);
	serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	while(1){
		drone_position.x = 1;
		drone_position.y = 2;
		drone_position.z = 3;
		memcpy(buffer, &drone_position, sizeof(drone_position));
		sendto(sockfd, buffer, sizeof(drone_position), MSG_DONTWAIT, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		printf("[+]Data sent: x:%lf, y:%lf, z:%lf \n", drone_position.x, drone_position.y, drone_position.z);
	}
}
