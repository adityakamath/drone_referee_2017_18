#include "udp.h"

void diep(char *s)
{
        perror(s);
        //exit(1);
}





int udp_client_open(char* srv_ip, int port, pudpData pdata)
{
        pdata->slen=sizeof(pdata->si_other);
        pdata->port=port;
    
        if ((pdata->s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
                diep("socket");
                return -1;
        }
    
        pdata->set=1;
        setsockopt(pdata->s, SOL_SOCKET, MSG_NOSIGNAL, (void*) &(pdata->set), sizeof(int));

//      for sending
        memset((char *) &(pdata->si_other), 0, pdata->slen);
        pdata->si_other.sin_family = AF_INET;
        pdata->si_other.sin_port = htons(pdata->port);

        if (inet_aton(srv_ip, &(pdata->si_other.sin_addr))==0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
        }

        return 0;
}





int udp_client_send(void* pd, int size, pudpData pdata)
{
        pdata->slen=sizeof(pdata->si_other);
        if (sendto(pdata->s, pd, size, 0, (struct sockaddr*) &(pdata->si_other), pdata->slen)==-1) {
               diep("sendto()");
               return -1;
        }

        return 0;
}





int udp_client_receive(void* pd, int size, pudpData pdata)
{
        if ((pdata->nread=(int) recvfrom(pdata->s, pd, size, MSG_DONTWAIT, (struct sockaddr*) &(pdata->si_other), (socklen_t*) &(pdata->slen)))==-1) {
               //diep("recvfrom()");
               return -1;
        }

        return 0;
}





int udp_client_close(pudpData pdata)
{
        close(pdata->s);

        return 0;
}

