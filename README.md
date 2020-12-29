**1 - STRUCTURE GENERALE DU CODE**

**1.1 le main:** Le `tsh.c` contient le *main* qui traitera toutes les commandes.
    Il a :

*  une variable **globale memory** de type `tsh_memory` qui contiendra tout sur l'état courant dans l'exécution :
        si on est dans un .tar (il contient l'ouverture pour ce .tar, le nom de ce .tar, la position courante dans ce .tar)

*  une boucle principale qui affiche le répertoire courant, puis read sur l'entrée standard puis exécute la commande correspondante ce qui est dans le *buffer* du read.


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
    
    -copyMemory(tsh_memory *source, tsh_memory *target) -> sauvegarde l'état de source dans target
    
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
`int mkdir_tar(char listOption[50][50], char listArgs[50][50], int size_option, int size_args, tsh_memory *memory)`

on execute la fonction `int makeDirectory(char listOption[50][50], char *dir_name, int size_option, tsh_memory *memory)` 
sur chaque arguments dans listAgs.

*  Si on est pas dans un .tar
		on crée un processus fils pour utilisé exec sur mkdir avec les options passées en argument

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
La fonction cat prend en argument  ***tsh_memory** , on regarde tout d'abord
si l'utilisateur est dans un tar...

- Si la condtion passe, on fait appel à notre méthode cat_in_tar
- Sinon on fait un exec d'un cat classique.


alors on affiche le contenu sans condition spécifique

La fonction  `cat_in_tar(int desc, char* path)` va prendre  en argument **le descripteur du fichier ouvert, le PATH**

Elle va se charger des conditions necessaires à l'ulitation d'un cat et le parcours du tar, ainsi elle va se charger de lire notre tar avec les conditions spécifiques indiqué pour lancer `read(desc, buffer, &myFile)` et  `write(1, buffer, strlen(buffer))`.

Elle fait également appel à une fonction `cat_all()`(si la conditon passe) qui à pour but de lire et d'afficher tant que les conditions le permettent à l'aide d'un read et write, (une fonction `display(char* str)` facultative a été créée pour cela).


 Pour se rapprocher au maximum de ce que l'on peut rencontrer dans l'utilisation d'un cat classique
certains réglages et améliorations, sans encore en cours d'élaboration...

**2.8 cp.c**
`int copy_tar(char listOption[50][50], char listArgs[50][50], int size_option, int size_args, tsh_memory *memory)`

Appliqué en boucle la fonction `int copy(char listOption[50][50], int size_option, char *source, char *real_target, tsh_memory *memory, int r)`

Etape 1 : identification du cas de copy
- tar -> tar
- outside -> tar
- tar -> outside
- outside -> outside

pour cela on fait un cd vers le target et on sauvegarde l'état du memory courant dans un `memoryTarget`, 
puis depuis l'état initial de la memory on fait un cd source et l'état, ce qui modifie memory en fonction de l'état du source.

Etape 2 : appel de cp associé a chaque cas
- (tar -> tar) cp_tar_tar
- (outside -> tar) cp_dir_tar si source est un dossier, sinon cp_file_tar
- (tar -> outside) cp_tar_outside
- (outside -> outside) cp_outside_outside

*Fonctionnement des cas particulier de cp*
on lit les données de source que ce soit dans un tar ou a l'exterieur, on les stock dans un tableau de couple struct_posix header et bloc contenu 
puis au target on lit ce tableau tout en écrivant ce qu'il y a dedans.

**bloc.c**

Pour mieux organiser le stockage et le parcours de donné on a utilisé la structure `content_bloc`,
les données de cp sont donc un tableau `content_bloc content[512]` et on se repère quant on nombre
de couple (header, contenue) on se repère via a la variable global `i_content`
```
typedef struct content_bloc{
    struct posix_header hd;//header
    char content[512][512];
    int nb_bloc;
}content_bloc;
```
**Les champs:**

    - hd: pour stocker le posix_header

    - content: un tableau de buffer pour stocker les contenues associée au header

    - nb_bloc: la taille du tableau de content 


**2.8 pipe.c**
`int pipe_tsh(tsh_memory *memory1, tsh_memory *memory2)`
Normalement un pipe prend deux commande en argument, mais dans notre modélisation 
une commande ne suffit pas il faut aussi le context d'execution de la commande (dans un tar ou pas, si oui quel est le descritpeur,.)
D'ailleurs la commande est déjà dans les memory (le champs command)

*  on sauvegarde d'abord l'ouverture du descritpeur zero, pour le remettre en place après
*  on crée un tube anonyme
*  creer un processus fils qui sera un écrivain sur le pipe, et executera la commande du contexte memory1.
*  le père sera lecteur du pipe, il attend la fin de l'execution de memory1 pour executer memory2
*  puis encore dans le père remettre le descripteur zéro a son ouverture de départ.(redirection vers la sauvegarde du début)


**3 - COMMANDE ET APPEL DE FONCTION**

**3.1 - traitement de commande**
*simpleCommande.c* `int execute(tsh_memory *memory)`

Etape 1 : verifié la commande dans memory si il y a un charactère '|' ou '>', '2>'
* si on a un '|' on split la commande en 2 par rapport au dernier slach, et on appel la fonction pipe_tsh
* si on a une redirection, on appel la fonction de redirection
* si on a pas de characère spécial on passe a l'étape 2


Pour faciliter l'appel de fonction pour plus tard on stock séparément le nom de la commande, les options et les arguments.
```
char co[50];
char option[50][50];
char args[50][50];
```
Etape 2: appel de la fonction `int execSimpleCommande(tsh_memory *memory)` sur le memory
* réinitialiser les tableaux co, option, args (du aux valeurs des appels précédents)
* remplir le tableau en fonction de la commande actuelle
* passage a l'étape 3

Pour automatiser l'appel des fonctions en fonction du nom de commande, on utilisé deux tableaux :
`char *listCommande[]` : pour chercher a quel indice se trouve le nom de la commande
`pt_adapter listFun [NB_FUN]` : un tableau de pointeur de fonction pour pouvoir invoqué la fonction par son indice

**pt_adapter**
pour pouvoir stocker toutes les pointeur de fonctions dans un seul tableau, 
il faut déjà que les fonction pointé ont la même signature, ce qui n'est pas notre cas 
vu que certaines fonctions prennent la liste des arguments et d'autres non, et le seul point communs
est qu'il prennent tous un `tsh_memory *` en arguments.

*solution* 
créer un adapteur de fonction de manière a ce qu'ils ont la même signature, qui prend un `tsh_memory *` et renvoie un `int`
Et comment faire pour passer les autres argument comme la liste des options, liste d'argument ?
-> facile on les a déjà stocker dans des tableau qui sont des varible global.

A ce point on a utilisé l' **adaptor pattern** pour uniformaliser la signature, puis
contourner le passage d'argument via les variables globales. 
Il reste plus qu'a déclarer le tableau de pointeur de fonction, qui contiendra des pointeurs d'adapteur.
Pour cela on déclare le type pt_adapter :
```
typedef int (*pt_adapter) (tsh_memory *memory);
```
Puis le tableau de pointeur d'adapteur :
```
pt_adapter listFun [NB_FUN] = {adapter_cd, adapter_pwd, adapter_mkdir,..
```
Etape 3: execution de la fonction associé a la commande
*  on cherche l'indice de la commande, dans le tableau de nom de commande
*  (fun_index = -1) si il existe pas dans le tableau de nom, on fait un fork puis exec de la commande
*  (fun_index = i) si il existe dans le tableau, on appel la fonction a l'indice `fun_index` du tableau de pointeur de fonction 
```
(*(listFun[fun_index]))(memory);
```


