## Enchaînements de commandes de test:
(tapez "clear" si besoin)
- ls
- cd ../arbo/archive1.tar/
- pwd
- rmdir dossier1/ (censé renvoyer une erreur)
- rmdir vide/
- cd ../archive2.tar/dossier1/../
- ls -l
- rm -R dossier1/
- ls
- cd ../
- ls -l inexistant archive2.tar archive1.tar/dossier3/dossier7/../ ../tsh 2>&1 fic
- cat fic
- cd /home/projet/archive1.tar/
- cat > catout
(entrez ce que vous voulez et faites CTRL+C pour sortir du cat)
- cat catout
- cd /home/projet/arbo/
- echo bon > redirf
- echo jour >> redirf
- cat redirf
- mkdir archive1.tar/bonsoir/
- mv svp archive1.tar/bonsoir/
- cd ../../
- cp documents archive2.tar/ (renvoie une erreur)
- cp -R documents archive2.tar
- cd ..
- ls > archive.tar1/lsredir | ls 2> inexistant archive2.tar/err | echo merci > derien
