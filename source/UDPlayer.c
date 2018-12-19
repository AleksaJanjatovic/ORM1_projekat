#include "../include/UDPlayer.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<sys/types.h>

int UDPinitTP(unsigned int portNumber, char ipAddress[16], transferPackage * tp) {

    if((tp->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {

        return -5;
	}
    memset((char *)&(tp->sourceHost), 0, sizeof(struct sockaddr_in)); //ovako je radjeno u klijentu sa primera, kontam da je ovo inicijalizacija
	tp->sourceHost.sin_family = AF_INET;
    tp->sourceHost.sin_port = htons(portNumber);

    struct timeval tv; //Ovo se koristi kao time_out posto sam stavio da sendData i recvData cekaju maskimalno 10 ms da prime paket, inace idu dalje
    tv.tv_sec = 0;
    tv.tv_usec = 10000; // 10ms
    if(setsockopt(tp->socket, IPPROTO_UDP, SO_SNDTIMEO, &tv, sizeof(SO_SNDTIMEO)) == -1) {//Podesavanje timeout na 10ms za sendto
        perror("zaba");
        return -6;
    }
    if(inet_aton(ipAddress, &(tp->sourceHost.sin_addr)) == 0) {
        return -7;
	}
	return 0;
}


int UDPinitRP(unsigned int portNumber, char ipAddress[16], routerPackage * rp) {
    if((rp->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        return -8;
	}
    memset((char *)&(rp->sourceHost), 0, sizeof(struct sockaddr_in)); //ovako je radjeno u klijentu sa primera, kontam da je ovo inicijalizacija na 0
	rp->sourceHost.sin_family = AF_INET;
    rp->sourceHost.sin_port = htons(portNumber);

    struct timeval tv; //Ovo se koristi kao time_out posto sam stavio da transferTPPackage ceka maskimalno 10 ms da primi paket, inace ide return
    tv.tv_sec = 0;
    tv.tv_usec = 10000; // 10ms
    if(setsockopt(rp->socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(SO_SNDTIMEO)) == -1) {//Podesavanje timeout na 10ms za sendto
        return -6;
    }

    if(inet_aton(ipAddress, &(rp->sourceHost.sin_addr)) == 0) {
        return -9;
	}
	return 0;
}

int UDPcloseTP(transferPackage * tp) {
	close(tp->socket);
}

int UDPcloseRP(routerPackage * rp) {
	close(rp->socket);
}
