#include <stdio.h>
#include <stdbool.h>
#include "struct.h"



void initialiser_tuile(Tuile t[],int *nb_tuiles){

    int index=0;
    int val,coul;


    for(int exemplaire=0;exemplaire<2;exemplaire++) {
        for(int valeur=0;valeur<=13;valeur++){
            for(int couleur=0;couleur<=3;couleur++){


            t[index].valeur=valeur;
            t[index].couleur=couleur;


            

            index++;
        }


    }
    } 

    *nb_tuiles=index;
        


}




void afficher_tuiles(Tuile t[], int nb_tuiles){
    const char *couleurs[] = {"J", "R", "N", "B"};
    
    for(int i = 0; i < nb_tuiles; i++){
        printf("%d[%s%d]\n ", i,couleurs[t[i].couleur], t[i].valeur);
    }
    printf("\n");
}
