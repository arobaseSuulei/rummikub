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
    
    // 7. SAUVEGARDER L'ÉTAT ACTUEL POUR POUVOIR ANNULER SI ÉCHEC
    char* table_avant = NULL;
    FILE* f_table = fopen("table.json", "r");
    if (f_table) {
        fseek(f_table, 0, SEEK_END);
        long fsize = ftell(f_table);
        fseek(f_table, 0, SEEK_SET);
        table_avant = malloc(fsize + 1);
        fread(table_avant, 1, fsize, f_table);
        table_avant[fsize] = 0;
        fclose(f_table);
    }
    
    char* chevalet_avant = NULL;
    FILE* f_chevalet = fopen(j->chevalet, "r");
    if (f_chevalet) {
        fseek(f_chevalet, 0, SEEK_END);
        long fsize = ftell(f_chevalet);
        fseek(f_chevalet, 0, SEEK_SET);
        chevalet_avant = malloc(fsize + 1);
        fread(chevalet_avant, 1, fsize, f_chevalet);
        chevalet_avant[fsize] = 0;
        fclose(f_chevalet);
    }
    
    // 8. Effectuer le remplacement (libérer le joker)
    Tuile joker_libre;
    if (!recuperer_joker(tuile_remplacement, comb_index, tuile_index, &joker_libre)) {
        printf("Échec de la récupération du joker.\n");
        if (table_avant) free(table_avant);
        if (chevalet_avant) free(chevalet_avant);
        return false;
    }
    
    printf("Joker libéré ! Vous DEVEZ l'utiliser immédiatement.\n");
    printf("Joker ID: %d\n", joker_libre.id);
    
    // 9. RETIRER la tuile utilisée du chevalet (MAIS NE PAS AJOUTER LE JOKER)
    Tuile nouveau_chevalet[MAX_TUILES];
    int nb_nouveau = 0;
    for (int i = 0; i < nb_tuiles; i++) {
        if (tuiles_joueur[i].id != id_tuile_remplacement) {
            nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
            nb_nouveau++;
        }
    }
    
    // Sauvegarder le chevalet SANS la tuile utilisée
    sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
    
    // 10. FORCER L'UTILISATION IMMÉDIATE DU JOKER LIBÉRÉ
    printf("\n=== UTILISATION IMMÉDIATE DU JOKER ===\n");
    printf("Vous devez utiliser le joker ID %d IMMÉDIATEMENT.\n", joker_libre.id);
    printf("Le joker n'est PAS dans votre chevalet.\n");
    
    // Afficher les combinaisons actuelles
    printf("\nTable actuelle :\n");
    afficher_combinaisons_table();
    
    // Afficher le chevalet actuel (SANS le joker)
    Tuile chevalet_actuel[MAX_TUILES];
    int nb_actuel = 0;
    charger_chevalet(j->chevalet, chevalet_actuel, &nb_actuel);
    
    printf("\nVotre chevalet ACTUEL (sans le joker) :\n");
    afficher_tuiles(chevalet_actuel, nb_actuel);
    
    // Proposer les options
    printf("\n=== COMMENT UTILISER CE JOKER ? ===\n");
    printf("1. Ajouter le joker à une combinaison existante sur la table\n");
    printf("2. Jouer une nouvelle combinaison avec le joker et mes tuiles\n");
    printf("3. Annuler et tout restaurer\n");
    printf("Votre choix (1-3) : ");
    
    int choix;
    scanf("%d", &choix);
    getchar();
    
    bool joker_utilise = false;
    
    if (choix == 1) {
        // Option 1: Ajouter à une combinaison existante
        printf("\nÀ quelle combinaison ajouter le joker ?\n");
        
        int nb_comb_table = compter_combinaisons_table();
        if (nb_comb_table == 0) {
            printf("Aucune combinaison sur la table.\n");
        } else {
            printf("Choisissez une combinaison (0-%d) : ", nb_comb_table - 1);
            int comb_choisie;
            scanf("%d", &comb_choisie);
            getchar();
            
            if (comb_choisie >= 0 && comb_choisie < nb_comb_table) {
                // Ajouter directement le joker à la combinaison
                if (ajouter_tuile_combinaison(&joker_libre, comb_choisie)) {
                    printf("✅ Joker ajouté à la combinaison [%d] !\n", comb_choisie);
                    joker_utilise = true;
                } else {
                    printf("❌ Impossible d'ajouter ce joker à cette combinaison.\n");
                }
            } else {
                printf("Numéro de combinaison invalide.\n");
            }
        }
    } else if (choix == 2) {
        // Option 2: Jouer une nouvelle combinaison avec le joker et les tuiles du joueur
        printf("\n=== JOUER UNE NOUVELLE COMBINAISON ===\n");
        printf("Le joker ID %d est forcément inclus.\n", joker_libre.id);
        printf("Quelles tuiles de votre chevalet voulez-y ajouter ? (IDs, min 2) : ");
        
        char ligne[256];
        if (!fgets(ligne, sizeof(ligne), stdin)) {
            printf("Erreur de lecture.\n");
        } else {
            // Analyser les IDs des tuiles additionnelles
            int ids_proposes[MAX_TUILES], nb_ids = 0;
            char *tok = strtok(ligne, " \n");
            while (tok && nb_ids < MAX_TUILES) {
                ids_proposes[nb_ids++] = atoi(tok);
                tok = strtok(NULL, " \n");
            }
            
            if (nb_ids < 2) {
                printf("Vous devez ajouter au moins 2 tuiles au joker.\n");
            } else {
                // Créer le tableau de la combinaison (joker + tuiles sélectionnées)
                Tuile combinaison[MAX_TUILES];
                int nb_comb = 0;
                
                // Ajouter le joker en premier
                combinaison[nb_comb++] = joker_libre;
                
                // Ajouter les tuiles sélectionnées du chevalet
                for (int i = 0; i < nb_ids; i++) {
                    bool trouve_tuile = false;
                    for (int k = 0; k < nb_actuel; k++) {
                        if (chevalet_actuel[k].id == ids_proposes[i]) {
                            combinaison[nb_comb++] = chevalet_actuel[k];
                            trouve_tuile = true;
                            break;
                        }
                    }
                    if (!trouve_tuile) {
                        printf("Tuile ID %d non trouvée dans votre chevalet.\n", ids_proposes[i]);
                        break;
                    }
                }
                
                if (nb_comb == nb_ids + 1) { // joker + toutes les tuiles trouvées
                    // Vérifier si la combinaison est valide
                    if (combinaison_valide(combinaison, nb_comb)) {
                        // Ajouter la combinaison à la table
                        ajouter_a_table(combinaison, nb_comb);
                        
                        // Retirer les tuiles utilisées du chevalet
                        Tuile chevalet_final[MAX_TUILES];
                        int nb_final = 0;
                        
                        for (int i = 0; i < nb_actuel; i++) {
                            bool utilisee = false;
                            for (int k = 0; k < nb_ids; k++) {
                                if (chevalet_actuel[i].id == ids_proposes[k]) {
                                    utilisee = true;
                                    break;
                                }
                            }
                            if (!utilisee) {
                                chevalet_final[nb_final++] = chevalet_actuel[i];
                            }
                        }
                        
                        // Sauvegarder le chevalet final
                        sauvegarder_chevalet(j->chevalet, *j, chevalet_final, nb_final, false);
                        
                        printf("✅ Nouvelle combinaison jouée avec succès !\n");
                        joker_utilise = true;
                    } else {
                        printf("❌ La combinaison n'est pas valide.\n");
                        printf("Joker %d + ", joker_libre.id);
                        for (int i = 0; i < nb_ids; i++) {
                            printf("%d ", ids_proposes[i]);
                        }
                        printf("= combinaison invalide.\n");
                    }
                }
            }
        }
    }
    
    // 11. VÉRIFICATION FINALE ET RESTAURATION SI ÉCHEC
    if (!joker_utilise) {
        printf("\n⚠️  ÉCHEC : Le joker n'a pas été utilisé.\n");
        printf("Restauration de l'état précédent...\n");
        
        // Restaurer la table
        if (table_avant) {
            FILE* f = fopen("table.json", "w");
            if (f) {
                fprintf(f, "%s", table_avant);
                fclose(f);
            }
        }
        
        // Restaurer le chevalet (avec la tuile qu'on avait utilisée)
        if (chevalet_avant) {
            FILE* f = fopen(j->chevalet, "w");
            if (f) {
                fprintf(f, "%s", chevalet_avant);
                fclose(f);
            }
        }
        
        printf("État restauré. Récupération du joker annulée.\n");
        
        if (table_avant) free(table_avant);
        if (chevalet_avant) free(chevalet_avant);
        return false;
    }
    
    // 12. NETTOYAGE
    if (table_avant) free(table_avant);
    if (chevalet_avant) free(chevalet_avant);
    
    printf("✅ Récupération et utilisation du joker réussies !\n");
    return true;
}

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
    
    // 5. VÉRIFICATION CRITIQUE : Est-ce que l'ajout est possible ?
    if (!peut_ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
        printf("❌ Impossible d'ajouter cette tuile à cette combinaison.\n");
        printf("   La combinaison serait invalide après ajout.\n");
        return false;
    }
    
    // 6. VÉRIFIER SI LA COMBINAISON CONTIENT UN JOKER
    bool contient_joker = false;
    int nb_t = compter_tuiles_combinaison(num_comb);
    for (int i = 0; i < nb_t; i++) {
        Tuile t;
        obtenir_tuile_table(num_comb, i, &t);
        if (t.joker) {
            contient_joker = true;
            break;
        }
    }
    
    // 7. LOGIQUE CONDITIONNELLE
    if (contient_joker) {
        // CAS 1 : AVEC JOKER → Ajouter seulement
        printf("\n⚠️  Cette combinaison contient un joker.\n");
        printf("Vous ne pouvez pas récupérer de tuile.\n");
        
        // L'ajout est déjà vérifié (étape 5)
        if (!ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
            printf("Échec technique de l'ajout.\n");
            return false;
        }
        
        // Retirer la tuile du chevalet
        Tuile nouveau_chevalet[MAX_TUILES];
        int nb_nouveau = 0;
        
        for (int i = 0; i < nb_tuiles; i++) {
            if (tuiles_joueur[i].id != id_tuile) {
                nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
                nb_nouveau++;
            }
        }
        
        // Sauvegarder
        sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
        
        printf("\n✅ Tuile ajoutée avec succès à la combinaison !\n");
        return true;
        
    } else {
        // CAS 2 : SANS JOKER
        printf("\n✅ Cette combinaison ne contient pas de joker.\n");
        
        // Vérifier si c'est une suite (seulement les suites permettent la récupération)
        if (!est_combinaison_suite(num_comb)) {
            printf("Cette combinaison n'est pas une suite.\n");
            printf("Ajout simple sans récupération.\n");
            
            // L'ajout est déjà vérifié (étape 5)
            if (!ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
                printf("Échec technique de l'ajout.\n");
                return false;
            }
            
            // Retirer la tuile du chevalet
            Tuile nouveau_chevalet[MAX_TUILES];
            int nb_nouveau = 0;
            
            for (int i = 0; i < nb_tuiles; i++) {
                if (tuiles_joueur[i].id != id_tuile) {
                    nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
                    nb_nouveau++;
                }
            }
            
            // Sauvegarder
            sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
            
            printf("\n✅ Tuile ajoutée avec succès !\n");
            return true;
        }
        
        // C'EST UNE SUITE SANS JOKER
        // Obtenir les tuiles aux extrémités
        Tuile premiere, derniere;
        obtenir_tuile_table(num_comb, 0, &premiere);
        obtenir_tuile_table(num_comb, nb_t - 1, &derniere);
        
        // VÉRIFICATION SPÉCIALE POUR LES JOKERS
        bool peut_gauche = false;
        bool peut_droite = false;
        
        if (tuile_a_ajouter->joker) {
            // Pour un joker, tester avec combinaison_valide
            // Tester ajout à gauche
            Tuile test_gauche[MAX_TUILES + 1];
            test_gauche[0] = *tuile_a_ajouter;
            for (int i = 0; i < nb_t; i++) {
                Tuile t;
                obtenir_tuile_table(num_comb, i, &t);
                test_gauche[i + 1] = t;
            }
            peut_gauche = combinaison_valide(test_gauche, nb_t + 1);
            
            // Tester ajout à droite
            Tuile test_droite[MAX_TUILES + 1];
            for (int i = 0; i < nb_t; i++) {
                Tuile t;
                obtenir_tuile_table(num_comb, i, &t);
                test_droite[i] = t;
            }
            test_droite[nb_t] = *tuile_a_ajouter;
            peut_droite = combinaison_valide(test_droite, nb_t + 1);
        } else {
            // Pour une tuile normale
            peut_gauche = (tuile_a_ajouter->couleur == premiere.couleur && 
                          tuile_a_ajouter->valeur == premiere.valeur - 1);
            
            peut_droite = (tuile_a_ajouter->couleur == derniere.couleur &&
                          tuile_a_ajouter->valeur == derniere.valeur + 1);
        }
        
        if (!peut_gauche && !peut_droite) {
            // Ne peut pas ajouter aux extrémités → ajout simple
            printf("Impossible d'ajouter aux extrémités. Ajout simple sans récupération.\n");
            
            if (!ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
                printf("Échec technique de l'ajout.\n");
                return false;
            }
            
            // Mettre à jour chevalet
            Tuile nouveau_chevalet[MAX_TUILES];
            int nb_nouveau = 0;
            for (int i = 0; i < nb_tuiles; i++) {
                if (tuiles_joueur[i].id != id_tuile) {
                    nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
                    nb_nouveau++;
                }
            }
            
            sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
            printf("✅ Tuile ajoutée.\n");
            return true;
        }
        
        // Peut ajouter aux extrémités → proposer la récupération
        printf("C'est une suite. Vous pouvez récupérer la tuile à l'autre extrémité.\n");
        printf("Voulez-vous récupérer une tuile ? (o/n) : ");
        char choix;
        scanf("%c", &choix);
        getchar();
        
        if (choix == 'o' || choix == 'O') {
            // Demander la direction
            bool gauche = false;
            if (peut_gauche && !peut_droite) {
                gauche = true;
            } else if (!peut_gauche && peut_droite) {
                gauche = false;
            } else {
                printf("Ajouter à gauche (1) ou droite (2) ? ");
                int dir;
                scanf("%d", &dir);
                getchar();
                gauche = (dir == 1);
            }
            
            // Effectuer l'extension avec récupération
            Tuile tuile_recuperee;
            if (!etendre_suite(tuile_a_ajouter, num_comb, gauche, &tuile_recuperee)) {
                printf("Échec de l'extension.\n");
                return false;
            }
            
            printf("Tuile récupérée : ID %d\n", tuile_recuperee.id);
            
            // 8. METTRE À JOUR LE CHEVALET AVEC LA TUILE RÉCUPÉRÉE
            Tuile nouveau_chevalet[MAX_TUILES];
            int nb_nouveau = 0;
            
            // Ajouter toutes les tuiles SAUF celle utilisée pour l'extension
            for (int i = 0; i < nb_tuiles; i++) {
                if (tuiles_joueur[i].id != id_tuile) {
                    nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
                    nb_nouveau++;
                }
            }
            
            // Ajouter la tuile récupérée
            nouveau_chevalet[nb_nouveau] = tuile_recuperee;
            nb_nouveau++;
            
            // Sauvegarder IMMÉDIATEMENT le nouveau chevalet
            sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
            
            // 9. FORCER L'UTILISATION IMMÉDIATE DE LA TUILE RÉCUPÉRÉE
            printf("\n=== UTILISATION IMMÉDIATE DE LA TUILE RÉCUPÉRÉE ===\n");
            printf("Vous devez utiliser la tuile ID %d immédiatement.\n", tuile_recuperee.id);
            
            // Recharger pour être sûr
            Tuile chevalet_actuel[MAX_TUILES];
            int nb_actuel = 0;
            charger_chevalet(j->chevalet, chevalet_actuel, &nb_actuel);
            
            printf("\nVotre chevalet ACTUEL (avec la tuile récupérée) :\n");
            afficher_tuiles(chevalet_actuel, nb_actuel);
            
            printf("\nTable actuelle :\n");
            afficher_combinaisons_table();
            
            printf("\nMaintenant, vous devez jouer une combinaison.\n");
            printf("Assurez-vous d'inclure la tuile ID %d dans votre combinaison.\n", tuile_recuperee.id);
            
            // Appeler jouer_combinaison
            jouer_combinaison(j);
            
            // Vérifier si la tuile a été utilisée
            charger_chevalet(j->chevalet, chevalet_actuel, &nb_actuel);
            bool tuile_toujours_presente = false;
            for (int i = 0; i < nb_actuel; i++) {
                if (chevalet_actuel[i].id == tuile_recuperee.id) {
                    tuile_toujours_presente = true;
                    break;
                }
            }
            
            if (!tuile_toujours_presente) {
                printf("✅ Tuile récupérée utilisée avec succès !\n");
                return true;
            } else {
                printf("❌ La tuile récupérée n'a pas été utilisée !\n");
                printf("Pour l'instant, on continue, mais c'est contraire aux règles.\n");
                return true;
            }
            
        } else {
            // Ajouter simplement sans récupération
            if (!ajouter_tuile_combinaison(tuile_a_ajouter, num_comb)) {
                printf("Échec de l'ajout.\n");
                return false;
            }
            
            // Mettre à jour chevalet
            Tuile nouveau_chevalet[MAX_TUILES];
            int nb_nouveau = 0;
            for (int i = 0; i < nb_tuiles; i++) {
                if (tuiles_joueur[i].id != id_tuile) {
                    nouveau_chevalet[nb_nouveau] = tuiles_joueur[i];
                    nb_nouveau++;
                }
            }
            
            sauvegarder_chevalet(j->chevalet, *j, nouveau_chevalet, nb_nouveau, false);
            printf("✅ Tuile ajoutée sans récupération.\n");
            return true;
        }
    }
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
        // VÉRIFICATION : La combinaison contient-elle un joker ?
        bool contient_joker = false;
        int nb_t = compter_tuiles_combinaison(i);
        for (int k = 0; k < nb_t; k++) {
            Tuile t;
            obtenir_tuile_table(i, k, &t);
            if (t.joker) {
                contient_joker = true;
                break;
            }
        }
        
        if (contient_joker) {
            // On continue à chercher dans d'autres combinaisons
            continue;
        }
        
        // Vérifier si c'est une suite (seulement si pas de joker)
        if (!est_combinaison_suite(i)) continue;
        
        // Chercher la tuile
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
        printf("Tuile ID %d non trouvée sur la table ou la combinaison contient un joker.\n", id_tuile_division);
        printf("Division impossible sur une suite contenant un joker.\n");
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
