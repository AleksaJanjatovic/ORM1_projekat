#include "../include/routerHost.h"
#include "../include/initRoutingP.h"
#include "../include/utilityFun.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void convertRTableToArray(routerModel * rm, unsigned char array[ROUTERBUFFER]);
void convertArrayToRTable(unsigned char array[ROUTERBUFFER], routerModel * rm);
void parseRouterTable(routerModel * rm);
void findPath(unsigned char currentRouter, unsigned char destRouter, routerModel * rt, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished);
void shiftNeighbours(unsigned char array[MAXROUTERS]);
int sendRouterTable(routerModel * rm);
int parseTP(routerModel * rm, struct sockaddr_in * recv_address);

int sendRouterTable(routerModel * rm) { // ova funkcija ide u tajmer

    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    int i, neighbourNumber = rm->routerTable[getRouterNumber(rm->routerAddress)][0];

    convertRTableToArray(rm, rm->sendTableBuffer);
    pthread_mutex_unlock(&rm->routerTableMutex);
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
    //while(1) {
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
        usleep(20000);
    //}
}


void findPath(unsigned char currentRouter, unsigned char destRouter, routerModel * rm, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished) {
//    int numOfcurrentRouter = 0; // broj susednih rutera
//    int wrongPath;
//    for(unsigned char i = 1; i != 0; i++) {
//        if(!(*finished)) {
//            wrongPath = 0;
//            if(rt->routerTable[currentRouter][i]) {
//                ++numOfcurrentRouter;
//                if(i == destRouter) {
//                    path[*nodeNum] = destRouter; //
//                    *finished = 1;
//                    return;
//                } else {
//                    for(int j = 0; j < *nodeNum; j++) {
//                        if(path[j] == i) { //moramo proveriti da li smo vec kopali po trenutnom ruteru
//                            --numOfcurrentRouter;
//                            wrongPath = 1;
//                            break;
//                        }
//                    }
//                    if(!wrongPath) {
//                        path[(*nodeNum)++] = i;
//                        findPath(i, destRouter, rt, path, nodeNum, finished);
//                    }
//                }
//            }
//        }

//    }
//    if(!numOfcurrentRouter && !(*finished)) {
//        path[(*nodeNum)--] = 0;
//    }
    printf("Poziv za ruter %d\n", currentRouter);
    int numOfcurrentRouter = 0; // broj susednih rutera
    int wrongPath;
    unsigned char i, j;
    for(i = 1; i <= rm->routerTable[currentRouter][0]; i++) {
        if(!(*finished)) {
            wrongPath = 0;
            if(rm->routerTable[currentRouter][i]) {
                ++numOfcurrentRouter;
                if(rm->routerTable[currentRouter][i] == destRouter) {
                    path[*nodeNum] = destRouter; //
                    *finished = 1;
                    return;
                } else {
                    for(j = 1; j < *nodeNum; j++) {
                        if(path[j] == rm->routerTable[currentRouter][i]) { //moramo proveriti da li smo vec kopali po trenutnom ruteru
                            wrongPath = 1;
                            break;
                        }
                    }
                    if(!wrongPath) {
                        path[(*nodeNum)++] = rm->routerTable[currentRouter][i];
                        printf("Poziv rekurzije za %d\n", rm->routerTable[currentRouter][i]);
                        findPath(rm->routerTable[currentRouter][i], destRouter, rm, path, nodeNum, finished);
                    }
                }
            }
        } else {
            return;
        }

    }
    if(!(*finished)) {
        printf("Brisanje clana %d\n", path[(*nodeNum) - 1]);
        path[(*nodeNum)--] = 0;
    }
}

int parseReceivedData(routerModel * rm) {
    struct sockaddr_in recv_address;
    unsigned short byteNum;
    size_t len = sizeof(struct sockaddr_in);
    while(1) {
        //////printf("Tp parssed\n");
        if(memset(&recv_address, 0, sizeof(struct sockaddr_in)) == 0) {
            perror("Wrong memory: ");
            return -1;
        }
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
    }
}

int parseTP(routerModel * rm, struct sockaddr_in * recv_address) {
    transferPackage tp;
    convertArrayToTPackage(rm->receiveBuffer, &tp);
    unsigned char path[MAXROUTERS];
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    unsigned char destinationRouter = getRouterNumber(tp.destinationAddress);
    size_t len = sizeof(struct sockaddr_in);
    unsigned char i;
    char pathFinished = 0;
    switch(tp.packageType) {
        case 0: // prosledjivanje paketa
            printf("Transfer package passed\n");
            convertTPackageToArray(&tp, rm->sendTPBuffer);
            if(destinationRouter == thisRouter) {
                if(rm->users[destinationRouter] != 0) {
                    if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->userHosts[destinationRouter]), sizeof(struct sockaddr_in)) == -1)  {
                        return -4;
                    }
                } else {
                    printf("This router doesn't have that user!\n");
                }
            } else {
                unsigned char nodeNum = 0;

                for(i = 1; i < MAXROUTERS; i++) {
                    path[i] = 0;
                }
                if(thisRouter == destinationRouter) {
                    path[1] = destinationRouter;
                } else {
                    pthread_mutex_lock(&(rm->routerTableMutex));
                    findPath(thisRouter, destinationRouter, rm, path, &nodeNum, &pathFinished);
                    pthread_mutex_unlock(&(rm->routerTableMutex));
                }
                if(!path[0]) { //ako imamo putanju do rutera, onda ce path[0] biti razlicito od 0
                    printf("No valid path to this router %d\n", destinationRouter);
                } else {

                    if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->routerHosts[path[0]]), sizeof(struct sockaddr_in)) == -1) {\
                        return -3;
                    }
                }
            }
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
            printf("Postavljanje vrednosti usera%d\n", i);
            setUserNumber(i, tp.sourceAddress);

            convertTPackageToArray(&tp, rm->sendTPBuffer);
            if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->userHosts[i]), sizeof(struct sockaddr_in)) == -1) {
                return -1;
            }

            printRouterModel(rm);

        break;
        case 2:;
            //printf("Router connection package passed\n");
            unsigned char otherRouter = getRouterNumber(tp.sourceAddress);

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
            convertTPackageToArray(&tp, rm->sendTPBuffer);

            //printf("Sending package to %d\n", otherRouter);
            if(sendto(rm->socket, rm->sendTPBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&rm->routerHosts[otherRouter], (socklen_t)sizeof(struct sockaddr_in)) == -1) { //posalji drugom ruteru svoju adresu
                return -2;
            }
            //printf("Response package sent\n");
            ////printRouterModel(rm);
            ////printRouterTable(rm);
        break;
        default:
            //printf("Wrong package type\n");
        break;
    }
}

void parseRouterTable(routerModel * rm) {
    pthread_mutex_lock(&rm->routerTableMutex);
    convertArrayToRTable(rm->receiveBuffer, rm);
    ////printf("Router table received\n");
    pthread_mutex_unlock(&rm->routerTableMutex);
}
