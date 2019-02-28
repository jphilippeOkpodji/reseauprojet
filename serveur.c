#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/select.h>
#include <strings.h>
#include <string.h>

#define NBRCONNEXION 512 // nombre de connexion max au serveur en même temps (ie taille de la file d'attente)
#define NBCONNEXIONMAX 512
#define NBROCTETLU 512
#define TRUE 1
#define FALSE 0
#define TAILLEPSEUDO 16
#define TAILLENOM 5
#define TAILLEPRIVATE 8
#define TAILLEBANIR 6
#define EXIT_PSEUDO_ADD 4
#define EXIT_PSEUDO_CHANGE 3
#define EXIT_PSEUDO_EXIST 2
#define EXIT_PSEUDO_NOT_EXIST 1

char* trouverMessage(char* data, int taille_donnee, int indiceDebut);
char * findPseudoAfterName(char* data, int taille_donnee);
char* findPseudoAfterPrivate(char* data, int taille_donnee);
char * conception_message(char* m1, char* m2);
char * conception_message_n(char* m1, char* m2, int n);
char* findPseudoAfterBanir(char* data, int taille_donnee);

int nb_connecte = 0;

enum classe {
    OPERATEUR, VOICE, USER
}; // niveau de l'utilisateur

typedef struct structCase {
    int id_sock;
    char* pseudo;
    char* pseudo_ancien;
    enum classe type;
    struct structCase *nextp;
} Case;

typedef struct structList {
    Case* beginp;
    Case* lastp;
    int nb_joueur;
} Liste_Clients;

Liste_Clients* init();
int recherchePseudo(char* pseudo, Liste_Clients* list);
char* recherchePseudoAncien(int id_sock, Liste_Clients* list);
char* rechercheSocket(int id_sock, Liste_Clients* list);
int ajouterChanger(int id_sock, char* pseudo, Liste_Clients* list);
int supprimer(int id_sock, Liste_Clients* list);
int imprimer_liste_connecter(Liste_Clients* list);
char* message_list(Liste_Clients* list);


Liste_Clients* init() {
    Liste_Clients* list = (Liste_Clients*) malloc(sizeof (Liste_Clients));
    list->beginp = NULL;
    list->lastp = NULL;
    list->nb_joueur = 0;
    
    return list;
}


char* trouverMessage(char* data, int taille_donnee, int indiceDebut) {
    char* msg = (char*) malloc((taille_donnee - indiceDebut+1) * sizeof (char));
    int i = indiceDebut;
    int j = 0;
    
    while(i< taille_donnee){
        *(msg+j) = data[i];
        i++;
        j++;
    }
    *(msg+j)='\0';
    return msg;
}

char* findPseudo(char* data, int taille_donnee, int tailleChamp) {
    char* pseudo = (char*) malloc((taille_donnee - tailleChamp) * sizeof (char));
    int i = 0;
    
    while ((i < taille_donnee - tailleChamp - 1)&&(data[i + tailleChamp + 1] != ' ')) {
        *(pseudo + i) = data[i + tailleChamp + 1];
        i++;
    }
    *(pseudo + i) = '\0';
    return pseudo;
}

char* findPseudoAfterName(char* data, int taille_donnee) {
    return findPseudo(data, taille_donnee, TAILLENOM);
}


char* findPseudoAfterPrivate(char* data, int taille_data) {
    return findPseudo(data, taille_data, TAILLEPRIVATE);
}

char* findPseudoAfterBanir(char* data, int taille_data) {
    return findPseudo(data, taille_data, TAILLEBANIR);
}

int imprimer_pseudo(char* pseudo) {
    int i = 0;
    
    while (*(pseudo + i) != '\0') {
        fprintf(stderr, "%c", *(pseudo + i));
        i++;
    }
    fprintf(stderr, "\n");
    return EXIT_SUCCESS;
}