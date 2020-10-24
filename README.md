Fonctionnement de la commande ls:

La fonction ls prend en argument la structure tsh_memory:
si l'utilisateur n'est pas dans un tar alors on fait un ls normal sinon on fait appel à une méthode ls_in_tar qui fait ce qui suit.

ls_in_tar prend en argument le descripteur, le chemin local PATH du tar situés dans la structure tsh_memory
et un entier arg_l, valant 1 si l'argument "-l" a été entré sinon 0.

Dans la boucle principale while du parcous du tarball, dans un premier temps on récupère uniquement le chemin du fichier
d'entête, on vérifie que celle-ci est égale à PATH, si oui cela veut dire que
c'est un élément à afficher: on fait donc tout une procédure afin de récupérer
uniquement le nom du fichier ou dossier, c'est-à-dire qu'on incrémente un entier à partir
de la fin du chemin du fichier d'entête (i.e la taille de PATH) tant que qu'on ne rencontre
pas le "/" ou le "\0" qui marque la fin pour un répertoire ou un fichier ordinaire.

On a ainsi l'index de début (taille de PATH) et de fin du nom du fichier (grâce à la procédure précédente), on le coupe et on le copie dans le tableau global ARRAY:
on utilise un tableau pour ne pas avoir d'éventuels doublons.

Dans le cas où l'utilisateur aurait rentré l'argument "-l" les informations
(droits, nom d'utilisateur, nom de groupe, nom) sont remplies parallèlement dans le tableau global
FILE_INFO.

Quand on arrive à la fin de l'archive tar on affiche le tableau ARRAY (et éventuellement FILE_INFO).*

Fonctionnement de la commande rmdir:

Comme pour le ls on vérifie si l'utilisateur n'est dans un tar si oui on fait le rmdir normal sinon on fait appel
à notre méthode.

Lors du parcours du tar, on vérifie tous les noms des répetoires de celui-ci et on vérifie s'il est égal à celui donné en argument PATH
si oui et si on trouve une unique correspondante alors cela signifie que le dossier est vide et on procède à sa suppression.
