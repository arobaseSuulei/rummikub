#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include <stdlib.h>
#include <string.h>

int nbr_joueur(){

    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);

    return n;
}



// création du joueur
Joueur* joueur_create(int *nb){

    char filename[50];
    

    *nb = nbr_joueur();

    // On alloue un tableau de Joueurs de taille nb
    Joueur *players = malloc((*nb) * sizeof(Joueur));
    

    if (players==NULL){
        printf("pas assez de memoire");
        exit(1);
    }

    

    for (int i=0;i<*nb;i++){
        printf("------PLAYER %d-------",i+1);
        printf("|                    |");
        printf("|                    |");
        printf("|                    |");
        printf("|                    |");
        printf("| Pseudo le plus cool : \n|");
        scanf("%s",players[i].pseudo);

        // son fichier pour stocker ses tuiles

        sprintf(filename,"joueur%d.json",i); // on crée le fichier d'ou le s, du coup on crée joueur0.json,joueu1.json etc
        
        FILE *f=fopen(filename,"w"); //

        fprintf(f, "{ \"pseudo\": \"%s\" }\n", players[i].pseudo); // on écrit dans le fichier

        

        strcpy(players[i].chevalet, filename);

        // écrire les tuiles dans le fichier

        

        fclose(f);
        }
   


    


    return players;
}





