#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "struct.h"
#include "joueur.h"
#include "Tuile.h"
#include "table.h"
#include "manipulation.h"

/* ------------------------------------------------------------------------- */
// Fonction principale pour manipuler la table
bool manipuler_table(Joueur* j) {
    printf("\n=== MANIPULATION DE LA TABLE ===\n");
    
    // Afficher d'abord les tuiles du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    if (nb_tuiles == 0) {
        printf("Vous n'avez plus de tuiles !\n");
        return false;
    }
    
    printf("Vos tuiles :\n");
    afficher_tuiles(tuiles_joueur, nb_tuiles);
    
    // La table est déjà affichée par executer_option(), donc on ne la réaffiche pas ici
    // afficher_combinaisons_table();  // SUPPRIMÉ
    
    // Demander quelle manipulation effectuer
    afficher_options_manipulation();
    
    int choix;
    printf("Votre choix (1-5, 0 pour annuler) : ");
    scanf("%d", &choix);
    getchar(); // Consommer le newline
    
    bool succes = false;
    
    switch(choix) {
        case 0:
            printf("Manipulation annulée.\n");
            return false;
            
        case 1:
            succes = traiter_joker(j);
            break;
            
        case 2:
            succes = traiter_extension_suite(j);
            break;
            
        case 3:
            succes = traiter_remplacement(j);
            break;
            
        case 4:
            succes = traiter_division(j);
            break;
            
        case 5:
            succes = traiter_retrait(j);
            break;
            
        default:
            printf("Choix invalide.\n");
            return false;
    }
    
    if (succes) {
        // Vérifier que toutes les combinaisons sont toujours valides
        if (!validation_table_complete()) {
            printf("ERREUR : La manipulation a créé des combinaisons invalides !\n");
            // Il faudrait annuler la manipulation ici
            return false;
        }
        printf("Manipulation réussie !\n");
    }
    
    return succes;
}

/* ------------------------------------------------------------------------- */
void afficher_options_manipulation(void) {
    printf("\nOptions de manipulation :\n");
    printf("1. Récupérer un joker\n");
    printf("2. Étendre une suite et récupérer une tuile\n");
    printf("3. Remplacer une tuile dans une combinaison\n");
    printf("4. Diviser une suite en deux\n");
    printf("5. Retirer une tuile d'une combinaison\n");
    printf("0. Annuler / Retour\n");
}

/* ------------------------------------------------------------------------- */
// Récupérer un joker
bool traiter_joker(Joueur* j) {
    printf("\n=== RÉCUPÉRATION D'UN JOKER ===\n");
    
    // 1. Afficher les jokers disponibles sur la table
    printf("Jokers disponibles sur la table :\n");
    int nb_comb = compter_combinaisons_table();
    bool joker_trouve = false;
    
    for (int i = 0; i < nb_comb; i++) {
        int nb_t = compter_tuiles_combinaison(i);
        for (int k = 0; k < nb_t; k++) {
            Tuile t;
            obtenir_tuile_table(i, k, &t);
            if (t.joker) {
                printf("- Combinaison [%d], position %d : Joker ID %d\n", i, k, t.id);
                joker_trouve = true;
            }
        }
    }
    
    if (!joker_trouve) {
        printf("Aucun joker sur la table.\n");
        return false;
    }
    
    // 2. Demander quel joker récupérer
    printf("\nQuel joker voulez-vous récupérer ? (ID) : ");
    int id_joker;
    scanf("%d", &id_joker);
    getchar();
    
    // 3. Trouver ce joker sur la table
    int comb_index = -1, tuile_index = -1;
    Tuile joker_sur_table;
    bool trouve = false;
    
    for (int i = 0; i < nb_comb && !trouve; i++) {
        int nb_t = compter_tuiles_combinaison(i);
        for (int k = 0; k < nb_t && !trouve; k++) {
            obtenir_tuile_table(i, k, &joker_sur_table);
            if (joker_sur_table.id == id_joker && joker_sur_table.joker) {
                comb_index = i;
                tuile_index = k;
                trouve = true;
            }
        }
    }
    
    if (!trouve) {
        printf("Joker non trouvé sur la table.\n");
        return false;
    }
    
    // 4. Afficher les tuiles du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    printf("\nVos tuiles disponibles :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("%d: %d%c%s\n", 
               tuiles_joueur[i].id,
               tuiles_joueur[i].valeur,
               tuiles_joueur[i].couleur,
               tuiles_joueur[i].joker ? " (J)" : "");
    }
    
    // 5. Demander quelle tuile utiliser pour remplacer le joker
    printf("\nAvec quelle tuile voulez-vous remplacer ce joker ? (ID) : ");
    int id_tuile_remplacement;
    scanf("%d", &id_tuile_remplacement);
    getchar();
    
    // Trouver la tuile de remplacement
    Tuile* tuile_remplacement = NULL;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id == id_tuile_remplacement) {
            tuile_remplacement = &tuiles_joueur[i];
            break;
        }
    }
    
    if (!tuile_remplacement) {
        printf("Tuile non trouvée dans votre chevalet.\n");
        return false;
    }
    
    if (tuile_remplacement->joker) {
        printf("Vous ne pouvez pas utiliser un joker pour remplacer un joker !\n");
        return false;
    }
    
    // 6. Vérifier si le remplacement est valide
    // La fonction peut_recuperer_joker doit vérifier la validité
    int test_comb, test_tuile;
    if (!peut_recuperer_joker(tuile_remplacement, &test_comb, &test_tuile)) {
        printf("Cette tuile ne peut pas remplacer ce joker (combinaison invalide).\n");
        return false;
    }
    
    // 7. Effectuer le remplacement
    Tuile joker_recupere;
    if (!recuperer_joker(tuile_remplacement, comb_index, tuile_index, &joker_recupere)) {
        printf("Échec de la récupération du joker.\n");
        return false;
    }
    
    printf("Joker récupéré ! Vous devez l'utiliser immédiatement.\n");
    printf("Joker ID: %d\n", joker_recupere.id);
    
    // Retirer la tuile utilisée du chevalet
    Tuile restantes[MAX_TUILES];
    int nb_restantes = 0;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile_remplacement) {
            restantes[nb_restantes] = tuiles_joueur[i];
            nb_restantes++;
        }
    }
    
    // Ajouter le joker au chevalet
    restantes[nb_restantes] = joker_recupere;
    nb_restantes++;
    
    // Sauvegarder le nouveau chevalet
    sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, false);
    
    printf("Le joker a été ajouté à votre chevalet. Vous devez l'utiliser maintenant.\n");
    printf("Voulez-vous jouer une combinaison avec ce joker ? (o/n) : ");
    char reponse;
    scanf("%c", &reponse);
    getchar();
    
    if (reponse == 'o' || reponse == 'O') {
        // Option: forcer l'utilisation du joker
        printf("Appel à jouer_combinaison()...\n");
    }
    
    return true;
}
/* ------------------------------------------------------------------------- */
// Étendre une suite
bool traiter_extension_suite(Joueur* j) {
    printf("\n=== EXTENSION DE SUITE ===\n");
    
    // Charger les tuiles du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    // Afficher les suites sur la table
    printf("Suites disponibles sur la table :\n");
    int nb_comb = compter_combinaisons_table();
    bool suite_trouvee = false;
    
    for (int i = 0; i < nb_comb; i++) {
        if (est_combinaison_suite(i)) {
            printf("Suite [%d] : ", i);
            int nb_t = compter_tuiles_combinaison(i);
            for (int k = 0; k < nb_t; k++) {
                Tuile t;
                obtenir_tuile_table(i, k, &t);
                if (t.joker) {
                    printf("[J] ");
                } else {
                    printf("%d%c ", t.valeur, t.couleur);
                }
            }
            printf("\n");
            suite_trouvee = true;
        }
    }
    
    if (!suite_trouvee) {
        printf("Aucune suite disponible sur la table.\n");
        return false;
    }
    
    // Demander quelle suite étendre
    printf("\nQuelle suite voulez-vous étendre ? (numéro) : ");
    int num_suite;
    scanf("%d", &num_suite);
    getchar();
    
    if (num_suite < 0 || num_suite >= nb_comb || !est_combinaison_suite(num_suite)) {
        printf("Suite invalide.\n");
        return false;
    }
    
    // Demander à quelle extrémité
    printf("À quelle extrémité ? (g=Gauche, d=Droite) : ");
    char extremite;
    scanf(" %c", &extremite);  // CORRECT
getchar();  // Pour consommer le newline
    
    bool gauche = (extremite == 'g' || extremite == 'G');
    
    // Demander quelle tuile utiliser
    printf("Vos tuiles :\n");
    afficher_tuiles(tuiles_joueur, nb_tuiles);
    
    printf("Quelle tuile voulez-vous utiliser ? (ID) : ");
    int id_tuile;
    scanf("%d", &id_tuile);
    getchar();
    
    // Trouver la tuile
    Tuile* tuile_a_utiliser = NULL;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id == id_tuile) {
            tuile_a_utiliser = &tuiles_joueur[i];
            break;
        }
    }
    
    if (!tuile_a_utiliser) {
        printf("Tuile non trouvée.\n");
        return false;
    }
    
    // Vérifier si l'extension est possible
    Tuile tuile_a_recuperer;
    if (!peut_etendre_suite(tuile_a_utiliser, num_suite, gauche, &tuile_a_recuperer)) {
        printf("Impossible d'étendre cette suite avec cette tuile.\n");
        return false;
    }
    
    printf("Vous allez récupérer la tuile : ");
    if (tuile_a_recuperer.joker) {
        printf("[Joker]\n");
    } else {
        printf("%d%c\n", tuile_a_recuperer.valeur, tuile_a_recuperer.couleur);
    }
    
    printf("Confirmer ? (o/n) : ");
    char confirmation;
    scanf("%c", &confirmation);
    getchar();
    
    if (confirmation != 'o' && confirmation != 'O') {
        return false;
    }
    
    // Effectuer l'extension
    Tuile tuile_recuperee;
    if (!etendre_suite(tuile_a_utiliser, num_suite, gauche, &tuile_recuperee)) {
        printf("Échec de l'extension.\n");
        return false;
    }
    
    // Retirer la tuile utilisée du chevalet
    Tuile restantes[MAX_TUILES];
    int nb_restantes = 0;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile) {
            restantes[nb_restantes] = tuiles_joueur[i];
            nb_restantes++;
        }
    }
    
    // Ajouter la tuile récupérée au chevalet
    restantes[nb_restantes] = tuile_recuperee;
    nb_restantes++;
    
    // Sauvegarder
    sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, false);
    
    printf("Suite étendue avec succès !\n");
    return true;
}

/* ------------------------------------------------------------------------- */
// Remplacer une tuile
bool traiter_remplacement(Joueur* j) {
    printf("\n=== REMPLACEMENT DE TUILE ===\n");
    
    // Charger les tuiles du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    // Afficher la table
    afficher_combinaisons_table();
    
    int nb_comb = compter_combinaisons_table();
    if (nb_comb == 0) {
        printf("Aucune combinaison sur la table.\n");
        return false;
    }
    
    // Demander quelle combinaison
    printf("Dans quelle combinaison voulez-vous remplacer une tuile ? (0-%d) : ", nb_comb-1);
    int num_comb;
    scanf("%d", &num_comb);
    getchar();
    
    if (num_comb < 0 || num_comb >= nb_comb) {
        printf("Combinaison invalide.\n");
        return false;
    }
    
    // Afficher les tuiles de cette combinaison
    printf("Tuiles de la combinaison [%d] :\n", num_comb);
    int nb_t = compter_tuiles_combinaison(num_comb);
    for (int i = 0; i < nb_t; i++) {
        Tuile t;
        obtenir_tuile_table(num_comb, i, &t);
        printf("%d: ", i);
        if (t.joker) {
            printf("[Joker]\n");
        } else {
            printf("%d%c\n", t.valeur, t.couleur);
        }
    }
    
    // Demander quelle tuile remplacer
    printf("Quelle tuile voulez-vous remplacer ? (0-%d) : ", nb_t-1);
    int index_tuile;
    scanf("%d", &index_tuile);
    getchar();
    
    if (index_tuile < 0 || index_tuile >= nb_t) {
        printf("Index invalide.\n");
        return false;
    }
    
    // Afficher les tuiles du joueur
    printf("Vos tuiles :\n");
    afficher_tuiles(tuiles_joueur, nb_tuiles);
    
    // Demander quelle tuile utiliser
    printf("Quelle tuile voulez-vous utiliser pour le remplacement ? (ID) : ");
    int id_tuile;
    scanf("%d", &id_tuile);
    getchar();
    
    // Trouver la tuile
    Tuile* tuile_a_utiliser = NULL;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id == id_tuile) {
            tuile_a_utiliser = &tuiles_joueur[i];
            break;
        }
    }
    
    if (!tuile_a_utiliser) {
        printf("Tuile non trouvée.\n");
        return false;
    }
    
    // Vérifier si le remplacement est possible
    if (!peut_remplacer_tuile(tuile_a_utiliser, num_comb, index_tuile)) {
        printf("Remplacement impossible.\n");
        return false;
    }
    
    // Effectuer le remplacement
    Tuile ancienne_tuile;
    if (!remplacer_tuile(tuile_a_utiliser, num_comb, index_tuile, &ancienne_tuile)) {
        printf("Échec du remplacement.\n");
        return false;
    }
    
    // Retirer la tuile utilisée du chevalet
    Tuile restantes[MAX_TUILES];
    int nb_restantes = 0;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile) {
            restantes[nb_restantes] = tuiles_joueur[i];
            nb_restantes++;
        }
    }
    
    // Ajouter l'ancienne tuile au chevalet
    restantes[nb_restantes] = ancienne_tuile;
    nb_restantes++;
    
    // Sauvegarder
    sauvegarder_chevalet(j->chevalet, *j, restantes, nb_restantes, false);
    
    printf("Tuile remplacée avec succès !\n");
    return true;
}

/* ------------------------------------------------------------------------- */
// Diviser une suite
bool traiter_division(Joueur* j) {
    printf("\n=== DIVISION DE SUITE ===\n");
    
    // Afficher les suites sur la table
    printf("Suites disponibles sur la table :\n");
    int nb_comb = compter_combinaisons_table();
    bool suite_trouvee = false;
    
    for (int i = 0; i < nb_comb; i++) {
        if (est_combinaison_suite(i)) {
            printf("Suite [%d] : ", i);
            int nb_t = compter_tuiles_combinaison(i);
            for (int k = 0; k < nb_t; k++) {
                Tuile t;
                obtenir_tuile_table(i, k, &t);
                if (t.joker) {
                    printf("[J] ");
                } else {
                    printf("%d%c ", t.valeur, t.couleur);
                }
            }
            printf("\n");
            suite_trouvee = true;
        }
    }
    
    if (!suite_trouvee) {
        printf("Aucune suite disponible sur la table.\n");
        return false;
    }
    
    // Demander quelle suite diviser
    printf("\nQuelle suite voulez-vous diviser ? (numéro) : ");
    int num_suite;
    scanf("%d", &num_suite);
    getchar();
    
    if (num_suite < 0 || num_suite >= nb_comb || !est_combinaison_suite(num_suite)) {
        printf("Suite invalide.\n");
        return false;
    }
    
    int nb_tuiles = compter_tuiles_combinaison(num_suite);
    
    // Demander où diviser
    printf("La suite a %d tuiles.\n", nb_tuiles);
    printf("Où voulez-vous diviser ? (position 2 à %d) : ", nb_tuiles - 2);
    int position;
    scanf("%d", &position);
    getchar();
    
    // Vérifier si la division est possible
    if (!peut_diviser_suite(num_suite, position)) {
        printf("Division impossible à cette position.\n");
        printf("Chaque partie doit avoir au moins 3 tuiles.\n");
        return false;
    }
    
    printf("La suite sera divisée en :\n");
    printf("- Partie 1 : tuiles 0 à %d\n", position - 1);
    printf("- Partie 2 : tuiles %d à %d\n", position, nb_tuiles - 1);
    
    printf("Confirmer ? (o/n) : ");
    char confirmation;
    scanf("%c", &confirmation);
    getchar();
    
    if (confirmation != 'o' && confirmation != 'O') {
        return false;
    }
    
    // Effectuer la division
    if (!diviser_suite(num_suite, position)) {
        printf("Échec de la division.\n");
        return false;
    }
    
    printf("Suite divisée avec succès !\n");
    return true;
}

/* ------------------------------------------------------------------------- */
// Retirer une tuile
bool traiter_retrait(Joueur* j) {
    printf("\n=== RETRAIT DE TUILE ===\n");
    
    // Afficher la table
    afficher_combinaisons_table();
    
    int nb_comb = compter_combinaisons_table();
    if (nb_comb == 0) {
        printf("Aucune combinaison sur la table.\n");
        return false;
    }
    
    // Demander quelle combinaison
    printf("De quelle combinaison voulez-vous retirer une tuile ? (0-%d) : ", nb_comb-1);
    int num_comb;
    scanf("%d", &num_comb);
    getchar();
    
    if (num_comb < 0 || num_comb >= nb_comb) {
        printf("Combinaison invalide.\n");
        return false;
    }
    
    int nb_t = compter_tuiles_combinaison(num_comb);
    if (nb_t <= 3) {
        printf("Cette combinaison n'a que %d tuiles. Impossible d'en retirer.\n", nb_t);
        return false;
    }
    
    // Afficher les tuiles
    printf("Tuiles de la combinaison [%d] :\n", num_comb);
    for (int i = 0; i < nb_t; i++) {
        Tuile t;
        obtenir_tuile_table(num_comb, i, &t);
        printf("%d: ", i);
        if (t.joker) {
            printf("[Joker]\n");
        } else {
            printf("%d%c\n", t.valeur, t.couleur);
        }
    }
    
    // Demander quelle tuile retirer
    printf("Quelle tuile voulez-vous retirer ? (0-%d) : ", nb_t-1);
    int index_tuile;
    scanf("%d", &index_tuile);
    getchar();
    
    if (index_tuile < 0 || index_tuile >= nb_t) {
        printf("Index invalide.\n");
        return false;
    }
    
    // Vérifier si le retrait est possible
    if (!peut_retirer_tuile(num_comb, index_tuile)) {
        printf("Retrait impossible.\n");
        return false;
    }
    
    printf("Confirmer le retrait ? (o/n) : ");
    char confirmation;
    scanf("%c", &confirmation);
    getchar();
    
    if (confirmation != 'o' && confirmation != 'O') {
        return false;
    }
    
    // Effectuer le retrait
    Tuile tuile_retiree;
    if (!retirer_tuile(num_comb, index_tuile, &tuile_retiree)) {
        printf("Échec du retrait.\n");
        return false;
    }
    
    // Ajouter la tuile retirée au chevalet du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    tuiles_joueur[nb_tuiles] = tuile_retiree;
    nb_tuiles++;
    
    sauvegarder_chevalet(j->chevalet, *j, tuiles_joueur, nb_tuiles, false);
    
    printf("Tuile retirée avec succès et ajoutée à votre chevalet !\n");
    return true;
}

