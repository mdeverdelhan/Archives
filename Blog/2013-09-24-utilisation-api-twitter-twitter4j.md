---
layout: post
title: Utilisation de l'API Twitter avec Twitter4J
---

L'[API Twitter](http://dev.twitter.com/) est [riche](http://dev.twitter.com/docs/api/1.1).

C'est à partir de ce constat que j'ai commencé à chercher s'il n'existait pas une bibliothèque Java facilitant son interrogation. Plus que d'ajouter un énième lib, le but était de trouver un composant externe prenant en charge ses évolutions.

J'ai donc découvert [Twitter4J](http://twitter4j.org/). Bien que non officielle cette lib Java est [activement maintenue](http://github.com/yusuke/twitter4j/). La suite de cet article constitue un exemple de son utilisation à travers la création d'une application postant des tweets pour nous.

### Identification de l'application Twitter

Pour utiliser l'API Twitter (même via Twitter4J) vous devez posséder des *access keys* Twitter. Pour obtenir ces clefs vous devez commencer par identifier votre application auprès de Twitter. Pour cela :

  - Allez à l'adresse <http://dev.twitter.com/apps/new> et identifiez-vous 
  - Saisissez le nom de votre application, sa description, une URL, etc. 
  - Acceptez les *terms of service* et cliquez sur ***Create your Twitter application*** 
  - Enfin, copiez les *consumer key* et *consumer secret* qui vous sont fournis
  
Sur la page de votre application, cliquez sur l'onglet ***Settings*** et définissez l'utilisation de l'API en "*Read and Write*". Par défaut les nouvelles applications sont créées en "*Read-only*" ce qui n'est pas suffisant pour poster un tweet (mais qui l'est si vous souhaitez simplement lire une timeline par exemple).


Une fois l'application créée, vous devez l'autoriser à accéder à votre compte Twitter (Je rappelle que l'objectif est que l'application poste des tweets **à votre place**.). Pour cela revenez sur l'onglet ***Details*** et cliquez sur le bouton ***Create my access token***. Copiez l'*access token* et l'*access token secret* qui vous sont alors fournis.

### Configuration de Twitter4J

Avec les informations récoltées créez un fichier `twitter4j.properties`. Comme son nom l'indique ce fichier va contenir la configuration permettant de faire fonctionner Twitter4J. Remplissez-le avec les *tokens* et *keys* que vous avez obtenus précédemment.

{% gist 6668777 twitter4j.properties %}

Faites en sorte que Twitter4J puisse retrouver ce fichier de configuration. Placez le par exemple à la racine du *classpath*. Bien entendu les propriétés de configuration de la lib ne se limitent pas à ça, et vous pouvez <a href="http://twitter4j.org/en/configuration.html">en ajouter d'autres</a>.


Cela va sans le dire mais il faut aussi ajouter Twitter4J parmi les dépendances de votre application.

{% gist 6668777 pom.xml %}

Elle est <a href="http://search.maven.org/#search|ga|1|g%3A%22org.twitter4j%22">sur le dépôt central de Maven</a>.

### Utilisation de Twitter4J


Une fois l'ensemble correctement configuré l'utilisation de Twitter4J est d'une simplicité déconcertante. Le code suivant fera poster un tweet par l'utilisateur (celui dont vous aurez fourni l'*access token* dans le fichier `twitter4j.properties`).

{% gist 6668777 Twitter4JExample.java %}

Et c'est tout ! Nous voulions envoyer un tweet via notre application ; il n'y a rien de plus à faire.

Vous pouvez trouver [d'autres exemples de codes sur le site de Twitter4J](http://twitter4j.org/en/code-examples.html).

### Exceptions courantes

*Section subsidiaire.*

Certaines exceptions sont couramment levées quand on débute avec Twitter4J. En voici quelques unes que vous êtes susceptibles de rencontrer, accompagnées de pistes de résolutions.


##### Bad Authentication data

{% gist 6668777 Twitter-Exception_Bad-Authentication-data %}

La requête ne comporte pas les informations d'authentification. Posez vous les questions :

  * Ai-je identifié mon application auprès de Twitter ? [Ai-je généré mes access tokens ?](http://dev.twitter.com/docs/auth/tokens-devtwittercom)
  * Mon fichier `twitter4j.properties` est-il [correctement configuré](http://twitter4j.org/en/configuration.html) ? Est-il situé à la racine de mon *classpath* ?


##### Read-only application cannot POST

{% gist 6668777 Twitter-Exception_Read-only-application-cannot-POST %}

Vérifiez que votre application est bien configurée en "*Read and Write*". Les *keys* et *tokens* ont-ils été regénérés après la modification ? Vous devez en outre attendre quelques minutes le temps que Twitter mette son cache à jour.

##### Could not authenticate you

{% gist 6668777 Twitter-Exception_Could-not-authenticate-you %}

Essayez de générer à nouveau vos *keys* et *tokens*. N'oubliez pas de les redéfinir dans le fichier `twitter4j.properties`.
