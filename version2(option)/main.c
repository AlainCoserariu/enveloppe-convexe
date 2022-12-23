/**
 * @file main.c
 * @author MAATOUK Yanis COSERARIU Alain
 * @brief Projet de programation C portant sur les enveloppes convexes modélisé par des listes chainées (2022-2023) \n
 *        Cette version propose des enveloppes convexes emboitées
 * @version 0.1
 * @date 2022-12-19
 */

#include <stdio.h>
#include <stdlib.h>
#include <MLV/MLV_all.h>
#include <time.h>
#include <math.h>

#define M_PI 3.14159265358979323846
#define BLOC 25

/**
 * @brief Coordonnées d'un point en 2 dimension
 * 
 */
typedef struct {
    double x;
    double y;
} Point;

/**
 * @brief Ensemble de points
 * 
 */
typedef struct {
    Point* tabPoints;  // Pointeur sur un tableau de points
    int nbPoints;
    int maxLen;  // taille du tableau
} Ensemble;

/**
 * @brief Cellule d'une liste polygone
 * 
 */
typedef struct _vertex_ {
    Point *p;  // Un point de l'ensemble
    struct _vertex_ *prec;  // Le vertex précédent
    struct _vertex_ *suiv;  // Le vertex suivant
} Vertex, *Polygone;

/**
 * @brief Cellule d'une liste **ListeEnveloppe** contenant un Polygone est 
 *        plusieurs renseignements utiles (longeur courent, longueur maximal, longueur moyenne)
 * 
 */
typedef struct _convex_ {
    Polygone p;  // Polygône représentant l'enveloppe convexe
    int curlen;  // Nombre de points du polygône
    int maxlen;  // Nombre de points maximale du polygône
    float avg;   // Nombre de points moyen du polynôme
    int nbMaj;  // Nombre de mise à jour de l'enveloppe
    struct _convex_ *suiv;  // Enveloppe convex contenue dans celle-ci
} ConvexHull, *ListeEnveloppes;

// --------------------Fonction de manipulations des points--------------------

/**
 * @brief Affiche les coordonnées de tous les point d'une liste **p** de longueur **lenP**
 * 
 * @param p Liste à afficher
 * @param lenP Longueur de la liste
 */
void printListePoint(Point *p, int lenP) {
    for (int k = 0; k < lenP; ++k) {
        printf("%f %f\n", p[k].x, p[k].y);
    }
}

// ---------------------Fonctions de gestion des Polygones---------------------

/**
 * @brief Alloue et renvoie un pointeur sur une zone mémoire pour un Vertex
 * 
 * @param p Addresse du point contenue dans le vertex
 * @return Vertex* zone mémoire alloué
 */
Vertex* allocCellVertex(Point *p) {
    Vertex *cell = (Vertex*)malloc(sizeof(Vertex));
    if (!cell) {
        return cell;
    }
    cell->p = p;
    cell->suiv = NULL;
    cell->prec = NULL;
    return cell;
}

/**
 * @brief Ajoute un vertex de point **p** en tant que premier point du polygone **Poly**
 * 
 * @param poly Polygône modifié
 * @param p Point ajouté
 * @return int 1 : l'allocation d'un Vertex a réussi \n
 *             0 : l'allocation d'un Vertex a échoué 
 */
int ajouteVertexPolygone(Polygone *poly, Point *p) {
    Vertex* cell = allocCellVertex(p);
    if (!cell) {
        printf("\nEchec de l'allocation de mémoire pour un Vertex\n");
        return 0;
    }

    // Si la liste est vide on l'initialise avec une cellule qui pointe sur elle même
    if (!(*poly)) {
        *poly = cell;
        cell->suiv = cell;
        cell->prec = cell;
        return 1;
    }

    // Insére cell en tant que premier élément de *poly 
    cell->suiv = *poly;
    cell->prec = (*poly)->prec;
    (*poly)->prec = cell;
    (cell->prec)->suiv = cell;
    *poly = cell;
    return 1;
}

/**
 * @brief Enlève un vertex du polygone
 * 
 * @param v Vertex à enveler
 * @param poly Polygone contenant v
 */
void supprimeVertexPolygone(Vertex *v, Polygone *poly) {
    if (v == *poly) {
        (*poly) = (*poly)->suiv;
    }

    if (v->suiv == v) {
        *poly = NULL;
    }

    (v->prec)->suiv = v->suiv;
    (v->suiv)->prec = v->prec;
}

/**
 * @brief Libére la mémoire d'une cellule Vertex **v** contenue dans le polygone **poy**
 * 
 * @param v Vertex à libérer
 * @param poly Polygône contenant le Vertex
 */
void freeVertex(Vertex *v, Polygone *poly) {
    supprimeVertexPolygone(v, poly);
    free(v);
}

/**
 * @brief Libére l'espace alloué par un polygone
 * 
 * @param Poly 
 */
void freePolygone(Polygone *Poly) {
    while (*Poly) {
        freeVertex(*Poly, Poly);
    }
}

/**
 * @brief Affiche les coordonnées de chaque Vertex d'un polygone **poly**
 * 
 * @param poly Polygone à afficher
 */
void printPolygone(Polygone poly) {
    if (!poly) {
        return;
    }

    Vertex* tete = poly;
    printf("%f %f\n", poly->p->x, poly->p->y);
    poly = poly->suiv;
    for (; poly != tete; poly = poly->suiv) {
        printf("%f %f\n", poly->p->x, poly->p->y);
    }
}

// ----------------Fonctions de gestion d'un ensemble de points----------------

/**
 * @brief Initialise un ensemble à partir d'un tableau de points
 * 
 * @param tabPoints tableau des points
 * @param lenTabPoints longueur max du tableau tabPoints
 * @return Ensemble ensemble initialisé
 */
Ensemble initEnsemble(Point* tabPoints, int lenTabPoints) {
    Ensemble e;
    e.tabPoints = tabPoints;
    e.nbPoints = 0;
    e.maxLen = lenTabPoints;
    return e;
}

/**
 * @brief Réallour la mémoire pour un tableau de point pour ajouter **bloc** places
 * 
 * @param tabPoints Tableau à réallouer
 * @return int 0 : Echec de la réallocation \n
 *             1 : Réussit de la réallocation
 */
int reAllocTabPoint(Ensemble *e) {
    e->tabPoints = (Point*)realloc(e->tabPoints, sizeof(Point) * (e->maxLen +  BLOC));
    if (e->tabPoints) {
        e->maxLen += BLOC;
        return 1;
    }
    free(e->tabPoints);
    return 0;
}

/**
 * @brief Ajoute un point **p** à un ensemble **e**
 * 
 * @param e ensemble à modifier
 * @param p point à ajouter
 * 
 * @return int 0 : Echec de la réallocation \n
 *             1 : Réussit de la réallocation
 */
int ajoutePointEnsemble(Ensemble *e, Point p) {
    if (e->nbPoints == e->maxLen) {
        if (!reAllocTabPoint(e)) {
            return 0;
        }
    }
    e->tabPoints[e->nbPoints] = p;
    e->nbPoints++;
    return 1;
}

/**
 * @brief Affiche les coordonnées des points de l'ensemble ainsi que ses informations
 * 
 * @param e 
 */
void printEnsemble(Ensemble e) {
    int k;
    for (k = 0; k < e.nbPoints; ++k) {
        printf("%f %f\n", e.tabPoints[k].x, e.tabPoints[k].y);
    }
    printf("longueur tableau : %d\n", e.nbPoints);
    printf("longueur max tableau : %d\n", e.maxLen);
}

// ---------------------Fonctions de gestion des ConvexHull--------------------

/**
 * @brief Initialise et renvoie une structure ConvexHull **c** avec des valeurs nulls
 * 
 * @return c Structure initialisé
 */
ConvexHull initConvexHull(void) {
    ConvexHull c;
    c.p = NULL;
    c.avg = 0;
    c.curlen = 0;
    c.maxlen = 0;
    c.nbMaj = 0;
    c.suiv = NULL;
    return c;
}

/**
 * @brief Initialise et alloue la mémoire nécessaire pour une cellule ConvexHull
 * 
 * @return ConvexHull* 
 */
ConvexHull* allocEnveloppeConvex(void) {
    ConvexHull* c = (ConvexHull*)malloc(sizeof(ConvexHull));
    if (!c) {
        return c;
    }

    *c = initConvexHull();
    return c;
}

/**
 * @brief Libére la mémoire allouer pour une liste d'enveloppe convex **l**
 * 
 * @param l 
 */
void freeListeEnveloppeConvex(ListeEnveloppes *l) {
    ConvexHull* tmp = *l;
    while (*l) {
        tmp = (*l)->suiv;
        freePolygone(&((*l)->p));
        free(*l);
        *l = tmp;
    }
}

/**
 * @brief Met à jour les données de **env_convex**
 * 
 * @param env_convex envoloppe à mettre à jour
 */
void majConvexHull(ConvexHull* env_convex) {
    // Détérmine si la longueur max a changé
    if (env_convex->curlen > env_convex->maxlen) {
        env_convex->maxlen = env_convex->curlen;
    }
    // Recalcule de la moyenne
    env_convex->avg = (env_convex->avg * env_convex->nbMaj + env_convex->curlen) / (env_convex->nbMaj + 1);

    env_convex->nbMaj++;
}

/**
 * @brief Renvoie la longueur de la liste **lst**
 * 
 * @param lst 
 * @return int longueur de **lst**
 */
int lenListeConvexHull(ListeEnveloppes lst) {
    int cmpt = 0;
    for (; lst; lst = lst->suiv) {
        cmpt++;
    }
    return cmpt;
}

// ----------------Algorithme d'ajout de points dans l'ensemble----------------

/**
 * @brief Détérmine si un triangle formé de **a** **b** et **c** est directe selon le sens trigonométrique
 * 
 * @param a 
 * @param b 
 * @param c 
 * @return int 1 : Le triangle est directe \n
 *             0 : Le triangle est indirecte
 */
int triangleDirecte(Point a, Point b, Point c) {
    // Comme notre axe des ordonnées n'a pas la même orientation, on change le signe des ordonnées dans la formule
    double scalaire = ((b.x - a.x) * ((-c.y) - (-a.y))) - ((c.x - a.x) * ((-b.y) - (-a.y)));
    if (scalaire >= 0) {
        return 1;
    }
    return 0;
}

/**
 * @brief Regarde si le point p est dans l'enveloppe ou non et la modifie en concéquence
 * 
 * @param env_convex Enveloppe à mettre à jour
 * @param p Point à vérifier
 */
void majEnveloppeConvex(ConvexHull *env_convex, Point *p) {
    // Si il y a moins de 2 points dans env_convex on ajoute p à l'enveloppe
    if (env_convex->curlen < 2) {
        if (!ajouteVertexPolygone(&(env_convex->p), p)) {
            printf("\nERREUR LORS DE L'ALLOCATION DE LA MEMOIRE FIN DU PROGRAMME\n");
            exit(1);
        }
        env_convex->curlen++;
        majConvexHull(env_convex);
        return;
    }

    // Orientation du premier triangle
    if (env_convex->curlen == 2) {
        // Si les trois premiers points forment un triangle indirecte, on reforme la liste pour avoir un direct
        if (!ajouteVertexPolygone(&(env_convex->p), p)) {
            printf("\nERREUR LORS DE L'ALLOCATION DE LA MEMOIRE FIN DU PROGRAMME\n");
            exit(1);
        }
        if (!triangleDirecte(*(env_convex->p->p), *(env_convex->p->suiv->p), *(env_convex->p->prec->p))) {
            Point *tmp = env_convex->p->suiv->p;
            env_convex->p->suiv->p = env_convex->p->prec->p;
            env_convex->p->prec->p = tmp;
        }
        env_convex->curlen++;
        majConvexHull(env_convex);
        return;
    }

    Vertex *s_i = env_convex->p;
    Vertex *s_j = s_i->suiv;
    do {
        s_i = s_j;
        s_j = s_j->suiv;
    } while (triangleDirecte(*p, *(s_i->p), *(s_j->p)) && s_i != env_convex->p);  // Tant que le triangle est directe est qu'on a pas vérifier tous les points

    // Si on est sortit de la boucle car tous les triangles sont directes, on met à jour les sous enveloppes
    if (triangleDirecte(*p, *(s_i->p), *(s_j->p)) && s_i == env_convex->p) {
    
        if (!env_convex->suiv) {  // Si il n'y a pas d'enveloppe convexe on en crée une nouvelle
            env_convex->suiv = allocEnveloppeConvex();
        }

        majEnveloppeConvex(env_convex->suiv, p);  // Démarre une récursivité en mettant à jours les sous enveloppes

        majConvexHull(env_convex);
        return;
    }

    // si on à un triangle indirecte
    // On insére p en tant que nouvelle tête de liste entre s_i et s_j
    env_convex->p = s_j;
    if (!ajouteVertexPolygone(&(env_convex->p), p)) {
        printf("\nERREUR LORS DE L'ALLOCATION DE LA MEMOIRE FIN DU PROGRAMME\n");
        exit(1);
    }
    env_convex->curlen++;
    
    // Nettoyage avant
    s_i = env_convex->p->suiv;
    s_j = s_i->suiv;
    while (!triangleDirecte(*(env_convex->p->p), *(s_i->p), *(s_j->p))) {
        if (!env_convex->suiv) {  // Si il n'y a pas d'enveloppe convexe on en crée une nouvelle
            env_convex->suiv = allocEnveloppeConvex();
        }
        majEnveloppeConvex(env_convex->suiv, s_i->p);  // Démarre une récursivité en mettant à jours les sous enveloppes
        
        freeVertex(s_i, &(env_convex->p));
        s_i = env_convex->p->suiv;
        s_j = s_i->suiv;
        env_convex->curlen--;
    }

    // Nettoyage arrière
    s_i = env_convex->p->prec->prec;
    s_j = env_convex->p->prec;
    while (!triangleDirecte(*(env_convex->p->p), *(s_i->p), *(s_j->p))) {
        if (!env_convex->suiv) {  // Si il n'y a pas d'enveloppe convexe on en crée une nouvelle
            env_convex->suiv = allocEnveloppeConvex();
        }
        majEnveloppeConvex(env_convex->suiv, s_j->p);  // Démarre une récursivité en mettant à jours les sous enveloppes

        freeVertex(s_j, &(env_convex->p));
        s_i = env_convex->p->prec->prec;
        s_j = env_convex->p->prec;
        env_convex->curlen--;
    }
    majConvexHull(env_convex);
}

// -------------------Fonctions gestions interface graphique-------------------

/**
 * @brief Attend un clic ou une entrée clavier, actualise la position de la souris en cas de clic
 * 
 * @param souris Coordonnées de la souris
 * @return MLV_Event 
 */
MLV_Event actualiseSouris(Point *souris){
    int souris_x;
    int souris_y;
    MLV_Event event_type = MLV_wait_keyboard_or_mouse(NULL, NULL, NULL, &souris_x, &souris_y);
    if (event_type == MLV_MOUSE_BUTTON) {
        souris->x = (double)souris_x;
        souris->y = (double)souris_y;

        // Ajout d'une petite fluctuation pour ne pas avoir deux points superposées
        souris->x += (double)rand() / RAND_MAX;
        souris->y += (double)rand() / RAND_MAX;
    }
    
    return event_type;
}

/**
 * @brief Dessine tous les points de l'ensemble **e** sur une fenêtre MLV
 * 
 * @param e Ensemble dessiné
 */
void dessineEnsemble(Ensemble e, MLV_Color couleur) {
    int k;
    for (k = 0; k < e.nbPoints; ++k) {
        MLV_draw_filled_circle((int)e.tabPoints[k].x, (int)e.tabPoints[k].y, 1, couleur);
    }
}

/**
 * @brief Dessine le polygône **poly**
 * 
 * @param poly Polygône à dessiner
 */
void dessinePolygone(Polygone poly, MLV_Color couleur) {
    Vertex *tete = poly;
    MLV_draw_filled_circle((int)poly->p->x, (int)poly->p->y, 1, couleur);
    MLV_draw_line((int)poly->p->x, (int)poly->p->y, (int)poly->suiv->p->x, (int)poly->suiv->p->y, couleur);
    poly = poly->suiv;
    for (; poly != tete; poly = poly->suiv) {
        MLV_draw_filled_circle((int)poly->p->x, (int)poly->p->y, 1, couleur);
        MLV_draw_line((int)poly->p->x, (int)poly->p->y, (int)poly->suiv->p->x, (int)poly->suiv->p->y, couleur);
    }
}

void dessineListeEnveloppe(ListeEnveloppes l) {
    float pasMilieu = (float)255 / (lenListeConvexHull(l) / 2);  // Nombre de pas necessaire pour aller du rouge au vert et du vert au bleu
    float r = 255, g = 0, b = 0;
    MLV_Color couleur = MLV_rgba(r, g, b, 255);

    for (; l; l = l->suiv) {
        if (l->curlen > 2) {
            dessinePolygone(l->p, couleur);
        }

        // Gradiant de couleur qui part du rouge pour tendre vers le bleu
        if (r > 0) {
            r -= pasMilieu;
            g += pasMilieu;
        } else {
            g -= pasMilieu;
            b += pasMilieu;
        }

        if (r < 0) {
            r = 0;
        }
        if (g < 0) {
            g = 0;
        }

        couleur = MLV_rgba(r, g, b, 255);
    }
}

// -----------------Fonctions de génération aléatoire de point-----------------

/**
 * @brief Génére un point dans une forme de centre **centre** et de demi-côté **rayon**
 * 
 * @param centre Centre de la forme
 * @param rayon rayon du demi-côté
 * @param forme 0 : Carré \n
 *              1 : Cercle
 * @return Point Point généré
 */
Point generationPoint(int centre, int rayon, int forme) {
    Point p;
    
    // Distribution dans un carré
    if (forme == 0) {
        p.x = (centre - rayon) + (2 * rayon) * (double)rand() / RAND_MAX;
        p.y = (centre - rayon) + (2 * rayon) * (double)rand() / RAND_MAX;
        
    } else {  // Distribution dans un cercle
        p.x = centre;
        p.y = centre;
        double distCentre = rayon * (double)rand() / RAND_MAX;  // distance par rapport au centre
        double angle = 2 * M_PI * (double)rand() / RAND_MAX;  // Angle aléatoire
        p.x += distCentre * cos(angle);
        p.y += distCentre * sin(angle);
    }
    return p;
}

// ---------------------------Vartientes du programme--------------------------

/**
 * @brief Lance une fenêtre MLV permettant de positionner des points avec la 
 * souris et forme l'enveloppe convexe contenant ces points tant qu'on appuie 
 * pas sur une touche du clavier
 * 
 */
void mainEnveloppeSouris() {
    int l = 0;
    ListeEnveloppes lst_env_convex = allocEnveloppeConvex();
    *lst_env_convex = initConvexHull();  // initialise la première enveloppe

    Point *tabPoints = (Point*)calloc(BLOC, sizeof(Point));

    Ensemble e = initEnsemble(tabPoints, BLOC);

    MLV_Event event_type = MLV_NONE;
    Point souris;  // Coordonnées de la souris
    
    int longueur_fen = 500;
    int largeur_fen = 500;
    MLV_create_window("Enveloppe convexe", "", longueur_fen, largeur_fen);
    MLV_clear_window(MLV_rgba(30, 30, 30, 255));
    MLV_actualise_window();

    // Actualise la position de la souris et ajoute une petite fluctuation
    event_type = actualiseSouris(&souris);
    while (event_type == MLV_MOUSE_BUTTON) {
        if (!ajoutePointEnsemble(&e, souris)) {
            printf("ERREUR LORS DE L'ALLOCATION DE LA MEMOIRE FIN DU PROGRAMME");
            exit(1);
        }

        // Si on a réalloué la mémoire il faut rediriger les pointeurs de l'enveloppe convexe vers la nouvelle zone mémoire
        if (!((e.nbPoints - 1) % (BLOC))) {
            freeListeEnveloppeConvex(&lst_env_convex);
            lst_env_convex = allocEnveloppeConvex();
            *lst_env_convex = initConvexHull();
            for (l = 0; l < e.nbPoints; ++l) {
                majEnveloppeConvex(lst_env_convex, &(e.tabPoints[l]));
            }
        } else {  // Sinon on met à jour juste avec le dernier point ajouter
            majEnveloppeConvex(lst_env_convex, &(e.tabPoints[e.nbPoints - 1]));
        }

        printf("ENVELOPPE CONVEXE INFO :\n");
        printf("    curlen = %d \n", lst_env_convex->curlen);
        printf("    maxlen = %d \n", lst_env_convex->maxlen);
        printf("    avg = %f \n", lst_env_convex->avg);
        printf("    nbMaj = %d \n", lst_env_convex->nbMaj);
        printf("\n");

        dessineEnsemble(e, MLV_COLOR_BLUE);
        dessineListeEnveloppe(lst_env_convex);

        MLV_actualise_window();
        MLV_clear_window(MLV_rgba(30, 30, 30, 255));
        
        event_type = actualiseSouris(&souris);
    }

    MLV_free_window();

    // Libération de la mémoire
    free(e.tabPoints);
    freeListeEnveloppeConvex(&lst_env_convex);
}

/**
 * @brief Place des points aléatoirement selon une répartition **forme** de 
 * centre **centre** et de demi-côté **rayon** puis forme l'enveloppe convexe 
 * contenant ces points \n
 * Si le distribution pseudo-spiral est activé, la forme grandit jusqu'à 
 * atteindre le bord de la fenêtre
 * 
 * @param centre Centre de la forme
 * @param rayon rayon initial du demi-côté
 * @param nbPoints Nombre de points à générer
 * @param forme 0 : Carré \n
 *              1 : Cercle
 * @param spiral : distribution pseudo-spiral des points        
 * @param dynamique 1 : Affichage point par point \n
 *                  0 : Affichage unique une fois l'algorithme fini
 * 
 */
void mainEnveloppeForme(int centre, double rayon, int nbPoints, int forme, int spiral, int dynamique) {
    ListeEnveloppes lst_env_convex = allocEnveloppeConvex();
    *lst_env_convex = initConvexHull();  // initialise la première enveloppe

    Point *tabPoints = (Point*)malloc(nbPoints * sizeof(Point));
    Ensemble e = initEnsemble(tabPoints, nbPoints);

    MLV_Keyboard_button sym = MLV_KEYBOARD_NONE;  // Récupére les potentielles entrées de touches
    int longueur_fen = 500;
    int largeur_fen = 500;
    MLV_create_window("Enveloppe convexe", "", longueur_fen, largeur_fen);
    MLV_clear_window(MLV_rgba(30, 30, 30, 255));
    MLV_actualise_window();

    double pas = 0;  /* Détérmine le pas pour augmenter le rayon au fur et à 
                        mesure de l'ajout des points dans le cas d'une distribution spirale */
    if (spiral) {
        pas = ((largeur_fen - 20) - (centre + rayon)) / nbPoints;
    }

    Point pointAleat;
    int k = 0;
    while (k < nbPoints && sym != MLV_KEYBOARD_ESCAPE) {
        //Génére un point de type double dans le carré
        pointAleat = generationPoint(centre, rayon, forme);

        if (!ajoutePointEnsemble(&e, pointAleat)) {
            printf("ERREUR LORS DE L'ALLOCATION DE LA MEMOIRE FIN DU PROGRAMME");
            exit(1);
        }

        majEnveloppeConvex(lst_env_convex, &(e.tabPoints[k]));

        if (spiral) {
            rayon += pas;
        }

        if (dynamique) {
            dessineEnsemble(e, MLV_COLOR_BLUE);
            dessineListeEnveloppe(lst_env_convex);

            MLV_actualise_window();
            MLV_clear_window(MLV_rgba(30, 30, 30, 255));
        }

        MLV_get_event(&sym, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (sym == MLV_KEYBOARD_SPACE) {
            dynamique = 0;
        }

        k++;
    }

    dessineEnsemble(e, MLV_COLOR_BLUE);
    dessineListeEnveloppe(lst_env_convex);

    MLV_actualise_window();

    // Attend une entrée utilisateur pour quitter le programme
    MLV_wait_keyboard_or_mouse(NULL, NULL, NULL, NULL, NULL);

    MLV_free_window();

    // Libération de la mémoire
    free(e.tabPoints);
    freeListeEnveloppeConvex(&lst_env_convex);
}

// ------------------------------------Menu------------------------------------

/**
 * @brief Vérifie un point de coordonnées (**x**, **y**) se situe dans un carré de **largeur** par **hauteur** \n
 * se situent en (**x1**, **y1**) et en (**x2**, **y2**)
 * 
 * @param x coordonnées en x du point à vérifier
 * @param y coordonnées en y du point à vérifier
 * @param x1 coordonnées en x du coin supérieur gauche du rectangle
 * @param y1 coordonnées en x du coin supérieur gauche du rectangle
 * @param largeur Largeur du rectangle
 * @param hauteur Hauteur du rectangle
 * @return int 1 : Le point est dans le carré \n
 *             0 : Le point n'est pas dans le carré
 */
int pointDansRectangle(int x, int y, int x1, int y1, int largeur, int hauteur) {
    if (x1 <= x && x <= x1 + largeur && y1 <= y && y <= y1 + hauteur) {
        return 1;
    }
    return 0;
}

/**
 * @brief Dessine un boutton de dimension **largeur** par **hauteur** contenant un texte **text** centré
 * 
 * @param x	La coordonnée en x du sommet Nord-Ouest du rectangle.
 * @param y	La coordonnée en y du sommet Nord-Ouest du rectangle.
 * @param largeur largeur du boutton
 * @param hauteur hauteur du boutton
 * @param text Texte au centre du boutton
 */
void dessineBoutton(int x, int y, int largeur, int hauteur, char* text) {
    MLV_draw_filled_rectangle(x, y, largeur, hauteur, MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255));
    MLV_draw_text_box(x + 5, y + 5, largeur - 10, hauteur - 10, text, 1, MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255), 
                        MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255), MLV_rgba(30, 30, 30, 255), MLV_TEXT_CENTER, 
                        MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
}

/**
 * @brief Dessine un boutton de dimension **largeur** par **hauteur** contenant un texte **text** centré
 * 
 * @param x	La coordonnée en x du sommet Nord-Ouest du rectangle.
 * @param y	La coordonnée en y du sommet Nord-Ouest du rectangle.
 * @param largeur largeur du boutton
 * @param hauteur hauteur du boutton
 * @param text Texte au centre du boutton
 */
void dessineBouttonStyle2(int x, int y, int largeur, int hauteur, char* text) {
    MLV_draw_filled_rectangle(x, y, largeur, hauteur, MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255));
    MLV_draw_text_box(x + 5, y + 5, largeur - 10, hauteur - 10, text, 1, MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255), 
                        MLV_rgba(30, 30, 30, 255), MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255), MLV_TEXT_CENTER, 
                        MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER);
}

/**
 * @brief Affiche deux boutton et attends un clic de souris sur l'un des bouttons
 * 
 * @param largeurFen Largeur de la fenêtre
 * @param hauteurFen Hauteur de la fenêtre
 * @return int 0 : Boutton souris choisit
 *             1 : Boutton aléatoire choisit
 */
int menu1(int largeurFen, int hauteurFen) {
    MLV_Keyboard_button sym = MLV_KEYBOARD_NONE;
    int souris_x;
    int souris_y;

    int largeurBoutton = 500;
    int hauteurBoutton = 100;
    dessineBoutton(largeurFen / 2 - largeurBoutton / 2, hauteurFen / 2 - hauteurBoutton - 15, largeurBoutton, 
                    hauteurBoutton, "Ajout de points par clic souris");
    dessineBoutton(largeurFen / 2 - largeurBoutton / 2, hauteurFen / 2 + 15, largeurBoutton, 
                    hauteurBoutton, "Points distribué aléatoirement");

    MLV_actualise_window();
    MLV_clear_window(MLV_rgba(30, 30, 30, 255));

    while (sym != MLV_KEYBOARD_ESCAPE) {
        MLV_wait_keyboard_or_mouse(&sym, NULL, NULL, &souris_x, &souris_y);
        // L'utilisateur clic sur distribution par clic souris
        if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 - largeurBoutton / 2, 
            hauteurFen / 2 - hauteurBoutton - 15, largeurBoutton, hauteurBoutton))
        {
            return 0;
        }
        // L'utilisateur clic sur distribution aléatoire
        else if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 - largeurBoutton / 2, 
            hauteurFen / 2 + 15, largeurBoutton, hauteurBoutton)) 
        {
            return 1;
        }
    }
    exit(0);  // Si l'utilisateur appuie sur escape
}

/**
 * @brief Menu permettant de choisir les paramètres de l'execution de l'application
 * 
 * @param largeurFen Largeur de la fenêtre
 * @param hauteurFen Hauteur de la fenêtre
 * @param nbPoints Nombre de point à générer
 * @param dynamique Affichage dynamique des points
 * @param forme Distribution qui tends vers un carré ou un cercle
 * @param spiral Forme qui grandit ou non
 */
void menu2(int largeurFen, int hauteurFen, int *nbPoints, int *dynamique, int *forme, int *spiral) {
    MLV_Keyboard_button sym = MLV_KEYBOARD_NONE;
    char *nombrePoint;
    int souris_x;
    int souris_y;

    int largeurBoutton = 300;
    int hauteurBoutton = 100;

    while (atoi(nombrePoint) <= 3) {
        MLV_wait_input_box(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 + 15, 
                    (largeurBoutton + 15) * 2, 
                    hauteurBoutton, 
                    MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255), MLV_rgba(255 - 30, 255 - 30, 255 - 30, 255),
                    MLV_rgba(30, 30, 30, 255), "Nombre de points à generer (strictement supérieur à 2): ", &nombrePoint);
    }
    *nbPoints = atoi(nombrePoint);
    free(nombrePoint);


    dessineBoutton(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Affichage dynamique");
    dessineBoutton(largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Forme croissante");
    dessineBoutton(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Converge vers un cercle");
    dessineBouttonStyle2(largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Converge vers un carré");
    dessineBoutton(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 + 15, (largeurBoutton + 15) * 2, 
                    hauteurBoutton, "Lancer la génération !");

    MLV_actualise_window();

    while (sym != MLV_KEYBOARD_ESCAPE) {
        MLV_wait_keyboard_or_mouse(&sym, NULL, NULL, &souris_x, &souris_y);
        // L'utilisateur clique sur affichage dynamique
        if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton))
        {
            *dynamique = 1 - *dynamique;  // Bascule de dynamique à non dynamique
            if (*dynamique) {
                dessineBouttonStyle2(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Affichage dynamique");
            } else {
                dessineBoutton(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Affichage dynamique");
            }
        }
        // L'utilisateur clique sur forme croissante
        else if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton)) 
        {
            *spiral = 1 - *spiral;  // Bascule de forme croissante à forme non croissante
            if (*spiral) {
                dessineBouttonStyle2(largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Forme croissante");
            } else {
                dessineBoutton(largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - 15 - hauteurBoutton - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Forme croissante");
            }
        }
        // L'utilisateur clique sur cercle
        else if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton)) 
        {
            *forme = 1;  // Passe de carré à cercle ou ne bouge pas
            dessineBouttonStyle2(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Converge vers un cercle");
            dessineBoutton(largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Converge vers un carré");
        }
        // L'utilisateur clique sur carré
        else if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton))
        {
            *forme = 0;  // Passe de cercle à carré ou ne bouge pas
            dessineBoutton(largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Converge vers un cercle");
            dessineBouttonStyle2(largeurFen / 2 + 15, hauteurFen / 2 - hauteurBoutton / 2 - hauteurBoutton, largeurBoutton, 
                    hauteurBoutton, "Converge vers un carré");
        } 
        // Si l'utilisateur clique sur lancer la génération
        else if (pointDansRectangle(souris_x, souris_y, largeurFen / 2 - largeurBoutton - 15, hauteurFen / 2 - hauteurBoutton / 2 + 15, (largeurBoutton + 15) * 2, 
                    hauteurBoutton)){
            return;
        }

        // Si on manipule le clavier et qu'on renitialise pas les coordonnées 
        // de la souris le dernier bouton cliqué s'active
        souris_x = 0;
        souris_y = 0;
        MLV_actualise_window();
    }

    exit(0);  // Si l'utilisateur appuie sur escape
}


// ----------------------------Programme principale----------------------------

int main(void) {
    srand(time(NULL));

    // Pour quitter le programme l'utilisateur doit appuyer sur echap ou fermer la fenêtre
    while (1) {
        int dynamique = 0;  // Affichaque dynamique ou seulement rendu final
        int forme = 0;  // 0 : carré 1 : cercle
        int spiral = 0;  // Rayon de la forme qui grandit ou non
        int nbPoints = 0;  // Nombre de points de l'ensemble

        MLV_create_window("Menu enveloppe convexe", "", 1000, 900);
        MLV_clear_window(MLV_rgba(30, 30, 30, 255));
        MLV_actualise_window();

        if (!menu1(1000, 900)) {
            MLV_free_window();
            mainEnveloppeSouris();
        } else {
            menu2(1000, 900, &nbPoints, &dynamique, &forme, &spiral);
            MLV_free_window();
            if (spiral) {
                mainEnveloppeForme(250, 10, nbPoints, forme, spiral, dynamique);
            } else {
                mainEnveloppeForme(250, 230, nbPoints, forme, spiral, dynamique);
            }
        }
    }

    printf("\n");
    return 0;
}