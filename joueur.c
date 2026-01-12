#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*-----------------------------------------------------------------------------------------------------------------------------*/
//fonction qui demande et renvoie le nbr de joueurs
int nbr_joueur(){
    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);
    FILE* f = fopen("nombre_joueurs.txt", "w");
    fprintf(f, "%d", n);
    fclose(f);
    return n;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
// Mélange la liste des joueurs (Fisher-Yates) et renvoie le même pointeur
Joueur* ordre_joueur(Joueur* players, int nb_joueurs) {
    srand(time(NULL));
    for (int i = nb_joueurs - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Joueur tmp = players[i];
        players[i] = players[j];
        players[j] = tmp;
    }
    return players;
}


/*-------------------------------------------------------------------------------------------------------------------------------*/
// création du joueur
Joueur* creer_joueur(int* nb_joueurs){
    int nb = nbr_joueur();
    *nb_joueurs = nb;

    Joueur *players = malloc(nb * sizeof(Joueur));
    if (!players){
        printf("pas assez de memoire\n");
        exit(1);
    }

    // Entrer les pseudos
    for (int i = 0; i < nb; i++){
        printf("Joueur %d: ", i+1);
        scanf("%49s", players[i].pseudo);
    }

    //  Mélanger les joueurs pour déterminer l'ordre
    ordre_joueur(players, nb);

    // Créer les fichiers selon l'ordre mélangé
    for (int i = 0; i < nb; i++){
        char filename[50];
        sprintf(filename,"%d.json", i+1);

        strcpy(players[i].chevalet, filename);
    }

    return players;
}


/*-------------------------------------------------------------*/
void distribuer_tuile() {
    int nb_joueurs;
    Joueur* players = creer_joueur(&nb_joueurs);
    if (!players) return;

    // créer fichiers joueurs avec pseudo et tour
    for (int j = 0; j < nb_joueurs; j++) {
        FILE* fj = fopen(players[j].chevalet, "w");
        if (!fj) {
            perror("Erreur création fichier joueur");
            continue;
        }
        fprintf(fj, "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"tuiles\": []\n}\n",
                players[j].pseudo, (j == 0) ? "true" : "false");
        fclose(fj);
    }

    // distribuer 14 tuiles à chaque joueur
    for (int i = 0; i < 14; i++)
        for (int j = 0; j < nb_joueurs; j++)
            piocher_tuile(players[j]);

    free(players);
}

/*-------------------------------------------------------------*/
void piocher_tuile(Joueur j) {
    Tuile pioche[MAX_TUILES], tuiles_joueur[MAX_TUILES];
    int np = 0, nj = 0;
    char ligne[256];
    char pseudo[50];
    int tour = 0;

    // --- lire pioche ---
    FILE* f = fopen("pioche.json", "r");
    if (!f) return;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strchr(ligne, '{')) {
            sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                   &pioche[np].valeur, &pioche[np].couleur);
            pioche[np].joker = strstr(ligne, "true") != NULL;
            np++;
        }
    }
    fclose(f);
    if (np == 0) return;

    Tuile t = pioche[0];

    // --- réécrire pioche sans la première tuile ---
    f = fopen("pioche.json", "w");
    fprintf(f, "[\n");
    for (int i = 1; i < np; i++)
        fprintf(f, "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                pioche[i].valeur, pioche[i].couleur,
                pioche[i].joker ? "true" : "false",
                (i < np - 1) ? "," : "");
    fprintf(f, "]\n");
    fclose(f);

    // --- lire joueur existant ---
    FILE* fj = fopen(j.chevalet, "r");
    if (!fj) return;

    // lire pseudo, tour, tuiles existantes
    nj = 0;
    while (fgets(ligne, sizeof(ligne), fj)) {
        if (sscanf(ligne, " \"pseudo\": \"%49[^\"]\"", pseudo) == 1) continue;
        if (strstr(ligne, "\"tour\": true")) tour = 1;
        else if (strstr(ligne, "\"tour\": false")) tour = 0;
        if (strchr(ligne, '{') && strstr(ligne, "valeur")) {
            sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                   &tuiles_joueur[nj].valeur, &tuiles_joueur[nj].couleur);
            tuiles_joueur[nj].joker = strstr(ligne, "true") != NULL;
            nj++;
        }
    }
    fclose(fj);

    // ajouter la nouvelle tuile
    tuiles_joueur[nj++] = t;

    // --- réécrire joueur proprement ---
    fj = fopen(j.chevalet, "w");
    fprintf(fj, "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"tuiles\": [\n",
            pseudo, tour ? "true" : "false");
    for (int i = 0; i < nj; i++)
        fprintf(fj, "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                tuiles_joueur[i].valeur,
                tuiles_joueur[i].couleur,
                tuiles_joueur[i].joker ? "true" : "false",
                (i < nj - 1) ? "," : "");
    fprintf(fj, "  ]\n}\n");
    fclose(fj);
}
