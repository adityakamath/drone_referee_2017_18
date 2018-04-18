#ifndef MULTICAST_H_
#define MULTICAST_H_

#include <sys/socket.h>
#include <arpa/inet.h>

#include "network_config.h"

#define TTL				64

#define RECEIVE_OUR_DATA 0

typedef struct multiSocket_tag{
	struct sockaddr_in  destAddress;
	int					socketID;
}multiSocket_t;

//	*************************
//  Open Socket
//
//	Input:
//		multiSocket_t* multiSocket = socket descriptor
//		network_config_t* nw_config = network configuration
//	Output:
//		errorno
//
int openSocket(multiSocket_t* multiSocket, network_config_t* nw_config);



//	*************************
//  Close Socket
//
//  Input:
//		multiSocket_t* multiSocket = socket descriptor
//
void closeSocket(multiSocket_t* multiSocket);



//	*************************
//  Send Data
//
//  Input:
//		multiSocket_t* multiSocket = socket descriptor
//		void* data = pointer to buffer with data
//		int dataSize = number of data bytes in buffer
//
int sendData(multiSocket_t* multiSocket, void* data, int dataSize);



//	*************************
//  Receive Data
//
//  Input:
//		multiSocket_t* multiSocket = socket descriptor
//		void* buffer = pointer to buffer
//		int bufferSize = total size of buffer
//
int receiveData(multiSocket_t* multiSocket, void* buffer, int bufferSize);

#endif /* MULTICAST_H_ */
