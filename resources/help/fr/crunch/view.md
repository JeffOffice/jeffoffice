# Modifier l'affichage
<!-- keywords: plan, grouper, dissocier, réduire, développer, détail, niveau, affichage, zoom, de droite à gauche, arabe, hébreu, miroir, figer, figer les volets, libérer les volets, quadrillage, en-têtes, barre de formule, réduire le ruban, plein écran, barre d'état, minimum, maximum, afficher les formules, onglets de feuille, feuilles masquées, en-têtes, entêtes -->

L'onglet **Affichage** modifie la façon dont la feuille s'affiche à l'écran. Il ne modifie jamais le classeur.

## Affichages

Le groupe **Affichages** de l'onglet Affichage (ainsi que les trois boutons en bas à droite de la fenêtre) change ce que la feuille montre. **Normal** affiche les cellules telles qu'elles sont. **Mise en page** affiche les pages telles qu'elles seront imprimées, avec leur en-tête et leur pied de page. **Aperçu des sauts de page** grise ce qui ne sera pas imprimé, numérote les pages et vous permet de faire glisser les lignes qui marquent le début des pages. Voir [Mettre en page les pages](page-layout) pour les deux derniers. Chaque feuille garde son affichage, et le classeur l'enregistre.

## Zoom

Utilisez le curseur en bas à droite, le bouton **Zoom** de l'onglet Affichage, ou maintenez la touche **Ctrl** enfoncée et faites tourner la molette de la souris. Vous pouvez zoomer de 10 % à 400 %. **100 %** rétablit la taille normale.

## Afficher et masquer

Dans le groupe **Afficher**, activez ou désactivez le **Quadrillage** (les lignes pâles entre les cellules), les **Titres** (les lettres des colonnes et les numéros des lignes) et la **Barre de formule**. Un classeur peut demander que son quadrillage ou ses titres soient désactivés ; ce choix vous appartient et vous pouvez le modifier.

Deux autres options se trouvent dans le même groupe. **Réduire le ruban** (**Ctrl+F1**) replie le ruban pour n'afficher que les noms des onglets, ce qui laisse plus de place aux cellules. Cliquez à nouveau, ou cliquez sur un onglet, pour le faire réapparaître. **Plein écran** (**Ctrl+Maj+F1**) fait occuper à la fenêtre tout l'écran. Cliquez à nouveau pour revenir à l'affichage précédent.

## Les valeurs de la barre d'état

Lorsque vous sélectionnez deux cellules ou plus, la barre d'état affiche des valeurs les concernant. **Cliquez avec le bouton droit** sur ces valeurs pour choisir celles qui s'affichent : **Moyenne**, **Nb (non vides)** (les cellules qui contiennent quelque chose), **Chiffres** (seulement les cellules qui contiennent des nombres), **Minimum** (le plus petit nombre), **Maximum** (le plus grand nombre) et **Somme**. Une coche indique celles qui sont activées. Au moins une reste activée. JeffCrunch garde votre choix en mémoire.

## Figer des lignes et des colonnes

Les lignes et colonnes **figées** restent en place pendant que le reste de la feuille défile, de sorte qu'une ligne d'en-tête reste toujours visible. Dans l'onglet **Affichage**, cliquez sur **Figer les volets** :

- **Figer les volets** fige les lignes situées au-dessus de la cellule sélectionnée et les colonnes situées à sa gauche, à partir de la première ligne et de la première colonne visibles. Les lignes et colonnes que vous avez fait défiler hors de vue restent hors de vue jusqu'à ce que vous libériez les volets.
- **Figer la ligne supérieure** fige la première ligne visible.
- **Figer la première colonne** fige la première colonne visible.
- **Libérer les volets** les rétablit, et la feuille reste à sa position.

Les lignes et colonnes figées sont enregistrées avec le classeur et sont rétablies à son ouverture.

Un classeur enregistré avec des volets figés s'ouvre avec ces volets figés.

## Feuilles

Les onglets en bas sont les feuilles du classeur. Cliquez sur un onglet pour afficher cette feuille. Chaque feuille garde votre position dans celle-ci. Les feuilles masquées ne sont pas affichées. Une ligne colorée sous un onglet correspond à la couleur de l'onglet de la feuille. Pour ajouter, renommer, déplacer, copier, masquer, supprimer ou colorer une feuille, voir [Gérer les feuilles](sheets).

## Feuilles qui se lisent de droite à gauche

Une feuille conçue pour une langue comme l'arabe ou l'hébreu se lit de droite à gauche, et JeffCrunch l'affiche ainsi : la colonne A se trouve au bord droit, les numéros de ligne sont à droite, et la barre de défilement horizontal va dans l'autre sens. Les touches fléchées suivent la feuille : la touche Flèche gauche va à la colonne suivante. Le texte d'une cellule commence du côté où sa langue se lit. Le classeur indique dans quel sens se lit une feuille, et vous pouvez le modifier : dans l'onglet **Mise en page**, cliquez sur **Feuille de droite à gauche**. Cliquez à nouveau pour la désactiver. Ce choix est enregistré avec la feuille.

## Grouper des lignes et des colonnes (plan)

Dans l'onglet **Données**, **Grouper les lignes** ajoute un niveau aux lignes sélectionnées, et **Dissocier les lignes** en retire un ; **Grouper les colonnes** et **Dissocier les colonnes** font la même chose pour les colonnes. Les lignes peuvent être groupées à l'intérieur d'autres groupes, jusqu'à sept niveaux. À côté des lignes (et au-dessus des colonnes), la feuille affiche un crochet pour chaque groupe et, sur la ligne qui résume le groupe (la ligne située sous les lignes, ou à droite des colonnes, sauf si votre fichier les place au-dessus ou à gauche), un bouton : **moins** masque les lignes du groupe, et **plus** les affiche à nouveau. Les boutons numérotés dans le coin affichent le plan jusqu'à un niveau donné : **1** n'affiche que les lignes qui ne figurent dans aucun groupe, **2** affiche aussi les lignes du premier niveau, et ainsi de suite. Un groupe situé dans un groupe masqué reste masqué lorsque le groupe englobant est affiché, jusqu'à ce que son propre bouton soit cliqué. Chacune de ces opérations correspond à une étape d'annulation, et les lignes masquées ainsi que les groupes sont enregistrés avec le classeur. Dissocier un groupe masqué affiche ses lignes.

**Affichage > Symboles du plan** active ou désactive les crochets et les boutons.

## Afficher les formules

Dans l'onglet **Formules**, **Afficher les formules** (Ctrl+`; sur Mac, la touche Contrôle et non la touche Commande) affiche la formule de chaque cellule qui en contient une, à la place de sa valeur. Les colonnes qui contiennent des formules sont élargies, dans une certaine limite, afin que les formules s'affichent en entier, puis reprennent leur largeur initiale lorsque vous désactivez l'option ; les largeurs de colonnes de votre fichier ne sont pas modifiées.
