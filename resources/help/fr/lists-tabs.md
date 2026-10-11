# Puces, numérotation et tabulations
<!-- keywords: liste à puces, liste numérotée, numérotation, puces, liste, taquets de tabulation, tabulations, tabulation, règle, points de suite, tabulation décimale, retrait négatif, retrait suspendu -->

Une liste place chaque élément sur sa propre ligne, précédé d'une puce ou d'un numéro. Les taquets de tabulation alignent le texte en colonnes nettes, comme les prix d'un menu.

## Créer une liste

1. Cliquez dans un paragraphe ou sélectionnez-en plusieurs.
2. Cliquez sur **Accueil > Paragraphe > Puces** ou sur **Accueil > Paragraphe > Numérotation**.

Cliquez de nouveau sur le même bouton pour désactiver la liste. Chaque bouton comporte aussi une flèche. La flèche à côté de **Puces** propose huit symboles de puce. La flèche à côté de **Numérotation** propose sept styles, par exemple 1. 2. 3., a. b. c., I. II. III. et (1) (2) (3). Choisissez **Aucun** pour supprimer une liste.

Pour plus de choix, choisissez **Puces et numérotation…** dans l'un ou l'autre menu. L'onglet **Puces** propose 12 symboles et un bouton **Caractère…** pour choisir le vôtre. L'onglet **Numérotation** comporte une zone **Commencer à**, qui permet par exemple de commencer une liste à 5.

**Augmenter le retrait** et **Diminuer le retrait** déplacent les éléments de liste d'un niveau vers l'intérieur ou l'extérieur, par pas de 0,6 cm. Un élément déplacé vers l'intérieur commence sa propre liste sous l'élément précédent : les numéros y deviennent a. b. c. (puis i. ii. iii. au niveau suivant), et les puces deviennent des cercles (puis des carrés). Par exemple, sélectionnez les éléments 2 et 3 d'une liste numérotée et cliquez sur **Augmenter le retrait** : ils deviennent a. et b. sous 1., et l'élément suivant reste 2.

## Taper une liste

- Tapez `*` ou `-` suivi d'un espace au début d'un paragraphe pour commencer une liste à puces.
- Tapez `1.` ou `1)` suivi d'un espace pour commencer une liste numérotée.
- Appuyez sur Entrée pour passer à l'élément suivant. Appuyez sur Entrée dans un élément vide pour terminer la liste.
- Appuyez sur Retour arrière au début d'un élément pour supprimer sa puce.

Pour désactiver cette fonction, décochez **Mise en forme automatique lors de la frappe** dans l'onglet **Vérification** de **Fichier > Options** (la fenêtre **Options de JeffPub**).

## Les taquets de tabulation

Un taquet de tabulation est un emplacement de la ligne où le texte se place quand vous appuyez sur la touche Tab. Si vous n'en définissez aucun, Tab passe au taquet par défaut suivant. Ceux-ci sont espacés de 1,27 cm, à partir du bord gauche du texte, sauf si vous les modifiez (voir **Modifier les taquets par défaut** ci-dessous). Si le texte qui suit une tabulation ne tient pas sur la ligne, la tabulation et ce texte passent ensemble au début de la ligne suivante, à condition que le premier mot y tienne après le taquet. Sinon, la tabulation reste en fin de ligne et le texte commence la ligne suivante au bord gauche du paragraphe. Il existe quatre types de taquets :

- **Gauche :** le texte commence au taquet.
- **Centré :** le texte est centré sur le taquet.
- **Droite :** le texte se termine au taquet.
- **Décimal :** les séparateurs décimaux s'alignent sur le taquet.

Un taquet peut aussi avoir des points de suite : une suite de points, de tirets, un trait ou des puces qui remplit l'espace avant le taquet. Une ligne de menu comme « Soupe ....... 4,50 € » est une tabulation droite avec des points de suite.

Les taquets de tabulation appartiennent aux paragraphes. Ils se mesurent à partir du bord gauche du texte, à l'intérieur de la marge de la zone. Les taquets par défaut font exception : ils s'appliquent à toute la composition.

## Définir des taquets sur la règle

1. Cliquez sur **Affichage > Afficher > Règles** si les règles sont masquées.
2. Cliquez dans le texte. La règle affiche en blanc la largeur du texte.
3. Cliquez sur la règle à l'endroit voulu. Un taquet de tabulation gauche apparaît.
4. Faites glisser un taquet pour le déplacer. Faites-le glisser vers le bas, hors de la règle, pour le supprimer.

## Définir des taquets dans une boîte de dialogue

1. Cliquez dans le paragraphe et double-cliquez sur la règle. La boîte de dialogue **Paragraphe** s'ouvre sur l'onglet **Tabulations**. (Vous pouvez aussi l'ouvrir depuis **Accueil > Paragraphe > Interligne > Paragraphe…**, puis cliquer sur l'onglet **Tabulations**.)
2. Cliquez sur **Ajouter**, puis double-cliquez sur la nouvelle **Position** pour taper une valeur.
3. Choisissez un **Alignement** (gauche, centré, droite ou décimal) et des **Points de suite** pour la ligne.
4. Cliquez sur **OK**.

**Supprimer** supprime la ligne sélectionnée. **Effacer tout** supprime tous les taquets.

Dans une cellule de tableau, la touche Tab passe plutôt à la cellule suivante. Voir [Créer et modifier des tableaux](tables).

## Modifier les taquets par défaut

Lorsqu'un paragraphe n'a pas de taquets propres, la touche Tab passe au taquet par défaut suivant. Ceux-ci sont initialement espacés de 1,27 cm, et un seul paramètre définit leur espacement pour tous les paragraphes de la composition.

1. Ouvrez la boîte de dialogue **Paragraphe** et cliquez sur l'onglet **Tabulations**, comme décrit ci-dessus. Inutile de sélectionner du texte au préalable.
2. Sous la liste des taquets, tapez une distance dans **Taquets par défaut**. Elle peut aller de 1 point à 1 584 points (55,88 cm). La zone utilise les unités de mesure choisies dans **Fichier > Options**, et vous pouvez taper une unité, par exemple 1 cm.
3. Cliquez sur **OK**.

Tout le texte qui utilise les taquets par défaut adopte immédiatement le nouvel espacement, et **Annuler** rétablit l'ancien. La composition conserve ce paramètre à l'enregistrement, en fichier JeffPub comme en fichier .pub. Les taquets que vous avez définis vous-même, sur la règle ou avec **Ajouter**, restent où vous les avez placés.

## Voir aussi

- [Mettre en forme le texte](formatting-text)
- [Utiliser les styles](styles)
- [L'onglet Accueil](tab-home)
