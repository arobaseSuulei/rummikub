// menu.h
#ifndef MENU_H
#define MENU_H

#include "joueur.h"  // Seulement ce qui est nécessaire pour les déclarations

void afficher_menu_principal(void);
int choisir_option_menu(void);
void executer_option(int choix, Joueur* j);
void executer_boucle_jeu(Joueur* joueurs, int nb_joueurs);

#endif