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
#define TAILLENOMENTREPRISE 50
#define TAILLENAME 5

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

