# enveloppe-convexe
Yanis MAATOUK TD 1 TP 2  
Alain COSERARIU TD 1 TP 2

Projet de fin de semestre portant sur les enveloppes convexe modélisé par des liste chainées en language C.

## documentation / utilisation :

Mode emploie pour les formes carrées et cercle :
* Escpace : Arrête l'affichage dynamique (pour aller plus vite notemment)
* Escape : Stop l'execution et attend une entrée de touche pour quitter le programme

Le programme propose deux version, une première contenant le projet de base et une seconde contenant l'option  
d'enveloppes convexes emboitées, ou chaque point de l'ensemble appartient à une et une seule enveloppe convexe  
Chacune des deux option ont le même fonctionnement du point de vu de l'utilisateur

### Compilation :
Bibliothèque externe utilisé :
* [libMLV](http://www-igm.univ-mlv.fr/~boussica/mlv/index.html "Page officiel librairie MLV")

ligne à utiliser pour la compilation (répertoire courent):
`clang -std=c17 -Wall -Wfatal-errors main.c -lMLV -lm`

Le fichier à executer sera a.out : `./a.out`
