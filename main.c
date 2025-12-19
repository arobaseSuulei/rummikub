
#include <stdio.h>
//#include "struct.h"
#include "Tuile.h"




int main(){

     

  Tuile t[104];
  int nb_tuiles;

  initialiser_tuile(t,&nb_tuiles); 
  afficher_tuiles(t,nb_tuiles);

    

    return 0;
}