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

    um->userHost.sin_family = AF_INET;
    um->userHost.sin_port = htons(portNumber);
    if(inet_aton(ipAddress, &(um->userHost.sin_addr)) == 0) {
        return -7;
    }

    memset(&(um->homeHost), 0, sizeof(struct sockaddr_in));
    memset(um->receiveTPBuffer, 0, CONVBUFFSIZETP);
    memset(um->sendTPBuffer, 0, CONVBUFFSIZETP);
    strcpy(um->userAddress, "000.000");

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

    if(!rpAddress) {
       strcpy(rm->routerAddress, "000.000");
       return 0;
    }

    strcpy(rm->routerAddress, rpAddress);
    setUserNumber(0, rm->routerAddress);
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
    memset(&tp, 0, sizeof(transferPackage));

    tp.packageType = 2;
    tp.dataSent = 0;
    tp.currentNode = 0;

    size_t len = sizeof(struct sockaddr_in);
    strcpy(tp.sourceAddress, sourceRouter->routerAddress);
    strcpy(tp.destinationAddress, destRouter->routerAddress);

    convertTPackageToArray(&tp, sourceRouter->sendTPBuffer);
    if(sendto(sourceRouter->socket, sourceRouter->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(destRouter->homeHost), len) == -1) {
        perror("ERROR! Router model connection");
        return -1;
    }
    return 0;
}

int connectUMToNetwork(userModel * user, routerModel * destRouter) {
    transferPackage tp;
    memset(&tp, 0, sizeof(transferPackage));

    tp.packageType = 1;
    tp.dataSent = 0;
    tp.currentNode = 0;

    strcpy(tp.destinationAddress, destRouter->routerAddress);
    strcpy(tp.sourceAddress, user->userAddress);

    user->homeHost.sin_addr.s_addr = destRouter->homeHost.sin_addr.s_addr;
    user->homeHost.sin_family = destRouter->homeHost.sin_family;
    user->homeHost.sin_port = destRouter->homeHost.sin_port;

    convertTPackageToArray(&tp, user->sendTPBuffer);
    if(sendto(user->socket, user->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)& (user->homeHost), sizeof(struct sockaddr_in)) == -1) {
        perror("ERROR! User model connection");
        return -1;
    }

    receiveTPfromRouter(&tp, user);
    strcpy(user->userAddress, tp.sourceAddress); // ruter je spakovao u sourceAddress adresu koju treba da sadrzi korisnik

    printUserModel(user);
    return 0;
}

int closeUM(userModel * um) {
    close(um->socket);
}

int closeRM(routerModel * rm) {
    close(rm->socket);
    pthread_mutex_destroy(&rm->routerTableMutex);
}
