#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif // WIN32
#include "marvelmind.h"
#include <time.h>
#include <math.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

bool terminateProgram=false;
int udp_port=3500;

#ifdef WIN32
BOOL CtrlHandler( DWORD fdwCtrlType )
{
    if ((fdwCtrlType==CTRL_C_EVENT ||
            fdwCtrlType==CTRL_BREAK_EVENT ||
            fdwCtrlType==CTRL_CLOSE_EVENT ||
            fdwCtrlType==CTRL_LOGOFF_EVENT ||
            fdwCtrlType==CTRL_SHUTDOWN_EVENT) &&
            (terminateProgram==false))
    {
        terminateProgram=true;
        return true;
    }
    else return false;
}
#else
void CtrlHandler(int signum)
{
    terminateProgram=true;
}
#endif

#ifdef WIN32
void sleep(unsigned int seconds)
{
    Sleep (seconds*1000);
}
#endif

int main (int argc, char *argv[])
{
    // get port name from command line arguments (if specified)
    const char * ttyFileName;
    if (argc==2) ttyFileName=argv[1];
    else ttyFileName=DEFAULT_TTY_FILENAME;

    //UDP parameters
	//int port = atoi(udp_port);
	int sockfd;
	struct hedgePacket{
		double x17;
		double y17;
		double z17;
		double x18;
		double y18;
		double z18;
		//double yaw_normal;
	} drone_position;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	socklen_t addr_size;
	sockfd=socket(PF_INET, SOCK_DGRAM, 0);
	memset(&serverAddr, '\0', sizeof(serverAddr));

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(udp_port);
	serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");

    // Init
    struct MarvelmindHedge * hedge=createMarvelmindHedge ();

    if (hedge==NULL)
    {
        puts ("Error: Unable to create MarvelmindHedge");
        return -1;
    }
    hedge->ttyFileName=ttyFileName;
    hedge->verbose=true; // show errors and warnings
    startMarvelmindHedge (hedge);

    // Set Ctrl-C handler
#ifdef WIN32
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE );
#else
    signal (SIGINT, CtrlHandler);
    signal (SIGQUIT, CtrlHandler);
#endif
    clock_t t1, t2;
    float diff;
    double curtime;
    FILE *fp17, *fp18;
    fp17=fopen("Ultrasound_data_beacon17.txt", "w");
    fp18=fopen("Ultrasound_data_beacon18.txt", "w");
    // Main loop
    while ((!terminateProgram) && (!hedge->terminationRequired))
    {
        t1 = clock();
        //sleep (3); //this
        //printPositionFromMarvelmindHedge (hedge, true);//this

        struct PositionValue position17, position18;
        getPositionFromMarvelmindHedgeByAddress(hedge, &position17, 17);
        getPositionFromMarvelmindHedgeByAddress(hedge, &position18, 18);
        t2 = clock();
        diff = ((float) t2 - t1)/CLOCKS_PER_SEC * 1000;
        //printf("Operation took %.3f milliseconds. Now sleeping for %.3d milliseconds.\n", diff, 200-(diff)); //this
        //sleep(1); //this
        //Sleep(200-diff); //this
    	struct timeval t;
    	gettimeofday(&t,NULL);
        curtime = (double)t.tv_sec+t.tv_usec/1000000.0;
		if (fabs(((double) position17.y)/1000.0)>0.000001)
		{
         	//printf ("Address: %d, X: %.3f, Y: %.3f, Z: %.3f at time T: %u\n", position17.address, ((double) position17.x)/1000.0,
                	//((double) position17.y)/1000.0, ((double) position17.z)/1000.0, position17.timestamp);
		fprintf(fp17, "%.4f \t %.4f \t %.4f \t %.4f \n", ((double) position17.x)/1000.0, ((double) position17.y)/1000.0, ((double) position17.z)/1000.0, curtime);
		printf("Written to files\n");
		drone_position.x17 = ((double) position17.x)/1000.0;
		drone_position.y17 = ((double) position17.y)/1000.0;
		drone_position.z17 = ((double) position17.z)/1000.0;
		memcpy(buffer, &drone_position, sizeof(drone_position));
		sendto(sockfd, buffer, sizeof(drone_position), MSG_DONTWAIT, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		printf("[%d]Data sent: x:%lf, y:%lf, z:%lf \n", position17.address, drone_position.x17, drone_position.y17, drone_position.z17);
		//printf("[+]Data sent: x:%lf, y:%lf, z:%lf \n", drone_position.x18, drone_position.y18, drone_position.z18);
		}
		if (fabs(((double) position18.y)/1000.0)>0.000001)
		{
        	printf ("Address: %d, X: %.3f, Y: %.3f, Z: %.3f at time T: %u\n", position18.address, ((double) position18.x)/1000.0,
         	        ((double) position18.y)/1000.0, ((double) position18.z)/1000.0, position18.timestamp);
		fprintf(fp18, "%.4f \t %.4f \t %.4f \t %.4f \n", ((double) position18.x)/1000.0, ((double) position18.y)/1000.0, ((double) position18.z)/1000.0, curtime);
		printf("Written to files\n");
		//UDP BROADCAST		
		
		drone_position.x18 = ((double) position18.x)/1000.0;
		drone_position.y18 = ((double) position18.y)/1000.0;
		drone_position.z18 = ((double) position18.z)/1000.0;
		memcpy(buffer, &drone_position, sizeof(drone_position));
		sendto(sockfd, buffer, sizeof(drone_position), MSG_DONTWAIT, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		//printf("[+]Data sent: x:%lf, y:%lf, z:%lf \n", drone_position.x17, drone_position.y17, drone_position.z17);
		printf("[%d]Data sent: x:%lf, y:%lf, z:%lf \n", position18.address, drone_position.x18, drone_position.y18, drone_position.z18);
		}
		
		//drone_position.yaw_normal = atan2((drone_position.y17 - drone_position.y18),(drone_position.x17 - drone_position.x18));
		//printf("[+] YAW = %lf \n", drone_position.yaw_normal);		

	
	
        //getPositionFromMarvelmindHedgeCustom(hedge, true, &xm17, &ym17, &zm17);
        //sleep(1.0);
        //printf("X17: %.3f\n", xm17);
        //sleep(0.1);
        //printStationaryBeaconsPositionsFromMarvelmindHedge (hedge, true);
    }

    // Exit
    fclose(fp17);
    fclose(fp18);
    stopMarvelmindHedge (hedge);
    destroyMarvelmindHedge (hedge);
    return 0;
}
