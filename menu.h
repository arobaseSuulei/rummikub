#ifndef MENU_H
#define MENU_H

#include "joueur.h"
#include "manipulation.h"

void afficher_menu_principal(void);
int choisir_option_menu(void);
void executer_option(int choix, Joueur* j);
void executer_boucle_jeu(Joueur* joueurs, int nb_joueurs);

#endif