#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#define TRUE 1
#define NBRECARACTERE 512
#define TAILLEPSEUDO 50
#define TAILLENOMNIVEAUETUDE 50
#define TAILLENOM 5

/* Tableau comprenant les noms des entreprises */
/* du groupe */
char NIVEAUETUDE[4][12]= {"Masterclassique", "Masteralternance", "Licence"};
                 

struct paramsUser {
    char * pseudo;
};

void appli(){
    printf("********************************************************************\n");
    printf("*                                                                  *\n");
    printf("*                  Bienvenue sur Réseaux Chat de L'université       *\n");
    printf("*                                                                  *\n");
    printf("********************************************************************\n");
  
}

int envoi_message(int sock_serv, char data[], int tailleData) {

    if (strstr(data, "/name")) {
        data[tailleData - 1] = '\0';
        tailleData--;
    } else if (strstr(data, "/quit")) {
        write(sock_serv, data, tailleData);
        close(sock_serv);
        exit(EXIT_SUCCESS);
    }

    write(sock_serv, data, tailleData);
    return EXIT_SUCCESS;
}