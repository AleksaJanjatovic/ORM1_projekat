#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/protocolStructure.h"

int setRouterNumber(unsigned char routerNumber, char * address) {
    if(routerNumber < 1 || routerNumber >= MAXROUTERS) {
		return -1;
    } 
    char num[3] = {routerNumber/100 + 48, (routerNumber%100)/10 + 48, routerNumber%10 + 48};
    address[0] = num[0];
    address[1] = num[1];
    address[2] = num[2];
    address[3] = '.';
    address[7] = 0;
}

unsigned char getRouterNumber(char * address) {
    char ret_val[4] = {'0', '0', '0', 0};
    for(int i = 0; i < 3; i++) {
        if(address[i] == '.') {
                ret_val[i] = 0;
                break;
        }
        ret_val[i] = address[i];
    }
    return (unsigned char)atoi(ret_val);
}

int setUserNumber(unsigned char userNumber, char * address) {
    if(userNumber < 1 || userNumber >= MAXUSERS) {
		return -2;
	}
    char num[3] = {userNumber/100 + 48, (userNumber%100)/10 + 48, userNumber%10 + 48};
    address[4] = num[0];
    address[5] = num[1];
    address[6] = num[2];
    address[3] = '.';
    address[7] = 0;
}	

unsigned char getUserNumber(char * address) {
	char ret_val[4] = {'0', '0', '0', 0};

	for(int i = 0; i < 4; i++) {
		ret_val[i] = address[i+4];
	}
    return (unsigned char)atoi(ret_val);
}

