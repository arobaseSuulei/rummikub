#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tuile.h"
#include "table.h"
#include "joueur.h"

int main(void) {

     /*creer_pioche();          // crée pioche.json
     distribuer_tuile();      // crée X.json + distribue 14 tuiles
  */
    Joueur j;
    strcpy(j.chevalet, "1.json");  // tester sur le joueur 2

    // Charger toutes les infos du joueur (pseudo, tour, premier_tour) depuis le fichier
    charger_joueur(&j);


     //creer_table(); // table de jeu, tuiles posées par les joueurs dans table.json
    jouer_combinaison(&j);  // <-- passer l'adresse


    // **Test : afficher les infos chargées**
    printf("\nPseudo      : %s\n", j.pseudo);
printf("Chevalet    : %s\n", j.chevalet);
printf("Tour actif  : %d\n", j.tour);
printf("Premier tour: %d\n", j.premier_tour);


    return 0;
}
