#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include <stdlib.h>

int nbr_joueur(){

    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);

    return n;
}




Joueur* joueur_create(int *nb){


    *nb = nbr_joueur();

    // On alloue un tableau de Joueurs de taille nb
    Joueur *players = malloc((*nb) * sizeof(Joueur));

    

    for (int i=0;i<*nb;i++){
        printf("------PLAYER %d-------",i+1);
        printf("|                    |");
        printf("|                    |");
        printf("|                    |");
        printf("|                    |");
        printf("| Pseudo le plus cool : \n|");
        scanf("%s",players[i].pseudo);

    }


    return players;
}
