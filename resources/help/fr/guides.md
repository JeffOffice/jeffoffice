# Règles, repères et alignement automatique
<!-- keywords: règle, point zéro, origine de la règle, repère, repère de règle, repère de marge, grille, colonnes, ligne de base, magnétisme, aligner sur les repères, aligner sur les objets, unités de mesure, pouces, centimètres, afficher les repères, limites -->

Les repères sont des lignes qui vous aident à aligner les éléments. Ils apparaissent à l'écran, mais ne s'impriment jamais. L'alignement automatique fait sauter les objets sur un repère ou sur un autre objet lorsqu'ils s'en approchent.

## Règles et unités

Affichez ou masquez les règles avec **Affichage > Afficher > Règles**. Sur chaque règle, une ligne rouge suit le pointeur, et la partie de la règle située au-dessus de l'objet sélectionné est grisée. Lorsque vous tapez dans une zone de texte, la règle du haut affiche les marques de retrait et de tabulation.

Pour changer d'unité, cliquez sur **Fichier > Options**, ouvrez l'onglet **Options avancées** et choisissez les **Unités de mesure** : pouces, centimètres, millimètres, points ou picas. Dans toute zone de taille ou de position, vous pouvez aussi taper une unité, par exemple 2 cm.

## Déplacer le point zéro

Chaque règle compte à partir du coin supérieur gauche de la page : 0 correspond au coin, et les valeurs augmentent à mesure que vous vous en éloignez. Pour compter à partir d'un autre endroit, par exemple le coin d'un objet à mesurer, déplacez le point zéro :

- Maintenez la touche **Maj** enfoncée et faites glisser le long d'une règle avec le bouton **droit** de la souris. Le zéro de cette règle se place à l'endroit où vous relâchez.
- Pour déplacer les deux règles à la fois, faites glisser à partir du petit carré situé à la jonction des deux règles.
- Pour ramener le zéro d'une règle au coin de la page, double-cliquez sur cette règle. Pour ramener les deux, double-cliquez sur le petit carré. (Lorsque vous tapez dans une zone de texte, un double-clic sur la règle du haut ouvre plutôt les paramètres **Tabulations**.)

Les valeurs et les graduations des deux règles de la page comptent à partir du point zéro, tout comme la position affichée dans la barre d'état. Si le zéro est placé à 8 cm du bord, un objet situé à 3 cm du bord gauche de la page y indique -5 cm, car il se trouve 5 cm avant le zéro. Les zones de position (comme celles de la fenêtre **Mesure**), la fenêtre **Repères de règle** et les repères que vous faites glisser continuent de mesurer à partir du coin de la page. Les marques de retrait et de tabulation de la règle du texte continuent de mesurer à partir de la zone de texte.

Le point zéro est enregistré avec la composition : vous le retrouvez à la prochaine ouverture du fichier. Une nouvelle composition commence avec les deux zéros au coin de la page. Le déplacement du zéro constitue une seule étape pour **Annuler**.

## Les quatre types de repères

- Les **repères de marge** (roses) indiquent les marges. Définissez-les dans **Création de page > Mise en page > Marges**.
- Les **repères de grille** (bleus) divisent la page en colonnes et en lignes. Ils appartiennent à une page maître.
- Les **repères de ligne de base** (pointillés beiges) indiquent les lignes sur lesquelles repose le texte.
- Les **repères de règle** (verts) sont ceux que vous placez vous-même.

Affichez-les ou masquez-les tous avec **Affichage > Afficher > Repères**. **Affichage > Afficher > Lignes de base** (Ctrl+F7) affiche ou masque les repères de ligne de base.

## Définir les colonnes et les lignes

Cliquez sur **Création de page > Mise en page > Repères**. Sous **Repères prédéfinis**, choisissez Aucune grille, 2 colonnes, 3 colonnes, 4 colonnes, ou une grille 2 × 2, 3 × 3 ou 4 × 4. Pour davantage de réglages, choisissez **Repères de grille et de ligne de base…**. La fenêtre **Repères de disposition** comporte alors ces onglets :

- **Repères de marge** : les quatre marges.
- **Repères de grille** : les colonnes, les lignes et l'espace qui les sépare. Vous pouvez aussi ajouter un repère central.
- **Repères de ligne de base** : l'espacement entre les lignes et le point de départ de la première.

Pour que le texte repose sur les repères de ligne de base, ouvrez la fenêtre **Paragraphe** et cochez **Aligner le texte sur les repères de ligne de base**.

## Ajouter des repères de règle

- Faites glisser à partir de la règle du haut pour créer un repère horizontal. Faites glisser à partir de la règle de gauche pour créer un repère vertical.
- Vous pouvez aussi choisir **Ajouter un repère de règle horizontal** ou **Ajouter un repère de règle vertical** dans le menu **Repères**. Chaque nouveau repère se place à côté du précédent.
- Faites glisser un repère pour le déplacer. Faites-le glisser sur sa règle pour le supprimer.
- **Repères de règle…** permet de taper des positions exactes. Cette commande ajoute aussi une série de repères espacés régulièrement.
- **Effacer tous les repères de règle** supprime les repères de la page active.

Les repères d'une page maître apparaissent sur toutes les pages qui l'utilisent. Pour les déplacer, accédez à la page maître.

## Activer ou désactiver l'alignement automatique

Utilisez **Création de page > Disposition** :

- **Aligner sur les repères** : les objets s'alignent sur les repères de marge, les repères de grille, les repères de règle, les bords et le centre de la page.
- **Aligner sur les objets** : les objets s'alignent sur les bords et le centre des autres objets.

Les deux options sont activées au départ. L'alignement se déclenche à quelques pixels de distance. Maintenez la touche **Alt** enfoncée pendant le glissement pour le désactiver le temps de ce déplacement.

## Voir aussi

- [Déplacer, aligner, grouper et superposer des objets](arrange)
- [Pages maîtres, en-têtes et numéros de page](master-pages)
- [Taille de la page, marges et disposition](page-setup)
- [L'onglet Création de page](tab-page-design)
