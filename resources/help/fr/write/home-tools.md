# Changer la casse, trier et coller autrement

<!-- keywords: bouton options de collage, couper-coller intelligent, espaces au collage, ligne horizontale, filet, révéler la mise en forme, inspecteur de style, maj+f1, tabulations, taquets de tabulation, taquet à barre, barre de tabulation, taquets par défaut, distance de tabulation, points de suite, leader, paragraphes solidaires, lignes solidaires, saut de page avant, veuve, orpheline, niveau du plan, sauts de ligne et de page, changer la casse, majuscules, minuscules, phrase, mettre en majuscule, inverser la casse, maj+f3, trier, trier les paragraphes, alphabétique, ordre, coller, collage spécial, poursuivre la liste, nouvelle liste, fusionner la liste, imbriquer le tableau, cellules de tableur, conserver uniquement le texte, fusionner la mise en forme, rtf, texte enrichi, texte mis en forme, glisser, glisser-déplacer, texte sans mise en forme, image, presse-papiers, casse, tri, collage, retrait, espacement -->

Ces outils se trouvent sous l'onglet **Accueil**. Ils agissent sur le texte que vous sélectionnez.

## Changer la casse des lettres

La **casse** d'une lettre indique si c'est une majuscule (A) ou une minuscule (a).

1. Sélectionnez le texte. Si rien n'est sélectionné, JeffWrite modifie le mot où se trouve le curseur.
2. Dans le groupe **Police**, cliquez sur **Modifier la casse**.
3. Choisissez l'une des options :
   - **Phrase** met en majuscule la première lettre de chaque phrase. Le reste reste en minuscules.
   - **minuscules** met toutes les lettres en minuscules.
   - **MAJUSCULES** met toutes les lettres en majuscules.
   - **Majuscule à chaque mot** met en majuscule la première lettre de chaque mot.
   - **iNVERSER lA cASSE** transforme chaque majuscule en minuscule, et chaque minuscule en majuscule.

Appuyez sur **Maj+F3** pour parcourir trois de ces options à tour de rôle : **Majuscule à chaque mot**, puis **MAJUSCULES**, puis **minuscules**.

L'aspect des lettres reste le même. Le gras reste du gras. **Ctrl+Z** annule la modification en une seule étape. Si le suivi des modifications est activé, la modification apparaît comme une suppression et une insertion.

## Trier des paragraphes

Le tri range les paragraphes dans un ordre, par exemple de A à Z. Utilisez-le pour une liste de noms, ou pour des lignes de tableau créées avec des tabulations ou des virgules.

1. Sélectionnez deux paragraphes ou plus.
2. Dans le groupe **Paragraphe**, cliquez sur **Trier**.
3. Choisissez le critère de tri. Chaque choix comporte un **champ**, un **type** et un **ordre**.
   - Le **champ** est une partie du paragraphe. Le champ 1 est la première partie. Indiquez si les parties sont séparées par des virgules ou des tabulations, dans la zone située sous les clés. Si vous laissez **Paragraphes**, le paragraphe entier constitue le seul champ.
   - Le **type** est **Texte**, **Nombre** ou **Date**. Un nombre se trie d'après sa valeur, donc 9 vient avant 10.
   - L'**ordre** est **Croissant** (de A à Z, du plus petit au plus grand, du plus ancien au plus récent) ou **Décroissant**.
4. Utilisez **Puis par** pour départager les égalités.
5. Cochez **Le premier paragraphe est un en-tête** pour le laisser à sa place.
6. Cliquez sur **OK**.

Chaque emplacement conserve la mise en forme de son propre paragraphe. Le tri ne peut pas être utilisé pendant que le suivi des modifications est activé, car un tri n'est pas une modification à afficher. Désactivez le suivi des modifications, triez, puis réactivez-le.

## Coller autrement

Cliquez sur la petite flèche sous **Coller** pour afficher d'autres choix :

- Lorsque le texte provient d'un autre programme qui place du texte enrichi (RTF) dans le presse-papiers, **Ctrl+V** conserve son aspect : polices, tailles, couleurs, gras, listes, tableaux, liens et images. Si le programme n'a fourni aucun texte enrichi, JeffWrite utilise sa version pour page web (HTML), puis le texte seul. Le texte enrichi de plus de 16 Mo n'est pas lu, et le collage utilise la forme suivante. Faire glisser un texte mis en forme depuis un autre programme vers la page produit le même résultat.
- **Fusionner la mise en forme** colle le texte dans l'aspect de l'emplacement où il arrive. Seuls le gras, l'italique, le soulignement et le barré sont conservés.
- **Conserver uniquement le texte** colle les mots sans aucune mise en forme.
- **Image** colle une image du presse-papiers. Elle est grisée lorsque le presse-papiers ne contient aucune image.
- **Collage spécial** (**Ctrl+Alt+V**) affiche toutes les façons de coller le contenu actuel du presse-papiers, et vous permet d'en choisir une, par exemple **Texte mis en forme (RTF)**, **Texte sans mise en forme** ou **HTML**.

Un choix est grisé lorsque le presse-papiers ne contient rien qu'il puisse utiliser.

Juste après un collage, un petit bouton **Options de collage** apparaît à côté du curseur. Cliquez dessus pour choisir **Conserver la mise en forme source**, **Fusionner la mise en forme** ou **Conserver uniquement le texte**. JeffWrite annule le collage et le refait de cette manière. La modification ne compte que pour une étape : un seul **Annuler** annule tout le collage, quel que soit le nombre de changements d'avis. Au clavier, appuyez sur la touche **Ctrl** (la touche **Contrôle** sur un Mac) après un collage pour ouvrir le menu du bouton, puis utilisez les touches fléchées et **Entrée** pour choisir. Le bouton disparaît dès que vous tapez, déplacez le curseur ou appuyez sur **Échap**, et il n'est pas proposé dans un document que vous ne pouvez pas modifier.

Lorsque le collage apporte des paragraphes de liste à côté d'une liste, le bouton propose aussi **Poursuivre la liste** (la numérotation continue à partir de la liste voisine), **Nouvelle liste** (la numérotation repart de 1) et **Fusionner la liste** (les éléments rejoignent la liste voisine et prennent son aspect). Un simple **Ctrl+V** insère les éléments tels quels, et un choix modifie ce comportement. Ces mêmes choix sont disponibles comme commandes dans le menu Coller de l'onglet Accueil et dans **Collage spécial**, donc accessibles au clavier.

Lorsque le presse-papiers contient des cellules de tableur (copiées depuis JeffCrunch ou un autre programme, sous forme de tableau ou de lignes avec des tabulations entre les cellules), un simple **Ctrl+V** les colle sous forme de texte. Pour les coller comme des cellules, choisissez **Tableau** dans le bouton, le menu Coller ou **Collage spécial**. Lorsque le curseur se trouve dans un tableau, le bouton propose **Imbriquer le tableau** (un tableau dans la cellule), **Fusionner le tableau** (les cellules rejoignent le tableau, qui reçoit les colonnes nécessaires), **Insérer comme nouvelles lignes**, **Insérer comme nouvelles colonnes** (les cellules sont placées dans de nouvelles colonnes après la cellule du curseur, à partir de sa ligne vers le bas ; un tableau peut compter au maximum 256 colonnes et 50 000 cellules, et l'opération est refusée avec un message lorsqu'une cellule fusionnée traverse l'emplacement des colonnes), et **Écraser les cellules** (les cellules remplacent celles du tableau, à partir de la cellule du curseur). Un tableau collé comporte au maximum 500 lignes et 63 colonnes, et une cellule contient au maximum 4 000 caractères ; ce qui dépasse est omis, et un message l'indique. Un tableau imbriqué dans un tableau peut descendre à 8 niveaux au maximum, et un tableau imbriqué dont les colonnes seraient trop étroites pour la cellule est refusé. Chaque collage ne compte que pour une étape : Annuler les annule tous, et Rétablir les restaure.

**Couper-coller intelligent** conserve des espaces corrects lorsque vous déplacez des mots. Si vous coupez des mots entiers situés entre deux espaces, JeffWrite supprime aussi l'un de ces espaces, pour que vous n'ayez pas une double espace. Si vous collez un mot directement contre un autre mot, il ajoute une espace entre les deux. Il ne modifie pas le milieu d'un mot. Pour le désactiver, cliquez sur **Fichier > Options > Avancé** et décochez **Couper-coller intelligent**.

## Sauts de ligne et de page, et tabulations

Cliquez sur le petit bouton situé dans le coin du groupe **Paragraphe** pour ouvrir la boîte **Paragraphe**. Sous les choix d'espacement figurent les options suivantes :

- **Contrôle des veuves et des orphelines** évite qu'une ligne isolée d'un paragraphe reste seule en haut ou en bas d'une page.
- **Paragraphes solidaires** garde le paragraphe sur la même page que le suivant. Utilisez-le pour un titre.
- **Lignes solidaires** garde toutes les lignes du paragraphe sur une même page.
- **Masquer la numérotation des lignes** exclut ce paragraphe lorsque les pages numérotent leurs lignes (voir Mise en page > Numéros de ligne).
- **Saut de page avant** place le paragraphe au début d'une nouvelle page.
- **Ne pas diviser les mots** empêche la coupure des mots du paragraphe par un trait d'union en fin de ligne.
- **Ne pas ajouter d'espace entre les paragraphes du même style** supprime l'espace entre deux paragraphes qui ont le même style.
- **Niveau du plan** indique la place du paragraphe dans le plan du document : **Corps de texte**, ou un niveau de 1 à 9.

Cliquez sur **Tabulations** dans la boîte pour définir des taquets de tabulation. Un taquet de tabulation est un emplacement où le texte saute lorsque vous appuyez sur **Tab**. Saisissez sa position, choisissez l'alignement du texte à cet endroit (à gauche, centré, à droite ou décimal), puis choisissez des points de suite. Un point de suite est la rangée de points, de tirets ou la ligne qui remplit l'espace avant le taquet. Cliquez sur **Définir** pour l'ajouter. Pour en supprimer un, sélectionnez-le dans la liste et cliquez sur **Effacer**. **Tout effacer** les supprime tous.

Choisissez **Barre** pour définir une barre de tabulation. Une barre n'arrête pas le texte. Elle trace un fin trait vertical, de haut en bas, à cet endroit sur chaque ligne du paragraphe. C'est un moyen de placer un filet entre des colonnes de texte. Une barre n'a pas de point de suite. La règle n'affiche pas les barres. Un paragraphe peut en contenir jusqu'à 64. Pas encore disponible : dans un paragraphe qui se lit de droite à gauche, les barres sont toujours mesurées depuis le bord gauche. Définissez-les et effacez-les dans la boîte **Tabulations**.

**Taquets par défaut** correspond à la distance entre les taquets qu'un paragraphe reçoit lorsqu'il n'en a pas de propres, et au-delà du dernier taquet qu'il possède. La distance habituelle est de 1,3 cm. Ce réglage concerne l'ensemble du document, et non seulement le paragraphe où vous vous trouvez. La nouvelle distance s'applique à chaque paragraphe dès que vous cliquez sur **OK**.

## Tracer une ligne horizontale

Cliquez sur **Ligne horizontale** dans le groupe **Paragraphe**. JeffWrite ajoute un paragraphe propre, avec un fin trait sous lui, après le paragraphe où se trouve le curseur. Le curseur passe à un paragraphe ordinaire placé sous le trait. Pour supprimer le trait, sélectionnez son paragraphe et supprimez-le.

## Voir ce qui est défini à l'emplacement du curseur

Appuyez sur **Maj+F1**, ou utilisez **Révéler la mise en forme**. Une boîte liste les styles, la police, la taille, l'aspect des lettres, la couleur, ainsi que l'alignement, les retraits et l'espacement du paragraphe à l'emplacement du curseur. Elle se contente de les afficher. Pour les modifier, utilisez les boîtes **Police** et **Paragraphe**.
