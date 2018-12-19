#include "../include/transferProtocol.h"
#include "../include/UDPlayer.h"
#include <unistd.h>

//Private functions
void sockaddr_inToArray(struct sockaddr_in * sadr, unsigned char buffer[8]) {
    buffer[0] = (unsigned char)(sadr->sin_family);
    buffer[1] = (unsigned char)((sadr->sin_family)>>8);
    buffer[2] = (unsigned char)(sadr->sin_port);
    buffer[3] = (unsigned char)((sadr->sin_port)>>8);
    buffer[4] = (unsigned char)(sadr->sin_addr.s_addr);
    buffer[5] = (unsigned char)((sadr->sin_addr.s_addr)>>8);
    buffer[6] = (unsigned char)((sadr->sin_addr.s_addr)>>16);
    buffer[7] = (unsigned char)((sadr->sin_addr.s_addr)>>24);
}

void arrayTosockaddr_in(unsigned char * buffer, struct sockaddr_in * sadr) {
    sadr->sin_family = buffer[0] + (((unsigned short)buffer[1])<<8);
    sadr->sin_port = buffer[2] + (((unsigned short)buffer[3])<<8);
    sadr->sin_addr.s_addr = buffer[4] + (((unsigned long)buffer[5])<<8) + (((unsigned long)buffer[6])<<16) + (((unsigned long)buffer[7])<<24);
    for(int i = 0; i < 8; i++) {
        sadr->sin_zero[i] = 0;
    }
}

void convertTPackageToArray( transferPackage * tp, unsigned char buffer[CONVBUFFSIZETP]) {
    int i;
    int currentSize = 0;
    for(i = 0; i < sizeof(RPAddress); i++) {
        buffer[i] = (tp->destinationAddress)[i];
        buffer[sizeof(RPAddress) + i] = (tp->sourceAddress)[i];
    }
    currentSize += i + sizeof(RPAddress);
    for(i = 0; i < PACKAGEMAX; i++) {
        buffer[currentSize + i] = tp->data[i];
    }
    currentSize += i;

    buffer[currentSize++] = (unsigned char)(tp->dataSent);//najmanja vrednost se cuva na najmanjem indeksu buffer-a, tj prvo idu prvih 8 bita pa drugih 8 itd.
    buffer[currentSize++] = (unsigned char)((tp->dataSent)>>8);

    buffer[currentSize++] = (unsigned char)(tp->socket);//najmanja vrednost se cuva na najmanjem indeksu buffer-a, tj prvo idu prvih 8 bita pa drugih 8 itd.
    buffer[currentSize++] = (unsigned char)((tp->socket)>>8);

    sockaddr_inToArray(&(tp->sourceHost), &buffer[currentSize]);
    currentSize += sizeof(struct sockaddr_in)/2; // Ne uzimamo sin_zero zato sto su ta polja uvek 0
    sockaddr_inToArray(&(tp->destinationHost), &buffer[currentSize]);
}

void convertArrayToTPackage(unsigned char * buffer, transferPackage * tp) {
    int i;
    int currentIterator = 0;
    for(i = 0; i < 8; i++) {
        tp->destinationAddress[i] = buffer[i];
        tp->sourceAddress[i] = buffer[i + sizeof(RPAddress)];
    }
    currentIterator += i + sizeof(RPAddress);
    for(i = 0; i < PACKAGEMAX; i++) {
        tp->data[i] = buffer[currentIterator + i];
    }
    currentIterator += PACKAGEMAX;
    tp->dataSent = buffer[currentIterator] + (((unsigned short)buffer[currentIterator+1])<<8);
    currentIterator += 2;
    tp->socket = buffer[currentIterator] + (((unsigned short)buffer[currentIterator+1])<<8);
    currentIterator += 2;

    arrayTosockaddr_in(&buffer[currentIterator], &(tp->sourceHost));
    currentIterator += sizeof(struct sockaddr_in)/2;
    arrayTosockaddr_in(&buffer[currentIterator], &(tp->destinationHost));
}


//Public functions
int sendTransferPackage(char * outBuff, size_t buffSize, transferPackage* tp, char attempts) {
	size_t dataSent = 0; //ukupno poslato
	size_t currentData = 0; // Trenutno za slanje
	int errorCounter = 0;	
	if(buffSize < 0) {
		return -3;
	}
	while(dataSent < buffSize) {
		currentData = 0;		
		for(int i = 0; i < PACKAGEMAX; i++) {			
            if(dataSent > buffSize) {
				break;			
			}
			tp->data[i] = outBuff[dataSent];
			dataSent++;
			currentData++;
		}
		tp->dataSent = --currentData; //Smanjiti za jedan posto currentData za poslednju iteraciju je za jedan veci
        char buffer[CONVBUFFSIZETP];
        convertTPackageToArray(tp, buffer);
        while(sendto(tp->socket, buffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(tp->destinationHost), sizeof(struct sockaddr_in)) == -1){
            printf("Upomoc\n");
            usleep(100); // ovo se desava jedino ako dodje do greske, tj ako se paket ne posalje(to moze i zato sto je isteklo vreme od 100us)
            if(++errorCounter >= attempts) {
				return -4;
			}
		}
		errorCounter = 0;
	}
	return 0;
}

int receiveTransferPackage(char * inBuff, size_t buffSize, transferPackage* tp) {
    size_t dataReceived = 0; // Trenutno za slanje
    if(buffSize < tp->dataSent) {
        return -3;
    }
    size_t len = sizeof(struct sockaddr_in);

    char buffer[CONVBUFFSIZETP];
    while(dataReceived < buffSize) {
        if(recvfrom(tp->socket, buffer, CONVBUFFSIZETP, 0, (struct sockaddr*)&(tp->sourceHost), (socklen_t*)&len) == -1) {
            return -4;
        }
        convertArrayToTPackage(buffer, tp);
        for(int i = 0; i < tp->dataSent; i++ ) {
            inBuff[dataReceived++] = tp->data[i];
        }
        printf("Primljen paket\n");
    }
    return 0;
}

int sendRouterPackage(routerPackage * rp, char attempts) {
    int errorCounter = 0;
    while(sendto(rp->socket, rp, sizeof(routerPackage), 0, (struct sockaddr*)&(rp->destinationHost), sizeof(struct sockaddr_in)) == -1) {
        usleep(100);
        if(++errorCounter <= attempts) {
            return -5;
        }
    }
    return 0;
}

int receiveRouterPackage(routerPackage * rp) {
    size_t len = sizeof(struct sockaddr_in);
    if(recvfrom(rp->socket, rp, sizeof(routerPackage), 0, (struct sockaddr*)&(rp->sourceHost), (socklen_t*)&len) == -1) {
        return -1;
    }
    return 0;
}



void convertRPackageToArray() {

}

void convertArrayToRPackage() {

}

