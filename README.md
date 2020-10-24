**1 - STRUCTURE GENERALE DU CODE**

**1.1 le main:** Le `tsh.c` contient le *main* qui traitera toutes les commandes.
    Il a :

*  une variable **globale memory** de type `tsh_memory` qui contiendra tout sur l'état courant dans l'exécution :
        si on est dans un .tar (il contient l'ouverture pour ce .tar, le nom de ce .tar, la position courante dans ce .tar)

*  une boucle principale qui affiche le répertoire courant, puis read sur l'entrée standard puis exécute la commande correspondante ce qui est dans le *buffer* du read.
      **(pour le moment on a pas encore fait le lien entre la commande écrit via le read et l’appel de fonction correspondante)**


**1.2 les commandes**
    Les commandes sont dans des fichiers `nomDeCommande.c` puis importé dans le main, et la logique des commandes est la suivante :
    chaque commande prend en plus de ses arguments **une variable de type tsh_memory**, qui nous renseigne sur **les circonstances de l’appel de la fonction:**

*  si on est dans une **circonstance normale (pas dans un .tar)** alors on crée un processus fils pour faire un exec.
*  si on est dans un .tar on simule traitement associé au nom de la commande sur le .tar



**2 - DETAILS SUR L'IMPLEMENTATION**

**2.1 tsh_memory.c**

```
typedef struct tsh_memory{
    char FAKE_PATH[BUFSIZE];
    char tar_name[BUFSIZE];
    char tar_descriptor[BUFSIZE];
    char comand[MAX_COMMAND];
} tsh_memory;
```


**Les champs:**

    - FAKE_PATH: Position courante depuis qu'on est rentré dans .tar
        ->il facilitera le pwd plus général  (chemin absolu + FakePath)
        ->il permettrait aussi de valider le cd dans un .tar (est ce que ca existe?? FakePath/iDossier)

    - tar_name: Pour stocker le nom du fichier.tar
        ->il facilitera le pwd plus général  (chemin absolu + tar_name + FakePath)

    - tar_descriptor: Pour stocker le descripteur du fichier.tar ouvert.

    - command: Sert de buffer pour le read dans le main.

**Les méthodes:**

    - create_memory() -> crée une instance de tsh_memory

    - free_tsh_memory(tsh_memory *state) -> libère toute la memoire alloué par state

    - instanciate_tsh_memory(char *tar_file_name, tsh_memory *result)
        -> initialise les champs de result en fonction du fichier .tar dont on passe le nom.

    - getPath(tsh_memory *state) -> retourne la position courante global depuis position absolue

    - in_a_tar(tsh_memory *state) -> retourne un 0 ou 1 si on est dans un faux dossier (.tar)

**2.2 cd.c:**  `cd(char *directory, tsh_memory *memory)`

*  Si on est dans un .tar
		si la positionCouranteDansTar/directory/ existe dans les header du tar
			postionCouranteDansTar = positionCouranteDansTar/directory/

*  Sinon,
		on découpe le *directory* sous 3 sous string
			**CheminAvantTar/  dossier.tar  CheminAprèsTar**
		Si il y a le Sous String CheminAvantTar/
			On fait un chdir sur le CheminAvantTar
		Si il y a le sous string dossier.tar
			On initialise les champs de memory
		Si il y a le sous string CheminAprès tar
			On vérifie si le cheminAprès existe dans les header du tar et changer
			la postionCouranteDansTar

**2.3 pwd.c**
`pwd( tsh_memory *memory)`
concatener le répertoire courant du processus, avec **tar_ name et FAKE_PATH**

**2.4 mkdir.c**
`mkdir( char *directory, tsh_memory *memory)`

*  Si on est pas dans un .tar
		on crée un processus fils pour utilisé exec sur mkdir

*  Si on est dans un .tar
		on crée une variable *posix_header* qui aura comme name FAKE_PATH/directory
		on crée le posix_header sur le premier bloc nul du .tar
		et on écrit un nouveau bloc null a la fin du tar pour compenser

**2.5 ls.c**

La fonction prend en argument un ***tsh_memory** , on regarde tout d'abord
si l'utilisateur est dans un tar.


*  Si oui, on fait appel à notre méthode ls_in_tar

*  Sinon on fait un exec d'un ls normal.

`ls_in_tar(int fd, char* PATH, int arg_l)`

La fonction `ls_in_tar` prend en argument **le descripteur du fichier ouvert, le PATH** (i.e le FAKE_PATH) et **un int arg_l qui vaut 1 si "-l" a été entré sinon 0.**

On fait un parcours du tarball, on récupère dans un premier temps **uniquement les chemins des différentes entêtes de fichiers**, s'il est égal à PATH ça veut dire que c'est un élément à afficher,
ce nom d'entête va être coupé: on incrémente un réel `taille_nom` tant qu'on ne rencontre pas le caractère **"/" ou "\0"** qui marque respectivement la fin pour un répertoire et un dossier.

On a donc un **index de début** (taille de PATH) et de **fin** (taille_nom), on coupe le nom d'entête, on le stocke dans un tableau global `ARRAY`, ceci afin d'éviter d'éventuels doublons.

Si l'argument "-l" a été donné, on récupère parallelement les informations sur les fichiers en les stockant dans un tablau global `FILE_INFO`.

A la fin on affiche le tableau ARRAY et éventuellemt FILE_INFO.

**2.6 rmdir.c**

La fonction prend en argument un ***tsh_memory** , on regarde tout d'abord
si l'utilisateur est dans un tar.


*  Si oui, on fait appel à notre méthode rmdir_in_tar

*  Sinon on fait un exec d'un rmdir normal.

`rmdir_in_tar(int fd, char* full_path)`

La fonction `rmdir_in_tar` prend en argument le **le descripteur du fichier ouvert** et **full_path qui est la concaténation du FAKE_PATH** (chemin dans le tar) et **du nom du répertoire** que l'utilisateur veut supprimer.

On fait appel à une méthode auxilliaire `occ_counter_path(int fd, char *full_path, off_t file_offset)` qui va se charger de parcourir tout le tar et de compter les **occurences d'apparition** de full_path dans le tarball: elle prend en plus un argument file_offset qui va renvoyer
la `position du pointeur` si on trouve une éventuelle égalitée entre full_path et le nom de l'entête.
Elle renvoie un entier, si celui-ci est différent de un ça veut dire que soit il n'existe pas un tel repertoire ou soit qu'il n'est pas vide.

Sinon, de retour dans la fonction `rmdir_in_tar` on va refaire un parcours tar mais on va positionner le descripteur à l'endroit où apparait le répertoire qu'on veut supprimer (i.e `file_offset`) et on va
faire un décalage de tous les blocs jusqu'à la fin.

**2.7 cat.c**
La fonction cat prend en argument le **le descripteur du fichier ouvert ** (int desc, char **args, int arg)
comme un cat dans un shell classique elle va elle va vérifier la première condition (si notre arg == 0) alors on affiche le contenu sans condition spécifique,
pour cela elle fait appel à une fonction `cat_all()` qui à pour but de parcourir le tar et afficher d'un write, (une fonction `display(char* str)` facultative a été créée pour cela).
Si la condition du `cat_all()` ne passe pas, c'est `int cat_2(int desc, char* path)` qui sera donc appeler est va lire notre tar avec les conditions spécifiques indiqué pour lancer `read(desc, buffer, &myFile)` et  `write(1, buffer, strlen(buffer))`. Pour se rapprocher au maximum de ce que l'on peut rencontrer dans l'utilisation d'un cat classique
certain réglage et améliotation sans encore en cours d'élaboration...






**3 - FUTURS FONCTIONALITÉS**
Pour associer les commandes et les fonctions à appeler, on pensait faire un tableau de string contenant la liste des commandes (tab1) et un second tableau de pointeur de fonction (tab2)
et utiliser une fonction `appel_de_fonction` qui:

*  prend en argument le nom de la commande en string et ses arguments

*  regarde l' indice de la commande dans tab1

*  appel sur le pointeur de fonction dans tab2 a cette indice avec les argument passé en paramètre

*  si jamais la commande n’est pas répertorié dans tab1 on fait un simple exec
ex: 	tab1 ["cd", "mkdir"]  tab2 [cd, mkdir]
	-> mkdir truc
	->( *( tab2[getIndice(tab1, "mkdir")] ) ) (truc)

**PROBLEME**
Mais pour cela il faudra déjà réussir récupérer efficacement toutes les **sous string correspondants au nom de la commande et ses arguments** (dans un char * buffer_du_read)
Ce qui s'avère très compliqué vu qu'on aura plusieurs commandes notamment à cause du pipe
- comment, dans un char* chercher chaque commande en sous string (vu qu'on a plusieurs )
et découper toutes ces commandes en paquet avec leur argument respectif  (vu que chaque commande auront une liste de (char *) argument)
et tout ca sans faire trop de malloc, car c'est une opération répétée à chaque fois que l'utilisateur rentre une commande.

**PROBLEMATIQUE**
Comment le shell gère les commandes qu'il prend par le read,
sachant que le buffer du read peut contenir plusieurs commandes avec des caractères pipe et chaque commandes peuvent avoir une liste d’arguments, le tout dans un buffer_du_read qui est un char*.
