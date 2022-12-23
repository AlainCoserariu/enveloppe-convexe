# enveloppe-convexe
Yanis MAATOUK TD 1 TP 2  
Alain COSERARIU TD 1 TP 2

Projet de fin de semestre portant sur les enveloppes convexe modélisé par des listes chainées en language C.

## documentation / utilisation :

Mode emploie lors de la génération des points :
* Espace : Arrête l'affichage dynamique (pour aller plus vite notemment)
* Echap : Stop l'execution  
  
Le programme propose deux version, une première contenant le projet de base et une seconde contenant l'option  
d'enveloppes convexes emboitées, où chaque point de l'ensemble appartient à une et une seule enveloppe convexe.  
Chacune des deux option ont le même fonctionnement du point de vu de l'utilisateur.

### Compilation :
Bibliothèque externe utilisé :
* [libMLV](http://www-igm.univ-mlv.fr/~boussica/mlv/index.html "Page officiel librairie MLV")

ligne à utiliser pour la compilation (Dans le dossier version1 pour la version de base, dans le dossier version2(option) pour les enveloppes emboitées):
`clang -std=c17 -Wall -Wfatal-errors main.c -lMLV -lm`

Le fichier à executer sera a.out : `./a.out`

## Ce qui n'a pas été fait

* Séparer le programme en plusieurs fichiers

## Autre

Le projet à été réalisé sur GitHub à l'addresse suivante : [https://github.com/AlainCoserariu/enveloppe-convexe.git](https://github.com/AlainCoserariu/enveloppe-convexe.git).  
D'où le readme en .md.
