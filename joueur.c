#include <stdio.h>
#include <stdbool.h>
#include "struct.h"
#include "Tuile.h"
#include "joueur.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*-----------------------------------------------------------------------------------------------------------------------------*/
//fonction qui demande et renvoie le nbr de joueurs
int nbr_joueur(){
    int n;
    printf("Entrer le nombre de joueur : ");
    scanf("%d",&n);
    FILE* f = fopen("nombre_joueurs.txt", "w");
    fprintf(f, "%d", n);
    fclose(f);
    return n;
}

/*------------------------------------------------------------------------------------------------------------------------------*/
// Mélange la liste des joueurs (Fisher-Yates) et renvoie le même pointeur
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
// création du joueur
Joueur* creer_joueur(int* nb_joueurs){
    int nb = nbr_joueur();
    *nb_joueurs = nb;

    Joueur *players = malloc(nb * sizeof(Joueur));
    if (!players){
        printf("pas assez de memoire\n");
        exit(1);
    }

    // Entrer les pseudos
    for (int i = 0; i < nb; i++){
        printf("Joueur %d: ", i+1);
        scanf("%49s", players[i].pseudo);
        getchar();  // <- consommer le '\n' restant
    }

    //  Mélanger les joueurs pour déterminer l'ordre
    ordre_joueur(players, nb);

    // Créer les fichiers selon l'ordre mélangé
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
        FILE* fj = fopen(players[j].chevalet, "w");
        if (!fj) continue;

        fprintf(fj,
            "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"premier_tour\": true,\n  \"tuiles\": []\n}\n",
            players[j].pseudo, (j == 0) ? "true" : "false");
        fclose(fj);
    }

    for (int i = 0; i < 14; i++)
        for (int j = 0; j < nb_joueurs; j++)
            piocher_tuile(players[j]);

    free(players);
}

/*------------------------------------------------------------------------------------------------------------*/
void piocher_tuile(Joueur j) {
    Tuile pioche[MAX_TUILES], tuiles_joueur[MAX_TUILES];
    int np = 0, nj = 0;
    char ligne[256];
    char pseudo[50] = "";   // <- initialisé
    int tour = 0;
    bool premier_tour = true;

    // --- lire pioche ---
    FILE* f = fopen("pioche.json", "r");
    if (!f) return;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strchr(ligne, '{')) {
            sscanf(ligne,
                " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                &pioche[np].valeur, &pioche[np].couleur);
            pioche[np].joker = strstr(ligne, "true") != NULL;
            np++;
        }
    }
    fclose(f);
    if (np == 0) return;

    Tuile t = pioche[0];

    // --- réécrire pioche sans la première tuile ---
    f = fopen("pioche.json", "w");
    fprintf(f, "[\n");
    for (int i = 1; i < np; i++)
        fprintf(f,
            "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            pioche[i].valeur, pioche[i].couleur,
            pioche[i].joker ? "true" : "false",
            (i < np - 1) ? "," : "");
    fprintf(f, "]\n");
    fclose(f);

    // --- lire joueur existant ---
    FILE* fj = fopen(j.chevalet, "r");
    if (!fj) return;

    while (fgets(ligne, sizeof(ligne), fj)) {
        if (strstr(ligne, "\"pseudo\"")) {           // <- lecture sûre
            sscanf(ligne, " \"pseudo\": \"%49[^\"]\"", pseudo);
            continue;
        }
        if (strstr(ligne, "\"tour\": true")) tour = 1;
        else if (strstr(ligne, "\"tour\": false")) tour = 0;

        if (strstr(ligne, "\"premier_tour\": false")) premier_tour = false;

        if (strchr(ligne, '{') && strstr(ligne, "valeur")) {
            sscanf(ligne,
                " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                &tuiles_joueur[nj].valeur,
                &tuiles_joueur[nj].couleur);
            tuiles_joueur[nj].joker = strstr(ligne, "true") != NULL;
            nj++;
        }
    }
    fclose(fj);

    // ajouter la nouvelle tuile
    tuiles_joueur[nj++] = t;

    // --- réécrire joueur proprement ---
    fj = fopen(j.chevalet, "w");
    fprintf(fj,
        "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"premier_tour\": %s,\n  \"tuiles\": [\n",
        pseudo,
        tour ? "true" : "false",
        premier_tour ? "true" : "false");

    for (int i = 0; i < nj; i++)
        fprintf(fj,
            "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
            tuiles_joueur[i].valeur,
            tuiles_joueur[i].couleur,
            tuiles_joueur[i].joker ? "true" : "false",
            (i < nj - 1) ? "," : "");

    fprintf(fj, "  ]\n}\n");
    fclose(fj);
}

/*------------------------------------------------------------------------------------------------------------------------------*/
bool combinaison_valide(Tuile* tuiles, int nb) {
    if (nb < 3) return false; // minimum 3 tuiles

    int jokers = 0;
    for (int i = 0; i < nb; i++)
        if (tuiles[i].joker) jokers++;

    // Vérifier si c'est un Set (même valeur)
    int valeur_set = -1;
    bool possible_set = true;
    char couleurs[nb]; // pour vérifier doublons de couleur
    int couleur_count = 0;

    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        if (valeur_set == -1) valeur_set = tuiles[i].valeur;
        else if (tuiles[i].valeur != valeur_set) {
            possible_set = false;
            break;
        }

        // Vérifier doublon couleur
        for (int j = 0; j < couleur_count; j++)
            if (couleurs[j] == tuiles[i].couleur) {
                possible_set = false;
                break;
            }
        couleurs[couleur_count++] = tuiles[i].couleur;
        if (!possible_set) break;
    }

    if (possible_set) return true;

    // Vérifier si c'est une Suite (même couleur, valeurs consécutives)
    possible_set = true;
    char couleur_suite = '\0';
    int valeurs[nb - jokers]; // stocker les valeurs non-joker
    int v_idx = 0;

    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) continue;
        if (couleur_suite == '\0') couleur_suite = tuiles[i].couleur;
        else if (tuiles[i].couleur != couleur_suite) {
            possible_set = false;
            break;
        }
        valeurs[v_idx++] = tuiles[i].valeur;
    }

    if (!possible_set) return false;

    // trier les valeurs pour vérifier la suite
    for (int i = 0; i < v_idx - 1; i++)
        for (int j = i + 1; j < v_idx; j++)
            if (valeurs[i] > valeurs[j]) {
                int tmp = valeurs[i]; valeurs[i] = valeurs[j]; valeurs[j] = tmp;
            }

    int gaps = 0;
    for (int i = 0; i < v_idx - 1; i++)
        gaps += valeurs[i + 1] - valeurs[i] - 1; // compter les écarts

    return (gaps <= jokers); // les jokers comblent les trous
}
/*-------------------------------------------------------------------------------------------------------------------------------------*/
bool combinaison_valide_30(Tuile* tuiles, int nb) {
    if (!combinaison_valide(tuiles, nb))
        return false; // la validité générale est déjà vérifiée

    int somme = 0;
    int jokers = 0;
    int min_val = 100, max_val = 0;

    // Calculer la somme des tuiles normales et trouver min/max
    for (int i = 0; i < nb; i++) {
        if (tuiles[i].joker) {
            jokers++;
        } else {
            somme += tuiles[i].valeur;
            if (tuiles[i].valeur < min_val) min_val = tuiles[i].valeur;
            if (tuiles[i].valeur > max_val) max_val = tuiles[i].valeur;
        }
    }

    // Ajouter les jokers
    if (jokers > 0) {
        // Estimer chaque Joker à la valeur max de la combinaison pour la somme
        somme += jokers * max_val;
    }

    if (somme < 30) {
        printf("Erreur : la somme de la combinaison est %d (<30 points requis pour le premier tour).\n", somme);
        return false;
    }

    return true;
}



/*-------------------------------------------------------------------------------------------------------------------------------------*/

void afficher_tuiles(Tuile tuiles[], int nb_tuiles) {
    printf("Vos tuiles :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("%d: %d%c%s\n",
               i + 1,
               tuiles[i].valeur,
               tuiles[i].couleur,
               tuiles[i].joker ? " (J)" : "");
    }
}

/*--------------------------------------------------------------------------------------------------------------------------------------*/

void charger_chevalet(const char* fichier, Tuile tuiles[], int* nb_tuiles) {
    char ligne[256];
    *nb_tuiles = 0;

    FILE* f = fopen(fichier, "r");
    if (!f) {
        perror("Impossible d'ouvrir le fichier joueur");
        return;
    }

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strchr(ligne, '{') && strstr(ligne, "valeur")) {
            sscanf(ligne,
                " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                &tuiles[*nb_tuiles].valeur,
                &tuiles[*nb_tuiles].couleur);

            tuiles[*nb_tuiles].joker = strstr(ligne, "true") != NULL;
            (*nb_tuiles)++;
        }
    }
    fclose(f);
}

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
void sauvegarder_chevalet(const char* fichier, Joueur j, Tuile tuiles[], int nb_tuiles) {
    FILE* f = fopen(fichier, "w");

    fprintf(f,
        "{\n  \"pseudo\":\"%s\",\n  \"tour\":%s,\n  \"premier_tour\": true,\n  \"tuiles\":[\n",
        j.pseudo, j.tour ? "true" : "false");

    for (int i = 0; i < nb_tuiles; i++) {
        if (i) fprintf(f, ",\n");
        fprintf(f,
            "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}",
            tuiles[i].valeur,
            tuiles[i].couleur,
            tuiles[i].joker ? "true" : "false");
    }

    fprintf(f, "\n  ]\n}\n");
    fclose(f);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
void ajouter_a_table(Tuile* comb, int n) {
    FILE* ft = fopen("table.json", "r+");
    if (!ft) {
        ft = fopen("table.json", "w");
        fprintf(ft, "[\n]\n");
        fclose(ft);
        ft = fopen("table.json", "r+");
    }

    fseek(ft, 0, SEEK_END);
    long size = ftell(ft);
    int empty_table = (size <= 4); // fichier vide
    if (!empty_table) fseek(ft, -2, SEEK_END); // avant le "]\n"
    else fseek(ft, -2, SEEK_END);

    if (!empty_table) fprintf(ft, ",\n");
    fprintf(ft, "  [\n");
    for (int i = 0; i < n; i++) {
        Tuile t = comb[i];
        fprintf(ft, "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                t.valeur, t.couleur, t.joker ? "true" : "false",
                (i < n - 1) ? "," : "");
    }
    fprintf(ft, "  ]\n]\n");
    fclose(ft);
}

/*---------------------------------------------------------------------------------------------------------------------------------------*/
bool est_premier_tour(Joueur j) {
    FILE* f = fopen(j.chevalet, "r");
    if (!f) return true; // par défaut

    char ligne[256];
    bool premier_tour = true;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strstr(ligne, "\"premier_tour\": true")) {
            premier_tour = true;
            break;
        }
        if (strstr(ligne, "\"premier_tour\": false")) {
            premier_tour = false;
            break;
        }
    }

    fclose(f);
    return premier_tour;
}



/*---------------------------------------------------------------------------------------------------------------------------------------*/
void jouer_combinaison(Joueur j) {
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    char ligne[256];
    char rep;

    bool premier_tour = est_premier_tour(j);

    do {
        // Charger le chevalet actuel
        charger_chevalet(j.chevalet, tuiles_joueur, &nb_tuiles);
        afficher_tuiles(tuiles_joueur, nb_tuiles);

        int indices[MAX_TUILES], nb_comb = 0;

        if (premier_tour)
            printf("Premier tour : vous devez poser une combinaison d'au moins 30 points.\n");
        else
            printf("Entrez les numeros de tuiles à poser (min 3), séparés par des espaces :\n");

        if (!fgets(ligne, sizeof(ligne), stdin)) return;

        // Récupérer les indices choisis
        char* tok = strtok(ligne, " \n");
        while (tok && nb_comb < MAX_TUILES) {
            int idx = atoi(tok) - 1;
            if (idx >= 0 && idx < nb_tuiles)
                indices[nb_comb++] = idx;
            tok = strtok(NULL, " \n");
        }

        if (nb_comb < 3) {
            printf("Une combinaison doit comporter au moins 3 tuiles.\n");
        }

        // Construire la combinaison choisie
        Tuile comb[MAX_TUILES];
        for (int i = 0; i < nb_comb; i++)
            comb[i] = tuiles_joueur[indices[i]];

        // --- Premier tour : boucle jusqu'à réussite ou abandon ---
        if (premier_tour) {
            while (!combinaison_valide_30(comb, nb_comb)) {
                printf("La combinaison n'est pas valide pour le premier tour (<30 points ou invalide).\n");
                printf("Voulez-vous réessayer ? (y/n) ");
                if (!fgets(ligne, sizeof(ligne), stdin)) return;
                if (ligne[0] == 'n' || ligne[0] == 'N') {
                    printf("Vous piochez une tuile.\n");
                    piocher_tuile(j);
                    return; // fin du tour
                } else if (ligne[0] == 'y' || ligne[0] == 'Y') {
                    // Refaire la saisie
                    printf("Re-entrez les numeros de tuiles à poser :\n");
                    if (!fgets(ligne, sizeof(ligne), stdin)) return;
                    nb_comb = 0;
                    tok = strtok(ligne, " \n");
                    while (tok && nb_comb < MAX_TUILES) {
                        int idx = atoi(tok) - 1;
                        if (idx >= 0 && idx < nb_tuiles)
                            indices[nb_comb++] = idx;
                        tok = strtok(NULL, " \n");
                    }
                    if (nb_comb < 3) {
                        printf("Une combinaison doit comporter au moins 3 tuiles.\n");
                        continue;
                    }
                    for (int i = 0; i < nb_comb; i++)
                        comb[i] = tuiles_joueur[indices[i]];
                } else {
                    printf("Réponse invalide. Tapez y ou n.\n");
                }
            }
        } 
        // --- Tours suivants ---
        else {
            if (!combinaison_valide(comb, nb_comb)) {
                printf("Combinaison invalide.\n");
                continue;
            }
        }

        // Ajouter la combinaison à la table
        ajouter_a_table(comb, nb_comb);

        // Supprimer les tuiles posées du chevalet
        Tuile restantes[MAX_TUILES];
        int nb_restantes = 0;
        for (int i = 0; i < nb_tuiles; i++) {
            bool garder = true;
            for (int k = 0; k < nb_comb; k++)
                if (i == indices[k]) garder = false;
            if (garder) restantes[nb_restantes++] = tuiles_joueur[i];
        }

        sauvegarder_chevalet(j.chevalet, j, restantes, nb_restantes);

        printf("Combinaison posée.\n");

        // Demander si le joueur veut poser une autre combinaison
        while (1) {
            printf("Voulez-vous poser une autre combinaison ? (y/n) ");
            if (!fgets(ligne, sizeof(ligne), stdin)) return;
            if (ligne[0] == 'y' || ligne[0] == 'Y') {
                rep = 'y';
                break;
            } else if (ligne[0] == 'n' || ligne[0] == 'N') {
                rep = 'n';
                break;
            } else {
                printf("Réponse invalide. Tapez y ou n.\n");
            }
        }

    } while (rep == 'y' || rep == 'Y');
}
