#include "../include/initRoutingP.h"
#include "../include/utilityFun.h"
#include "../include/userHost.h"
#include "../include/debugging.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<sys/types.h>
#include <sys/unistd.h>

int initUM(unsigned int portNumber, char ipAddress[16], userModel * um) {
    if((um->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        return -5;
    }
    struct timeval tv; //Ovo se koristi kao time_out posto sam stavio da sendData i recvData cekaju maskimalno 10 ms da prime paket, inace idu dalje
    tv.tv_sec = 0;
    tv.tv_usec = 10000; // 10ms
    if(setsockopt(um->socket, SOL_SOCKET, SO_SNDTIMEO, (void*)&tv, (socklen_t)sizeof(tv)) == -1) {//Podesavanje timeout na 10ms za sendto
        perror("Transfer package socket");
        return -6;
    }



    memset(&(um->userHost), 0, sizeof(struct sockaddr_in)); //ovako je radjeno u klijentu sa primera, kontam da je ovo inicijalizacija
    um->userHost.sin_family = AF_INET;
    um->userHost.sin_port = htons(portNumber);
    memset(&(um->homeHost), 0, sizeof(struct sockaddr_in));

    if(bind(um->socket, (struct sockaddr *)&(um->userHost), (socklen_t)sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }

    if(inet_aton(ipAddress, &(um->userHost.sin_addr)) == 0) {
        return -7;
    }
	return 0;
}

int initRM(unsigned int portNumber, char ipAddress[16], RPAddress rpAddress, routerModel * rm, char emptyInitialisation) {

    memset(&(rm->homeHost), 0, sizeof(struct sockaddr_in));
    rm->homeHost.sin_family = AF_INET;
    rm->homeHost.sin_port = htons(portNumber);
    rm->homeHost.sin_addr.s_addr = inet_addr(ipAddress);

    if(emptyInitialisation) return 0;

    strcpy(rm->routerAddress, rpAddress);
    ////printf("%s\n", );
    memset(rm->userHosts, 0, sizeof(struct sockaddr_in)*MAXUSERS);
    memset(rm->routerHosts, 0, sizeof(struct sockaddr_in)*MAXROUTERS);

    for(int i = 0; i < MAXUSERS; i++) {
        for(int j = 0; j < MAXUSERS; j++) {
            rm->users[i] = 0; //Nemamo nijednog host-a pa sve stavljamo na 0
            rm->routerTable[i][j] = 0;
        }
    }

    pthread_mutex_init(&(rm->routerTableMutex), NULL);

    if((rm->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket setup: ");
        return -8;
	}
    if(bind(rm->socket, (struct sockaddr *)&(rm->homeHost), (socklen_t)sizeof(struct sockaddr_in)) == -1) {
        perror("Bind error: ");
        return -5;
    }
}

int connectRMToNetwork(routerModel * sourceRouter, routerModel * destRouter) {
    transferPackage tp;
    tp.packageType = 2;
    size_t len = sizeof(struct sockaddr_in);
    strcpy(tp.sourceAddress, sourceRouter->routerAddress);
    //printTPPackage(&tp);
    convertTPackageToArray(&tp, sourceRouter->sendTPBuffer);
    //printf("Cekamo slanje\n");
    if(sendto(sourceRouter->socket, sourceRouter->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(destRouter->homeHost), len) == -1) {
        return -1;
    }
    //printf("Poslato. Cekamo primljanje\n");
    if(recvfrom(sourceRouter->socket, sourceRouter->receiveBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(destRouter->homeHost), (socklen_t*)&len) == -1) {
        return -3;
    }
    //printf("Primljeno\n");

    convertArrayToTPackage(sourceRouter->receiveBuffer, &tp);
    if(!strcmp(tp.sourceAddress,  "000.000")) {
        //printf("The address %s is already taken", sourceRouter->routerAddress);
        return -8;
    }
    unsigned char thisRouter = getRouterNumber(sourceRouter->routerAddress);
    unsigned char otherRouter = getRouterNumber(tp.sourceAddress); //nas strani ruter nam salje njegovu povratnu adresu

    //printf("This router value %d, otherRouter value %d\n", thisRouter, otherRouter);
    sourceRouter->routerTable[0][thisRouter] = REFRESHVALUE; //za osvezavanje putanje rutiranja
    sourceRouter->routerTable[0][otherRouter] = REFRESHVALUE;
    sourceRouter->routerTable[thisRouter][++sourceRouter->routerTable[thisRouter][0]] = otherRouter;

    sourceRouter->routerHosts[otherRouter].sin_addr.s_addr = destRouter->homeHost.sin_addr.s_addr;
    sourceRouter->routerHosts[otherRouter].sin_family = AF_INET;
    sourceRouter->routerHosts[otherRouter].sin_port = destRouter->homeHost.sin_port;
    ////printRouterTable(sourceRouter);
    return 0;
}

int connectUMToNetwork(userModel * user, routerModel * destRouter) {
    transferPackage tp;
    tp.packageType = 1;

    strcpy(tp.sourceAddress, user->userAddress);
    user->homeHost.sin_addr.s_addr = destRouter->homeHost.sin_addr.s_addr;
    user->homeHost.sin_family = destRouter->homeHost.sin_family;
    user->homeHost.sin_port = destRouter->homeHost.sin_port;

   // //printTPPackage(&tp);
    convertTPackageToArray(&tp, user->sendTPBuffer);
    if(sendto(user->socket, user->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)& (user->homeHost), sizeof(struct sockaddr_in)) == -1) {
        perror("Package not sent:");
        return -1;
    }
    ////printf("Package sent\n");
    receiveTPfromRouter(&tp, user);
    strcpy(user->userAddress, tp.sourceAddress); // ruter je spakovao u sourceAddress adresu koju treba da sadrzi korisnik
    ////printf("Package received\n");
    ////printTPPackage(&tp);
    //printUserModel(user);
    return 0;
}

int closeUM(userModel * um) {
    close(um->socket);
}

int closeRM(routerModel * rm) {
    close(rm->socket);
    pthread_mutex_destroy(&rm->routerTableMutex);
}
