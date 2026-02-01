#ifndef MENU_H
#define MENU_H

#include "joueur.h"

void afficher_menu_principal(void);
int choisir_option_menu(void);
void executer_option(int choix, Joueur* j);
void menu_manipulation(Joueur* j);
#endif