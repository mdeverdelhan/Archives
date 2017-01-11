---
layout: post
title: Accéder au SSD de la BlackBerry PlayBook via USB depuis Ubuntu
---

Alors que, jusqu'ici, les produits de RIM ne suscitaient chez moi qu'un intérêt (très) limité, j'ai été particulièrement enthousiaste en découvrant les possibilités de la [BlackBerry PlayBook](http://fr.blackberry.com/playbook-tablet/overview.jsp). Outre ses finitions, son OS stable, son écran de bonne qualité, ses dimensions (lui permettant de rentrer dans ma sacoche), son côté professionnel, sa housse, son interface utilisateur, la PlayBook possède un avantage indéniable par rapport à ses concurrents : elle me fut offerte gratuitement lors du [dernier concours](http://us.blackberry.com/developers/tablet/playbook_offer2012.jsp) visant à la promouvoir (via le portage d'applications Android). Ce simple fait me suffit pour tolérer les légères déconvenues liées à son utilisation. Car il faut être honnête : il y a encore quelques problèmes, notamment lorsque l'on est sous Linux.

Un des premiers écueils auxquels j'ai été confronté fut l'incompatibilité (*a priori*) entre la PlayBook et ma machine sous Ubuntu. Ainsi ai-je voulu copier des fichiers depuis mon ordinateur vers la tablette et... *Fatalitas!* Mon ordinateur fonctionne sous Linux et RIM n'envisage pas que l'on puisse tourner sous autre chose que Windows ou Mac : relier les deux appareils ne suffit donc pas. Voici la méthode que j'ai eue à mettre en œuvre.

### Instructions

J'ai testé avec succès cette méthode sur Ubuntu 11.10. Elle devrait également fonctionner avec les versions suivantes et sur d'autres distributions.

Branchez tout d'abord la PlayBook à votre machine à l'aide de votre cordon USB. Normalement, côté tablette, un nouvel écran apparaît vous indiquant que l'appareil est bien connecté à l'ordinateur. Vous pouvez appuyer sur le bouton ***Rejeter*** (je pars du principe que votre tablette est configurée en français, pour les autres c'est [par ici](http://wcdm.ca/using-blackberry-playbook-with-ubuntu-11-10/) ou [par là](http://forums.crackberry.com/blackberry-playbook-f222/playbook-linux-611875/index2.html#post6634357)). Côté Ubuntu rien ne se passe.

Il semble que la PlayBook utilise le [protocole/système de fichiers SMB](http://fr.wikipedia.org/wiki/Server_Message_Block) de Windows. Il vous faudra donc installer le paquet **smbfs** :

```bash
sudo apt-get install smbfs
```

Créez maintenant le point de montage du volume de stockage que constitue la tablette :

```bash
sudo mkdir /mnt/playbook
```

Sur votre PlayBook, faites glisser votre doigt de haut en bas pour accéder à l'interface de configuration. Puis, dans le menu ***Stockage et partage*** :

  * définissez l'option ***Connexions USB*** à ***Connecter à Windows*** (Par défaut cette option est à ***Détection automatique***)
  * activez l'option ***Partage de fichiers*** en appuyant sur le bouton correspondant


À cet instant-là un nouveau périphérique réseau apparait côté Ubuntu. Vous pouvez vérifier cela en tapant **ifconfig** dans votre terminal. Il devrait s'appeller **usb0**, **usb1** ou **usbN** (ou encore autrement, cela dépend de votre système).

Sur la tablette, remontez en haut de l'écran de configuration et entrez dans le menu ***À propos de***. Dans la liste déroulante située à droite du message ***Consulter les infos de votre tablette*** sélectionnez ***Réseau***. Vous devriez voir l'adresse IPv4 de la PlayBook pour son interface USB (par exemple : 169.254.221.225). Notez la.

Sur votre machine, montez enfin le volume de stockage sur **/mnt/playbook ** en remplaçant 169.254.221.225 par l'adresse de la PlayBook.

```bash
sudo smbmount //169.254.221.225/media /mnt/playbook
```

Vous pouvez maintenant accéder (`ls /mnt/playbook`) aux fichiers situés sur votre BlackBerry PlayBook et y copier films, e-books, musique, etc. Cela peut être fait soit en ligne de commande, soit en utilisant Nautilus :

```bash
gksudo nautilus /mnt/playbook
```

À la fin de vos manipulations vous pouvez démonter le volume et supprimer votre point de montage. Il vous suffit pour cela de saisir les deux commandes suivantes dans votre terminal :

```bash
# demontage du volume
sudo umount /mnt/playbook
# suppression du point de montage
sudo rm -rf /mnt/playbook
```
