#ifndef STRUCT_H // IL NOUS DIT SI STRUCT_H N'A PAS ENCORE CRéER alors
#define STRUCT_H // alors on le créée, le struct_h c'est une sorte de drapeau fake variable

#include <stdio.h>
#include <stdbool.h>

typedef struct Tuile{
    int valeur;     // 1..13, 0 = joker
    char couleur;   // 'B','R','O','V','J'
    bool joker;
} Tuile;

typedef struct Joueur{
    char pseudo[23];  // max 22 caractères + '\0'
    int estIa;        // 0 = humain, 1 = IA
    char chevalet[50];
} Joueur;




//typedef struct Plateau{
    //Tuile chevalet;
    //int taille_combinaison;

    // si il y'a un teste teste
//}Plateau;



#endif

