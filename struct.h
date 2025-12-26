#ifndef STRUCT_H // IL NOUS DIT SI STRUCT_H N'A PAS ENCORE CRéER alors
#define STRUCT_H // alors on le créée, le struct_h c'est une sorte de drapeau fake variable


#include <stdio.h>
#include <stdbool.h>



typedef struct Joueur{

    char pseudo[23];
    int estIa;
    //Tuile mainJoueur[40];






}Joueur;





typedef struct Tuile{

    

    int valeur;
    int couleur;

}Tuile;



typedef struct Plateau{

    Tuile chevalet;
    //int taille_combinaison; En faut il ?

    


}Plateau;

#endif

