#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "_struct.h"
#include "_Tuile.h"
#include "_joueur.h"
#include "_table.h"

/*-------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------*/
void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles) {
    *nb_tuiles = 0;
    FILE* f = fopen(fichier,"r");
    if(!f) return;
    
    fseek(f,0,SEEK_END);
    long fsize = ftell(f);
    fseek(f,0,SEEK_SET);
    char* data = malloc(fsize+1);
    fread(data,1,fsize,f);
    data[fsize] = 0;
    fclose(f);
    
    cJSON *root = cJSON_Parse(data);
    if(!root){ free(data); return; }
    
    cJSON *tuiles_array = cJSON_GetObjectItem(root,"tuiles");
    int count = cJSON_GetArraySize(tuiles_array);
    for(int i=0;i<count;i++){
        cJSON *item = cJSON_GetArrayItem(tuiles_array,i);
        tuiles[*nb_tuiles].id = cJSON_GetObjectItem(item,"id")->valueint;
        tuiles[*nb_tuiles].valeur = cJSON_GetObjectItem(item,"valeur")->valueint;
        tuiles[*nb_tuiles].couleur = cJSON_GetObjectItem(item,"couleur")->valuestring[0];
        tuiles[*nb_tuiles].joker = cJSON_IsTrue(cJSON_GetObjectItem(item,"joker"));
        (*nb_tuiles)++;
    }
    
    free(data);
    cJSON_Delete(root);
}

/*-------------------------------------------------------------------*/
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles, bool premier_tour_valide) {
    cJSON *root = cJSON_CreateObject();
    if (!root) return;
    
    cJSON_AddStringToObject(root, "pseudo", j.pseudo);
    cJSON_AddBoolToObject(root, "tour", false);
    cJSON_AddBoolToObject(root, "premier_tour", premier_tour_valide ? false : j.premier_tour);
    
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
    
    // CHANGEMENT ICI
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

/*-------------------------------------------------------------------*/
Joueur* creer_joueur(int nb_joueurs, char pseudos[][23]) {
    Joueur *players = malloc(nb_joueurs * sizeof(Joueur));
    if (!players){
        printf("pas assez de memoire\n");
        exit(1);
    }
    
    for (int i = 0; i < nb_joueurs; i++){
        strcpy(players[i].pseudo, pseudos[i]);
    }
    
    ordre_joueur(players, nb_joueurs);
    
    for (int i = 0; i < nb_joueurs; i++){
        char filename[50];
        sprintf(filename,"%d.json", i+1);
        strcpy(players[i].chevalet, filename);
    }
    
    return players;
}

/*-------------------------------------------------------------------*/
void ajouter_a_table(Tuile* comb, int n) {
    if (n < 3) return;
    
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
    
    if(!root) root = cJSON_CreateArray();
    
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
    
    // CHANGEMENT ICI
    char *str = cJSON_Print(root);
    FILE* fw = fopen("table.json","w");
    if(fw){ fprintf(fw,"%s",str); fclose(fw); }
    free(str);
    cJSON_Delete(root);
}

/*-------------------------------------------------------------------*/
bool jouer_combinaison(Joueur* j, int ids[], int nb_ids) {
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    bool premier_tour = est_premier_tour(*j);
    static int somme_cumulee = 0;
    static Tuile toutes_tuiles_jouees[MAX_TUILES];
    static int nb_toutes_tuiles_jouees = 0;
    
    // Premier tour: initialisation
    if(premier_tour && somme_cumulee == 0){
        cJSON *empty_array = cJSON_CreateArray();
        char *str = cJSON_Print(empty_array);
        FILE* ft = fopen("tampon.json","w");
        if(ft){ fprintf(ft,"%s",str); fclose(ft); }
        free(str);
        cJSON_Delete(empty_array);
    }
    
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    // Vérifier les IDs donnés
    if(nb_ids < 3) return false;
    
    int indices[MAX_TUILES], nb_comb = 0;
    for(int k = 0; k < nb_ids; k++){
        for(int i = 0; i < nb_tuiles; i++){
            if(tuiles_joueur[i].id == ids[k]){
                indices[nb_comb++] = i;
                break;
            }
        }
    }
    
    if(nb_comb < 3) return false;
    
    // Former la combinaison
    Tuile comb[MAX_TUILES];
    for(int i = 0; i < nb_comb; i++) comb[i] = tuiles_joueur[indices[i]];
    
    if(!combinaison_valide(comb, nb_comb)) return false;
    
    if(premier_tour){
        // Calcul des points
        int max_val = 0;
        for(int i = 0; i < nb_comb; i++) 
            if(!comb[i].joker && comb[i].valeur > max_val) 
                max_val = comb[i].valeur;
        
        int somme = 0;
        for(int i = 0; i < nb_comb; i++) 
            somme += comb[i].joker ? max_val : comb[i].valeur;
        
        somme_cumulee += somme;
        
        // Stocker pour plus tard
        for(int i = 0; i < nb_comb; i++){
            toutes_tuiles_jouees[nb_toutes_tuiles_jouees++] = comb[i];
        }
        
        // Ajouter au tampon
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
        cJSON_Delete(tampon);
        
        // L'interface décide si on continue ou valide
        return true; // Combinaison acceptée (attente validation finale)
    }
    else {
        // Tour normal: ajouter directement à la table
        ajouter_a_table(comb, nb_comb);
        
        // Mettre à jour le chevalet
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
        
        sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, false);
        return true;
    }
}

// Nouvelle fonction pour valider le premier tour
bool valider_premier_tour(Joueur* j) {
    static int somme_cumulee = 0;
    static Tuile toutes_tuiles_jouees[MAX_TUILES];
    static int nb_toutes_tuiles_jouees = 0;
    
    if(somme_cumulee < 30) return false;
    
    // Transfert du tampon vers la table
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
    
    // Mettre à jour le chevalet
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
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
    
    sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, true);
    
    // Réinitialiser pour le prochain joueur
    somme_cumulee = 0;
    nb_toutes_tuiles_jouees = 0;
    
    return true;
}

// Fonction pour annuler le premier tour
void annuler_premier_tour(void) {
    // Réinitialiser les statiques
    static int somme_cumulee = 0;
    static int nb_toutes_tuiles_jouees = 0;
    somme_cumulee = 0;
    nb_toutes_tuiles_jouees = 0;
    
    // Supprimer le fichier tampon
    remove("tampon.json");
}
/*-------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------*/
void charger_joueur(Joueur* j) {
    j->tour = false;
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
        j->pseudo[22] = '\0';
    }
    
    cJSON *tour = cJSON_GetObjectItem(json, "tour");
    if (tour) j->tour = cJSON_IsTrue(tour);
    
    cJSON *premier = cJSON_GetObjectItem(json, "premier_tour");
    if (premier) j->premier_tour = cJSON_IsTrue(premier);
    
    cJSON_Delete(json);
}

void passer_tour(Joueur* j, Joueur* next) {
    // 1. Mettre tour=false pour j
    FILE* f = fopen(j->chevalet, "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* content = malloc(fsize + 1);
        fread(content, 1, fsize, f);
        content[fsize] = 0;
        fclose(f);
        
        cJSON* root = cJSON_Parse(content);
        free(content);
        if (root) {
            cJSON* tour = cJSON_GetObjectItem(root, "tour");
            if (tour) cJSON_SetBoolValue(tour, false);
            
            // CHANGEMENT ICI (premier)
            char* new_json = cJSON_Print(root);
            f = fopen(j->chevalet, "w");
            if (f) {
                fprintf(f, "%s", new_json);
                fclose(f);
            }
            free(new_json);
            cJSON_Delete(root);
        }
    }
    
    // 2. Mettre tour=true pour next
    if (next) {
        f = fopen(next->chevalet, "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* content = malloc(fsize + 1);
            fread(content, 1, fsize, f);
            content[fsize] = 0;
            fclose(f);
            
            cJSON* root = cJSON_Parse(content);
            free(content);
            if (root) {
                cJSON* tour = cJSON_GetObjectItem(root, "tour");
                if (tour) cJSON_SetBoolValue(tour, true);
                
                // CHANGEMENT ICI (deuxième)
                char* new_json = cJSON_Print(root);
                f = fopen(next->chevalet, "w");
                if (f) {
                    fprintf(f, "%s", new_json);
                    fclose(f);
                }
                free(new_json);
                cJSON_Delete(root);
            }
        }
    }
}

bool a_fini(Joueur j) {
    Tuile tuiles[MAX_TUILES];
    int nb_tuiles = 0;
    
    charger_chevalet(j.chevalet, tuiles, &nb_tuiles);
    
    // Si le joueur a 0 tuiles, il a fini
    return (nb_tuiles == 0);
}

void sauvegarder_scores(Joueur* players, int nb_joueurs, int index_gagnant) {
    cJSON *root = cJSON_CreateObject();
    
    // Calculer les scores
    int score_total_gagnant = 0;
    
    for (int i = 0; i < nb_joueurs; i++) {
        Tuile tuiles[MAX_TUILES];
        int nb_tuiles = 0;
        charger_chevalet(players[i].chevalet, tuiles, &nb_tuiles);
        
        int score = 0;
        for (int k = 0; k < nb_tuiles; k++) {
            score += tuiles[k].joker ? 30 : tuiles[k].valeur;
        }
        
        if (i == index_gagnant) {
            // Le gagnant: somme des scores des autres
            int somme_autres = 0;
            for (int j = 0; j < nb_joueurs; j++) {
                if (j != index_gagnant) {
                    Tuile tuiles_autre[MAX_TUILES];
                    int nb_tuiles_autre = 0;
                    charger_chevalet(players[j].chevalet, tuiles_autre, &nb_tuiles_autre);
                    
                    for (int k = 0; k < nb_tuiles_autre; k++) {
                        somme_autres += tuiles_autre[k].joker ? 30 : tuiles_autre[k].valeur;
                    }
                }
            }
            cJSON_AddNumberToObject(root, players[i].pseudo, somme_autres);
            score_total_gagnant = somme_autres;
        } else {
            // Perdants: leur score en négatif
            cJSON_AddNumberToObject(root, players[i].pseudo, -score);
        }
    }
    
    // Sauvegarder dans fichier
    char *json_str = cJSON_Print(root);
    FILE *f = fopen("scores.json", "w");
    if (f) {
        fprintf(f, "%s", json_str);
        fclose(f);
    }
    free(json_str);
    cJSON_Delete(root);
}