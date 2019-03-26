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
char NIVEAUETUDE[4][12]= {"Mclassique", "Malternance", "Licence"};
                 

struct paramsUser {
    char * pseudo;
};

void appli(){
    printf("********************************************************************\n");
    printf("*                                                                  *\n");
    printf("*     Bienvenue sur Réseaux Chat des MIAGISTES de L'université     *\n");
    printf("*                                                                  *\n");
    printf("********************************************************************\n");
  
}

int envoi_message(int sock_serv, char data[], int tailleDonnee) {

    if (strstr(data, "/name")) {
        data[tailleDonnee - 1] = '\0';
        tailleDonnee--;
    } else if (strstr(data, "/quit")) {
        write(sock_serv, data, tailleDonnee);
        close(sock_serv);
        exit(EXIT_SUCCESS);
    }

    write(sock_serv, data, tailleDonnee);
    return EXIT_SUCCESS;
}

struct paramsUser choose_pseudo_connexion(char * pseudo, int sock_serv,  char * niveauEtude) {
    struct paramsUser user;
    char request[TAILLEPSEUDO + TAILLENOM + 1] = "/name ";
    char name[TAILLEPSEUDO] = "";

    strcat(pseudo, "@");
    strcat(pseudo, niveauEtude);

    strncat(request, pseudo, TAILLEPSEUDO);


    if (write(sock_serv, request, strlen(request)) != strlen(request))
        perror("write name failed");

    user.pseudo = strncat(name, pseudo, TAILLEPSEUDO); 

    if (strlen(pseudo) > TAILLEPSEUDO) {
        fprintf(stderr, "pseudo trop long, nouveau pseudo: %s\n", user.pseudo);
    }

    return user;
}

int routine_client(int sock_serv) {
    fd_set readfds;
    int resSelect;
    int resReadClient = 0;
    int resReadServ = 0;
    char dataServ[NBRECARACTERE];
    char dataClie[NBRECARACTERE];


    while (TRUE) {

        FD_ZERO(&readfds);
        FD_SET(1, &readfds);
        FD_SET(sock_serv, &readfds); 

        resSelect = select(sock_serv + 1, &readfds, NULL, NULL, NULL);

        switch (resSelect) {
            case 0:
                break;
            case -1:
                perror("select failed");
                break;
            default:
                if (FD_ISSET(1, &readfds)) {

                    resReadClient = read(1, dataClie, NBRECARACTERE);

                    if (resReadClient >= 2)
                        envoi_message(sock_serv, dataClie, resReadClient);
                }


                if (FD_ISSET(sock_serv, &readfds)) {


                    if ((resReadServ = read(sock_serv, dataServ, NBRECARACTERE)) < 0) {
                        perror("read failed in client");
                        break;
                    }
                    if(resReadServ == 0){
                        fprintf(stderr,"le serveur a quitté...\n fin du programme client\n");
                        close(sock_serv);
                        exit(0);
                    }
                    write(1, dataServ, resReadServ);
                }
        }
    }
}

int connect_serv(int port, char* ip) {
    int sock_client;
    struct hostent* hp;
    struct sockaddr_in adresse_serv;


    if ((hp = gethostbyname(ip)) == NULL) {
        perror("hostname");
        exit(1);
    }
    // on place l'adresse de la machine dans la structure de la socket du serveur
    bcopy(hp->h_addr, &adresse_serv.sin_addr, hp->h_length);
    adresse_serv.sin_family = hp->h_addrtype;

    adresse_serv.sin_port = htons(port);


    if ((sock_client = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(sock_client, (struct sockaddr *) &adresse_serv, sizeof (adresse_serv)) < 0) {
        perror("connect");
        close(sock_client);
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "connexion avec le serveur établie\n");


    return sock_client;
}

int main(int argc, char* argv[]) {
    int c;
    char* ip = "127.0.0.1";
    int port = 9999;
    char* name = "Anonyme";
    char* niveauEtude="Masterclassique";
    int sock;
    int trouve=0;
    struct paramsUser params;

    while ((c = getopt(argc, argv, "a:p:n:e:")) != -1)
        switch (c) {
            case 'a':
                ip = optarg;
                break;
                
            case 'p':
                port = atoi(optarg);
                break;
                
            case 'n':
                name = optarg;
                break;
                
            case 'e':
                niveauEtude = optarg;
                int i;
                for(i=0; i<4; i++){
             // On verifie si l'entreprise fait partie du groupe 
                if (strcmp(niveauEtude,NIVEAUETUDE[i]) == 0)
                {
                  trouve=1;
                }
                }               
                if(trouve != 1){                 
                  printf("L'niveau étude saisie ne fait pas partie du groupe !!\n");
                  return 0;                  
                }
                break;
                
            default:
                printf("./serveur [-pa]\n");
                printf("-p : numero de port\n");
                printf("-a : adresse ip du serveur\n");
                printf("-n : nom de l'utilisateur\n");
                printf("-e : Niveau étude \n");
                break;
        }
    printf("                             \n");
    printf("                             \n");
    printf("Adresse ip serveur   --->: %s \n", ip);
    printf("Numero de port       --->: %d \n", port);
    printf("Votre Nom            --->: %s \n", name);
    printf("Niveau étude         --->: %s \n", niveauEtude);
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    appli();

    sock = connect_serv(port, ip);
    params = choose_pseudo_connexion(name, sock, niveauEtude);
    routine_client(sock);
    return 1;
}
