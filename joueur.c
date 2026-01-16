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

    // créer fichiers joueurs avec pseudo et tour
    for (int j = 0; j < nb_joueurs; j++) {
        FILE* fj = fopen(players[j].chevalet, "w");
        if (!fj) {
            perror("Erreur création fichier joueur");
            continue;
        }
        fprintf(fj, "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"tuiles\": []\n}\n",
                players[j].pseudo, (j == 0) ? "true" : "false");
        fclose(fj);
    }

    // distribuer 14 tuiles à chaque joueur
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
    char pseudo[50];
    int tour = 0;

    // --- lire pioche ---
    FILE* f = fopen("pioche.json", "r");
    if (!f) return;

    while (fgets(ligne, sizeof(ligne), f)) {
        if (strchr(ligne, '{')) {
            sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
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
        fprintf(f, "  {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                pioche[i].valeur, pioche[i].couleur,
                pioche[i].joker ? "true" : "false",
                (i < np - 1) ? "," : "");
    fprintf(f, "]\n");
    fclose(f);

    // --- lire joueur existant ---
    FILE* fj = fopen(j.chevalet, "r");
    if (!fj) return;

    // lire pseudo, tour, tuiles existantes
    nj = 0;
    while (fgets(ligne, sizeof(ligne), fj)) {
        if (sscanf(ligne, " \"pseudo\": \"%49[^\"]\"", pseudo) == 1) continue;
        if (strstr(ligne, "\"tour\": true")) tour = 1;
        else if (strstr(ligne, "\"tour\": false")) tour = 0;
        if (strchr(ligne, '{') && strstr(ligne, "valeur")) {
            sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                   &tuiles_joueur[nj].valeur, &tuiles_joueur[nj].couleur);
            tuiles_joueur[nj].joker = strstr(ligne, "true") != NULL;
            nj++;
        }
    }
    fclose(fj);

    // ajouter la nouvelle tuile
    tuiles_joueur[nj++] = t;

    // --- réécrire joueur proprement ---
    fj = fopen(j.chevalet, "w");
    fprintf(fj, "{\n  \"pseudo\": \"%s\",\n  \"tour\": %s,\n  \"tuiles\": [\n",
            pseudo, tour ? "true" : "false");
    for (int i = 0; i < nj; i++)
        fprintf(fj, "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                tuiles_joueur[i].valeur,
                tuiles_joueur[i].couleur,
                tuiles_joueur[i].joker ? "true" : "false",
                (i < nj - 1) ? "," : "");
    fprintf(fj, "  ]\n}\n");
    fclose(fj);
}
/*------------------------------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------------------------------*/
void jouer_combinaison(Joueur j) {
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    char ligne[256];

    // --- Lire le chevalet ---
    FILE* fj = fopen(j.chevalet, "r");
    if (!fj) { perror("Impossible d'ouvrir le fichier du joueur"); return; }

    while (fgets(ligne, sizeof(ligne), fj)) {
        if (strchr(ligne, '{') && strstr(ligne, "valeur")) {
            sscanf(ligne, " {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%*[^t]true}",
                   &tuiles_joueur[nb_tuiles].valeur,
                   &tuiles_joueur[nb_tuiles].couleur);
            tuiles_joueur[nb_tuiles].joker = strstr(ligne, "true") != NULL;
            nb_tuiles++;
        }
    }
    fclose(fj);

    // --- Afficher les tuiles ---
    printf("Vos tuiles :\n");
    for (int i = 0; i < nb_tuiles; i++)
        printf("%d: %d%c%s\n", i + 1, tuiles_joueur[i].valeur,
               tuiles_joueur[i].couleur, tuiles_joueur[i].joker ? " (J)" : "");

    char rep;
    do {
        int indices[MAX_TUILES], nb_comb = 0;

        // --- Boucle tant que combinaison invalide ---
        while (true) {
            printf("Entrez les numeros de tuiles à poser (min 3), séparés par des espaces :\n");
            if (!fgets(ligne, sizeof(ligne), stdin)) continue;
            nb_comb = 0;

            char* tok = strtok(ligne, " \n");
            while (tok && nb_comb < MAX_TUILES) {
                int idx = atoi(tok) - 1;
                if (idx >= 0 && idx < nb_tuiles) indices[nb_comb++] = idx;
                tok = strtok(NULL, " \n");
            }

            if (nb_comb < 3) {
                printf("Une combinaison doit comporter au moins 3 tuiles.\n");
                continue;
            }

            Tuile comb[MAX_TUILES];
            for (int i = 0; i < nb_comb; i++) comb[i] = tuiles_joueur[indices[i]];

            if (!combinaison_valide(comb, nb_comb)) {
                printf("Combinaison invalide selon les règles du Rummikub.\n");
                continue;
            }

            // --- Ajouter la combinaison à table.json ---
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
            for (int i = 0; i < nb_comb; i++) {
                Tuile t = comb[i];
                fprintf(ft, "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}%s\n",
                        t.valeur, t.couleur, t.joker ? "true" : "false",
                        (i < nb_comb - 1) ? "," : "");
            }
            fprintf(ft, "  ]\n]\n");
            fclose(ft);

            // --- Supprimer les tuiles posées du chevalet ---
            FILE* fjw = fopen(j.chevalet, "w");
            fprintf(fjw, "{\n  \"pseudo\":\"%s\",\n  \"tour\":%s,\n  \"tuiles\":[\n",
                    j.pseudo, j.tour ? "true" : "false");
            int written = 0;
            for (int i = 0; i < nb_tuiles; i++) {
                int keep = 1;
                for (int k = 0; k < nb_comb; k++)
                    if (i == indices[k]) keep = 0;
                if (keep) {
                    if (written++) fprintf(fjw, ",\n");
                    Tuile t = tuiles_joueur[i];
                    fprintf(fjw, "    {\"valeur\":%d,\"couleur\":\"%c\",\"joker\":%s}",
                            t.valeur, t.couleur, t.joker ? "true" : "false");
                }
            }
            fprintf(fjw, "\n  ]\n}\n");
            fclose(fjw);

            printf("Combinaison ajoutée à la table.\n");
            break; // sortir de la boucle while
        }

        printf("Voulez-vous poser une autre combinaison ? (y/n) ");
        scanf(" %c", &rep);
        getchar(); // consommer le \n
    } while (rep == 'y' || rep == 'Y');
}
