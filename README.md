Un **rapport de projet** est fourni sur le git sur les détails de l'implémentation des différentes commandes.

## Mise à jour du 04/03/2021:

Commandes de test fonctionnelles:

cd ../arbo
ls documents
mkdir documents/dossier1 dossier3  (creer dossier a l’extérieur )
ls documents
ls archive1.tar archive2.tar
mkdir archive1.tar/nouveau archive2.tar/dossier3 (creer un dossier dans un tarball)
ls archive1.tar archive2.tar
rmdir dossier3 archive1.tar/nouveau
ls
ls archive1.tar

pwd (attention, affiche le chemin courant de la meme manière que le bash)

(oublie du cas le plus simple qui utilise directement exec)
ls desktop   (cp (fichier ou dossier) dans .tar -> vers exterieur)(modifié)
cp -r archive1.tar/fic archive2.tar/dossier1 desktop
ls desktop
ls archive2.tar/dossier3

cp -r archive1.tar/dossier1 fifi archive2.tar/dossier3   (cp (dossier) dans .tar -> vers .tar) et (cp (fichier) exterieur -> vers .tar)(modifié)
ls archive2.tar/dossier3

rm archive1.tar/fic fifi
rm -r archive1.tar/dossier1
ls archive1.tar

(mv ne fonctionne pas: on a mal enchaîné le cp et rm)

cat archive2.tar/dossier1/titi
Les espaces sont très importants
cat > res (ctrl + c pour quitter, redirige les entrées vers le fichier res)
cat res
ls existepas 2> err (redirection vers la sortie erreur)
cat err

ls archive1.tar existepas 2>&1 mix (redirige la sortie standard et erreur vers mix)
cat mix
echo salut | wc
(pipe arrête son execution si une partie de ses sous commande renvoie une erreur, du a une mauvaise communication, cat et ls return 1 en succes, alors que pipe vois ca comme une code erreur, il attend 0)

ls > out | ls inexistant 2> faux | echo merci > derien (combinaison de redirection et de pipe)

## Instructions afin de tester le programme:

- Ouvrir un terminal dans le répetoire où le Dockerfile se situe.

- Tapez `sudo docker build -t tsh_55 .` afin de construire l'image.

- Une fois terminée, tapez `sudo docker run -ti tsh_55` afin d'exécuter l'image.

- Rentrez `cd /home/projet/tsh` et `make` afin de compiler le programme.

- Exécutez en tapant `./tsh`.

- Un répertoire `arbo` de teste, regroupant des répertoires et des tar, ainsi que des commandes de teste sont fournies si besoin.
