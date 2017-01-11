---
layout: post
title: Monter un iPad Mini sous Ubuntu
---

Dernièrement j'ai gagné une tablette numérique en participant à un petit concours d'idées innovantes. Pas n'importe quelle tablette puisqu'après une Archos 5IT et [une PlayBook](2012/05/30/acceder-ssd-blackberry-playbook-usb-ubuntu/) (BlackBerry) c'est un iPad Mini que j'ai remporté.

Au départ je ne suis pas un [Apple-addict](http://www.youtube.com/watch?v=EHQCvSbHW-k) (à l'arrivée non plus d'ailleurs). D'autres sont de bien meilleures références que moi quant à la configuration et l'utilisation au quotidien de ce genre d'appareils. Je tenais néanmoins à partager la technique qui suit pour ceux qui, comme moi, ont l'audace de posséder un équipement Apple tout en étant sous GNU/Linux par ailleurs.

Mon problème est survenu lorsque j'ai voulu <del>effectuer une manipulation ésotérique</del> copier des vidéos sur l'iPad. Bien sûr on vous dira qu'avec Apple "on branche et *ça juste marche*". Pas de chance pour moi, au branchement mon Ubuntu m'a accueilli avec le charmant message :

```
Impossible de monter iPad de XXXX
Erreur de verrouillage non gérée (-15)
```

En outre l'iPad affiche "*Aucune recharge en cours*" dans sa barre de notification, ce qui, néanmoins, ne gène en rien la recharge de l'appareil ou son utilisation.

### libimobiledevice

La solution à ce problème nous vient d'une lib nommée *[libimobiledevice](http://www.libimobiledevice.org/)*. Son ambition étant d'*enseigner aux manchots à parler aux fruits* sans nécessiter de jailbreaker son appareil. Et elle y réussit très bien pour le peu que j'en ai eu besoin. Sous Ubuntu il suffit de l'installer en cliquant sur [ce lien](apt://libimobiledevice-utils) ou en saisissant la commande suivante dans un terminal :

```bash
sudo apt-get install libimobiledevice-utils
```

Une fois la bibliothèque installée : branchez l'iPad en USB, débloquez-le (si vous avez défini un mot de passe de verrouillage de l'appareil, saisissez-le), puis entrez la commande suivante dans votre terminal :

```bash
idevicepair unpair && idevicepair pair
```

Si tout s'est bien passé vous devriez voir s'afficher un message du type :

```bash
SUCCESS: Unpaired with device 2b6cc33eea37633274f08b7edc8d8183d8bb9849
SUCCESS: Paired with device 2b6cc33eea37633274f08b7edc8d8183d8bb9849
```

Voilà, vous pouvez maintenant monter votre iPad depuis votre Ubuntu et y copier des fichiers. J'ajoute qu'il n'est pas nécessaire de refaire cette manipulation la fois suivante : l'iPad se monte tout seul.

*Note : On me signale dans l'oreillette que les nouvelles versions d'Ubuntu (depuis la 12.04 vraisemblablement) ne nécessitent plus cette manipulation. Merci à la communauté d'avoir fait l'effort de faire le travail d'Apple.*
