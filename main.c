/**
 * @file main.c
 * @author MAATOUK Yanis COSERARIU Alain
 * @brief Projet de programation C portant sur les enveloppes convexes modélisé par des listes chainées (2022-2023)
 * @version 0.1
 * @date 2022-12-19
 */

#include <stdio.h>
#include <stdlib.h>
#include <MLV/MLV_all.h>
#include <time.h>

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
 * @brief Contient un Polygone est plusieurs renseignements utiles (longeur courent, longueur maximal, longueur moyenne)
 * 
 */
typedef struct {
    Polygone p;  // Polygône représentant l'enveloppe convexe
    int curlen;  // Nombre de points du polygône
    int maxlen;  // Nombre de points maximale du polygône
    float avg;   // Nombre de points moyen du polynôme
    int nbMaj;  // Nombre de mise à jour de l'enveloppe
} ConvexHull;

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
 * @brief Libére la mémoire d'une cellule Vertex **v** contenue dans le polygone **poy**
 * 
 * @param v Vertex à libérer
 * @param poly Polygône contenant le Vertex
 */
void freeVertex(Vertex *v, Polygone *poly) {
    if (v == *poly) {
        (*poly) = (*poly)->suiv;
    }

    if (v->suiv == v) {
        *poly = NULL;
    }

    (v->prec)->suiv = v->suiv;
    (v->suiv)->prec = v->prec;
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
    return c;
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
    int scalaire = ((b.x - a.x) * ((-c.y) - (-a.y))) - ((c.x - a.x) * ((-b.y) - (-a.y)));
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

    // Si on est sortit de la boucle car tous les triangles sont directes, on arrête là
    if (triangleDirecte(*p, *(s_i->p), *(s_j->p)) && s_i == env_convex->p) {
        majConvexHull(env_convex);
        return;
    }

    // si on à un triangle indirecte
    // On insére p en tant que nouvelle tête de liste
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
        freeVertex(s_i, &(env_convex->p));
        s_i = s_j;
        s_j = s_i->suiv;
        env_convex->curlen--;
    }

    // Nettoyage arrière
    s_i = env_convex->p->prec->prec;
    s_j = env_convex->p->prec;
    while (!triangleDirecte(*(env_convex->p->p), *(s_i->p), *(s_j->p))) {
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
    }
    
    return event_type;
}

/**
 * @brief Dessine tous les points de l'ensemble **e** sur une fenêtre MLV
 * 
 * @param e Ensemble dessiné
 */
void dessineEnsemble(Ensemble e) {
    int k;
    for (k = 0; k < e.nbPoints; ++k) {
        MLV_draw_filled_circle((int)e.tabPoints[k].x, (int)e.tabPoints[k].y, 3, MLV_COLOR_BLUE);
    }
}

/**
 * @brief Dessine le polygône **poly**
 * 
 * @param poly Polygône à dessiner
 */
void dessinePolygone(Polygone poly) {
    Vertex *tete = poly;
    MLV_draw_filled_circle((int)poly->p->x, (int)poly->p->y, 3, MLV_COLOR_RED);
    MLV_draw_line((int)poly->p->x, (int)poly->p->y, (int)poly->suiv->p->x, (int)poly->suiv->p->y, MLV_COLOR_RED);
    poly = poly->suiv;
    for (; poly != tete; poly = poly->suiv) {
        MLV_draw_filled_circle((int)poly->p->x, (int)poly->p->y, 3, MLV_COLOR_RED);
        MLV_draw_line((int)poly->p->x, (int)poly->p->y, (int)poly->suiv->p->x, (int)poly->suiv->p->y, MLV_COLOR_RED);
    }
}

// ---------------------------Vartientes du programme--------------------------

void mainEnveloppeSouris () {
    ConvexHull env_convex = initConvexHull();

    Point *tabPoints = (Point*)malloc(BLOC * sizeof(Point));

    Ensemble e = initEnsemble(tabPoints, BLOC);

    MLV_Event event_type = MLV_NONE;
    Point souris;  // Coordonnées de la souris
    int longueur_fen = 500;
    int largeur_fen = 500;

    MLV_create_window("Enveloppe convexe", "", longueur_fen, largeur_fen);
    
    // Actualise la position de la souris 
    event_type = actualiseSouris(&souris);
    while (event_type == MLV_MOUSE_BUTTON) {
        if (!ajoutePointEnsemble(&e, souris)) {
            printf("ERREUR LORS DE L'ALLOCATION DE LA MEMOIRE FIN DU PROGRAMME");
            exit(1);
        }
        // Ajout d'une petite fluctuation pour ne pas avoir deux points superposées
        souris.x += 1 / ((((rand() % 101) - 50) + 0.001)  * 100);
        souris.y += 1 / ((((rand() % 101) - 50) + 0.001)  * 100);
        printf("%f %f", souris.x, souris.y);

        // Si on a réalloué la mémoire il faut rediriger les pointeurs de l'enveloppe convexe vers la nouvelle zone mémoire
        if (!(e.nbPoints % (BLOC))) {
            freePolygone(&(env_convex.p));
            env_convex = initConvexHull();
            for (int l = 0; l < e.nbPoints; ++l) {
                majEnveloppeConvex(&env_convex, &(e.tabPoints[l]));
            }
        } else {  // Sinon on met à jour juste avec le dernier point ajouter
            majEnveloppeConvex(&env_convex, &(e.tabPoints[e.nbPoints - 1]));
        }

        printf("ENVELOPPE CONVEXE INFO :\n");
        printf("    curlen = %d \n", env_convex.curlen);
        printf("    maxlen = %d \n", env_convex.maxlen);
        printf("    avg = %f \n", env_convex.avg);
        printf("    nbMaj = %d \n", env_convex.nbMaj);
        printf("\n");

        dessineEnsemble(e);
        dessinePolygone(env_convex.p);

        MLV_actualise_window();
        MLV_clear_window(MLV_COLOR_BLACK);
        
        event_type = actualiseSouris(&souris);
    }

    MLV_free_window();

    // Libération de la mémoire
    free(e.tabPoints);
    freePolygone(&(env_convex.p));
}

// ----------------------------Programme principale----------------------------

int main(void) {
    srand(time(NULL));
    
    mainEnveloppeSouris();

    printf("\n");
    return 0;
}