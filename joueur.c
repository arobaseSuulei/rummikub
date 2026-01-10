#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include "Tuile.h"
#include <stdlib.h>
#include <string.h>

//fonction qui demande et renvoie le nbr de joueurs
int nbr_joueur(){
    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);
    return n;
}


// création du joueur
Joueur* creer_joueur(int* nb_joueurs){
    char filename[50];
    int nb = nbr_joueur();
    *nb_joueurs = nb;

    Joueur *players = malloc(nb * sizeof(Joueur));
    if (!players){
        printf("pas assez de memoire\n");
        exit(1);
    }

    for (int i = 0; i < nb; i++){
        printf("Joueur %d: ", i+1);
        scanf("%49s", players[i].pseudo); // limite à 49 caractères

        sprintf(filename,"joueur%d.json",i);
        FILE *f = fopen(filename,"w");
        fprintf(f, "{ \"pseudo\": \"%s\" }\n", players[i].pseudo);
        fclose(f);

        strcpy(players[i].chevalet, filename);
    }

    return players;
}


/*-------------------------------------------------------------------------------------------------- */
void distribuer_tuile() {
    int nb_joueurs;
    Joueur* players = creer_joueur(&nb_joueurs); // nb_joueurs rempli ici
    if (!players) return;

    // ouvrir pioche.json
    FILE* fpioche = fopen("pioche.json", "r");
    if (!fpioche) {
        perror("Erreur ouverture pioche.json");
        free(players);
        return;
    }

    // lire toutes les tuiles de pioche.json
    Tuile pioche[MAX_TUILES];
    int index = 0;
    char ligne[200];

    while (fgets(ligne, sizeof(ligne), fpioche) && index < MAX_TUILES) {
        // ignorer crochets ou lignes vides
        if (strchr(ligne, '{') == NULL) continue;

        int valeur;
        char couleur;
        bool joker;

        // parser manuellement la ligne
        if (sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%5s}", 
                   &valeur, &couleur, ligne) == 3) {
            // enlever la virgule ou le \n
            char* p = strchr(ligne, ',');
            if (p) *p = '\0';
            p = strchr(ligne, '\n');
            if (p) *p = '\0';

            // détecter joker
            if (strncmp(ligne, "true", 4) == 0)
                joker = true;
            else
                joker = false;

            pioche[index++] = (Tuile){valeur, couleur, joker};
        }
    }
    fclose(fpioche);

    int tuiles_par_joueur = 14;
    int pioche_index = 0;

    // distribuer les tuiles à chaque joueur
    for (int i = 0; i < nb_joueurs; i++) {
        if (pioche_index + tuiles_par_joueur > index) {
            printf("Pas assez de tuiles dans la pioche pour tous les joueurs!\n");
            break;
        }

        FILE* fjoueur = fopen(players[i].chevalet, "w");
        if (!fjoueur) {
            perror("Erreur ouverture fichier joueur");
            continue;
        }

        fprintf(fjoueur, "[\n");
        for (int j = 0; j < tuiles_par_joueur; j++) {
            Tuile t = pioche[pioche_index++];
            fprintf(fjoueur, "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                    t.valeur, t.couleur, t.joker ? "true" : "false",
                    (j < tuiles_par_joueur - 1) ? "," : "");
        }
        fprintf(fjoueur, "]\n");
        fclose(fjoueur);
    }

    // réécrire pioche.json avec les tuiles restantes
    fpioche = fopen("pioche.json", "w");
    if (!fpioche) {
        perror("Erreur réécriture pioche.json");
        free(players);
        return;
    }

    fprintf(fpioche, "[\n");
    for (int i = pioche_index; i < index; i++) {
        fprintf(fpioche, "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                pioche[i].valeur,
                pioche[i].couleur,
                pioche[i].joker ? "true" : "false",
                (i < index - 1) ? "," : "");
    }
    fprintf(fpioche, "]\n");
    fclose(fpioche);

    free(players);
}
