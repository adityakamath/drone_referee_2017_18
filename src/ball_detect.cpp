#include <vector>
#include <iostream>
#include <ctime>

#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 

#include<arpa/inet.h>
#include<sys/socket.h>
#include <errno.h>


using namespace std;


#define UDP_BUFLEN 512

//can be changed to whatever is required
struct STRUCT_ball_position {
    double x;
    double y;
    double z;
};


int main(int argc, char* argv[])
{

    // -- UDP_CONNECTION --
    //Structure for address of server
    struct sockaddr_in myaddr;
    myaddr.sin_addr.s_addr=0;//htonl(INADDR_ANY); // ip addres INADDR_ANY = local host
    myaddr.sin_port=htons(51234); //port number
    myaddr.sin_family = AF_INET; //http://stackoverflow.com/questions/337422/how-to-udp-broadcast-with-c-in-linux

    // Create Socket
    int sockfd; // file descriptor
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return 0;
    }


    // -- VARIABLE TO STORE RESULTS --
    STRUCT_ball_position ball_position; // variable to safe position of drone/camera

     // -- INFINIT LOOP --
    cout << "Start tx " << endl;
    clock_t timer_begin = clock(); // timer to measure execution time
    double timer_elapsed_time;
    while (1)
    {
        // Show sample time
        //if(DEBUG) {
            timer_elapsed_time = double(clock() - timer_begin) / CLOCKS_PER_SEC;
            //cout << "Tsample: "<< timer_elapsed_time << endl;
            timer_begin = clock();
        //}

     
         ball_position.x = 1;
         ball_position.y = 2;
         ball_position.z = 3;

	 // Send to UDP port
         char buf[UDP_BUFLEN]; // Create clear buffer
         memcpy(buf, &ball_position, sizeof(ball_position)); // copy drone_position to buffer
         int bytes_sent = sendto(sockfd, buf, sizeof(ball_position), MSG_DONTWAIT, (struct sockaddr *)&myaddr, sizeof(myaddr)); // send bytes to UDP port
         if(bytes_sent == -1) { printf("Error sending msg: %s\n", strerror(errno)); } // error if bytes not sent

         // Print results on terminal
         cout <<"Bytes sent: "<< bytes_sent << " @ "<< 1/timer_elapsed_time <<" Hz"<< " x"<< ball_position.x << " y"<< ball_position.y << " z"<< ball_position.z << endl;

                
            
        
	 //WAITKEY IS AN OPENCV FUNCTION, SO UNCOMMENT IT WHEN YOU ADD THE BALL DETECTION CODE
         // Wait some time to give operating system time to do other stuff
	 //  waitKey(5);

    } // end of while loop

    // Close program
    return 0;
}
