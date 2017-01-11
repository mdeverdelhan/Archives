---
layout: post
title: Modifier l'UI des composants Swing non contrôlés
---

Modifier le *look and feel* d'un composant Swing est assez facile. Pour un bouton il suffit de créer une classe héritant de [JButton](http://docs.oracle.com/javase/6/docs/api/javax/swing/JButton.html) (qu'on peut appeler MyPrettyButton), de redéfinir la méthode [paint()](http://docs.oracle.com/javase/6/docs/api/javax/swing/JComponent.html#paint%28java.awt.Graphics%29), etc. et d'instancier tous nos boutons à partir de cette classe. Cette technique trouve néanmoins ses limites dès lors qu'on ne contrôle pas les composants qu'on utilise.

C'est par exemple le cas avec les boites de dialogue issues de [JOptionPane](http://docs.oracle.com/javase/6/docs/api/javax/swing/JOptionPane.html) (mais également les [JFileChooser](http://docs.oracle.com/javase/6/docs/api/javax/swing/JFileChooser.html), [JColorChooser](http://docs.oracle.com/javase/6/docs/api/javax/swing/JColorChooser.html), etc.).

{% gist 4078934 CallingDialog.java %}

Dans cet exemple la *ConfirmDialog* comporte deux boutons, "Yes" et "No", qui héritent directement de JButton et pas de notre MyPrettyButton. Leur rendu graphique est donc celui du [look and feel par défaut](http://docs.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html). Il en résulte que la charte graphique de notre logiciel n'est pas unifiée.

![Avant](/blog/res/swing-ui-dialog-before.png)

### Définition de l'UI

Pour corriger cela il faut redéfinir l'UI de **tous** les boutons. On doit donc travailler directement au niveau de l'[UIManager](http://docs.oracle.com/javase/6/docs/api/javax/swing/UIManager.html). Nous verrons par la suite comment le paramétrer. En attendant il faut construire la classe d'UI en question :

{% gist 4078934 DefaultButtonUI.java %}

Comme vous le constatez, on hérite d'une classe d'UI déjà existante et on peut ensuite faire ce qu'on veut. En l'occurrence j'ai choisi de redéfinir la méthode paint(), notamment pour mettre un effet bleuté au survol du bouton par le curseur. J'ai également pris le parti de modifier la façon dont est calculée la [PreferredSize](http://docs.oracle.com/javase/6/docs/api/javax/swing/JComponent.html#getPreferredSize%28%29).

### Configuration de l'UIManager

Il ne nous reste plus qu'à dire à l'UIManager que, par défaut, les boutons doivent avoir l'UI que nous venons de décrire.

{% gist 4078934 InitUI.java %}

Bien entendu la première ligne suffit. Notez simplement qu'il est possible de spécifier de nombreux paramètres du look and feel. Cela vous évite d'avoir à créer une classe à chaque fois que vous voulez changer une couleur par défaut. En suivant [ce lien](http://tips4java.wordpress.com/2008/10/09/uimanager-defaults/) vous trouverez une application très bien faite listant les clefs et valeurs par défaut de chaque look and feel et pour chaque composant Swing.


Pour terminer, voici maintenant à quoi ressemblent les boutons dont nous voulions changer le graphisme.

![Apres](/blog/res/swing-ui-dialog-after.png)
