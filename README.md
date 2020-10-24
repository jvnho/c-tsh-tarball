Tagué : 	tsh_memory et tsh_memory * et posix_header  dans les phrases

1 - STRUCTURE GENERAL DU CODE

1.1 le main: Le tsh.c contient le main qui traiteras toutes les commandes.
    Il a :
    - une variable global memory de type tsh_memory qui contiendra tous sur l'état courant dans l'execution : 
        -> si on est dans un .tar (il contient l'ouverture pour ce .tar, le nom de ce .tar, la position courante dans ce .tar)
    - une boucle principal qui affiche le repertoire courant, puis read sur l'entre standard puis execute la commande correspondant ce qui est dans le buffer du read.
      (pour le moment on a pas encore fait le lien entre la commande écrit via le read et l’appel de  fonction correspondante)
      

1.2 les commandes
    Les commandes sont dans des fichiers nomDeCommande.c puis importé dans le main, et la 	logique des commandes est la suivante :
    chaque commande prend en plus de ses arguments une variable de type tsh_memory *, qui nous renseigne sur la circonstance de l’ appel de la fonction.
        a) si on est dans une circonstance normal (pas dans un .tar) alors on cree un processus fils pour faire un exec.
        b) si on est dans un .tar on simule traitement associé au nom de la commande sur le .tar



2 - DETAIL SUR L'IMPLEMENTATION 

2.1 tsh_memory.c

typedef struct tsh_memory{
    char FAKE_PATH[BUFSIZE];
    char tar_name[BUFSIZE];
    char tar_descriptor[BUFSIZE];
    char comand[MAX_COMMAND];
}tsh_memory;

Les champs :
    -FAKE_PATH : Position courant depuis qu'on est rentré dans .tar 
        ->il facilitera le pwd plus général  (chemin absolu + FakePath)
        ->il permettrait aussi de valider le cd dans un .tar (est ce que ca existe?? FakePath/iDossier)
    -tar_name : Pour stocker le nom du fichier.tar
        ->il facilitera le pwd plus général  (chemin absolu + tar_name + FakePath)
    -tar_descriptor : Pour stocker le descripteur du fichier.tar ouvert.
    -comand : Sert de buffer pour le read dans le main.

Les methode :
    - create_memory() -> crée une instance de tsh_memory 
    - free_tsh_memory(tsh_memory *state) -> libère toute la memoire alloué par state
    - instanciate_tsh_memory(char *tar_file_name, tsh_memory *result) -> initialise les champs de result en fonction du fichier .tar dont on passe le nom.
    - getPath(tsh_memory *state) -> retourne la position courante global depuis position absolue
    - in_a_tar(tsh_memory *state) -> retourne un 0 ou 1 si on est dans un faux dossier (.tar)
    
2.2 cd.c  :     
cd(char *directory, tsh_memory *memory) 
	Si on est dans un .tar
		si la positionCouranteDansTar/directory/ existe dans les header du tar
			postionCouranteDansTar = positionCouranteDansTar/directory/
	Si pas dans .tar
		On découpe le directory sous 3 sous String 
			CheminAvantTar/  dossier.tar  CheminAprèsTar
		Si il y a le Sous String CheminAvantTar/
			On fait un chdir sur le CheminAvantTar
		Si il y a le sous string dossier.tar
			On initialise les champs de memory
		Si il y a le sous string CheminAprès tar
			On vérifie si le cheminAprès existe dans les header du tar et changer
			la postionCouranteDansTar

2.3 pwd.c
pwd( tsh_memory *memory) 
concatener le repertoire courant du processus, avec tar_ name et FAKE_PATH

2.4 mkdir.c
mkdir( char *directory, tsh_memory *memory)
	Si on est pas dans un .tar
		créer un processus fils pour utilisé exec sur mkdir
	Si on est dans un .tar
		Créer une variable posix_header qui aura comme name FAKE_PATH/directory
		écrire le posix_header sur le premier bloc nul du .tar
		écrire un nouveau bloc null a la fin du tar pour compenser 
		
2.5 ls.c

2.6 rmdir.c

2.7 cat.c


3 - FUTUR FONCTIONALITÉ
Pour associer les commandes et les fonctions a appelé on pensait faire un tableau de string contenant la liste des commandes (tab1) et un second tableau de pointeur de fonction (tab2) 
Et utilisé une fonction "appel_de_fonction" qui 
_>prend en argument le nom de la commande en string et ses arguments
_>regarde l' indice de la commande dans tab1
_>appel sur le pointeur de fonction dans tab2 a cette indice avec les argument passé en paramètre
_>si jamais la commande n’est pas répertorié dans tab1 on fait un simple exec
ex: 	tab1 ["cd", "mkdir"]  tab2 [cd, mkdir]
	-> mkdir truc
	->( *( tab2[getIndice(tab1, "mkdir")] ) ) (truc)

PROBLEME
Mais pour cela il faudra déjà réussir récupérer efficacement toutes les sous string correspondants au nom de la commande et ses arguments (dans un char * buffer_du_read)
Ce qui s'avère très compliqué vu qu'on aura plusieurs commandes notamment a cause du pipe
- comment, dans un char* chercher chaque commande en sous string (vu qu'on a plusieurs ) 
et découper toutes ces commandes en paquet avec leur argument respectif  (vu que chaque commande auront une liste de (char *) argument)
et tout ca sans faire trop de malloc, car c'est une opération répétée a chaque fois que l'utilisateur rentre une commande.

PROBLEMATIQUE
Comment le shell gère les commandes qu'il prend par le read, 
sachant que le buffer du read peut contenir plusieurs commandes avec des caractères pipe et chaque commandes peuvent avoir une liste d’argument, le tout dans un buffer_du_read qui est un char*.