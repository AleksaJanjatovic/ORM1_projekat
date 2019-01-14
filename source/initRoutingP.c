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

    memset(&(um->userHost), 0, sizeof(struct sockaddr_in)); //ovako je radjeno u klijentu sa primera, kontam da je ovo inicijalizacija
    um->userHost.sin_family = AF_INET;
    um->userHost.sin_port = htons(portNumber);
    memset(&(um->homeHost), 0, sizeof(struct sockaddr_in));

    if(inet_aton(ipAddress, &(um->userHost.sin_addr)) == 0) {
        return -7;
    }

    if(bind(um->socket, (struct sockaddr *)&(um->userHost), (socklen_t)sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }


	return 0;
}

int initRM(unsigned int portNumber, char ipAddress[16], RPAddress rpAddress, routerModel * rm) {

    memset(&(rm->homeHost), 0, sizeof(struct sockaddr_in));
    rm->homeHost.sin_family = AF_INET;
    rm->homeHost.sin_port = htons(portNumber);
    if((rm->homeHost.sin_addr.s_addr = inet_addr(ipAddress)) == 0) {
        return -1;
    };

    if(!rpAddress) return 0;

    strcpy(rm->routerAddress, rpAddress);
    memset(rm->userHosts, 0, sizeof(struct sockaddr_in)*MAXUSERS);
    memset(rm->routerHosts, 0, sizeof(struct sockaddr_in)*MAXROUTERS);
    memset(rm->users, 0, sizeof(unsigned char)*MAXUSERS);
    memset(rm->routerTable, 0, sizeof(unsigned char)*MAXROUTERS*MAXROUTERS);
    memset(rm->sendTableBuffer, 0, ROUTERBUFFER);
    memset(rm->sendTPBuffer, 0, CONVBUFFSIZETP);
    memset(rm->receiveBuffer, 0, ROUTERBUFFER);

    pthread_mutex_init(&(rm->routerTableMutex), NULL);

    if((rm->socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("ERROR! Socket setup: ");
        return -8;
	}
    if(bind(rm->socket, (struct sockaddr *)&(rm->homeHost), (socklen_t)sizeof(struct sockaddr_in)) == -1) {
        perror("ERROR! Bind: ");
        return -5;
    }
}

int connectRMToNetwork(routerModel * sourceRouter, routerModel * destRouter) {
    transferPackage tp;
    tp.packageType = 2;
    tp.dataSent = 0;
    size_t len = sizeof(struct sockaddr_in);
    strcpy(tp.sourceAddress, sourceRouter->routerAddress);
    memset(sourceRouter->sendTPBuffer, 0, CONVBUFFSIZETP);
    convertTPackageToArray(&tp, sourceRouter->sendTPBuffer);
    if(sendto(sourceRouter->socket, sourceRouter->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(destRouter->homeHost), len) == -1) {
        perror("ERROR! Sendto: ");
        return -1;
    }
    return 0;
}

int connectUMToNetwork(userModel * user, routerModel * destRouter) {
    transferPackage tp;
    tp.packageType = 1;
    tp.dataSent = 0;
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
    printf("Pozvana tpfromrouter iz konekcie\n");
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
