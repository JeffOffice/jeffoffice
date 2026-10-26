# Formats de nombre et langue

<!-- keywords: format de nombre, nombres, décimale, séparateur décimal, virgule décimale, nombre de décimales, milliers, séparateur de milliers, groupement des chiffres, virgule, point, espace, langue, paramètres régionaux, monétaire, devise, pourcentage, date, heure, scientifique, fraction, texte, comptabilité, standard, personnalisé, code, exemple, Format de cellule, Ctrl+1, Ctrl+Maj+4, Ctrl+Maj+5, ajouter une décimale, réduire les décimales, séparateur de milliers, séparateur de formule, point-virgule -->

Une cellule contient un nombre, et son **format de nombre** détermine la façon dont ce nombre s'affiche à l'écran. Le format change ce que vous voyez, jamais ce que contient la cellule. Exemple : la cellule contient 0,256. Avec le format Pourcentage, elle affiche **25,6 %**, avec le format Monétaire **0,26 €**, et avec le format Standard **0,256**. Si vous cliquez sur la cellule, la barre de formule indique toujours 0,256.

## Les nombres s'écrivent comme sur votre ordinateur

JeffCrunch utilise les séparateurs de la langue définie sur votre ordinateur. En anglais (États-Unis), mille deux cent trente-quatre et demi s'écrit **1,234.5**. En allemand, on écrit **1.234,5**, et en français **1 234,5**. Le classeur ne change pas : il conserve une forme neutre unique, de sorte que le même fichier affiche 1,234.5 pour une personne et 1 234,5 pour une autre.

Cette règle s'applique partout où un nombre est affiché ou saisi :

- Une cellule, la copie d'une cellule et la largeur attribuée à une colonne par **Ajuster la largeur de colonne** suivent les séparateurs de votre langue.
- La **Barre de formule** et une cellule que vous modifiez avec **F2** présentent un nombre tel que vous le tapez. Si votre langue écrit les décimales avec une virgule, tapez **2,5** et non 2.5.
- Dans une langue qui écrit les décimales avec une virgule, les arguments d'une formule sont séparés par des **points-virgules** : **=SUM(A1;A2)**. Les noms des fonctions restent en anglais.
- Les dates et les heures suivent le code du format : un format indiquant « mois, jour, année » les affiche dans cet ordre, et un format prévu pour votre langue (les formats prédéfinis de la liste) utilise l'ordre et les noms de votre langue.

Le texte que le format place entre guillemets, par exemple "unités", s'affiche tel quel.

## Choisir un format

1. Sélectionnez les cellules.
2. Utilisez le groupe **Nombre** de l'onglet **Accueil**, ou appuyez sur **Ctrl+1** (sur Mac, **Commande+1**) pour ouvrir la boîte de dialogue **Format de cellule**, qui s'ouvre sur l'onglet **Nombre**.

La zone du groupe **Nombre** répertorie les formats courants : Standard, Nombre, Monétaire, Comptabilité, Date courte, Date longue, Heure, Pourcentage, Fraction, Scientifique, Texte et Personnalisé. **Autres formats numériques**, en bas de la liste, ouvre la boîte de dialogue Format de cellule. Les boutons situés sous la zone sont les suivants :

| Bouton | Rôle |
| --- | --- |
| Monétaire | Affiche le nombre comme une somme d'argent, avec le symbole monétaire de votre langue (**Ctrl+Maj+4**). La flèche située à côté propose Standard et **Autres formats numériques**. |
| Style de pourcentage | Multiplie par 100 et ajoute un signe pourcentage, sans décimales (**Ctrl+Maj+5**). Cliquez de nouveau pour le supprimer. |
| Séparateur de milliers | Groupe les chiffres par milliers, affiche deux décimales et aligne les séparateurs décimaux dans une colonne, sans symbole. |
| Ajouter une décimale, Réduire les décimales | Ajoute ou supprime une décimale. |
| Standard (dans la zone et dans la flèche à côté de Monétaire) | Supprime le format : le nombre s'affiche tel quel (**Ctrl+Maj+~**). |

## L'onglet Nombre de la boîte de dialogue Format de cellule

Choisissez une **Catégorie** à gauche ; ses options apparaissent à droite, et la zone **Exemple** montre l'aspect de la première cellule sélectionnée, dans votre langue.

- **Nombre** : nombre de décimales, groupement ou non des chiffres par milliers, et présentation des nombres négatifs (signe moins, en rouge ou entre parenthèses).
- **Monétaire** et **Comptabilité** : nombre de décimales et symbole. Le format Comptabilité aligne les symboles et les séparateurs décimaux dans une colonne.
- **Date** et **Heure** : liste de formats prédéfinis, avec l'ordre et les noms de votre langue.
- **Pourcentage**, **Scientifique** : nombre de décimales.
- **Fraction** : formats prédéfinis tels que « Jusqu'à un chiffre » (1/4), « Jusqu'à deux chiffres » (21/25), « En quarts » et « En seizièmes ».
- **Spécial** : formats prédéfinis pour un code postal, un code postal + 4, un numéro de téléphone et un numéro de sécurité sociale.
- **Texte** : la cellule conserve ce que vous tapez sous forme de texte, de sorte qu'un nombre comme 00123 garde ses zéros.
- **Personnalisé** : tapez votre propre code, ou partez de l'un de ceux de la liste (les codes courants et ceux que le classeur utilise déjà).

Le **code** est le court texte qui indique à JeffCrunch comment écrire un nombre. Exemple : **#,##0.00** signifie « grouper les chiffres par milliers et toujours afficher deux décimales » ; ainsi, 1234,5 s'affiche 1 234,50. Chaque **0** représente un chiffre toujours affiché, chaque **#** un chiffre affiché uniquement si nécessaire, et **%** multiplie par 100. Un code peut comporter jusqu'à quatre sections séparées par des points-virgules : pour les nombres positifs, les nombres négatifs, le zéro et le texte.

## Ce qui est conservé

Le code est enregistré dans le classeur sous sa forme neutre, de sorte que le classeur s'affiche correctement sur n'importe quel ordinateur. La mise en forme constitue une seule étape que vous pouvez annuler, et un classeur qui utilisait déjà un format le conserve à l'identique : un format présent dans la liste est retrouvé, et aucun format n'est créé en double.
