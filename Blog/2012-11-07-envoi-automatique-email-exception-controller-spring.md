---
layout: post
title: Envoi automatique d'e-mail lors d'une exception dans un controller Spring
---

La mise en production d'un application Web est toujours source de l'immense plaisir que procure la découverte de bugs, cas non testés et autres joyeusetés logicielles du même genre. Ces problèmes deviennent graves lorsque vous n'êtes pas alerté quand ils surviennent. Un bug crée une frustration chez l'utilisateur. Répétez-la et c'est autant d'utilisateurs/membres que vous perdrez. Aussi, avant la première mise en production, il est capital de mettre en place un mécanisme d'alerte sur exception.

C'est ce que je vous propose de faire aujourd'hui, dans le cas d'une utilisation du [framework Spring](http://www.springsource.org/).

### Dépendances et pré-requis

De quoi avons-nous besoin ?

  - Un [compte Gmail](http://mail.google.com/), prenons *myerrorsender@gmail.com*. Il s'agit de l'adresse à partir de laquelle les alertes seront envoyées.
  - Votre adresse e-mail, mettons *developer@myapp.tld*. Il s'agit de celle qui recevra les alertes.
  - Spring MVC, que nous récupérons via [Maven](http://maven.apache.org/).

Le *pom.xml* devrait d'ailleurs contenir quelque chose comme ceci :

{% gist 3990188 pom.xml %}

On trouve là les dépendances Spring, AOP et JavaMail™.

### Interception des exceptions

Nous poursuivons en créant un composant d'interception des exceptions.

{% gist 3990188 ExceptionInterceptor.java %}

Son principe est simple : il s'agit d'un décorateur qui, à chaque appel d'une méthode qu'il décore, va préalablement exécuter le code en question et envoyer un e-mail en cas d'exception.
Le template de l'e-mail est ici construit à la volée pour plus de simplicité. Il serait toutefois préférable de le décrire dans le fichier de configuration du bean, tel qu'expliqué [dans cet article](http://www.mkyong.com/spring/spring-define-an-e-mail-template-in-bean-configuration-file/).

### Envoi asynchrone des e-mails

Enfin l'envoi des e-mails se fait en utilisant l'[API JavaMail™](http://docs.oracle.com/javaee/6/api/javax/mail/package-summary.html). On crée pour cela un composant d'envoi asynchrone d'emails.
Note : Bien qu'un envoi de mail puisse être exécuté de manière synchrone il est toujours préférable de le rendre asynchrone afin qu'il ne monopolise pas le serveur lors de l'envoi et bloque les requêtes sur le moment. 

Notre composant est assez simple. Il doit juste hériter de l'implémentation JavaMail de Spring. 

{% gist 3990188 AsyncMailSender.java %}

Remarquez l'annotation *@Async* qui décore les méthodes d'envoi. A elle seule elle ne suffit pas à le rendre asynchrone. Il faut en plus préciser, dans le *applicationContext.xml*, que les annotations de ce type seront transmises à un *taskExecutor* spécifique.

Et justement voici ce que doit comporter le fichier de configuration de l'application :

{% gist 3990188 applicationContext.xml %}

Outre le taskExecutor, on y trouve les beans pour lesquels l'intercepteur d'exceptions sera activé (en l'occurrence les composants correspondant au *pattern* "*Controller*"), et la [configuration SMTP pour l'envoi de mails via Gmail](http://support.google.com/mail/bin/answer.py?hl=en&amp;answer=78799). 


Voilà ! Vous avez maintenant la garantie qu'à la moindre exception survenant dans un de vos controllers vous serez averti. Pour tester vous n'avez renvoyer une exception dans l'un d'entre eux et appeler la méthode correspondante. Si vous avez correctement fait les choses vous devriez recevoir un e-mail.
