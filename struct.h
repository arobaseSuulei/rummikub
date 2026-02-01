#ifndef STRUCT_H // IL NOUS DIT SI STRUCT_H N'A PAS ENCORE CRéER alors
#define STRUCT_H // alors on le créée, le struct_h c'est une sorte de drapeau fake variable

#include <stdio.h>
#include <stdbool.h>

#define MAX_TUILES 106

typedef struct Tuile{
    int id;
    int valeur;     // 1..13, 0 = joker
    char couleur;   // 'B','R','O','V','J'
    bool joker;
} Tuile;

typedef struct Joueur{
    char pseudo[23];  // max 22 caractères + '\0'
    char chevalet[50];
    bool tour;
    bool premier_tour;
} Joueur;




//typedef struct Plateau{
    //Tuile chevalet;
    //int taille_combinaison;

    // si il y'a un teste teste
//}Plateau;



#endif

