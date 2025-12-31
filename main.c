#include <stdio.h>
#include <stdlib.h>
#include "Tuile.h"
#include "joueur.h"

int main(){

    Tuile t[106];
    int nb_tuiles;

    int nb_joueurs;

    // Initialiser le paquet
    initialiser_tuile(t, &nb_tuiles);

    // Sauvegarder paquet.json si inexistant
    stocker_tuile(t, nb_tuiles, "paquet.json");

    // Créer pioche.json mélangé
    creer_pioche();

    // Crée le tableau dynamique de joueurs
    Joueur *players = joueur_create(&nb_joueurs);

    if(players == NULL){
        fprintf(stderr, "Erreur allocation joueurs\n");
        return 1;
    }

    // Afficher pseudos
    printf("\nVous avez %d joueurs:\n", nb_joueurs);
    for(int i = 0; i < nb_joueurs; i++){
        printf("- %22s\n", players[i].pseudo);
    }

    free(players);
    return 0;
}
