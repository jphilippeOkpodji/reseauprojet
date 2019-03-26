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
char * conception_message(char* message1, char* message2);
char * conception_message_n(char* message1, char* message2, int n);
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
int recherchePseudo(char* pseudo, Liste_Clients* liste);
char* recherchePseudoAncien(int id_sock, Liste_Clients* liste);
char* rechercheSocket(int id_sock, Liste_Clients* liste);
int ajouterChanger(int id_sock, char* pseudo, Liste_Clients* liste);
int supprimer(int id_sock, Liste_Clients* liste);
int imprimer_liste_connecter(Liste_Clients* liste);
char* message_list(Liste_Clients* liste);


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

/*
 * Cette fonction va permettre de concatener des messages
 */

char * conception_message(char* message1, char* message2) {
    char * message = (char*) malloc((strlen(message1) + strlen(message2) + 1) * sizeof (char));
    int i = 0, j = 0;
    
    while (*(message1 + i) != '\0') {
        *(message + i) = *(message1 + i);
        i++;
    }
    
    while (*(message2 + j) != '\0') {
        *(message + i) = *(message2 + j);
        i++;
        j++;
    }
    *(message + i) = '\0';
    return message;
}

/*
 * cette fonction fait la même chose que conception_message mais ne prend que les n premiers bit de m2
 */

char * conception_message_n(char* message1, char* message2, int n) {
    char * message = (char*) malloc((strlen(message1) + n + 1) * sizeof (char));
    int i = 0, j = 0;
    
    while (*(message1 + i) != '\0') {
        *(message + i) = *(message1 + i);
        i++;
    }
    
    while ((*(message2 + j) != '\0')&&(j < n)) {
        *(message + i) = *(message2 + j);
        i++;
        j++;
    }
    *(message + i) = '\0';
    return message;
}

/*
 * pseudoExist renvoi 2 si le pseudo existe déjà 1 sinon
 */

int pseudoExist(char* pseudo, Liste_Clients* liste) {
    Case* pointeur = liste->beginp;
    
    while ((pointeur != NULL)&&(strcmp(pointeur->pseudo, pseudo) != 0)) {
        
        pointeur = pointeur->nextp;
    }
    
    if (pointeur == NULL)
        return EXIT_PSEUDO_NOT_EXIST;
    else return EXIT_PSEUDO_EXIST;
}

/*
 * recherchePseudo retourne l'id de socket associé au pseudo passé en paramêtre
 * retourne 0 si le pseudo n'a pas été trouvé
 */

int recherchePseudo(char* pseudo, Liste_Clients* liste) {
    Case* pointeur = liste->beginp;
    
    while ((pointeur != NULL)&&(strcmp(pointeur->pseudo, pseudo) != 0)) {
        
        pointeur = pointeur->nextp;
        
    }
    
    if (pointeur == NULL)
        return 0;
    else return pointeur->id_sock;
}

/*
 * rechercheSocket retourne le pseudo associé à l'id de socket
 * retourne NULL sinon
 */
char* rechercheSocket(int id_sock, Liste_Clients* liste) {
    Case* p = liste->beginp;
    
    while ((p != NULL)&&(p->id_sock != id_sock)) {
        p = p->nextp;
    }
    
    if (p == NULL)
        return NULL;
    else return p->pseudo;
}

/*
 * fait la même chose que rechercheSocket mais retourne le pseudo ancien.
 * On supposera que cette fonction ne sera appeler quand cas de changement de pseudo
 */

char* recherchePseudoAncien(int id_sock, Liste_Clients* liste) {
    Case* p = liste->beginp;
    
    while ((p != NULL)&&(p->id_sock != id_sock)) {
        p = p->nextp;
    }
    
    if (p == NULL)
        return NULL;
    else return p->pseudo_ancien;
}

int ajouter_pseudo(int id_sock, char* pseudo, Liste_Clients* liste) {
    
    Case* newCase = (Case *) malloc(sizeof (Case));
    
    newCase->id_sock = id_sock;
    newCase->type = USER;
    
    newCase->pseudo = (char*) malloc((strlen(pseudo) + 1) * sizeof (char));
    strcpy(newCase->pseudo, pseudo);
    newCase->nextp = NULL;
    newCase->pseudo_ancien = NULL;
    
    if (liste->nb_joueur == 0) {
        liste->beginp = liste->lastp = newCase;
    } else {
        liste->lastp = liste->lastp->nextp = newCase;
    }
    liste->nb_joueur++;
    return EXIT_SUCCESS;
    
}

int ajouterChanger(int id_sock, char* pseudo, Liste_Clients* liste) {
    Case* p = liste->beginp;
    
    if (pseudoExist(pseudo, liste) == EXIT_PSEUDO_NOT_EXIST) {
        while ((p != NULL)&&(p->id_sock != id_sock)) {
            p = p->nextp;
        }
        
        if (p == NULL) {
            ajouter_pseudo(id_sock, pseudo, liste);
            return EXIT_PSEUDO_ADD;
        }
        free(p->pseudo_ancien);
        p->pseudo_ancien = (char*) malloc((strlen(p->pseudo) + 1) * sizeof (char));
        strcpy(p->pseudo_ancien, p->pseudo);
        free(p->pseudo);
        p->pseudo = (char*) malloc((strlen(pseudo) + 1) * sizeof (char));
        strcpy(p->pseudo, pseudo);
        return EXIT_PSEUDO_CHANGE;
        
    } else
        return EXIT_PSEUDO_EXIST;
}

int supprimer(int id_sock, Liste_Clients* liste) {
    Case* pointeur = liste->beginp;
    
    if (liste->beginp == NULL) {
        perror("liste vide");
        return EXIT_FAILURE;
    }
    
    if (liste->nb_joueur == 1) {
        if (pointeur->id_sock == id_sock) {
            free(pointeur->pseudo);
            free(pointeur->pseudo_ancien);
            liste->beginp = liste->lastp = NULL;
            liste->nb_joueur = 0;
            return EXIT_SUCCESS;
        } else
            return EXIT_FAILURE;
        
    } else {
        
        if (pointeur->id_sock == id_sock) {
            liste->beginp = pointeur->nextp;
            free(pointeur->pseudo);
            free(pointeur->pseudo_ancien);
            free(pointeur);
            liste->nb_joueur--;
            return EXIT_SUCCESS;
        } else {
            
            Case* pred = pointeur;
            pointeur = pointeur->nextp;
            
            while ((pointeur != NULL)&&(pointeur->id_sock != id_sock)) {
                pointeur = pointeur->nextp;
                pred = pred->nextp;
            }
            if (pointeur == NULL)
                return EXIT_FAILURE;
            else {
                pred->nextp = pointeur->nextp;
                if (pointeur == liste->lastp)
                    liste->lastp = pred;
                free(pointeur->pseudo);
                free(pointeur->pseudo_ancien);
                free(pointeur);
                liste->nb_joueur--;
                return EXIT_SUCCESS;
            }
        }
    }
}

int imprimer_liste_connecter(Liste_Clients* liste) {
    Case* p = liste->beginp;
    
    while (p != NULL) {
        fprintf(stderr, "le client de nom %s est connecté et il a l'id socket = %d\n", p->pseudo, p->id_sock);
        p = p->nextp;
    }
    
    return EXIT_SUCCESS;
}

/*
 * va mettre la liste des joueurs connectés dans le data qui sera ensuite envoyé à celui qui a demandé la liste des joueurs
 */

char* message_list(Liste_Clients* liste) {
    char* data = "liste des personnes connectées :\n";
    Case* p = liste->beginp;
    
    while (p != NULL) {
        data = conception_message(data, p->pseudo);
        data = conception_message(data, ", ");
        p = p->nextp;
    }
    
    data = conception_message(data,"\n");
    
    return data;
}

char* menu(){
    
    char *data = "*************************************************************************************************";
    data = "**********************************|    MENU CHAT MIAGISTE UNIVERSITY  |********************************";
    data = conception_message(data,"\n");
    char *data1 = "* /menu pour afficher le menu utilisateur                                                      *";
    data = conception_message(data, data1);
    data = conception_message(data,"\n");
    char *data2 = "* /liste afficher la liste des utilisateurs connectes                                           *";
    data = conception_message(data, data2);
    data = conception_message(data,"\n");
    char *data3 = "* /prive pour envoyer un message prive a un autre utilisateur  : /prive [pseudo] [message] *" ;
    data = conception_message(data, data3);
    data = conception_message(data,"\n");
    char *data4 = "* /quitter pour quiter le chat                                                                     *" ;
    data = conception_message(data, data4);
    data = conception_message(data,"\n");
    char *data5 = "* Taper un message dans le terminal pour l'envoyer à tous le monde                              *" ;
    data = conception_message(data, data5);
    data = conception_message(data,"\n");
    char *data6 = "* /admin pour se connecter en tant qu'admin : /admin [pseudo_admin]                             *" ;
    data = conception_message(data, data6);
    data = conception_message(data,"\n");
    char *data7 = "* /name changer de pseudo";
    data = conception_message(data, data7);
    data = conception_message(data, "\n");
    return data;
}






int envoyer_message_tous(int sock_courante, char data[], int sock_serv, int nbre_descr_sock, int taille_data, Liste_Clients* liste) {
    int i = 1;

    for (i = 1; i <= nbre_descr_sock; i++) {
        if ((i != sock_courante)&&(i != sock_serv)&&(rechercheSocket(i, liste) != NULL)) {
            fprintf(stderr, "messages envoyé au client %d\n", i);
            if (write(i, data, taille_data) != taille_data)
                perror("erreur d'envoi de message");
        }
    }

    return EXIT_SUCCESS;
}

int envoyer_message_unique(int sock_courante, char data[], int sock_serv, int nbre_descr_sock, int taille_donne) {

    if (write(sock_courante, data, taille_donne) != taille_donne)
        perror("erreur d'envoi de message");

    return EXIT_SUCCESS;
}

int gestion_message(int sock_courante, char data[], int sock_serv, int* nbre_descr_sock, int taille_data, fd_set* save_fds, Liste_Clients* liste) {

    int res = 0;
    int taille_new_data;
    char* new_data;
    char* pseudo;
    char* pseudo_ancien;

    if (strstr(data, "/name")) {
        pseudo = findPseudoAfterName(data, taille_data);
        res = ajouterChanger(sock_courante, pseudo, liste);
        switch (res) {
            case (EXIT_PSEUDO_EXIST):
                fprintf(stderr, "le pseudo %s est pris par un autre client\n", pseudo);
                new_data = conception_message("ce pseudo est déjà utilisé", "\n");
                taille_new_data = strlen(new_data);
                envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
                imprimer_liste_connecter(liste);
                break;
            case(EXIT_PSEUDO_CHANGE):
                pseudo_ancien = recherchePseudoAncien(sock_courante, liste);
                new_data = conception_message(pseudo_ancien, " s'appelle maintenant ");
                new_data = conception_message(new_data, pseudo);
                new_data = conception_message(new_data, "\n");
                taille_new_data = strlen(new_data);
                envoyer_message_tous(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data, liste);
                fprintf(stderr, "le client %d a changé de pseudo, il s'appel maintenant: %s\n", sock_courante, pseudo);
                imprimer_liste_connecter(liste);
                break;
            case(EXIT_PSEUDO_ADD):
                fprintf(stderr, "le client %d vient de prendre le pseudo %s\n", sock_courante, pseudo);
                imprimer_liste_connecter(liste);
                new_data = conception_message(pseudo, " vient de se connecter\n");
                taille_new_data = strlen(new_data);
                envoyer_message_tous(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data, liste);
                break;
            default:
                fprintf(stderr, "cas d'ajout ou de changement de pseudo non géré");
                break;
        }


    } else if (strstr(data, "/liste")) {
        new_data = message_list(liste);
        taille_new_data = strlen(new_data);
        fprintf(stderr, "le client %d demande la liste des connectés\n", sock_courante);
        envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
    }
    else if (strstr(data,"/menu")){
        new_data = menu();
        taille_new_data = strlen(new_data);
        envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
    }
    
    
    else if (strstr(data, "/quitter")) {
        if ((pseudo = rechercheSocket(sock_courante, liste)) != NULL) {
            new_data = conception_message(pseudo, " s'est déconnecté\n");
            taille_new_data = strlen(new_data);
            envoyer_message_tous(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data, liste);
        }

        close(sock_courante);
        supprimer(sock_courante, liste);
        fprintf(stderr, "la connexion du client %d a été fermée\n", sock_courante);
        nb_connecte --;
        fprintf(stderr, "Nombre de clients connectés : %d\n", nb_connecte);
        FD_CLR(sock_courante, save_fds);
        if (sock_courante == *nbre_descr_sock) {
            while (!(FD_ISSET(*nbre_descr_sock, save_fds)))
                *nbre_descr_sock = *(nbre_descr_sock) - 1;
        }
    }
    else {
        if ((pseudo = rechercheSocket(sock_courante, liste)) == NULL) {
            new_data = conception_message("vous ne pouvez pas parler sur ce channel, vous n'avez pas encore pris de pseudo", "\n");
            taille_new_data = strlen(new_data);
            fprintf(stderr, "###ce client ne peut pas parler car il n'a pas pris un pseudo valide###\n");
            envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
            }else if (strstr(data, "/admin")) {
                new_data = "**********************************|    MENU ADMINISTRATEUR    |***********************************";
                new_data = conception_message(new_data,"\n");
                char *data1 = "* /statut pour modifier le statut                                                                *";
                new_data = conception_message(new_data, data1);
                new_data = conception_message(new_data,"\n");
                char *data2 = "* /banir pour banir un user : /banir [pseudo_a_banir] [message_banissement]                       *";
                new_data = conception_message(new_data, data2);
                new_data = conception_message(new_data,"\n");
                char *data3 = "**************************************************************************************************";
                new_data = conception_message(new_data, data3);
                new_data = conception_message(new_data,"\n");            
                taille_new_data = strlen(new_data);
                fprintf(stderr, "###Connexion d'un administrateur###\n");
                envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
        }  else if (strstr(data, "/prive")) {
            new_data = conception_message("[mp de ", pseudo);
            new_data = conception_message(new_data, "] ");

            pseudo = findPseudoAfterPrivate(data, taille_data);
            if ((res = recherchePseudo(pseudo, liste)) == 0) {
                new_data = conception_message("le pseudo ", pseudo);
                new_data = conception_message(new_data, " n'existe pas\n");
                taille_new_data = strlen(new_data);
                fprintf(stderr, "###ce client essaye de parler à une personne non enregistrée###\n");
                envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
            } else {
                char* message = trouverMessage(data, taille_data, strlen(pseudo) + TAILLEPRIVATE + 2);
                fprintf(stderr, "%s\n", message);
                new_data = conception_message(new_data, message);
                taille_new_data = strlen(new_data);
                fprintf(stderr, "message de taille %d envoyé au client %d de la part du client %d\n", taille_new_data, res, sock_courante);
                envoyer_message_unique(res, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
            }

        }else if (strstr(data, "/banir")) {
            new_data = conception_message("Vous avez été banni!!!\n", " ");

            pseudo = findPseudoAfterBanir(data, taille_data);
            if ((res = recherchePseudo(pseudo, liste)) == 0) {
                new_data = conception_message("le pseudo ", pseudo);
                new_data = conception_message(new_data, " n'existe pas\n");
                taille_new_data = strlen(new_data);
                fprintf(stderr, "###l'administrateur essaye de bannir une personne non enregistrée###\n");
                envoyer_message_unique(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
                
                    
            } else {
                fprintf(stderr, "message de taille %d envoyé au client %d de la part du client %d\n", taille_new_data, res, sock_courante);
                envoyer_message_unique(res, new_data, sock_serv, *nbre_descr_sock, taille_new_data);
         close(res);
        supprimer(res, liste);
        nb_connecte --;
        fprintf(stderr, "Nombre de clients connectés : %d\n", nb_connecte);
        fprintf(stderr, "la connexion du client %d a été fermée\n", res);
        FD_CLR(res, save_fds);
        if (res == *nbre_descr_sock) {
            while (!(FD_ISSET(*nbre_descr_sock, save_fds)))
                *nbre_descr_sock = *(nbre_descr_sock) - 1;
            }  
            }

        } else {
            new_data = conception_message("[", pseudo);
            new_data = conception_message(new_data, "] ");
            new_data = conception_message_n(new_data, data, taille_data);
            //data[taille_data] = '\0';
            taille_new_data = strlen(new_data);
            envoyer_message_tous(sock_courante, new_data, sock_serv, *nbre_descr_sock, taille_new_data, liste);
        }
    }

    return EXIT_SUCCESS;
}

int creation_serv(char* ip, int port) {
    int ma_sock, client_sock;
    socklen_t* taille_sock;
    struct sockaddr_in sock_addr_serv, sock_addr_client;
    /* nbre_descr nombre -1 de descripteur à surveiller
     * readfds pointeur vers un masque binaire qui indique les descripteurs de sockets à rechercher en entrée
     */
    int nbre_descr_sock = 0;
    int nbre_sock_restant = 0;
    int resSelect = 0;
    int resRead = 0;
    int res = 0;
    int i = 0;
    fd_set readfds, save_fds; // obligé de faire une sauvegarde des fd sinon on le met à jour en temps réel --> ça va planter
    char data[NBROCTETLU];
    Liste_Clients* liste = init();

    taille_sock = (socklen_t*) malloc(sizeof (sock_addr_client));
    *taille_sock = sizeof (sock_addr_client);

    /*
     * on crée la structure de notre socket
     */

    sock_addr_serv.sin_family = AF_INET; // protocole tcp/ip
    sock_addr_serv.sin_port = htons(port);
    sock_addr_serv.sin_addr.s_addr = inet_addr(ip);

    //on crée notre socket

    if ((ma_sock = socket(AF_INET, SOCK_STREAM, PF_UNSPEC)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //on lie notre socket à notre structure

    if ((bind(ma_sock, (struct sockaddr *) &sock_addr_serv, sizeof (sock_addr_serv))) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    //on place le serveur en écoute passive
    listen(ma_sock, NBRCONNEXION);

    FD_ZERO(&readfds);
    FD_ZERO(&save_fds);
    FD_SET(ma_sock, &save_fds);
    FD_SET(1, &save_fds);
    nbre_descr_sock = ma_sock;

    while (TRUE) {
        FD_ZERO(&readfds);
        bcopy(&save_fds, &readfds, sizeof (save_fds));
        resSelect = select(nbre_descr_sock + 1, &readfds, NULL, NULL, NULL);

        switch (resSelect) {
            case 0:
                break;
            case -1:
                perror("select failed");
                break;
            default:
                nbre_sock_restant = resSelect;
                for (i = 1; (i <= nbre_descr_sock)&&(nbre_sock_restant > 0); i++) {
                    if (FD_ISSET(i, &readfds)) { // si la socket est prête
                        /*
                         * traitement quand on est sur le socket du serveur --> il faudra à tous les coups faire du parallélisme
                         */

                        if (i == 1) { // commande taper sur le réseau
                            if((resRead = read(i, data, NBROCTETLU))<0){
                                perror("failed read in network");
                                break;
                            }
                            
                            if(strstr(data, "/quitter")){
                                int j = 0;
                                fprintf(stderr,"le serveur est en train de quitter...\n fermeture des connexions\n");
                                while(j<=nbre_descr_sock){
                                    close(j);
                                    j++;
                                }
                                exit(0);
                            }
                            
                        } else if (i == ma_sock) {
                            fprintf(stderr, "en attente de connexion de client...\n");
                            if ((client_sock = accept(ma_sock, (struct sockaddr *) &sock_addr_client, taille_sock)) < 0) {
                                perror("accept failed");
                                break;
                            }
                            
                            if (nb_connecte == NBCONNEXIONMAX) {
                                perror("Connexions max atteint");
                                 close(client_sock);
                                 supprimer(client_sock, liste);
                                break;
                            }
                            
                            nb_connecte ++;

                            fprintf(stderr, "connexion d'un client d'id socket :%d accepté\n", client_sock);
                            fprintf(stderr, "Nombre de clients connectés : %d\n", nb_connecte);
                            FD_SET(client_sock, &save_fds);
                            if (client_sock > nbre_descr_sock)
                                nbre_descr_sock = client_sock;

                        } else { // on traite cette fois un socket d'un client 
                            resRead = read(i, data, NBROCTETLU);

                            if (resRead < 0) { // erreur 
                                perror("read failed");
                                break;
                            }
                            if (resRead == 0) { // connexion fermée dût à un crashage 
                                fprintf(stderr, "la connexion du client %d a été fermée\n", i);
                                res = supprimer(i, liste);
                                if (res == EXIT_FAILURE)
                                    fprintf(stderr, "ce client n'était pas enregistrer dans la liste\n");
                                else
                                    fprintf(stderr, "ce client a bien été supprimé de la liste\n");
                                close(i);
                                FD_CLR(i, &save_fds);

                                // mise à jour du max de descripteur 
                                if (i == nbre_descr_sock) {
                                    while (!(FD_ISSET(nbre_descr_sock, &save_fds)))
                                        nbre_descr_sock--;
                                }

                            } else { // on a reçu des messages de la part d'un client pour le moment on envoi à tous les clients les messages
                                fprintf(stderr, "le serveur a reçu un message de %d bits de la part du client %d\n", resRead, i);
                                gestion_message(i, data, ma_sock, &nbre_descr_sock, resRead, &save_fds, liste);
                            }
                        }
                        nbre_sock_restant--;
                    }
                }
                break;
        }


    }

    close(ma_sock);
    exit(EXIT_SUCCESS);

}











int main(int argc, char *argv[]) {
    int c;
    char* ip = "127.0.0.1";
    int port = 9999;
    while ((c = getopt(argc, argv, "a:p:")) != -1)
        switch (c) {
            case 'a':
                ip = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            default:
                printf("./serveur [-pa]\n");
                printf("-p : numero de port\n");
                printf("-a : adresse ip du serveur\n");
                break;
        }

    printf("adresse ip : %s \n", ip);
    printf("port : %d \n", port);

    creation_serv(ip, port);


    return EXIT_SUCCESS;
}
