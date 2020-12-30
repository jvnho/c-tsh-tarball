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

**2.01 tsh_memory.c**

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
    
**2.02 cd.c:**  `cd(char *directory, tsh_memory *memory)`

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

**2.03 pwd.c**
`pwd( tsh_memory *memory)`
concatener le répertoire courant du processus, avec **tar_ name et FAKE_PATH**

**2.04 mkdir.c**
`int mkdir_tar(char listOption[50][50], char listArgs[50][50], int size_option, int size_args, tsh_memory *memory)`

on execute la fonction `int makeDirectory(char listOption[50][50], char *dir_name, int size_option, tsh_memory *memory)` 
sur chaque arguments dans listAgs.

*  Si on est pas dans un .tar
		on crée un processus fils pour utilisé exec sur mkdir avec les options passées en argument

*  Si on est dans un .tar
		on crée une variable *posix_header* qui aura comme name FAKE_PATH/directory
		on crée le posix_header sur le premier bloc nul du .tar
		et on écrit un nouveau bloc null a la fin du tar pour compenser

**2.05 ls.c**

`ls(tsh_memory *memory,char args[50][50],nb_arg,char option[50][50],int nb_option)` situé dans `ls.c`

Dans le cas où aucun argument n'a été donné on applique le ls directement sur le répertoire courant, sinon, on boucle et on l'applique sur chaque élément du tableau args : 
`do_ls(tsh_memory *memory, char *dir, char option[50][50], int nb_option, int l_opt)`

*Mais concrètement comment applique-t-on le ls ?*

**2.05.1 Comment fonctionne le ls ?**

Comme pour toute les commandes, la **première étape** est de vérifier si un chemin a été rentré par l'utilisateur, on va d'abord se charger de faire un `cd` sur le *préfixe* (exemple: pour "rep1/rep2/rep3", le **préfixe** est "rep1/rep2/" et le **suffixe** "rep3"), tout en faisant une **sauvegarde de l'état courant** (c'est-à-dire qu'on stocke les données courant du `tsh_memory`).

Remarque: si le cd échoue (chemin qui n'existe pas que ce soit en dehors d'un tar ou dedans) on n'arrête tout de suite.

Ensuite, qu'un chemin a été donné ou non, on vérifie grâce au `tsh_memory` si on se situe dans un tar:

* Cas 1: on ne se situe pas dans un tar alors on fait une procédure qui remplit un tableau `execvp_array` avec le suffixe de l'argument de l'utilisateur ainsi qu'avec chaque option tapée par l'utilisateur `(char option[][]`), on crée un processus fils qui exécute ceci: `execvp("ls", execvp_array)`. Ces fonctions de remplissage ainsi qu'exécution se situent dans le fichier `exec_funcs.c` (car elles seront utilisées pour d'autres commandes)

* Cas 2: on se situe dans un tar et on distingue deux sous-cas:
	* L'utilisateur est **explicite** dans son souhait il veut faire un *ls* d'un répertoire: il a mis un '/' à la fin de la commande.
	* Sinon il ne l'a pas mis et donc on ne sait pas s'il veut lister un répertoire ou un fichier. 

Deux cas, donc deux fonctions :

* `int ls_in_tar_file(int fd,char* full_path,int arg_l)`: retourne 1 si on trouve une entête de fichier tar ayant comme nom *full_path* qui est la **concaténation** du `FAKE_PATH` (tsh_memory) et de **l'argument** donné par l'utilisateur (ou le suffixe s'il a donné un chemin), sinon 0.

* `int ls_in_tar_directory(int fd, char* full_path, int arg_l)`: on stocke dans un tableau (afin d'éviter les éventuels doublons) les fichiers ayant pour préfixe le *full_path* **qui n'est pas ici une concaténation** mais uniquement le `FAKE_PATH`, par exemple si celui-ci est "rep1/rep2/" et qu'on trouve un fichier (ou n'importe quoi) "rep1/rep2/fic" on coupe le préfixe et on stocke "fic" dans le tableau (s'il ne l'est pas déjà !).
A la fin du parcours du tar, on a stocké tous les fichiers qui appartiennent au FAKE_PATH et on les affiche.

Remarque 1: pour ces deux fonctions si `arg_l` vaut 1 ça veut dire que l'option -l a été donnée et qu'on stocke dans un autre tableau les informations, l'affichage sera donc différent.

Remarque 2: ces tableaux sont regroupés dans un type `struct ls_memory` (*la mémoire du ls* actuel en quelque sorte) dans le fichier `ls.h`.

Que ce soit un ls effectué dans ou en dehors d'un tar, on restaure l'état précédent. 


**2.06 rmdir.c**

Fonction `rmdir_func` située dans le fichier *rmdir.c*

Comme d'habitude, on boucle sur chaque préfixe et on fait un éventuel `cd` sur le préfixe du chemin donné en argument par l'utilisateur, ensuite pour chaque argument on distingue deux cas: 

Cas 1: l'utilisateur se situe en dehors d'un tar, alors c'est simple, on crée un processus fils qui va se charger d'exécuter `execvp("rmdir", execvp_array)` où `execvp_array` contiennent les éventuels options rentrées par l'utilisateur.

**Attention**: cas particulier si l'utilisateur donne l'option`--parents` qui supprime aussi récursivement les parents du dossier donné en argument s'il sont vides. Dans ce cas là, on restaure d'abord l'état du `tsh_memory` (au lieu de le faire à la fin comme pour les autres fonctions)

Cas 2: on se situe dans un tar, pour supprimer un répertoire du tar, il suffit de **supprimer son en-tête** mais on doit veiller à ce que le répertoire soit vide (et qu'il existe !):

Premièrement, on cherche dans le tar si le `full_path`  (concaténation du `FAKE_PATH` et suffixe de l'argument) **n'apparait bien qu' une seule et une fois dans le tar**: c'est le rôle de la fonction 
 `int occ_counter_path(int fd, char* full_path, off_t* file_offset, off_t *tar_size)` renvoie le nombre qu'il trouve le `FAKE_PATH`dans les en-tête. Si le retour est 0, le répertoire n'existe pas ou strictement supérieur à 1 le répertoire n'est pas vide, auxquels cas on n'arrête tout de suite.
 (on vérifie également que le typeflag de l'en-tête est '5')
 
 Au fait, la fonction se charge aussi de modifier le pointeur `file_offset` la localisation de l'en-tête qu'on veut supprimer et également la taille totale du tar `tar_size`. Ces données sont pertinentes **seulement si la fonction renvoie 1**.

Pour supprimer l'en-tête, on refait un parcours du tar **mais** à partir de `file_offset` octets à l'aide d'un `lseek`, on lit et stocke dans un buffer ce que contient le bloc qui suit celui qu'on veut supprimer, on se repositionne à`file_offset`, et on écrit par-dessus: en gros on fait un décalage de chaque bloc.

Ainsi tous les blocs qui suivent celui qu'on voulait supprimer se retrouvent à une position n-1. Après avoir tout décalé, on effectue un `ftruncate`de la taille du tar, qu'on avait calculé précédemment, **moins 512 octets** (taille d'un bloc).


**2.07 cat.c**
La fonction cat prend en argument  ***tsh_memory** , on regarde tout d'abord
si l'utilisateur est dans un tar...

- Si la condtion passe, on fait appel à notre méthode cat_in_tar
- cat_int_tar va vérifier si le chemin existe dans le tar en question.


Alors on affiche le contenu sans condition spécifique

La fonction  `cat_in_tar(int desc, char* path)` va prendre  en argument **le descripteur du fichier ouvert, le PATH**

Elle va vérifier si le chemin existe dans le tar.

La fonction 'exitFromCat' :
Va etre utilisé pour le **SIGACTION** qui lui va changer le comportement du ctrl c...

La fonction 'exec_cat' Crée un processus "fils" qui va écrire sur le pipe et le "père" va lire et réecrire dans le terminal en question. "1 = STOP" 0 ok.

La fonction 'int cat' 
- va au debut redefinir le **SIGACTION** avec un appel à la fonction 'exitFromCat'

- Si répertoire on passe au suivant, si un repertoire copy l'etat de la memoire actuelle.
- Si le chemin est correct  et si est bien dans le tar 'in_a_tar(memory)' (fonction prédéfinie), alors on concataner le chemin avec l'argument.
- A chaque fin la fonction utilise 'restoreLastState(old_memory,memory)' qui a pour but de retourner la ou on était avant l'utilisation de la commande cd.
- 


**2.08 cp.c**
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


**2.09 rm.c**

Fonction `int rm(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option)` située dans le fichier *rm.c*

Cette fonction est très similaire à `rmdir`.

Tout d'abord on **boucle** sur le nombre d'argument donné et on fait un éventuel `cd` chaque préfixe d'argument, on distingue deux cas:

Cas 1: l'utilisateur se situe en dehors d'un tar, on crée un processus fils qui va se charger d'exécuter `execvp("rm", execvp_array)` où `execvp_array` contiennent les éventuels options rentrées par l'utilisateur.

Cas 2: l'utilisateur se situe à l'intérieur d'un tar et si l'utilisateur veut supprimer, que ce soit un fichier ou un répertoire, on va compter le nombre de blocs qui lui appartiennent et faire un décalage des blocs mais contrairement au `rmdir`, **ce décalage ne va pas juste être d'un bloc mais d'un certain nombre n de bloc**, et une fois le décalage fait on va faire un `ftruncate` du tarball `t - (n*512)`

Comme pour le rmdir, on va séparer la procédure en deux étapes: la première, qui localise et renvoie en pointeur l'**offset de la première en-tête** trouvé qui est égal à **full_path** (concaténation du `FAKE_PATH` et au suffixe) mais aussi **le nombre de  bloc à supprimer** (i.e à décaler):

 - Si c'est un fichier que l'utilisateur veut supprimer, on retourne en plus de l'offset, son **nombre de bloc contenu**.
 
 - Si c'est un répertoire (option -R doit être rentré par l'utilisateur), retourne l'offset **ET la somme des blocs de contenu (+1 pour l'entête) de toutes les autres en-têtes suivantes, dont le préfixe est égal à *full_path***, en gros les fichiers ou dossiers qui appartiennent à l'arborescence.
 
 - Sinon la fonction retourne -1 si aucun fichier ou répertoire ne correspond à la demande de l'utilisateur.

Disons que le nombre de bloc à supprimer est n.

On procède à la **dernière étape**:
On refait un parcours du tar en se positionnant à **offset octets depuis le début** avec un `lseek`, on lit et stocke dans un buffer le bloc situé à `(current_offset + (n*512))`, on revient à `current_offset` et on écrit par dessus avec `write(fd, buf, 512)`, ceci jusqu'à la fin du tar.
Ceci fait, on peut réduire la taille du tar avec le `ftruncate` de `(taille_du_tar - (n*512))`.


**2.10 mv.c**
Le principe de la fonction `mv` est plutôt simple, on effectue un `cp` suivi d'un `rm`. Mais une étape intermédiaire s'impose car nos fonctions cp et rm prennent un argument `arg_r` pour savoir si l'utilisateur a rentré l'argument -r étant donné que la procédure diffère selon si on veut supprimer un répertoire ou un fichier (que ce soit dans un tarball ou à l'extérieur).

On distingue plusieurs cas:
- L'utilisateur a mis un slash `"/"` à la fin de son argument, on est sûr de sa demande et `arg_r = 1`.

Sinon, si le répertoire source se trouve :
* En dehors d'un tarball, si la fonction `opendir` retourne `NULL` ça veut dire que la source est un fichier et que `arg_r = 0` sinon c'est un répertoire et `arg_r = 1`.

* Sinon, on concatène le `FAKE_PATH` du tsh_memory en ajouter un slash à l'argument donné par l'utilisateur et on cherche si une en-tête de ce nom existe, s' il existe, c'est un répertoire, sinon on considère que c'est un fichier.

On peut maintenant appeler `cp` et `rm`.

**2.11 pipe.c**

`int pipe_tsh(tsh_memory *memory1, tsh_memory *memory2)`
Normalement un pipe prend deux commande en argument, mais dans notre modélisation 
une commande ne suffit pas il faut aussi le context d'execution de la commande (dans un tar ou pas, si oui quel est le descritpeur,.)
D'ailleurs la commande est déjà dans les memory (le champs command)

*  on sauvegarde d'abord l'ouverture du descritpeur zero, pour le remettre en place après
*  on crée un tube anonyme
*  creer un processus fils qui sera un écrivain sur le pipe, et executera la commande du contexte memory1.
*  le père sera lecteur du pipe, il attend la fin de l'execution de memory1 pour executer memory2
*  puis encore dans le père remettre le descripteur zéro a son ouverture de départ.(redirection vers la sauvegarde du début)


**2.12 redirection.c**

**Préambule**

 - Cas 1: L'utilisateur veut effectuer une redirection vers un fichier dans un tar
 - Cas 2: Il veut le fichier en dehors d'un tar
 
Dans les deux cas, on **créer un fichier en dehors d'un tar** mais à la seule différence que si l'utilisateur veut ce fichier effectivement de dehors d'un tar, on le créer **DIRECTEMENT** à l'emplacement voulu alors que pour le cas contraire on le crée au répertoire courant de travail (chemin donné par `getcpwd`) et celui-ci sera déplacer vers l'endroit du tar voulu avec un `mv`.

Cette procédure facilite le tout car on a une phase de création et une éventuel phase de déplacement.

**Fonctionnement**

On utilise une `struct` qui permettra de savoir à l'exécution le **nom du fichier**, son **chemin de redirection dans le tar**, la **sortie standard intéressé**, si on **append**, si la redirection voulu est dans un tar et le nombre de redirection:

    typedef struct redirection_array{
	     char NAME[NB_ENTRY][MAX_SIZE];
	     int STD[NB_ENTRY];
	     int APPEND[NB_ENTRY]; 
	     int IN_A_TAR[NB_ENTRY];
	     char REDIR_PATH[NB_ENTRY][MAX_SIZE];
	     int NUMBER;}
On utilise une fonction qui détecte les redirections et remplit cette structure:
`struct redirection_array* associate_redirection(tsh_memory *memory, char *cmd)`

Exemple: l'utilisateur rentre `ls rep >> archive.tar/fic 2> rep2/ficerr`, comme d'habitude on fait un cd sur le *préfixe* de chaque argument `cd archive.tar` on a l'information que le fichier `rep1` recevra ce qui est sorti sur **la sortie standard** ainsi les champ pour l'argument 0 sont:

    NAME[0] = rep1;
    STD[0] = 1;
    APPEND[0] = 1;
    IN_A_TAR[0] = 1; 
    REDIR_PATH[0] = "" //(racine du tar);
    NUMBER++`
   
   Remarque: le champ `REDIR_PATH` est utile que si l'utilisateur veut une redirection vers un tar, exemple pour l'argument 1: `rep2/ficerr` le champ `NAME[1] = "rep2/ficerr"`.

Après avoir remplit la structure, on a une étape intermédiaire qui va juste transformer la commande entrée par l'utilisateur par une commande exécutable pour notre programme, dans le cas de l'exemple précédent, la nouvelle commande sera `ls rep`.

Après ces étapes intermédiaires, on boucle sur le le nombre de redirection (champ `NUMBER` de la structure), et on va créer les fichiers de redirection (fonction `open`), ensuite on fait appel à `dup2` qui fait une redirection des sorties intéressées vers les fichiers correspondants.

Et dernièrement, on fait un dernier parcours de la structure, si le champ `IN_A_TAR[index] = 1` pour un certain index,  on utilise la fonction `mv` pour déplacer ce fichier vers le tar avec comme chemin (=nom d'en-tête) la concaténation du champ `REDIR_PATH[index]` et `NAME[index]`.


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
pt_adapter listFun [NB_FUN] = {adapter_exit, adapter_cd, adapter_pwd, adapter_mkdir, adapter_ls, adapter_rmdir, adapter_rm, adapter_cp}
```
Etape 3: execution de la fonction associé a la commande
*  on cherche l'indice de la commande, dans le tableau de nom de commande
*  (fun_index = -1) si il existe pas dans le tableau de nom, on fait un fork puis exec de la commande
*  (fun_index = i) si il existe dans le tableau, on appel la fonction a l'indice `fun_index` du tableau de pointeur de fonction 
```
(*(listFun[fun_index]))(memory);
```


