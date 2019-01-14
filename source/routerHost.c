#include "../include/routerHost.h"
#include "../include/initRoutingP.h"
#include "../include/utilityFun.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void convertRTableToArray(routerModel * rm, unsigned char array[ROUTERBUFFER]);
void convertArrayToRTable(unsigned char array[ROUTERBUFFER], routerModel * rm);
void parseRouterTable(routerModel * rm);
int findPath(unsigned char currentRouter, unsigned char destRouter, routerModel * rt, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished);
int sendTPToNextRouter(routerModel *rm, transferPackage *tp);
void shiftNeighbours(unsigned char array[MAXROUTERS]);
int sendRouterTable(routerModel * rm);
int parseTP(routerModel * rm, struct sockaddr_in * recv_address);

int sendRouterTable(routerModel * rm) { // ova funkcija ide u tajmer

    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    unsigned char i, neighbourNumber = rm->routerTable[getRouterNumber(rm->routerAddress)][0];

    convertRTableToArray(rm, rm->sendTableBuffer);
    for(i = 1; i <= neighbourNumber; i++){ // u sledecim redovima je negde segfault
        if(sendto(rm->socket, rm->sendTableBuffer, ROUTERBUFFER, 0,
    (struct sockaddr *)&(rm->routerHosts[rm->routerTable[thisRouter][i]]), (socklen_t)sizeof(struct sockaddr_in)) == -1) { //salje se na svaki ruter
            //perror("RouterTable sending error");
        }
        //printf("Router table sent to router %d\n", rm->routerTable[thisRouter][i]);
    }

    return 0;
}

void convertRTableToArray(routerModel * rm, unsigned char array[ROUTERBUFFER]) {
    int a = 0, i, j;
    for(i = 0; i < MAXROUTERS; i++) { //preskacemo prvi red
        for(j = 0; j < MAXROUTERS; j++) {
            //if(j == 0) ////printf("Router table neighbour number %d\n", rm->routerTable[i][j]);
            array[a++] = rm->routerTable[i][j];
        }
    }
}

void convertArrayToRTable(unsigned char array[ROUTERBUFFER], routerModel * rm) {
    int i, j, a = MAXROUTERS; //zato sto pocinjemo od prvog reda
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    for(i = 0; i < MAXROUTERS; i++) {
        if(array[i] > rm->routerTable[0][i]) {
            rm->routerTable[0][i] = array[i];
        }
    }
    for(i = 1; i < MAXROUTERS; i++) {
        if(array[a] > rm->routerTable[i][0]){ //Ako je  broj komsija primljenog paketa veci od broja komsija trenutnog onda popuni
            rm->routerTable[i][0] = array[a++];
            ////printf("Neighbour router value: %d\n", array[a-1]);
            for(j = 1; j < MAXROUTERS; j++) {
                rm->routerTable[i][j] = array[a++];
            }
        } else {
            a += MAXROUTERS; // ako je nula preskoci citav taj red posto nema potrebe da se popunjava
        }
    }
}

void parseRouterTable(routerModel * rm) {
    pthread_mutex_lock(&rm->routerTableMutex);
    convertArrayToRTable(rm->receiveBuffer, rm);
    //printf("Router table received\n");
    pthread_mutex_unlock(&rm->routerTableMutex);
}

void shiftNeighbours(unsigned char array[MAXROUTERS]) {
    int zero_indx = 1;
    int i, j;
    for(i = 1; i < array[0]; i++) {
        if(array[i] == 0) {
            zero_indx = i;
            for(j = zero_indx; j < array[0]; j++) {
                array[j] = array[j + 1];
            }
            break;
        }
    }
}

//Ovo se poziva pre nego sto se sam ruterPridruzi mrezi
void routerTableTimeControl(routerModel * rm) {
    while(1) {
        int i,j,k;
        pthread_mutex_lock(&rm->routerTableMutex);
        rm->routerTable[0][getRouterNumber(rm->routerAddress)] = REFRESHVALUE; //stavljanje refresh counter-a na 20
        for(i = 1; i < MAXROUTERS; i++) {
            if(rm->routerTable[0][i]) {
                rm->routerTable[0][i]--; // uslovna kontrola smanjuje svaki put dok ne dodje do 0
                //////printf("%d\n", rm->routerTable[0][i]);
            } else { // brisemo red
                if(rm->routerTable[i][0]) { //ako je broj komsija vec jednak nuli, onda je taj red vec obrisan pa ne moramo da prolazimo kroz ovo
                    for(j = 1; j <= rm->routerTable[i][0]; j++) {
                       rm->routerTable[i][j] = 0; //brisanje tog reda u potpunosti
                    }
                    rm->routerTable[i][0] = 0;

                    for(j = 1; j < MAXROUTERS; j++) { //proveravamo za svaki ruter da li poseduju uklonjenog rutera
                        if(rm->routerTable[j][0]) { // ovo radimo jedino ako je broj suseda razlicit od nule
                            for(k = 1; k <= rm->routerTable[j][0]; k++) {
                                if(rm->routerTable[j][k] == i) {
                                        shiftNeighbours(rm->routerTable[j]);
                                        ////printf("Smanjivanje broja suseda od rutera %d\n zato sto je na poziciji %d pronadjen %d", j, k, i);
                                        (rm->routerTable[j][0])--;
                                        rm->routerTable[j][k] = 0;
                                        break;
                                }
                            }

                        }
                    }
                }
            }
        }
        sendRouterTable(rm);
        pthread_mutex_unlock(&rm->routerTableMutex);
        usleep(2000000);
    }
    //}

    printf("Router Table time control broken\n");
}

int sendTPToNextRouter(routerModel * rm, transferPackage * tp) {
    unsigned char i;
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    char nextRouterPresent = 0, finished = 0;
    char pathFound = 1;
    pthread_mutex_lock(&rm->routerTableMutex);
    for(i = 1; i <= rm->routerTable[thisRouter][0]; i++) {
        if(rm->routerTable[thisRouter][i] == tp->path[tp->nodeNum]) {
            nextRouterPresent = 1;
            break;
        }
    }
    if(!nextRouterPresent) {
        memset(tp->path, 0, MAXROUTERS);
        tp->nodeNum = 0;
        if(!(pathFound = findPath(thisRouter, getRouterNumber(tp->destinationAddress), rm, tp->path, &tp->nodeNum, &finished))) {
            printf("No valid path found from this router\n");
        }
    }
    if(pathFound) {
        convertTPackageToArray(tp, rm->sendTPBuffer);
        printf("Sending to router %d\n ", tp->path[tp->nodeNum + 1]);
        if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0,(struct sockaddr*)&(rm->routerHosts[tp->path[++tp->nodeNum]]), (socklen_t)sizeof(struct sockaddr_in)) == -1) {
            perror("ERROR! Send to next router failed");
            return -5;
        }
    }
    pthread_mutex_unlock(&rm->routerTableMutex);
}

int findPath(unsigned char currentRouter, unsigned char destRouter, routerModel * rm, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished) {
    path[(*nodeNum)++] = currentRouter;
    unsigned char i,j;
    printf("Find Path called for %d\n", currentRouter);
    char routerAlreadyInPath = 0;
    for(i = 1; i <= rm->routerTable[currentRouter][0]; i++) {
        if(!(*finished)) {
            printf("Entering search for neighbour %d\n", rm->routerTable[currentRouter][i]);
            for(j = 0; j < *nodeNum; j++) {
                if(path[j] == rm->routerTable[currentRouter][i]) {
                    routerAlreadyInPath = 1;
                    printf("Router is already in path %d\n", path[j]);
                    break; // ako postoji ruter u pathu onda ga valja samo zaobici
                }
            }
            if(!routerAlreadyInPath) {
                if(rm->routerTable[currentRouter][i] == destRouter) {
                    printf("Stignuto do cilja. Broj nodova %d\n", *nodeNum);
                    *finished = 1;
                    path[*nodeNum];
                    return 1;
                } else {
                    printf("Recursion called for %d\n", rm->routerTable[currentRouter][i]);
                    findPath(rm->routerTable[currentRouter][i], destRouter, rm, path, nodeNum, finished);
                }
            }
        } else {
            printf("Find path finished\n");
            nodeNum = 0;
            return 1;
        }
    }
    path[--(*nodeNum)] = 0;
    return 0;
}

int parseReceivedData(routerModel * rm) {
    struct sockaddr_in recv_address;
    unsigned short byteNum = 0;
    transferPackage tp;
    size_t len = sizeof(struct sockaddr_in);
    while(1) {
        //////printf("Tp parssed\n");
        if(memset(&recv_address, 0, sizeof(struct sockaddr_in)) == 0) {
            perror("Wrong memory: ");
            return -1;
        }
        //printf("parseReceivedData invoked\n");
        //////printf("POsle ovoga je greska\n");
        if((byteNum = recvfrom(rm->socket, rm->receiveBuffer, ROUTERBUFFER, 0, (struct sockaddr *)&recv_address, (socklen_t*)&len)) == -1) {
            perror("Message not recieved: ");
            return -2;
        } else if(byteNum == ROUTERBUFFER) {
            //printf("Parsing router table. Recieved data size was %d\n", byteNum);
            parseRouterTable(rm);
        } else if(byteNum == CONVBUFFSIZETP) {
            //printf("Parsing transfer package. Recieved data size was %d\n", byteNum);
            parseTP(rm, &recv_address);
        }
        byteNum = 0;
    }
}

int parseTP(routerModel * rm, struct sockaddr_in * recv_address) {
    transferPackage tp;
    convertArrayToTPackage(rm->receiveBuffer, &tp);
    unsigned char path[MAXROUTERS];
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    unsigned char destinationRouter = getRouterNumber(tp.destinationAddress);
    unsigned char receivingUser = getUserNumber(tp.destinationAddress);
    unsigned char otherRouter, i, pathFinished = 0;
    size_t len = sizeof(struct sockaddr_in);
    printf("DOING PARSETP\n");
    switch(tp.packageType) {
        case 0: // prosledjivanje paketa
            printTPPackage(&tp);
            printf("User number %d\n", receivingUser);
            printf("%d\n", rm->userHosts[receivingUser].sin_addr.s_addr);
            printf("%d\n", rm->userHosts[receivingUser].sin_port);
            if(thisRouter == destinationRouter) {
                convertTPackageToArray(&tp, rm->sendTPBuffer);
                if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr*)&rm->userHosts[receivingUser], (socklen_t)sizeof(struct sockaddr_in)) == -1) {
                    perror("ERROR! Message not sent to user");
                    return -1;
                }
            } else {
                sendTPToNextRouter(rm, &tp);
            }
            printTPPackage(&tp);
        break;
        case 1:
            printf("User connection package passed\n");
            for(i = 1; i < MAXUSERS; i++) {
                if(!(rm->users[i])) {
                    rm->users[i] = 1;
                    break;
                }
            }
            rm->userHosts[i].sin_addr.s_addr = recv_address->sin_addr.s_addr;
            rm->userHosts[i].sin_family = AF_INET;
            rm->userHosts[i].sin_port = recv_address->sin_port;
            setRouterNumber(getRouterNumber(rm->routerAddress), tp.sourceAddress); // u ove dve linije ruter dodeljuje RPadresu useru
            printf("Postavljanje vrednosti usera: %d\n", i);
            setUserNumber(i, tp.sourceAddress);

            convertTPackageToArray(&tp, rm->sendTPBuffer);
            printf("%d\n", rm->userHosts[i].sin_addr.s_addr);
            printf("%d\n", rm->userHosts[i].sin_port);
            if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->userHosts[i]), sizeof(struct sockaddr_in)) == -1) {
                return -1;
            }

            printRouterModel(rm);

        break;
        case 2:
            //printTPPackage(&tp);
            otherRouter = getRouterNumber(tp.sourceAddress);

            pthread_mutex_lock(&rm->routerTableMutex);
            for(i = 1; i <= rm->routerTable[thisRouter][0]; i++) {
                if(rm->routerTable[thisRouter][i] == otherRouter)  {
                    i = -1;
                    break;
                }
            }
            if(i != -1) {
                rm->routerTable[thisRouter][++(rm->routerTable[thisRouter][0])] = otherRouter;
                rm->routerTable[0][thisRouter] = REFRESHVALUE;
                rm->routerTable[0][otherRouter] = REFRESHVALUE;
                rm->routerHosts[otherRouter].sin_addr.s_addr = recv_address->sin_addr.s_addr;
                rm->routerHosts[otherRouter].sin_port = recv_address->sin_port;
                rm->routerHosts[otherRouter].sin_family = AF_INET;
                setRouterNumber(thisRouter, tp.sourceAddress);
                //printf("This router addres set to sent %d\n", thisRouter);
            } else {
                setRouterNumber(0, tp.sourceAddress); // Saljemo mu adresu 000.000 nazad ako je adresa vec zauzeta
            }
            pthread_mutex_unlock(&rm->routerTableMutex);

            setUserNumber(0, tp.sourceAddress); //popunjavamo doji deo adrese
            //printf("Paket za slanje\n");
            tp.packageType = 3;
            //printTPPackage(&tp);
            convertTPackageToArray(&tp, rm->sendTPBuffer);

            //printf("Sending package to %d\n", otherRouter);
            if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&rm->routerHosts[otherRouter], (socklen_t)sizeof(struct sockaddr_in)) == -1) { //posalji drugom ruteru svoju adresu
                return -2;
            }
            printf("Response package sent\n");
            ////printRouterModel(rm);
            ////printRouterTable(rm);
        break;
        case 3:
            convertArrayToTPackage(rm->receiveBuffer, &tp);
            if(!strcmp(tp.sourceAddress,  "000.000")) {
                return -8;
            }
            thisRouter = getRouterNumber(rm->routerAddress);
            otherRouter = getRouterNumber(tp.sourceAddress); //nas strani ruter nam salje njegovu povratnu adresu
            pthread_mutex_lock(&rm->routerTableMutex);
            rm->routerTable[0][thisRouter] = REFRESHVALUE; //za osvezavanje putanje rutiranja
            rm->routerTable[0][otherRouter] = REFRESHVALUE;
            rm->routerTable[thisRouter][++rm->routerTable[thisRouter][0]] = otherRouter;
            rm->routerHosts[otherRouter].sin_addr.s_addr = recv_address->sin_addr.s_addr;
            rm->routerHosts[otherRouter].sin_port = recv_address->sin_port;
            rm->routerHosts[otherRouter].sin_family = AF_INET;
            printRouterTable(rm);
            pthread_mutex_unlock(&rm->routerTableMutex);
        break;
        default:
            //printf("Wrong package type\n");
        break;
    }
}


