# Formats de nombres et votre langue
<!-- keywords: format de nombre, nombres, décimale, séparateur décimal, décimales, milliers, séparateur, groupement, virgule, point, langue, paramètres régionaux, monétaire, devise, pourcentage, date, heure, scientifique, fraction, texte, comptabilité, standard, personnalisé, code, exemple, Format de cellule, Ctrl+1, Ctrl+Maj+4, Ctrl+Maj+5, augmenter les décimales, réduire les décimales, séparateur de milliers, séparateur de formules, point-virgule, format, mise en forme des nombres -->

Une cellule contient un nombre, et son **format de nombre** indique comment ce nombre s'affiche à l'écran. Le format change ce que vous voyez, jamais ce que la cellule contient. Exemple : la cellule contient 0,256. Avec le format Pourcentage, elle affiche **25,6 %**, avec Monétaire elle affiche **0,26 €**, et avec Standard elle affiche **0,256**. Cliquez sur la cellule : la barre de formule affiche toujours 0,256.

## Les nombres s'écrivent comme votre ordinateur les écrit

JeffCrunch utilise les signes de la langue que votre ordinateur est configuré pour utiliser. En anglais (États-Unis), mille deux cent trente-quatre et demi s'écrit **1,234.5**. En allemand, il s'écrit **1.234,5**, et en français **1 234,5**. Le classeur ne change pas : il conserve une forme neutre, de sorte que le même fichier affiche 1,234.5 pour une personne et 1.234,5 pour une autre.

Cela s'applique partout où un nombre est affiché ou saisi :

- Une cellule, la copie d'une cellule et la largeur que donne **Ajuster la largeur de colonne** à une colonne suivent les signes de votre langue.
- La **barre de formule** et une cellule modifiée avec **F2** affichent un nombre tel que vous le saisissez. Si votre langue écrit les décimales avec une virgule, tapez **2,5**, et non 2.5.
- Dans une langue qui écrit les décimales avec une virgule, une formule utilise des **points-virgules** entre ses arguments : **=SOMME(A1;A2)**. Les noms des fonctions restent en anglais.
- Les dates et les heures suivent le code du format : un format qui indique « mois, jour, année » les affiche dans cet ordre, et un format conçu pour votre langue (ceux de la liste prédéfinie) utilise l'ordre et les noms de votre langue.

Le texte que le format place entre guillemets, comme « unités », est affiché tel quel.

## Choisir un format

1. Sélectionnez les cellules.
2. Utilisez le groupe **Nombre** de l'onglet **Accueil**, ou appuyez sur **Ctrl+1** (sur Mac, **Commande+1**) pour ouvrir **Format de cellule**, qui s'ouvre sur l'onglet **Nombre**.

La zone du groupe **Nombre** liste les formats courants : Standard, Nombre, Monétaire, Comptabilité, Date courte, Date longue, Heure, Pourcentage, Fraction, Scientifique, Texte et Personnalisé. **Autres formats de nombre** à la fin ouvre la fenêtre Format de cellule. Les boutons situés sous la zone sont :

| Bouton | Ce qu'il fait |
| --- | --- |
| Monétaire | Affiche le nombre comme une somme d'argent, avec le symbole monétaire de votre langue (**Ctrl+Maj+4**). La flèche à côté propose **Format de nombre comptable** avec une devise à choisir (dollar américain, livre sterling, euro, yen japonais, yuan chinois ou franc suisse), Standard et **Autres formats de nombre**. |
| Style pourcentage | Multiplie par 100 et ajoute un signe pourcentage, sans décimales (**Ctrl+Maj+5**). Cliquez à nouveau pour le supprimer. |
| Style millier | Groupe les chiffres par milliers, affiche deux décimales et aligne les virgules décimales en colonne, sans symbole. |
| Augmenter la décimale, Réduire la décimale | Ajoute ou retire une décimale. |
| Standard (dans la zone, et dans la flèche à côté de Monétaire) | Supprime le format : le nombre s'affiche tel quel (**Ctrl+Maj+~**). |

## L'onglet Nombre de Format de cellule

Choisissez une **Catégorie** à gauche ; les options correspondantes apparaissent à droite, et l'**Exemple** montre l'aspect que prendra la première cellule sélectionnée, dans votre langue.

- **Nombre** : nombre de décimales, groupement des chiffres par milliers et présentation des nombres négatifs (signe moins, rouge ou entre parenthèses).
- **Monétaire** et **Comptabilité** : nombre de décimales et symbole. La comptabilité aligne les symboles et les virgules décimales en colonne.
- **Date** et **Heure** : une liste de formats prédéfinis, avec l'ordre et les noms de votre langue.
- **Pourcentage**, **Scientifique** : nombre de décimales.
- **Fraction** : formats prédéfinis tels que « jusqu'à un chiffre » (1/4), « jusqu'à deux chiffres » (21/25), « en quarts » et « en seizièmes ».
- **Spécial** : formats prédéfinis pour un code postal, un code postal plus 4, un numéro de téléphone et un numéro de sécurité sociale.
- **Texte** : la cellule contient ce que vous saisissez comme texte, si bien qu'un nombre tel que 00123 conserve ses zéros.
- **Personnalisé** : saisissez un code de votre choix, ou partez d'un code de la liste (les codes courants et ceux que le classeur utilise déjà).

Le **code** est le court texte qui indique à JeffCrunch comment écrire un nombre. Exemple : **#,##0.00** signifie « grouper les chiffres par milliers et toujours afficher deux décimales », de sorte que 1234,5 s'affiche 1 234,50. Chaque **0** est un chiffre toujours affiché, chaque **#** un chiffre affiché seulement si nécessaire, et **%** multiplie par 100. Un code peut comporter jusqu'à quatre sections séparées par des points-virgules, pour les nombres positifs, les nombres négatifs, zéro et le texte.

## Ce qui est conservé

Le code est enregistré dans le classeur sous sa forme neutre, afin que le classeur s'affiche correctement sur tout ordinateur. La mise en forme est une étape que vous pouvez annuler, et un classeur qui utilisait déjà un format le conserve exactement : un format présent dans la liste est retrouvé, et aucun n'est créé deux fois.
