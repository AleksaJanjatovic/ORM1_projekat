#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/protocolStructure.h"

int setRouterNumber(unsigned int routerNumber, char* address) {
    if(routerNumber < 0 || routerNumber > 256) {
		return -1;
    } 
    char * point = strstr(address, "."); //point nam je pointer na tacku
    char num[4]; 
    snprintf(num, 4, "%d", routerNumber); 
    if(routerNumber < 100) {
            point[-3] = '0';
            point[-2] = num[0];
            point[-1] = num[1];
    } else if (routerNumber < 10) {
            point[-3] = '0';
            point[-2] = '0';
            point[-1] = num[0];
    } else {
            point[-3] = num[0];
            point[-2] = num[1];
            point[-1] = num[2];
    }
}

int getRouterNumber(char * address) {
    char ret_val[4] = {'0', '0', '0', 0};

    for(int i = 0; i < 4; i++) {
        if(address[i] == '.') {
                ret_val[i] = 0;
                break;
        }
        ret_val[i] = address[i];
    }
    return atoi(ret_val);
}

int setHostNumber(unsigned int hostNumber, char * address) {
	if(hostNumber < 0 || hostNumber > 256) {
		return -2;
	}
    char * point = strstr(address, "."); //point nam je pointer na tacku
    char num[4]; 
    snprintf(num, 4, "%d", hostNumber); 
    if(hostNumber < 100) {
            point[1] = '0';
            point[2] = num[0];
            point[3] = num[1];
    } else if (hostNumber < 10) {
            point[1] = '0';
            point[2] = '0';
            point[3] = num[0];
    } else {
            point[1] = num[0];
            point[2] = num[1];
            point[3] = num[2];
    }
}	

int getHostNumber(char * address) {
	char ret_val[4] = {'0', '0', '0', 0};

	for(int i = 0; i < 4; i++) {
		ret_val[i] = address[i+4];
	}
	return atoi(ret_val);
}

