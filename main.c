#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tuile.h"
#include "table.h"
#include "joueur.h"

int main(void) {

    creer_pioche();          // crée pioche.json
    distribuer_tuile();      // crée X.json + distribue 14 tuiles

    Joueur j;
    strcpy(j.chevalet, "1.json");  // tester sur le joueur 1

    piocher_tuile(j);        /*pioche 1 tuile → ajoutée à 1.json (c'est pour tester
     la fonctionnalité piocher une tuile, à mettre en commentaire après)*/

    creer_table();//c'est la table de jeu, celle où sont posées les tuiles des joueurs dans le fichier table.json
    jouer_combinaison(j);

    return 0;
}
