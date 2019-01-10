#include "../include/routerHost.h"
#include "../include/initRoutingP.h"
#include "../include/utilityFun.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void convertRTableToArray(routerModel * rm, unsigned char array[ROUTERBUFFER]);
void convertArrayToRTable(unsigned char array[ROUTERBUFFER], routerModel * rm);

int sendRouterTable(routerModel * rm) { // ova funkcija ide u tajmer

    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    int i, neighbourNumber = rm->routerTable[getRouterNumber(rm->routerAddress)][0];

    struct sockaddr_in distantTableHost;

    pthread_mutex_lock(&rm->rT_mutex);
    convertRTableToArray(rm, rm->sendTableBuffer);
    pthread_mutex_unlock(&rm->rT_mutex);
    for(i = 1; i <= neighbourNumber; i++){ // u sledecim redovima je negde segfault
        distantTableHost  = rm->routerHosts[rm->routerTable[thisRouter][i]];
        distantTableHost.sin_port++;
        if(sendto(rm->socketTable, rm->sendTableBuffer, ROUTERBUFFER, 0,
    (struct sockaddr *)&(rm->routerHosts[rm->routerTable[thisRouter][i]]), (socklen_t)sizeof(struct sockaddr_in)) == -1) { //salje se na svaki ruter
            perror("RouterTable sending error");
            printf("Trying to send to %d with: \nportNumber %d\nip %d\n", rm->routerTable[thisRouter][i], rm->routerHosts[rm->routerTable[thisRouter][i]].sin_port, rm->routerHosts[rm->routerTable[thisRouter][i]].sin_addr.s_addr);
        }
        printf("Table sent to %d\n", rm->routerTable[thisRouter][i]);
    }

    return 0;
}

int receiveRouterTable(routerModel * rm) {
    struct sockaddr sck;
    memset(&sck, 0, sizeof(struct sockaddr));
    size_t s = sizeof(struct sockaddr_in);
    while(1) {
        if(recvfrom(rm->socketTable, rm->receiveTableBuffer, ROUTERBUFFER, 0, &sck, (socklen_t*)&s)) {
            return -5;
        }
        pthread_mutex_lock(&rm->rT_mutex);
        convertArrayToRTable(rm->transferBuffer, rm);
        printf("Router table received\n");
        pthread_mutex_unlock(&rm->rT_mutex);
    }
    return 0;
}

void convertRTableToArray(routerModel * rm, unsigned char array[ROUTERBUFFER]) {
    int a = 0, i, j;
    for(i = 0; i < MAXROUTERS; i++) { //preskacemo prvi red
        for(j = 0; j < MAXROUTERS; j++) {
            array[a++] = rm->routerTable[i][j];
        }
    }
    //array[a] = getRouterNumber(rm->routerAddress); // spakujemo broj rutera u paket
}

void convertArrayToRTable(unsigned char array[ROUTERBUFFER], routerModel * rm) {
    int a = MAXROUTERS; //zato sto pocinjemo od prvog reda
    int neighbour;
    int i;
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    for(i = 0; i < MAXROUTERS; i++) {
        if(array[i] > 0) {
            rm->routerTable[0][i] = array[i];
        }
    }
    for(i = 1; i < MAXROUTERS; i++) {
        if(array[a] > rm->routerTable[i][0]){ //Ako je  broj komsija primljenog paketa veci od broja komsija trenutnog onda popuni
            for(int j = 0; j < MAXROUTERS; j++) {
                rm->routerTable[i][j] = array[a++];
            }
        } else {
            a += MAXROUTERS; // ako je nula preskoci citav taj red posto nema potrebe da se popunjava
        }
    }
    rm->routerTable[thisRouter][array[a]] = 100; //paket od koga je stigao je sigurno komsijski, tako da nam oon osvezava odgovarajucu tabelu
}


void shiftNeighbours(unsigned char array[MAXROUTERS]) {
    int zero_indx = 1;
    int i;
    for(i = 1; i < array[0]; i++) {
        if(array[i] == 0) {
            zero_indx = i;
            for(int j = zero_indx; j < array[0]; j++) {
                array[j] = array[j + 1];
            }
            break;
        }
    }
}

//Ovo se poziva pre nego sto se sam ruterPridruzi mrezi
void routerTableTimeControl(routerModel * rm) {
    //printf("Zapoceta kontrola tabele\n");
    int i,j,k;
    pthread_mutex_lock(&(rm->rT_mutex));
    //printf("Zakljucan mutex\n");
    rm->routerTable[0][getRouterNumber(rm->routerAddress)] = 255; //stavljanje refresh counter-a na 20
    for(i = 1; i < MAXROUTERS; i++) {
        if(rm->routerTable[0][i]) {
            rm->routerTable[0][i]--; // uslovna kontrola smanjuje svaki put dok ne dodje do 0
            //printf("%d\n", rm->routerTable[0][i]);
        } else { // brisemo red
            if(rm->routerTable[i][0]) { //ako je broj komsija vec jednak nuli, onda je taj red vec obrisan pa ne moramo da prolazimo kroz ovo
                for(j = 0; j <= rm->routerTable[i][0]; j++) {
                   rm->routerTable[i][j] = 0; //brisanje tog reda u potpunosti
                }
                for(j = 1; j < MAXROUTERS; j++) { //proveravamo za svaki ruter da li poseduju uklonjenog rutera
                    if(!(rm->routerTable[j][0])) { // ovo radimo jedino ako je broj suseda razlicit od nule
                        for(k = 1; k <= rm->routerTable[j][0]; k++) {
                            if(rm->routerTable[j][k] == i)
                                rm->routerTable[j][k] = 0;
                        }
                        shiftNeighbours(rm->routerTable[j]);
                        (rm->routerTable[j][0])--;
                    }
                }
            }
        }
    }
    //printf("Zavrsena kontrola tabele\n");
    //printf("Zavrseno slanje tabele\n");
    pthread_mutex_unlock(&(rm->rT_mutex));
    //printf("Otkljucan mutex\n");
    usleep(1000000); //privremeno resenje da nemoram da stavljam da je to u vremensko kontrolisanoj niti
}


void findPath(unsigned char nextRouter, unsigned char destRouter, routerModel * rm, unsigned char path[MAXROUTERS], unsigned char * nodeNum, char * finished) {
//    int numOfNextRouter = 0; // broj susednih rutera
//    int wrongPath;
//    for(unsigned char i = 1; i != 0; i++) {
//        if(!(*finished)) {
//            wrongPath = 0;
//            if(rt->routerTable[nextRouter][i]) {
//                ++numOfNextRouter;
//                if(i == destRouter) {
//                    path[*nodeNum] = destRouter; //
//                    *finished = 1;
//                    return;
//                } else {
//                    for(int j = 0; j < *nodeNum; j++) {
//                        if(path[j] == i) { //moramo proveriti da li smo vec kopali po trenutnom ruteru
//                            --numOfNextRouter;
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
//    if(!numOfNextRouter && !(*finished)) {
//        path[(*nodeNum)--] = 0;
//    }
    int numOfNextRouter = 0; // broj susednih rutera
    int wrongPath;
    path[0] = nextRouter;
    if(nextRouter == destRouter) return; //ako imamo samo jedan ruter
    for(unsigned char i = 1; i <= rm->routerTable[nextRouter][0]; i++) {
        if(!(*finished)) {
            printf("Iteracija %d \n", i);
            wrongPath = 0;
            if(rm->routerTable[nextRouter][i]) {
                printf("Upali smo za %d\n", rm->routerTable[nextRouter][i]);
                ++numOfNextRouter;
                if(rm->routerTable[nextRouter][i] == destRouter) {
                    path[*nodeNum] = destRouter; //
                    printf("Zavrsavanje sa ruterom %d\n", rm->routerTable[nextRouter][i]);
                    *finished = 1;
                    return;
                } else {
                    printf("Usli smo u else \n");
                    for(int j = 0; j < *nodeNum; j++) {
                        if(path[j] == rm->routerTable[nextRouter][i]) { //moramo proveriti da li smo vec kopali po trenutnom ruteru
                            printf("Putanja vec proverena %d\n", rm->routerTable[nextRouter][i]);
                     //       --numOfNextRouter;
                            wrongPath = 1;
                            break;
                        }
                    }
                    if(!wrongPath) {
                        printf("Rekurzija za %d \n", rm->routerTable[nextRouter][i]);
                        path[(*nodeNum)++] = rm->routerTable[nextRouter][i];
                        findPath(rm->routerTable[nextRouter][i], destRouter, rm, path, nodeNum, finished);
                    }
                }
            }
        } else {
            return;
        }

    }
    if(!(*finished)) {
        //printf("Brisanje clana %d\n", path[(*nodeNum) - 1]);
        path[(*nodeNum)--] = 0;
    }
}

int parseTP(routerModel * rm) {
    transferPackage tp;
    struct sockaddr_in recv_address;
    size_t len = sizeof(struct sockaddr_in);
    unsigned char thisRouter = getRouterNumber(rm->routerAddress);
    unsigned char path[MAXROUTERS];
    unsigned short byteNum;
    char pom = 0;
    int i;
    while(1) {
        //printf("Tp parssed\n");
        if(memset(&recv_address, 0, sizeof(struct sockaddr_in)) == 0) {
            perror("Wrong memory: ");
            return -1;
        }

        //printf("POsle ovoga je greska\n");
        if((byteNum = recvfrom(rm->socketTransferPackage, rm->transferBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&recv_address, (socklen_t*)&len)) == -1) {
            perror("Message not recieved: ");
            return -2;
        } else if(byteNum == ROUTERBUFFER) {

        } else if(byteNum == CONVBUFFSIZETP) {

        }
        printf("POsle ovoga je greska\n");
        convertArrayToTPackage(rm->transferBuffer, &tp);
        printTPPackage(&tp);
        switch(tp.packageType) {
            case 0:
                if(getRouterNumber(&tp.destinationAddress) == thisRouter) {
                    if(rm->users[getUserNumber(&tp.destinationAddress)] != 0) {
                        if(sendto(rm->socketTransferPackage, rm->transferBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->userHosts[getUserNumber(tp.destinationAddress)]), sizeof(struct sockaddr_in)) == -1)  {
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
                    pthread_mutex_lock(&(rm->rT_mutex));
                    findPath(thisRouter, getRouterNumber(&tp.destinationAddress), rm, path, &nodeNum, &pom);
                    pthread_mutex_unlock(&(rm->rT_mutex));
                    if(nodeNum == 0) {
                        printf("No valid path to this router!\n");
                    } else {
                        if(sendto(rm->socketTransferPackage, rm->transferBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->routerHosts[path[1]]), sizeof(struct sockaddr_in)) == -1) {\
                            return -3;
                        }
                    }
                }
            break;
            case 1:             
                for(i = 0; i < MAXUSERS; i++) {
                    if(!(rm->users[i])) {
                        rm->users[i] = 1;
                        break;
                    }
                }
                rm->userHosts[i].sin_addr.s_addr = recv_address.sin_addr.s_addr;
                rm->userHosts[i].sin_family = AF_INET;
                rm->userHosts[i].sin_port = recv_address.sin_port;
                setRouterNumber(getRouterNumber(rm->routerAddress), tp.sourceAddress); // u ove dve linije ruter dodeljuje RPadresu useru
                setUserNumber(i, tp.sourceAddress);
                //printTPPackage(&tp);
                convertTPackageToArray(&tp, rm->transferBuffer);
                if(sendto(rm->socketTransferPackage, rm->transferBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(rm->userHosts[i]), sizeof(struct sockaddr_in)) == -1) {
                    return -1;
                }
                printRouterModel(rm);
            break;
            case 2:;
                unsigned char otherRouter = getRouterNumber(tp.sourceAddress);
                pthread_mutex_lock(&rm->rT_mutex);
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
                    rm->routerHosts[otherRouter].sin_addr.s_addr = recv_address.sin_addr.s_addr;
                    rm->routerHosts[otherRouter].sin_family = AF_INET;
                    rm->routerHosts[otherRouter].sin_port = recv_address.sin_port;
                    setRouterNumber(thisRouter, tp.sourceAddress);
                } else {
                    setRouterNumber(0, tp.sourceAddress); // Saljemo mu adresu 000.000 nazad ako je adresa vec zauzeta
                }
                pthread_mutex_unlock(&rm->rT_mutex);
                setUserNumber(0, tp.sourceAddress);
                convertTPackageToArray(&tp, rm->transferBuffer);
                if(sendto(rm->socketTransferPackage, rm->transferBuffer, CONVBUFFSIZETP, 0, (struct sockaddr *)&(recv_address), (socklen_t)sizeof(struct sockaddr_in)) == -1) { //posalji drugom ruteru svoju adresu
                    return -2;
                }
                printRouterModel(rm);
                printRouterTable(rm);
                printf("Poslato\n");
            break;
            default:
                printf("Wrong package type\n");
            break;
        }
    }
}
