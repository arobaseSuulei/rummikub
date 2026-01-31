#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"

/*-----------------------------------------------------------------------------------------------------------------------------*/
int nbr_joueur(){
    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);

    cJSON *root = cJSON_CreateNumber(n);
    char *str = cJSON_Print(root);
    FILE* f = fopen("nombre_joueurs.txt", "w");
    if(f) { fprintf(f, "%s", str); fclose(f); }
    cJSON_Delete(root);
    free(str);

    return n;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
Joueur* ordre_joueur(Joueur* players, int nb_joueurs) {
    srand(time(NULL));
    for (int i = nb_joueurs - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Joueur tmp = players[i];
        players[i] = players[j];
        players[j] = tmp;
    }
    return players;
}

/*-------------------------------------------------------------------------------------------------------------------------------*/
Joueur* creer_joueur(int* nb_joueurs){
    int nb = nbr_joueur();
    *nb_joueurs = nb;

    Joueur *players = malloc(nb * sizeof(Joueur));
    if (!players){
        printf("pas assez de memoire\n");
        exit(1);
    }

    for (int i = 0; i < nb; i++){
        printf("Joueur %d: ", i+1);
        scanf("%49s", players[i].pseudo);
        getchar();
    }

    ordre_joueur(players, nb);

    for (int i = 0; i < nb; i++){
        char filename[50];
        sprintf(filename,"%d.json", i+1);
        strcpy(players[i].chevalet, filename);
    }

    return players;
}

/*---------------------------------------------------------------------------------------------------------*/
void distribuer_tuile() {
    int nb_joueurs;
    Joueur* players = creer_joueur(&nb_joueurs);
    if (!players) return;

    for (int j = 0; j < nb_joueurs; j++) {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "pseudo", players[j].pseudo);
        cJSON_AddBoolToObject(root, "tour", (j==0));
        cJSON_AddBoolToObject(root, "premier_tour", true);
        cJSON_AddItemToObject(root, "tuiles", cJSON_CreateArray());

        char *str = cJSON_Print(root);
        FILE* fj = fopen(players[j].chevalet, "w");
        if(fj){ fprintf(fj, "%s", str); fclose(fj); }
        cJSON_Delete(root);
        free(str);
    }

    for (int i = 0; i < 14; i++)
        for (int j = 0; j < nb_joueurs; j++)
            piocher_tuile(&players[j]);

    free(players);
}

/*------------------------------------------------------------------------------------------------------------*/
void piocher_tuile(Joueur* j) {
    FILE* f = fopen("pioche.json", "r");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = malloc(fsize+1);
    fread(data, 1, fsize, f);
    data[fsize] = 0;
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    if (!root) { free(data); return; }

    cJSON *first = cJSON_GetArrayItem(root, 0);
    if(!first){ cJSON_Delete(root); free(data); return; }

    Tuile t;
    t.id = cJSON_GetObjectItem(first,"id")->valueint;
    t.valeur = cJSON_GetObjectItem(first,"valeur")->valueint;
    t.couleur = cJSON_GetObjectItem(first,"couleur")->valuestring[0];
    t.joker = cJSON_IsTrue(cJSON_GetObjectItem(first,"joker"));

    cJSON_DeleteItemFromArray(root,0);
    char *new_data = cJSON_Print(root);
    FILE* fw = fopen("pioche.json","w");
    if(fw){ fprintf(fw,"%s",new_data); fclose(fw); }
    free(new_data);
    free(data);
    cJSON_Delete(root);

    f = fopen(j->chevalet,"r");
    if(!f) return;
    fseek(f,0,SEEK_END);
    fsize = ftell(f);
    fseek(f,0,SEEK_SET);
    data = malloc(fsize+1);
    fread(data,1,fsize,f);
    data[fsize] = 0;
    fclose(f);

    root = cJSON_Parse(data);
    if(!root){ free(data); return; }

    cJSON *tuiles_obj = cJSON_GetObjectItem(root,"tuiles");
    cJSON *new_tuile = cJSON_CreateObject();
    cJSON_AddNumberToObject(new_tuile,"id",t.id);
    cJSON_AddNumberToObject(new_tuile,"valeur",t.valeur);
    char str_color[2]={t.couleur,0};
    cJSON_AddStringToObject(new_tuile,"couleur",str_color);
    cJSON_AddBoolToObject(new_tuile,"joker",t.joker);
    cJSON_AddItemToArray(tuiles_obj,new_tuile);

    new_data = cJSON_Print(root);
    fw = fopen(j->chevalet,"w");
    if(fw){ fprintf(fw,"%s",new_data); fclose(fw); }
    free(new_data);
    free(data);
    cJSON_Delete(root);
}
/*------------------------------------------------------------------------------------------------------------------------------*/
bool combinaison_valide(Tuile* tuiles, int nb) {
    if (nb < 3) return false;

    int jokers = 0;
    for (int i = 0; i < nb; i++)
        if (tuiles[i].joker) jokers++;

    // === VÉRIFICATION BRELAN ===
    int valeur_set = -1;
    bool possible_set = true;
    char couleurs[4];
    int couleur_count = 0;

    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        
        if (valeur_set == -1) {
            valeur_set = tuiles[i].valeur;
        } else if (tuiles[i].valeur != valeur_set) { 
            possible_set = false; 
            break; 
        }
        
        // Vérifier doublon de couleur
        bool couleur_deja_presente = false;
        for (int j = 0; j < couleur_count; j++) {
            if (couleurs[j] == tuiles[i].couleur) {
                couleur_deja_presente = true;
                break;
            }
        }
        
        if (couleur_deja_presente) {
            possible_set = false;
            break;
        }
        
        // Ajouter la couleur si pas déjà présente
        if (couleur_count < 4) {
            couleurs[couleur_count++] = tuiles[i].couleur;
        } else {
            possible_set = false; // Plus de 4 couleurs différentes
            break;
        }
    }

    // Si c'est un brelan possible, vérifier le nombre total de tuiles
    if (possible_set) {
        // Un brelan ne peut pas avoir plus de 4 tuiles au total
        // (3-4 tuiles normales + éventuellement jokers)
        if (nb <= 4) {
            return true;
        }
        // Si plus de 4 tuiles, ce n'est pas un brelan valide
        possible_set = false;
    }

    // === VÉRIFICATION SUITE ===
    // Si pas assez de tuiles non-jokers pour une suite
    if (nb - jokers < 2) return false;

    char couleur_suite = '\0';
    int valeurs[nb - jokers];
    int v_idx = 0;
    
    // Première passe : collecter les valeurs non-jokers
    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        
        if (couleur_suite == '\0') {
            couleur_suite = tuiles[i].couleur;
        } else if (tuiles[i].couleur != couleur_suite) {
            return false; // Couleurs différentes
        }
        
        valeurs[v_idx++] = tuiles[i].valeur;
    }

    // Trier les valeurs
    for (int i = 0; i < v_idx - 1; i++) {
        for (int j = i + 1; j < v_idx; j++) {
            if (valeurs[i] > valeurs[j]) {
                int tmp = valeurs[i];
                valeurs[i] = valeurs[j];
                valeurs[j] = tmp;
            }
        }
    }

    // Vérifier les doublons
    for (int i = 0; i < v_idx - 1; i++) {
        if (valeurs[i] == valeurs[i + 1]) return false;
    }

    // === NOUVELLE VÉRIFICATION : Position des jokers ===
    // Créer un tableau complet avec les positions des jokers
    int positions_jokers[jokers];
    int joker_idx = 0;
    
    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) {
            positions_jokers[joker_idx++] = i;
        }
    }
    
    // Pour chaque joker, vérifier qu'il ne crée pas de problème
    for (int j = 0; j < jokers; j++) {
        int pos_joker = positions_jokers[j];
        
        // Trouver la valeur précédente non-joker
        int val_prev = -1;
        for (int i = pos_joker - 1; i >= 0; i--) {
            if (!tuiles[i].joker) {
                val_prev = tuiles[i].valeur;
                break;
            }
        }
        
        // Trouver la valeur suivante non-joker
        int val_next = -1;
        for (int i = pos_joker + 1; i < nb; i++) {
            if (!tuiles[i].joker) {
                val_next = tuiles[i].valeur;
                break;
            }
        }
        
        // Vérification des cas problématiques
        if (val_prev == -1 && val_next == -1) {
            // Que des jokers
            continue;
        }
        
        if (val_prev == -1) {
            // Joker en première position
            if (val_next == 1) return false; // Joker avant un 1
        }
        
        if (val_next == -1) {
            // Joker en dernière position
            if (val_prev == 13) return false; // Joker après un 13
        }
        
        if (val_prev != -1 && val_next != -1) {
            // Joker entre deux valeurs
            if (val_next - val_prev == 1) return false; // Entre deux consécutifs
        }
    }

    // Calculer les "trous" dans la suite
    int gaps = 0;
    for (int i = 0; i < v_idx - 1; i++) {
        gaps += valeurs[i + 1] - valeurs[i] - 1;
    }

    // Les jokers peuvent combler les trous
    return (gaps <= jokers);
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/
void afficher_tuiles(Tuile tuiles[], int nb_tuiles) {
    printf("Vos tuiles :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("%d: %d%c%s\n",
               tuiles[i].id,
               tuiles[i].valeur,
               tuiles[i].couleur,
               tuiles[i].joker ? " (J)" : "");
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------*/
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles) {
    *nb_tuiles=0;
    FILE* f=fopen(fichier,"r");
    if(!f) return;
    fseek(f,0,SEEK_END);
    long fsize=ftell(f);
    fseek(f,0,SEEK_SET);
    char* data=malloc(fsize+1);
    fread(data,1,fsize,f);
    data[fsize]=0;
    fclose(f);

    cJSON *root=cJSON_Parse(data);
    if(!root){ free(data); return; }

    cJSON *tuiles_array=cJSON_GetObjectItem(root,"tuiles");
    int count=cJSON_GetArraySize(tuiles_array);
    for(int i=0;i<count;i++){
        cJSON *item=cJSON_GetArrayItem(tuiles_array,i);
        tuiles[*nb_tuiles].id=cJSON_GetObjectItem(item,"id")->valueint;
        tuiles[*nb_tuiles].valeur=cJSON_GetObjectItem(item,"valeur")->valueint;
        tuiles[*nb_tuiles].couleur=cJSON_GetObjectItem(item,"couleur")->valuestring[0];
        tuiles[*nb_tuiles].joker=cJSON_IsTrue(cJSON_GetObjectItem(item,"joker"));
        (*nb_tuiles)++;
    }

    free(data);
    cJSON_Delete(root);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) return;

    // pseudo
    cJSON_AddStringToObject(root, "pseudo", j.pseudo);

    // tour = false à la fin du tour, toujours
    cJSON_AddBoolToObject(root, "tour", false);

    // premier_tour = false si validé, sinon on conserve l'ancienne valeur
    cJSON_AddBoolToObject(root, "premier_tour", premier_tour_valide ? false : j.premier_tour);

    // tuiles
    cJSON *array = cJSON_CreateArray();
    for (int i = 0; i < nb_tuiles; i++) {
        cJSON *tile = cJSON_CreateObject();
        cJSON_AddNumberToObject(tile, "id", tuiles[i].id);
        cJSON_AddNumberToObject(tile, "valeur", tuiles[i].valeur);
        char couleur_str[2] = { tuiles[i].couleur, '\0' };
        cJSON_AddStringToObject(tile, "couleur", couleur_str);
        cJSON_AddBoolToObject(tile, "joker", tuiles[i].joker);
        cJSON_AddItemToArray(array, tile);
    }
    cJSON_AddItemToObject(root, "tuiles", array);

    // écrire dans le fichier
    char *json_str = cJSON_Print(root);
    if (json_str) {
        FILE *f = fopen(fichier, "w");
        if (f) {
            fprintf(f, "%s\n", json_str);
            fclose(f);
        }
        free(json_str);
    }

    cJSON_Delete(root);
}


/*---------------------------------------------------------------------------------------------------------------------------*/
void ajouter_a_table(Tuile* comb, int n)
{
    if (n < 3) return;

    // Charger table existante
    cJSON *root = NULL;
    FILE* f = fopen("table.json", "r");
    if(f){
        fseek(f,0,SEEK_END);
        long fsize = ftell(f);
        fseek(f,0,SEEK_SET);
        char *data = malloc(fsize+1);
        fread(data,1,fsize,f);
        data[fsize]=0;
        fclose(f);

        root = cJSON_Parse(data);
        free(data);
    }

    if(!root) root=cJSON_CreateArray();

    cJSON *new_comb = cJSON_CreateArray();
    for(int i=0;i<n;i++){
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item,"id",comb[i].id);
        cJSON_AddNumberToObject(item,"valeur",comb[i].valeur);
        char str_color[2]={comb[i].couleur,0};
        cJSON_AddStringToObject(item,"couleur",str_color);
        cJSON_AddBoolToObject(item,"joker",comb[i].joker);
        cJSON_AddItemToArray(new_comb,item);
    }
    cJSON_AddItemToArray(root,new_comb);

    char *str = cJSON_Print(root);
    FILE* fw = fopen("table.json","w");
    if(fw){ fprintf(fw,"%s",str); fclose(fw); }
    free(str);
    cJSON_Delete(root);
}

/*---------------------------------------------------------------------------------------------------------------------------*/
bool est_premier_tour(Joueur j) {
    FILE* f = fopen(j.chevalet,"r");
    if(!f) return true;

    fseek(f,0,SEEK_END);
    long fsize = ftell(f);
    fseek(f,0,SEEK_SET);
    char *data = malloc(fsize+1);
    fread(data,1,fsize,f);
    data[fsize]=0;
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    free(data);
    if(!root) return true;

    cJSON *premier = cJSON_GetObjectItem(root,"premier_tour");
    bool result = premier && cJSON_IsTrue(premier);
    cJSON_Delete(root);
    return result;
}

/*---------------------------------------------------------------------------------------------------------------------------*/
void charger_joueur(Joueur* j) {
    j->tour = false;        // valeurs par défaut
    j->premier_tour = false;
    j->pseudo[0] = '\0';

    FILE* f = fopen(j->chevalet, "r");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(fsize + 1);
    if (!content) { fclose(f); return; }
    fread(content, 1, fsize, f);
    content[fsize] = '\0';
    fclose(f);

    cJSON *json = cJSON_Parse(content);
    free(content);
    if (!json) return;

    cJSON *pseudo = cJSON_GetObjectItem(json, "pseudo");
    if (pseudo && pseudo->valuestring) {
        strncpy(j->pseudo, pseudo->valuestring, 22);
        j->pseudo[22] = '\0'; // garantir terminaison
    }

    cJSON *tour = cJSON_GetObjectItem(json, "tour");
    if (tour) j->tour = cJSON_IsTrue(tour);

    cJSON *premier = cJSON_GetObjectItem(json, "premier_tour");
    if (premier) j->premier_tour = cJSON_IsTrue(premier);

    cJSON_Delete(json);
}

/*---------------------------------------------------------------------------------------------------------------------------*/
void jouer_combinaison(Joueur* j)
{
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    bool premier_tour = est_premier_tour(*j);
    int somme_cumulee = 0;
    
    // Variable pour stocker toutes les tuiles jouées pendant le premier tour
    Tuile toutes_tuiles_jouees[MAX_TUILES];
    int nb_toutes_tuiles_jouees = 0;

    if(premier_tour){
        cJSON *empty_array = cJSON_CreateArray();
        char *str = cJSON_Print(empty_array);
        FILE* ft = fopen("tampon.json","w");
        if(ft){ fprintf(ft,"%s",str); fclose(ft); }
        free(str);
        cJSON_Delete(empty_array);
        printf(">>> Premier tour : au moins 30 points.\n");
    }

    char ligne[256];
    while(1){
        charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
        afficher_tuiles(tuiles_joueur, nb_tuiles);

        int indices[MAX_TUILES], nb_comb=0;
        printf("Entrez les IDs des tuiles à poser (min 3) :\n");
        if(!fgets(ligne,sizeof(ligne),stdin)) break;

        char *tok = strtok(ligne," \n");
        while(tok){
            int id = atoi(tok);
            for(int i=0;i<nb_tuiles;i++)
                if(tuiles_joueur[i].id==id) indices[nb_comb++]=i;
            tok=strtok(NULL," \n");
        }

        if(nb_comb<3){ printf("Au moins 3 tuiles.\n"); continue; }

        Tuile comb[MAX_TUILES];
        for(int i=0;i<nb_comb;i++) comb[i]=tuiles_joueur[indices[i]];

        if(!combinaison_valide(comb, nb_comb)){ printf("Combinaison invalide.\n"); continue; }

        if(premier_tour){
            int somme=0, max_val=0;
            for(int i=0;i<nb_comb;i++) if(!comb[i].joker && comb[i].valeur>max_val) max_val=comb[i].valeur;
            for(int i=0;i<nb_comb;i++) somme+=comb[i].joker? max_val : comb[i].valeur;
            somme_cumulee += somme;

            // Stocker ces tuiles jouées
            for(int i=0;i<nb_comb;i++){
                toutes_tuiles_jouees[nb_toutes_tuiles_jouees++] = comb[i];
            }

            // Charger tampon.json
            cJSON *tampon = NULL;
            FILE* ft = fopen("tampon.json","r");
            if(ft){
                fseek(ft,0,SEEK_END);
                long fsize = ftell(ft);
                fseek(ft,0,SEEK_SET);
                char *data = malloc(fsize+1);
                fread(data,1,fsize,ft);
                data[fsize]=0;
                fclose(ft);
                tampon = cJSON_Parse(data);
                free(data);
            }
            if(!tampon) tampon = cJSON_CreateArray();

            cJSON *new_comb = cJSON_CreateArray();
            for(int i=0;i<nb_comb;i++){
                cJSON *item = cJSON_CreateObject();
                cJSON_AddNumberToObject(item,"id",comb[i].id);
                cJSON_AddNumberToObject(item,"valeur",comb[i].valeur);
                char str_color[2]={comb[i].couleur,0};
                cJSON_AddStringToObject(item,"couleur",str_color);
                cJSON_AddBoolToObject(item,"joker",comb[i].joker);
                cJSON_AddItemToArray(new_comb,item);
            }
            cJSON_AddItemToArray(tampon,new_comb);

            char *str = cJSON_Print(tampon);
            ft = fopen("tampon.json","w");
            if(ft){ fprintf(ft,"%s",str); fclose(ft); }
            free(str);
            
            printf("Somme cumulée = %d\n", somme_cumulee);
            printf("Encore une combinaison ? (o/n) ");
            fgets(ligne,sizeof(ligne),stdin);
            if(ligne[0]=='o'||ligne[0]=='O') continue;

            // VÉRIFICATION FINALE
            if(somme_cumulee<30){
                printf("Premier tour raté. Vous piochez une tuile.\n");
                piocher_tuile(j);
                cJSON_Delete(tampon);
                return;
            }

            // PREMIER TOUR RÉUSSI - retirer TOUTES les tuiles jouées
            // Recharger le chevalet complet
            charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
            
            // Créer la liste des tuiles restantes (en enlevant toutes celles jouées)
            Tuile restantes[MAX_TUILES];
            int nb_restantes = 0;
            
            for(int i=0;i<nb_tuiles;i++){
                bool garder = true;
                for(int k=0;k<nb_toutes_tuiles_jouees;k++){
                    if(tuiles_joueur[i].id == toutes_tuiles_jouees[k].id){
                        garder = false;
                        break;
                    }
                }
                if(garder){
                    restantes[nb_restantes] = tuiles_joueur[i];
                    nb_restantes++;
                }
            }

            // Transférer tampon à table
            FILE* ftam = fopen("tampon.json","r");
            if(ftam){
                fseek(ftam,0,SEEK_END);
                long fsize = ftell(ftam);
                fseek(ftam,0,SEEK_SET);
                char *data = malloc(fsize+1);
                fread(data,1,fsize,ftam);
                data[fsize]=0;
                fclose(ftam);

                cJSON *tampon_data = cJSON_Parse(data);
                free(data);
                if(tampon_data){
                    int n = cJSON_GetArraySize(tampon_data);
                    for(int i=0;i<n;i++){
                        cJSON *comb_json = cJSON_GetArrayItem(tampon_data,i);
                        int m = cJSON_GetArraySize(comb_json);
                        Tuile tmp[MAX_TUILES];
                        for(int k=0;k<m;k++){
                            cJSON *tile = cJSON_GetArrayItem(comb_json,k);
                            tmp[k].id = cJSON_GetObjectItem(tile,"id")->valueint;
                            tmp[k].valeur = cJSON_GetObjectItem(tile,"valeur")->valueint;
                            tmp[k].couleur = cJSON_GetObjectItem(tile,"couleur")->valuestring[0];
                            tmp[k].joker = cJSON_IsTrue(cJSON_GetObjectItem(tile,"joker"));
                        }
                        ajouter_a_table(tmp,m);
                    }
                    cJSON_Delete(tampon_data);
                }
            }

            // Sauvegarder le chevalet FINAL
            sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, true);
            printf("Premier tour validé avec %d points !\n", somme_cumulee);
            printf("Tuiles retirées du chevalet : %d\n", nb_toutes_tuiles_jouees);
            cJSON_Delete(tampon);
            return;
        }

        // TOURS SUIVANTS
        ajouter_a_table(comb,nb_comb);

        // Retirer IMMÉDIATEMENT du chevalet
        Tuile restantes[MAX_TUILES];
        int nb_restantes = 0;
        for(int i=0;i<nb_tuiles;i++){
            bool garder = true;
            for(int k=0;k<nb_comb;k++){
                if(tuiles_joueur[i].id == comb[k].id){
                    garder = false;
                    break;
                }
            }
            if(garder){
                restantes[nb_restantes] = tuiles_joueur[i];
                nb_restantes++;
            }
        }

        // Sauvegarder
        sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, false);
        
        printf("Combinaison posée. %d tuiles retirées.\n", nb_comb);
        printf("Encore une ? (o/n) ");
        fgets(ligne,sizeof(ligne),stdin);
        if(ligne[0]!='o' && ligne[0]!='O') return;
        
        // Recharger pour la prochaine itération
        charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    }
}