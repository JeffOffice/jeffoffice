# Formats de nombre et langue
<!-- keywords: format de nombre, format numérique, nombres, décimale, séparateur décimal, virgule, point, nombre de décimales, milliers, séparateur de milliers, groupement, langue, paramètres régionaux, monétaire, devise, pourcentage, date, heure, scientifique, fraction, texte, comptabilité, standard, personnalisé, code, exemple, Format de cellule, Ctrl+1, Ctrl+Maj+4, Ctrl+Maj+5, ajouter une décimale, réduire les décimales, séparateur de milliers, séparateur de formule, point-virgule -->

Une cellule contient un nombre, et son **format de nombre** indique comment ce nombre s'affiche à l'écran. Le format modifie ce que vous voyez, jamais le contenu de la cellule. Exemple : la cellule contient 0,256. Avec le format Pourcentage, elle affiche **25,6 %**, avec le format Monétaire **0,26 €**, et avec le format Standard **0,256**. Cliquez sur la cellule : la barre de formule indique toujours 0,256.

## Les nombres s'écrivent comme sur votre ordinateur

JeffCrunch utilise les séparateurs de la langue définie sur votre ordinateur. En anglais (États-Unis), mille deux cent trente-quatre et demi s'écrit **1,234.5**. En allemand, on écrit **1.234,5**, et en français **1 234,5**. Le classeur ne change pas : il conserve une forme neutre unique, si bien que le même fichier affiche 1,234.5 pour une personne et 1 234,5 pour une autre.

Cela s'applique partout où un nombre est affiché ou saisi :

- Une cellule, la copie d'une cellule et la largeur que **Ajuster la largeur de colonne** donne à une colonne suivent les séparateurs de votre langue.
- La **barre de formule** et une cellule que vous modifiez avec **F2** présentent un nombre tel que vous le tapez. Si votre langue écrit les décimales avec une virgule, tapez **2,5** et non 2.5.
- Dans une langue qui écrit les décimales avec une virgule, les arguments d'une formule sont séparés par des **points-virgules** : **=SUM(A1;A2)**. Les noms des fonctions restent en anglais.
- Les dates et les heures suivent le code du format : un format qui indique « mois, jour, année » les affiche dans cet ordre, et un format prévu pour votre langue (les formats prédéfinis de la liste) utilise l'ordre et les noms de votre langue.

Le texte que le format place entre guillemets, par exemple "unités", s'affiche tel quel.

## Choisir un format

1. Sélectionnez les cellules.
2. Utilisez le groupe **Nombre** de l'onglet **Accueil**, ou appuyez sur **Ctrl+1** (sur Mac, **Commande+1**) pour ouvrir la boîte de dialogue **Format de cellule**, qui s'ouvre sur l'onglet **Nombre**.

La liste du groupe **Nombre** propose les formats courants : Standard, Nombre, Monétaire, Comptabilité, Date courte, Date longue, Heure, Pourcentage, Fraction, Scientifique, Texte et Personnalisé. **Autres formats numériques…**, à la fin de la liste, ouvre la fenêtre Format de cellule. Les boutons situés sous la liste sont les suivants :

| Bouton | Action |
| --- | --- |
| Monétaire | Affiche le nombre sous forme de montant, avec le symbole monétaire de votre langue (**Ctrl+Maj+4**). La flèche à côté propose Standard et **Autres formats numériques…**. |
| Style de pourcentage | Multiplie par 100 et ajoute un signe pourcentage, sans décimales (**Ctrl+Maj+5**). Cliquez de nouveau pour l'annuler. |
| Séparateur de milliers | Groupe les chiffres par milliers, affiche deux décimales et aligne les séparateurs décimaux dans une colonne, sans symbole. |
| Ajouter une décimale, Réduire les décimales | Ajoute ou retire une décimale. |
| Standard (dans la liste, et dans la flèche à côté de Monétaire) | Supprime le format : le nombre s'affiche tel quel (**Ctrl+Maj+~**). |

## L'onglet Nombre de la boîte de dialogue Format de cellule

Choisissez une **Catégorie** à gauche ; ses options s'affichent à droite, et la zone **Exemple** montre l'aspect de la première cellule sélectionnée, dans votre langue.

- **Nombre** : nombre de décimales, groupement des chiffres par milliers et présentation des nombres négatifs (signe moins, en rouge ou entre parenthèses).
- **Monétaire** et **Comptabilité** : nombre de décimales et symbole. Le format Comptabilité aligne les symboles et les séparateurs décimaux dans une colonne.
- **Date** et **Heure** : une liste de formats prédéfinis, avec l'ordre et les noms de votre langue.
- **Pourcentage**, **Scientifique** : nombre de décimales.
- **Fraction** : des formats prédéfinis tels que « Jusqu'à un chiffre » (1/4), « Jusqu'à deux chiffres » (21/25), « En quarts » et « En seizièmes ».
- **Spécial** : des formats prédéfinis pour un code postal, un code postal + 4, un numéro de téléphone et un numéro de sécurité sociale.
- **Texte** : la cellule conserve ce que vous tapez sous forme de texte, si bien qu'un nombre comme 00123 garde ses zéros.
- **Personnalisé** : tapez votre propre code, ou partez d'un code de la liste (les codes courants et ceux déjà utilisés dans le classeur).

Le **code** est le court texte qui indique à JeffCrunch comment écrire un nombre. Exemple : **#,##0.00** signifie « grouper les chiffres par milliers et toujours afficher deux décimales » ; 1234,5 s'affiche donc 1 234,50. Chaque **0** est un chiffre toujours affiché, chaque **#** un chiffre affiché uniquement si nécessaire, et **%** multiplie par 100. Un code peut comporter jusqu'à quatre sections séparées par des points-virgules : nombres positifs, nombres négatifs, zéro et texte.

## Ce qui est conservé

Le code est enregistré dans le classeur sous sa forme neutre, de sorte que le classeur se lit correctement sur n'importe quel ordinateur. La mise en forme constitue une seule action que vous pouvez annuler, et un classeur qui utilisait déjà un format le conserve à l'identique : un format présent dans la liste est retrouvé, et aucun n'est créé en double.
