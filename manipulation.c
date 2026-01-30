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
    int test_comb, test_tuile;
    if (!peut_recuperer_joker(tuile_remplacement, &test_comb, &test_tuile)) {
        printf("Cette tuile ne peut pas remplacer ce joker (combinaison invalide).\n");
        return false;
    }
    
    // IMPORTANT: Avant de récupérer le joker, vérifier que le joueur PEUT l'utiliser
    printf("\n=== CONTRÔLE PRÉALABLE ===\n");
    printf("Avant de récupérer le joker, vous devez prouver que vous pouvez l'utiliser.\n");
    
    // Créer une copie temporaire du chevalet avec le joker
    Tuile chevalet_avec_joker[MAX_TUILES];
    int nb_avec_joker = 0;
    
    // Copier toutes les tuiles sauf celle qui remplacera le joker
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile_remplacement) {
            chevalet_avec_joker[nb_avec_joker] = tuiles_joueur[i];
            nb_avec_joker++;
        }
    }
    
    // Ajouter le joker à la copie
    Tuile joker_simule;
    joker_simule.id = id_joker; // ID du joker qu'on va récupérer
    joker_simule.valeur = 0;
    joker_simule.couleur = 'J';
    joker_simule.joker = true;
    chevalet_avec_joker[nb_avec_joker] = joker_simule;
    nb_avec_joker++;
    
    printf("\nVotre chevalet SIMULÉ avec le joker :\n");
    afficher_tuiles(chevalet_avec_joker, nb_avec_joker);
    
    // Demander au joueur de montrer une combinaison possible avec le joker
    printf("\nMontrez-moi une combinaison que vous pourriez faire avec ce joker.\n");
    printf("Entrez les IDs des tuiles (dont le joker %d) pour une combinaison valide : ", id_joker);
    
    char ligne[256];
    if (!fgets(ligne, sizeof(ligne), stdin)) {
        printf("Erreur de lecture.\n");
        return false;
    }
    
    // Analyser la combinaison proposée
    int ids_proposes[MAX_TUILES], nb_ids = 0;
    char *tok = strtok(ligne, " \n");
    while (tok && nb_ids < MAX_TUILES) {
        ids_proposes[nb_ids++] = atoi(tok);
        tok = strtok(NULL, " \n");
    }
    
    if (nb_ids < 3) {
        printf("Une combinaison doit avoir au moins 3 tuiles.\n");
        printf("Récupération du joker refusée.\n");
        return false;
    }
    
    // Vérifier que le joker est dans la combinaison proposée
    bool joker_dans_combinaison = false;
    for (int i = 0; i < nb_ids; i++) {
        if (ids_proposes[i] == id_joker) {
            joker_dans_combinaison = true;
            break;
        }
    }
    
    if (!joker_dans_combinaison) {
        printf("La combinaison doit inclure le joker ID %d !\n", id_joker);
        printf("Récupération du joker refusée.\n");
        return false;
    }
    
    // Vérifier que toutes les tuiles proposées sont dans le chevalet simulé
    Tuile combinaison_test[MAX_TUILES];
    int nb_comb_test = 0;
    
    for (int i = 0; i < nb_ids; i++) {
        bool trouve_tuile = false;
        for (int k = 0; k < nb_avec_joker; k++) {
            if (chevalet_avec_joker[k].id == ids_proposes[i]) {
                combinaison_test[nb_comb_test] = chevalet_avec_joker[k];
                nb_comb_test++;
                trouve_tuile = true;
                break;
            }
        }
        if (!trouve_tuile) {
            printf("Tuile ID %d non disponible dans votre chevalet.\n", ids_proposes[i]);
            printf("Récupération du joker refusée.\n");
            return false;
        }
    }
    
    // Vérifier si la combinaison proposée est valide
    if (!combinaison_valide(combinaison_test, nb_comb_test)) {
        printf("La combinaison proposée n'est pas valide.\n");
        printf("Récupération du joker refusée.\n");
        return false;
    }
    
    printf("✅ Combinaison valide ! Vous pouvez récupérer le joker.\n");
    
    // 7. Effectuer le remplacement pour de vrai
    Tuile joker_recupere;
    if (!recuperer_joker(tuile_remplacement, comb_index, tuile_index, &joker_recupere)) {
        printf("Échec de la récupération du joker.\n");
        return false;
    }
    
    printf("Joker récupéré ! Vous DEVEZ l'utiliser immédiatement.\n");
    printf("Joker ID: %d\n", joker_recupere.id);
    
    // 8. Effectuer le remplacement réel dans le chevalet
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
    
    printf("\n=== UTILISATION IMMÉDIATE DU JOKER ===\n");
    printf("Maintenant, vous DEVEZ utiliser le joker ID %d dans une combinaison.\n", joker_recupere.id);
    
    // FORCER le joueur à utiliser le joker maintenant
    bool joker_utilise = false;
    
    // Afficher l'état actuel
    Tuile chevalet_actuel[MAX_TUILES];
    int nb_actuel = 0;
    charger_chevalet(j->chevalet, chevalet_actuel, &nb_actuel);
    
    printf("\nVotre chevalet ACTUEL (avec le joker) :\n");
    afficher_tuiles(chevalet_actuel, nb_actuel);
    
    printf("\nTable actuelle :\n");
    afficher_combinaisons_table();
    
    printf("\nOptions :\n");
    printf("1. Jouer une combinaison qui utilise le joker\n");
    printf("2. Ajouter le joker à une combinaison existante sur la table\n");
    printf("Votre choix : ");
    
    int choix_utilisation;
    scanf("%d", &choix_utilisation);
    getchar();
    
    if (choix_utilisation == 1) {
        // Le joueur doit jouer une combinaison
        printf("\nVous allez maintenant jouer une combinaison.\n");
        printf("Assurez-vous d'inclure le joker ID %d dans votre combinaison.\n", joker_recupere.id);
        
        // Appeler jouer_combinaison
        jouer_combinaison(j);
        
        // Vérifier si le joker a été utilisé
        charger_chevalet(j->chevalet, chevalet_actuel, &nb_actuel);
        bool joker_toujours_present = false;
        for (int i = 0; i < nb_actuel; i++) {
            if (chevalet_actuel[i].id == joker_recupere.id) {
                joker_toujours_present = true;
                break;
            }
        }
        
        if (!joker_toujours_present) {
            printf("✅ Joker utilisé avec succès !\n");
            joker_utilise = true;
        } else {
            printf("❌ Le joker n'a pas été utilisé !\n");
            // Dans un vrai jeu, il faudrait annuler toute l'action ici
            printf("Pour l'instant, on continue, mais c'est contraire aux règles.\n");
        }
    }
    else if (choix_utilisation == 2) {
        printf("\nFonctionnalité 'ajouter à une combinaison existante' à implémenter.\n");
        printf("Pour l'instant, considérons que le joker est utilisé.\n");
        joker_utilise = true;
    }
    else {
        printf("Choix invalide.\n");
    }
    
    if (!joker_utilise) {
        printf("\n⚠️ ATTENTION : Le joker n'a pas été utilisé immédiatement.\n");
        printf("Dans une partie réelle, cette action serait annulée.\n");
    }
    
    return joker_utilise;
}

// Dans manipulation.c
bool ajouter_tuile_combinaison_existante(Joueur* j) {
    printf("\n=== AJOUTER UNE TUILE À UNE COMBINAISON EXISTANTE ===\n");
    
    // 1. Afficher les tuiles du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    if (nb_tuiles == 0) {
        printf("Vous n'avez plus de tuiles !\n");
        return false;
    }
    
    printf("Vos tuiles :\n");
    afficher_tuiles(tuiles_joueur, nb_tuiles);
    
    // 2. Demander quelle tuile utiliser
    printf("\nQuelle tuile voulez-vous ajouter à une combinaison ? (ID) : ");
    int id_tuile;
    scanf("%d", &id_tuile);
    getchar();
    
    // Trouver la tuile
    Tuile* tuile_a_ajouter = NULL;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id == id_tuile) {
            tuile_a_ajouter = &tuiles_joueur[i];
            break;
        }
    }
    
    if (!tuile_a_ajouter) {
        printf("Tuile non trouvée.\n");
        return false;
    }
    
    // 3. Afficher les combinaisons sur la table
    printf("\nCombinaisons sur la table :\n");
    afficher_combinaisons_table();
    
    int nb_comb = compter_combinaisons_table();
    if (nb_comb == 0) {
        printf("Aucune combinaison sur la table.\n");
        return false;
    }
    
    // 4. Demander à quelle combinaison l'ajouter
    printf("\nÀ quelle combinaison voulez-vous ajouter cette tuile ? (0-%d) : ", nb_comb-1);
    int num_comb;
    scanf("%d", &num_comb);
    getchar();
    
    if (num_comb < 0 || num_comb >= nb_comb) {
        printf("Numéro de combinaison invalide.\n");
        return false;
    }
    
    // 5. Vérifier si l'ajout est possible
    if (!peut_ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
        printf("Impossible d'ajouter cette tuile à cette combinaison.\n");
        return false;
    }
    
    // 6. Effectuer l'ajout
    if (!ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
        printf("Échec de l'ajout.\n");
        return false;
    }
    
    // 7. Retirer la tuile du chevalet
    Tuile nouveau_chevalet[MAX_TUILES];
    int nb_nouveau = 0;
    
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile) {
            nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
            nb_nouveau++;
        }
    }
    
    // 8. Sauvegarder
    sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
    
    printf("\n✅ Tuile ajoutée avec succès à la combinaison !\n");
    return true;
}
/* ------------------------------------------------------------------------- */
// Étendre une suite
bool traiter_extension_suite(Joueur* j) {
    printf("\n=== EXTENSION DE SUITE ===\n");
    
    // 1. Afficher la table
    printf("Table :\n");
    afficher_combinaisons_table();
    
    // 2. Charger les tuiles du joueur
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    if (nb_tuiles == 0) {
        printf("Chevalet vide.\n");
        return false;
    }

    printf("\nVos tuiles :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("ID %d: %d%c", 
               tuiles_joueur[i].id,
               tuiles_joueur[i].valeur,
               tuiles_joueur[i].couleur);
        if (tuiles_joueur[i].joker) printf(" (Joker)");
        printf("\n");
    }
    
    // 3. Demander quelle tuile utiliser
    printf("\nVotre tuile pour étendre ? (ID) : ");
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
        printf("Tuile introuvable.\n");
        return false;
    }
    
    if (tuile_a_utiliser->joker) {
        printf("Les jokers ne peuvent pas étendre des suites.\n");
        return false;
    }
    
    // 4. Chercher les suites extensibles
    int nb_comb = compter_combinaisons_table();
    int suites_trouvees[MAX_TUILES];
    int nb_suites_trouvees = 0;
    
    for (int i = 0; i < nb_comb; i++) {
        if (!est_combinaison_suite(i)) continue;
        
        int nb_t = compter_tuiles_combinaison(i);
        Tuile premiere, derniere;
        obtenir_tuile_table(i, 0, &premiere);
        obtenir_tuile_table(i, nb_t - 1, &derniere);
        
        bool peut_gauche = (!premiere.joker && 
                           tuile_a_utiliser->couleur == premiere.couleur && 
                           tuile_a_utiliser->valeur == premiere.valeur - 1);
        
        bool peut_droite = (!derniere.joker &&
                           tuile_a_utiliser->couleur == derniere.couleur &&
                           tuile_a_utiliser->valeur == derniere.valeur + 1);
        
        if (peut_gauche || peut_droite) {
            suites_trouvees[nb_suites_trouvees] = i;
            nb_suites_trouvees++;
        }
    }
    
    if (nb_suites_trouvees == 0) {
        printf("Aucune suite extensible avec cette tuile.\n");
        return false;
    }
    
    // 5. Demander quelle suite étendre
    int num_suite;
    if (nb_suites_trouvees == 1) {
        num_suite = suites_trouvees[0];
    } else {
        printf("\nQuelle suite ? (");
        for (int i = 0; i < nb_suites_trouvees; i++) {
            printf("%d", suites_trouvees[i]);
            if (i < nb_suites_trouvees - 1) printf(", ");
        }
        printf(") : ");
        scanf("%d", &num_suite);
        getchar();
    }
    
    // 6. Déterminer la direction
    int nb_t = compter_tuiles_combinaison(num_suite);
    Tuile premiere, derniere;
    obtenir_tuile_table(num_suite, 0, &premiere);
    obtenir_tuile_table(num_suite, nb_t - 1, &derniere);
    
    bool peut_gauche = (!premiere.joker && 
                       tuile_a_utiliser->couleur == premiere.couleur && 
                       tuile_a_utiliser->valeur == premiere.valeur - 1);
    
    bool peut_droite = (!derniere.joker &&
                       tuile_a_utiliser->couleur == derniere.couleur &&
                       tuile_a_utiliser->valeur == derniere.valeur + 1);
    
    bool gauche;
    if (peut_gauche && !peut_droite) {
        gauche = true;
    } else if (!peut_gauche && peut_droite) {
        gauche = false;
    } else {
        printf("Extension à gauche (1) ou droite (2) ? ");
        int choix;
        scanf("%d", &choix);
        getchar();
        gauche = (choix == 1);
    }
    
    // 7. Effectuer l'extension
    Tuile tuile_recuperee;
    if (!etendre_suite(tuile_a_utiliser, num_suite, gauche, &tuile_recuperee)) {
        printf("Échec extension.\n");
        return false;
    }
    
    printf("Tuile récupérée : ID %d\n", tuile_recuperee.id);
    
    // 8. Préparer le nouveau chevalet
    Tuile nouveau_chevalet[MAX_TUILES];
    int nb_nouveau = 0;
    
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile) {
            nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
            nb_nouveau++;
        }
    }
    
    nouveau_chevalet[nb_nouveau] = tuile_recuperee;
    nb_nouveau++;
    
    // 9. Forcer l'utilisation immédiate
    printf("\nUtilisez la tuile ID %d maintenant (liste d'IDs, min 3) : ", tuile_recuperee.id);
    
    int ids_combinaison[MAX_TUILES];
    int nb_ids = 0;
    char ligne[256];
    
    if (!fgets(ligne, sizeof(ligne), stdin)) {
        printf("Annulé.\n");
        return false;
    }
    
    char *token = strtok(ligne, " \n");
    while (token != NULL && nb_ids < MAX_TUILES) {
        int id = atoi(token);
        
        bool trouve = false;
        for (int i = 0; i < nb_nouveau; i++) {
            if (nouveau_chevalet[i].id == id) {
                ids_combinaison[nb_ids] = i;
                nb_ids++;
                trouve = true;
                break;
            }
        }
        
        if (!trouve) {
            printf("ID %d introuvable.\n", id);
        }
        
        token = strtok(NULL, " \n");
    }
    
    if (nb_ids < 3) {
        printf("Trop court.\n");
        return false;
    }
    
    // Vérifier inclusion de la tuile récupérée
    bool inclu = false;
    for (int i = 0; i < nb_ids; i++) {
        if (nouveau_chevalet[ids_combinaison[i]].id == tuile_recuperee.id) {
            inclu = true;
            break;
        }
    }
    
    if (!inclu) {
        printf("Doit inclure ID %d.\n", tuile_recuperee.id);
        return false;
    }
    
    // Vérifier validité
    Tuile combinaison[MAX_TUILES];
    for (int i = 0; i < nb_ids; i++) {
        combinaison[i] = nouveau_chevalet[ids_combinaison[i]];
    }
    
    if (!combinaison_valide(combinaison, nb_ids)) {
        printf("Combinaison invalide.\n");
        return false;
    }
    
    // 10. Ajouter à la table et mettre à jour chevalet
    ajouter_a_table(combinaison, nb_ids);
    
    Tuile chevalet_final[MAX_TUILES];
    int nb_final = 0;
    
    for (int i = 0; i < nb_nouveau; i++) {
        bool utilisee = false;
        for (int k = 0; k < nb_ids; k++) {
            if (nouveau_chevalet[i].id == nouveau_chevalet[ids_combinaison[k]].id) {
                utilisee = true;
                break;
            }
        }
        
        if (!utilisee) {
            chevalet_final[nb_final] = nouveau_chevalet[i];
            nb_final++;
        }
    }
    
    sauvegarder_chevalet(j->chevalet, *j, chevalet_final, nb_final, false);
    
    printf("Succès.\n");
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
    printf("\n=== DIVISION DE SUITE + AJOUT ===\n");
    
    // 1. Afficher la table
    printf("Table :\n");
    afficher_combinaisons_table();
    
    // 2. Afficher le chevalet
    Tuile tuiles_joueur[MAX_TUILES];
    int nb_tuiles = 0;
    charger_chevalet(j->chevalet, tuiles_joueur, &nb_tuiles);
    
    printf("\nVotre chevalet :\n");
    for (int i = 0; i < nb_tuiles; i++) {
        printf("ID %d: %d%c", tuiles_joueur[i].id, tuiles_joueur[i].valeur, tuiles_joueur[i].couleur);
        if (tuiles_joueur[i].joker) printf(" (Joker)");
        printf("\n");
    }
    
    // 3. Demander APRÈS QUELLE TUILE diviser (par ID)
    printf("\nAprès quelle tuile de la table voulez-vous diviser ? (ID) : ");
    int id_tuile_division;
    scanf("%d", &id_tuile_division);
    getchar();
    
    // Trouver cette tuile sur la table
    int nb_comb = compter_combinaisons_table();
    int comb_index = -1;
    int position = -1;
    
    for (int i = 0; i < nb_comb; i++) {
        if (!est_combinaison_suite(i)) continue;
        
        int nb_t = compter_tuiles_combinaison(i);
        for (int k = 0; k < nb_t; k++) {
            Tuile t;
            obtenir_tuile_table(i, k, &t);
            if (t.id == id_tuile_division) {
                comb_index = i;
                position = k; // Diviser APRÈS cette position
                break;
            }
        }
        if (comb_index != -1) break;
    }
    
    if (comb_index == -1) {
        printf("Tuile ID %d non trouvée sur la table.\n", id_tuile_division);
        return false;
    }
    
    // Vérifier que la suite est assez longue
    int nb_t = compter_tuiles_combinaison(comb_index);
    if (nb_t < 5) {
        printf("Suite trop courte (min 5 tuiles).\n");
        return false;
    }
    
    // Vérifier que la position laisse assez de tuiles de chaque côté
    // position est l'index de la tuile APRÈS laquelle on divise
    if (position < 1 || position > nb_t - 3) {
        printf("Division impossible : chaque partie doit avoir au moins 3 tuiles.\n");
        return false;
    }
    
    // 4. Demander quelle tuile ajouter
    printf("\nQuelle tuile de votre chevalet ajouter ? (ID) : ");
    int id_tuile_ajout;
    scanf("%d", &id_tuile_ajout);
    getchar();
    
    // Trouver la tuile
    Tuile* tuile_a_ajouter = NULL;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id == id_tuile_ajout) {
            tuile_a_ajouter = &tuiles_joueur[i];
            break;
        }
    }
    
    if (!tuile_a_ajouter) {
        printf("Tuile non trouvée dans votre chevalet.\n");
        return false;
    }
    
    // 5. Demander à quelle partie ajouter
    printf("\nÀ quelle partie ajouter la tuile ?\n");
    printf("1. Première partie (avant la division)\n");
    printf("2. Deuxième partie (après la division)\n");
    printf("Choix (1-2) : ");
    
    int choix_partie;
    scanf("%d", &choix_partie);
    getchar();
    
    bool ajouter_a_premiere = (choix_partie == 1);
    
    // 6. Simuler et vérifier
    Tuile partie1[MAX_TUILES];
    Tuile partie2[MAX_TUILES];
    int nb_partie1 = 0, nb_partie2 = 0;
    
    // Construire partie 1 (tuiles 0 à position)
    for (int i = 0; i <= position; i++) {
        Tuile t;
        obtenir_tuile_table(comb_index, i, &t);
        partie1[nb_partie1++] = t;
    }
    
    // Construire partie 2 (tuiles position+1 à fin)
    for (int i = position + 1; i < nb_t; i++) {
        Tuile t;
        obtenir_tuile_table(comb_index, i, &t);
        partie2[nb_partie2++] = t;
    }
    
    // Ajouter la nouvelle tuile
    if (ajouter_a_premiere) {
        partie1[nb_partie1++] = *tuile_a_ajouter;
    } else {
        partie2[nb_partie2++] = *tuile_a_ajouter;
    }
    
    // Vérifier validité
    if (!combinaison_valide(partie1, nb_partie1) || !combinaison_valide(partie2, nb_partie2)) {
        printf("Division impossible : combinaisons invalides.\n");
        return false;
    }
    
    // 7. Confirmation
    printf("\nDivision :\n");
    printf("- Suite 1 : ");
    for (int i = 0; i < nb_partie1; i++) {
        printf("%d%c ", partie1[i].valeur, partie1[i].couleur);
    }
    printf("\n- Suite 2 : ");
    for (int i = 0; i < nb_partie2; i++) {
        printf("%d%c ", partie2[i].valeur, partie2[i].couleur);
    }
    printf("\nConfirmer ? (o/n) : ");
    
    char confirmation;
    scanf("%c", &confirmation);
    getchar();
    
    if (confirmation != 'o' && confirmation != 'O') {
        return false;
    }
    
    // 8. Exécuter
    if (!diviser_suite_avec_ajout(comb_index, position, tuile_a_ajouter, ajouter_a_premiere)) {
        printf("Échec de la division.\n");
        return false;
    }
    
    // 9. Mettre à jour le chevalet
    Tuile nouveau_chevalet[MAX_TUILES];
    int nb_nouveau = 0;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile_ajout) {
            nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
            nb_nouveau++;
        }
    }
    
    sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
    
    printf("✅ Division réussie !\n");
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

