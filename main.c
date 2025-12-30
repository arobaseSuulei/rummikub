
#include <stdio.h>
//#include "struct.h"
#include "Tuile.h"
#include <stdlib.h> 
#include "joueur.h"




int main(){

     

  Tuile t[104];
  int nb_tuiles;

  
  int nb_joueurs;

  initialiser_tuile(t,&nb_tuiles); 
  //afficher_tuiles(t,nb_tuiles);
  stocker_tuile(t, nb_tuiles);



    
    // Crée le tableau dynamiquement
    Joueur *players = joueur_create(&nb_joueurs);
    
    // Maintenant tu as :
    // - players : ton tableau de joueurs
    // - nb_joueurs : le nombre exact de joueurs
    
    printf("\nVous avez %d joueurs:\n", nb_joueurs);
    for(int i = 0; i < nb_joueurs; i++){
        printf("- %s\n", players[i].pseudo);
    }
    
    // Libère la mémoire
    free(players);
    

    return 0;
}