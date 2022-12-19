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

typedef struct {
    double x;
    double y;
} Point;

typedef struct _vertex_ {
    Point *p;  // Un point de l'ensemble
    struct _vertex_ *prec;  // Le vertex précédent
    struct _vertex_ *suiv;  // Le vertex suivant
} Vertex, *Polygone;

typedef struct {
    Polygone p;  // Polygône représentant l'enveloppe convexe
    int curlen;  // Nombre de points du polygône
    int maxlen;  // Nombre de points maximale du polygône
    float avg;   // Nombre de points moyen du polynôme
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
        printf("Echec de l'allocation de mémoire pour un Vertex");
        return 0;
    }

    // Si la liste est vide on l'initialise avec une cellule qui pointe sur elle même
    if (!(*poly)) {
        *poly = cell;
        cell->suiv = cell;
        cell->prec = cell;
        return 1;
    }

    cell->suiv = *poly;
    cell->prec = (*poly)->prec;
    (*poly)->prec = cell;
    (cell->prec)->suiv = cell;
    return 1;
}

/**
 * @brief Affiche les coordonnées de chaque Vertex d'un polygone **poly**
 * 
 * @param poly Polygone à afficher
 */
void printPolygone(Polygone poly) {
    Vertex* tete = poly;
    printf("%f %f\n", poly->p->x, poly->p->y);
    poly = poly->suiv;
    for (; poly != tete; poly = poly->suiv) {
        printf("%f %f\n", poly->p->x, poly->p->y);
    }
}

// ---------------------Fonctions de gestion des ConvexHull--------------------

/**
 * @brief Initialise une structure ConvexHull **c** avec des valeurs nulls
 * 
 * @param c Structure à initialiser
 */
void initConvexHull(ConvexHull *c) {
    c->p = NULL;
    c->avg = 0;
    c->curlen = 0;
    c->maxlen = 0;
}


// ----------------------------Programme principale----------------------------

int main(void) {
    int k;
    Polygone Poly = NULL;
    Point tabPoints[100];
    Point a = {1, 1};
    for (k = 0; k < 100; ++k) {
        tabPoints[k] = a;
        a.x++;
        a.y++;
    }

    Vertex* cell = allocCellVertex(&(tabPoints[0]));
    printf("x : %f      y : %f\n", cell->p->x, cell->p->y);

    for (k = 0; k < 100; ++k) {
        ajouteVertexPolygone(&Poly, &(tabPoints[k]));
    }

    printPolygone(Poly);

    int longueur_fen = 500;
    int largeur_fen = 500;
    MLV_create_window("Enveloppe convexe", "", longueur_fen, largeur_fen);

    MLV_wait_mouse_or_seconds(NULL, NULL, 20);
    MLV_free_window();

    printf("\n");
    return 0;
}