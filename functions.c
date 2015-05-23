#include "functions.h"


/****************************************************************************************
 * fonction pour changer le chemin - donne le nouveau chemin                            *
 ****************************************************************************************/
Liste *change_chemin(void) {
  fflush(stdin);
  char nouveau_chemin[30];
  if(fgets(nouveau_chemin, 30, stdin) == NULL) {
    perror("Error");
    return NULL;
  }
  unsigned int i = 0;
  for(i = 0; i < 30; i++) {
    if(nouveau_chemin[i] == '\n') {
      nouveau_chemin[i] = '\0';
      break;
    }
  }
  char chemin[i+1];
  unsigned int j = 0;
  for(j = 0; j < i+1; j++) {
    chemin[j] = nouveau_chemin[j];
  }
  //check
  if(chemin == NULL) {
    puts("-> Impossible de créer une table de hashage!\n");
    return NULL;
  }
  if(creer_hashtable(chemin) == NULL) {
    puts("-> Impossible de créer une table de hashage!\n");
    return NULL;
  } else {
    return creer_hashtable(chemin);  
  }
}


/****************************************************************************************
 * fonction de hashage - donne la valeur de hashage                                     *
 * @param - str:  le mot pour lequel on crée une valeur de hashage                      *
 ****************************************************************************************/
int hash(const unsigned char *str) {
  int hash_value = 0;
  int k = strlen(str);
  int multiplier = 31;
  int i;
  for(i = 0; i < k; i++) {
    hash_value = (hash_value * multiplier + str[i]) % TAILLE_HASHTABLE;
  }
  //printf( "\nhash_value: %d\n", hash_value);
  return hash_value;
}


/****************************************************************************************
 * fonction pour ajouter un mot dans la table de hashage                                *
 * @param - mot: le mot à ajouter                                                       *
 * @param - longueur_mot: le longueur du mot à ajouter                                  *
 * @param - hashtable: la table de hashage crée, dans laquelle on ajoute le mot         *
 ****************************************************************************************/
void ajout_mot(const unsigned char *mot, const unsigned int taille_mot, Liste *hashtable) {
  int index = hash(mot);
  if(hashtable[index] == NULL) {
    //Key unused: create new entry
    hashtable[index] = ajout_tete(mot, taille_mot, NULL);
  } else {
    //Collision: add to list
    hashtable[index]  = ajout_tete(mot, taille_mot, hashtable[index]);
    //printf("\nAdded to list: %s", mot); 
  }
  //Count actual size of Hashtable
  //taille_table++;
}


/****************************************************************************************
 * fonction de crèation d'une table de hashage - donne la tete du tableau               *
 * @param - f: le chemin d'un fichier *.txt avec les mots                               *
 ****************************************************************************************/
Liste *creer_hashtable(const char *f) {
  FILE *fichier = NULL;
  //Open a text file with path+name f
  fichier = fopen(f,"r+");
  //Error if file cannot be opened
  if(!fichier) {
    perror("Impossible de lire/ouvrir le fichier");
    return NULL;
  }
  //Hashtable needs to have a static position in memory
  static Liste hashtable[TAILLE_HASHTABLE];
  //Keep track of how many elements we add
  //static int taille_table = 0;
  //Read text file
  char mot[MAX_TAILLE_MOT];
  while(fgets(mot, MAX_TAILLE_MOT + 1, fichier) != NULL) {
    if(mot[0] != '\n') {
      unsigned int vraie_longueur_mot = 0;
      unsigned int i = 0;
      while(mot[vraie_longueur_mot] != '\n') {
	vraie_longueur_mot++;
      }
      char cpymot[vraie_longueur_mot];
      for(i = 0; i < vraie_longueur_mot; i++) {
	cpymot[i] = mot[i];
      }
      ajout_mot(cpymot, vraie_longueur_mot, hashtable);
    }
  }
  fclose(fichier);
  return hashtable;
}


/****************************************************************************************
 * fonction pour créer un graphe des mots avec leur cout et père                        *
 * @param - longueur_mot: le longueur du mot de départ                                  *                                
 * @param - hashtable: la table de hashage crée                                         *
 ****************************************************************************************/
Cout_Liste creer_graphe(const unsigned int longueur_mot, const Liste *hashtable) {
  Cout_Liste graphe_liste = NULL;
  Liste p = NULL;
  unsigned int i = 0;
  for(i = 0; i < 1000; i++) {
    if( hashtable[i] != NULL) {
      for(p = hashtable[i]; p != NULL; p = p->suiv) {
	if(longueur_mot == strlen(p->val)) {
	    graphe_liste = ajout_cout_tete(p->val, longueur_mot, INT_MAX, NULL, graphe_liste);
	    //printf("\n%s",p->val);
	}
      }
    }
  }
  return graphe_liste;
}


/****************************************************************************************
 * fonction qui trouve tous les proches voisins à distance 1 - les donne comme liste    *
 * @param - mot: le mot pour lequel on cherche les proches voisins                      *
 * @param - hashtable: la table de hashage crée                                         *
 ****************************************************************************************/
Liste get_proche_voisins(const unsigned char *mot, const Liste *hashtable) {
  unsigned int i;
  unsigned char c;
  unsigned int longueur_mot = strlen(mot);
  unsigned char voisin_mot[longueur_mot];
  Liste head_of_collision_list = NULL;
  Liste liste_voisins = NULL;
  for(i = 0; i < longueur_mot; i++) {
    for(c = 'a'; c <= 'z'; c++) {
      strcpy(voisin_mot, mot);
      voisin_mot[i] = c;
      head_of_collision_list = hashtable[hash(voisin_mot)];
      //printf("\n%s", head_of_collision_list->val);
      if(head_of_collision_list && !compare_mots(mot, voisin_mot, longueur_mot)) {
	if(recherche_liste(voisin_mot, longueur_mot, head_of_collision_list)) {
	  liste_voisins = ajout_tete(voisin_mot, longueur_mot, liste_voisins);
	}
      }
    }
  }
  return liste_voisins;
}


/****************************************************************************************
 * fonction pour trouver le plus court chemin en utilisant l'algorithme de Dijkstra     *
 * @param - mot_depart: le mot de départ                                                *
 * @param - mot_cible: le mot cible                                                     *
 * @param - hashtable: la table de hashage crée                                         *
 ****************************************************************************************/
void get_court_chemin(const unsigned char *mot_depart, const unsigned char *mot_cible, const Liste *hashtable) {
  if(strlen(mot_depart) != strlen(mot_cible)) {
    puts("\n\nThe words you have entered have different lengths and are thus not comparable!\n");
    return;
  }
  unsigned int longueur_mot = strlen(mot_depart);
  if(compare_mots(mot_depart, mot_cible, longueur_mot)) {
    puts("\n\nYou have entered the same word twice!\n");
  }
  static Cout_Liste chemin_liste = NULL;
  
  Cout_Liste graphe_liste = creer_graphe(longueur_mot, hashtable);
  Cout_Liste j = recherche_cout_liste(mot_depart, longueur_mot, graphe_liste);
  j->cout = 0;
  Cout_Liste but = NULL;
  do {
    j = trouve_sommet_min_cout(graphe_liste);
    Cout_Liste k = NULL;
    Liste l = NULL;
    Liste distance_1 = get_proche_voisins(j->val, hashtable);
    chemin_liste = ajout_cout_tete(j->val, longueur_mot, j->cout, j->pere, chemin_liste);
    //printf("TO KILL: %s\t%d\n", j->val, j->cout);
    graphe_liste = supprime_cout_sommet(j, graphe_liste);
    //puts("GOT KILLED\n");
    for(l = distance_1; l != NULL; l = l->suiv) {
      k = recherche_cout_liste(l->val, longueur_mot, chemin_liste);
      if(k == NULL) {
	k = recherche_cout_liste(l->val, longueur_mot, graphe_liste);
	if(k->cout > (chemin_liste->cout + 1)) {
	  k->cout = chemin_liste->cout + 1;
	  k->pere = chemin_liste;
	}
      }
      //printf("PROCHE VOISIN: %s\t%d\n", k->val, k->cout);
    }
    but = recherche_cout_liste(mot_cible, longueur_mot, chemin_liste);
  } while((but == NULL) && (j->cout != INT_MAX));
  if(but == NULL) {
    puts("\n\nThere is no path in the dictionary between your words!");
    return;
  }
  Cout_Liste l = but;
  while(l != NULL) {
    printf("\t%s\t%d\n", l->val, l->cout);
    l=l->pere;
    }
  puts("\n\nPoussez RETURN pour rentrer dans le menu.\n");
  return;
}

/****************************************************************************************
 * fontion pour lire les mots et trouver le chemin en utilisant get_proche_voisins()    *
 * @param - hashtable: la table de hashage crée                                         *
 ****************************************************************************************/
void trouve_chemin(const Liste *hashtable) {
  char mot_d[MAX_TAILLE_MOT+1];
  char mot_c[MAX_TAILLE_MOT+1];
  fflush(stdin);
  puts("\nEntrez votre mot de départ:\t");
  if(fgets(mot_d, MAX_TAILLE_MOT+1, stdin) == NULL) {
    perror("\nError");
    return;
  }
  fflush(stdin);
  puts("\nEntrez votre mot cible:\t");
  if(fgets(mot_c, MAX_TAILLE_MOT+1, stdin) == NULL) {
    perror("\nError");
    return;
  }
  
  unsigned int i = 0;
  for(i = 0; i < MAX_TAILLE_MOT+1; i++) {
    if(mot_d[i] == '\n') {
      mot_d[i] = '\0';
      break;
    }
  }
  char mot_depart[i+1];
  strcpy(mot_depart, mot_d);
  
  unsigned int k = 0;
  for(k = 0; k < MAX_TAILLE_MOT+1; k++) {
    if(mot_c[k] == '\n') {
      mot_c[k] = '\0';
      break;
    }
  }
  char mot_cible[k+1];
  strcpy(mot_cible, mot_c);

  puts("\n\n");
  get_court_chemin(mot_depart, mot_cible, hashtable);
}
  
  
