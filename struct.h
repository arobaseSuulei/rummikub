#ifndef STRUCT_H 
#define STRUCT_H 

#include <stdio.h>
#include <stdbool.h>

#define MAX_TUILES 106

typedef struct Tuile{
    int id;
    int valeur;     
    char couleur;   
    bool joker;
} Tuile;

typedef struct Joueur{
    char pseudo[23];  
    char chevalet[50];
    bool tour;
    bool premier_tour;
} Joueur;

#endif

