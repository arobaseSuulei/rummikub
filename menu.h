#ifndef MENU_H
#define MENU_H

#include "joueur.h"

/* Menu principal */
void afficher_menu_principal(void);
int choisir_option_menu(void);
bool executer_option(int choix, Joueur* j);  

/* Menu de manipulation */
bool menu_manipulation(Joueur* j); 

#endif