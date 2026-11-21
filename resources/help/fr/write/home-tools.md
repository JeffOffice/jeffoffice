# Changer la casse, trier et coller autrement

<!-- keywords: bouton options de collage, couper-coller intelligent, espaces lors du collage, ligne horizontale, filet, révéler la mise en forme, inspecteur de style, maj+F1, tabulations, taquets de tabulation, taquet à barre, barre, taquets par défaut, distance de tabulation, points de suite, garder avec le paragraphe suivant, garder les lignes ensemble, saut de page avant, veuve, orpheline, niveau du plan, sauts de ligne et de page, changer la casse, majuscules, minuscules, phrase, majuscule initiale, inverser la casse, maj+F3, trier, trier les paragraphes, ordre alphabétique, ordre, coller, collage spécial, poursuivre la liste, nouvelle liste, fusionner la liste, imbriquer le tableau, cellules de tableur, conserver le texte seul, fusionner la mise en forme, rtf, texte enrichi, texte mis en forme, glisser, glisser-déplacer, texte non mis en forme, image, presse-papiers -->

Ces outils se trouvent dans l'onglet **Accueil**. Ils agissent sur le texte que vous sélectionnez.

## Changer la casse des lettres

La **casse** d'une lettre indique s'il s'agit d'une majuscule (A) ou d'une minuscule (a).

1. Sélectionnez le texte. Si rien n'est sélectionné, JeffWrite modifie le mot où se trouve le curseur.
2. Dans le groupe **Police**, cliquez sur **Modifier la casse**.
3. Choisissez une option :
   - **Phrase** met en majuscule la première lettre de chaque phrase. Le reste reste en minuscules.
   - **minuscules** met toutes les lettres en minuscules.
   - **MAJUSCULES** met toutes les lettres en majuscules.
   - **Majuscule à chaque mot** met en majuscule la première lettre de chaque mot.
   - **iNVERSER lA cASSE** transforme chaque majuscule en minuscule, et chaque minuscule en majuscule.

Appuyez sur **Maj+F3** pour parcourir trois de ces options à tour de rôle : **Majuscule à chaque mot**, puis **MAJUSCULES**, puis **minuscules**.

L'aspect des lettres ne change pas. Le gras reste du gras. **Ctrl+Z** annule la modification en une seule étape. Si le suivi des modifications est activé, le changement apparaît comme une suppression et une insertion.

## Trier des paragraphes

Le tri range les paragraphes dans un ordre, par exemple de A à Z. Utilisez-le pour une liste de noms, ou pour des lignes de tableau faites avec des tabulations ou des virgules.

1. Sélectionnez deux paragraphes ou plus.
2. Dans le groupe **Paragraphe**, cliquez sur **Trier**.
3. Choisissez le critère de tri. Chaque choix comporte un **champ**, un **type** et un **ordre**.
   - Le **champ** est une partie du paragraphe. Le champ 1 est la première partie. Indiquez si les parties sont séparées par des virgules ou des tabulations, dans la zone située sous les clés. Si vous laissez **Paragraphes**, le paragraphe entier constitue le seul champ.
   - Le **type** est **Texte**, **Nombre** ou **Date**. Un nombre se trie selon sa valeur, donc 9 vient avant 10.
   - L'**ordre** est **Croissant** (de A à Z, du plus petit au plus grand, du plus ancien au plus récent) ou **Décroissant**.
4. Utilisez **Puis par** pour départager les éléments égaux.
5. Cochez **La première ligne est un en-tête** pour la laisser à sa place.
6. Cliquez sur **OK**.

Chaque paragraphe conserve sa propre présentation. Le tri ne peut pas être utilisé lorsque le suivi des modifications est activé, car un tri n'est pas une modification à afficher. Désactivez le suivi des modifications, triez, puis réactivez-le.

## Coller autrement

Cliquez sur la petite flèche sous **Coller** pour voir d'autres choix :

- Lorsque le texte provient d'un autre programme qui place du texte enrichi (RTF) dans le presse-papiers, **Ctrl+V** conserve son aspect : polices, tailles, couleurs, gras, listes, tableaux, liens et images. Si le programme n'a fourni aucun texte enrichi, JeffWrite utilise sa forme de page web (HTML), puis les mots seuls. Le texte enrichi de plus de 16 Mo n'est pas lu, et le collage utilise la forme suivante. Faire glisser un texte mis en forme depuis un autre programme sur la page produit le même résultat.
- **Fusionner la mise en forme** colle le texte dans l'aspect de l'endroit où il est placé. Seuls le gras, l'italique, le soulignement et le barré sont conservés.
- **Conserver le texte seul** colle les mots sans aucune mise en forme.
- **Image** colle une image du presse-papiers. Elle est grisée lorsque le presse-papiers ne contient pas d'image.
- **Collage spécial** (**Ctrl+Alt+V**) affiche toutes les façons de coller le contenu du presse-papiers à ce moment, et vous permet d'en choisir une, comme **Texte mis en forme (RTF)**, **Texte non mis en forme** ou **HTML**.

Un choix est grisé lorsque le presse-papiers ne contient rien qu'il puisse utiliser.

Juste après un collage, un petit bouton **Options de collage** apparaît à côté du curseur. Cliquez dessus pour choisir **Conserver la mise en forme source**, **Fusionner la mise en forme** ou **Conserver le texte seul**. JeffWrite annule le collage et le refait de cette manière. Le changement compte pour une seule étape : un seul **Annuler** annule tout le collage, quel que soit le nombre de changements d'avis. Au clavier, appuyez sur la touche **Ctrl** (la touche **Commande** sur Mac) après un collage pour ouvrir le menu du bouton, puis utilisez les flèches et **Entrée** pour choisir. Le bouton disparaît dès que vous tapez, déplacez le curseur ou appuyez sur **Échap**, et il n'est pas proposé dans un document que vous ne pouvez pas modifier.

**Définir le collage par défaut** se trouve dans la page **Avancé** de **Options**. Elle comporte une zone pour le collage à l'intérieur du même document, une pour le collage entre documents, et une pour le collage depuis d'autres programmes. Chacune peut être **Conserver la mise en forme source** (comportement d'origine), **Fusionner la mise en forme** ou **Conserver le texte seul**, et **Coller** (**{key:Paste}**) applique ce qui est défini pour l'origine du contenu. Exemple : réglez le collage depuis d'autres programmes sur **Conserver le texte seul**, et les mots copiés depuis une page web prendront la mise en forme de l'endroit où ils sont placés. Vous pouvez toujours choisir une autre forme pour un collage donné avec le bouton **Options de collage**.

Lorsque le collage apporte des paragraphes de liste à côté d'une liste, le bouton propose aussi **Poursuivre la liste** (les numéros continuent à partir de la liste voisine), **Nouvelle liste** (la numérotation recommence à 1) et **Fusionner la liste** (les éléments rejoignent la liste voisine et prennent son aspect). Un **Ctrl+V** simple insère les éléments tels qu'ils étaient, et un choix modifie ce comportement. Ces mêmes choix sont des commandes du menu Coller de l'onglet Accueil et de **Collage spécial**, de sorte que le clavier y accède aussi.

Lorsque le presse-papiers contient des cellules de tableur (copiées depuis JeffCrunch ou un autre programme sous forme de tableau ou de lignes avec des tabulations entre les cellules), un **Ctrl+V** simple les colle sous forme de texte. Pour les coller comme cellules, choisissez **Tableau** dans le bouton, dans le menu Coller ou dans **Collage spécial**. Lorsque le curseur se trouve dans un tableau, le bouton propose **Imbriquer le tableau** (un tableau dans la cellule), **Fusionner le tableau** (les cellules rejoignent le tableau, qui reçoit les colonnes nécessaires), **Insérer comme nouvelles lignes**, **Insérer comme nouvelles colonnes** (les cellules sont placées dans de nouvelles colonnes après la cellule du curseur, à partir de sa ligne vers le bas ; un tableau peut comporter au maximum 256 colonnes et 50 000 cellules, et le collage est refusé avec un message lorsqu'une cellule fusionnée traverse l'emplacement des colonnes), et **Remplacer les cellules** (les cellules remplacent celles du tableau, à partir de la cellule du curseur). Un tableau collé comporte au maximum 500 lignes et 63 colonnes, et une cellule contient au maximum 4 000 caractères ; ce qui dépasse est omis, et un message le signale. Un tableau imbriqué dans un tableau peut descendre au maximum à 8 niveaux, et un tableau imbriqué dont les colonnes seraient trop étroites pour la cellule est refusé. Chaque collage correspond à une seule étape : **Annuler** le retire entièrement, et **Rétablir** le remet en place.

**Couper-coller intelligent** conserve les espaces correctes lorsque vous déplacez des mots. Si vous coupez des mots entiers situés entre deux espaces, JeffWrite supprime aussi l'une de ces espaces, pour éviter un double espace. Si vous collez un mot directement contre un autre mot, il ajoute une espace entre eux. Il ne touche pas au milieu d'un mot. Pour le désactiver, cliquez sur **Fichier > Options > Avancé** et décochez **Couper-coller intelligent**.

## Sauts de ligne et de page, et tabulations

Cliquez sur le petit bouton dans le coin du groupe **Paragraphe** pour ouvrir la boîte **Paragraphe**. Sous les options d'espacement, on trouve :

- **Contrôle des veuves et des orphelines** évite qu'une ligne isolée d'un paragraphe reste seule en haut ou en bas d'une page.
- **Paragraphes solidaires** conserve le paragraphe sur la même page que le suivant. Utilisez-le pour un titre.
- **Lignes solidaires** conserve toutes les lignes du paragraphe sur une même page.
- **Supprimer la numérotation des lignes** exclut ce paragraphe lorsque les lignes des pages sont numérotées (voir Mise en page > Numéros de ligne).
- **Saut de page avant** commence le paragraphe sur une nouvelle page.
- **Ne pas diviser les mots** empêche les mots du paragraphe d'être coupés par un trait d'union en fin de ligne.
- **Ne pas ajouter d'espace entre les paragraphes de même style** supprime l'espace entre deux paragraphes ayant le même style.
- **Niveau du plan** indique la position du paragraphe dans le plan du document : **Corps de texte**, ou un niveau de 1 à 9.

Cliquez sur **Tabulations** dans la boîte pour définir des taquets de tabulation. Un taquet de tabulation est un endroit où le texte saute lorsque vous appuyez sur **Tab**. Saisissez sa position, choisissez l'alignement du texte à cet endroit (à gauche, centré, à droite ou décimal), puis choisissez des points de suite. Un point de suite est la rangée de points, de tirets ou le trait qui remplit l'espace avant le taquet. Cliquez sur **Définir** pour l'ajouter. Pour en retirer un, sélectionnez-le dans la liste et cliquez sur **Effacer**. **Tout effacer** les supprime tous.

Choisissez **Barre** pour définir une barre. Une barre n'arrête pas le texte. Elle trace un trait vertical fin, de haut en bas, à cet endroit sur chaque ligne du paragraphe. C'est un moyen de placer un filet entre des colonnes de texte. Une barre n'a pas de point de suite. La règle n'affiche pas les barres. Un paragraphe peut en comporter jusqu'à 64. Pas encore : dans un paragraphe qui se lit de droite à gauche, les barres sont toujours mesurées à partir du bord gauche. Définissez-les et effacez-les dans la boîte **Tabulations**.

**Distance des tabulations par défaut** est la distance entre les taquets qu'un paragraphe reçoit lorsqu'il n'en a aucun en propre, et au-delà du dernier taquet qu'il possède. La distance habituelle est d'un centimètre et quart. Ce réglage concerne tout le document, et pas seulement le paragraphe où vous vous trouvez. La nouvelle distance s'applique à tous les paragraphes dès que vous cliquez sur **OK**.

## Bordures et trame des paragraphes

Sélectionnez un ou plusieurs paragraphes. Dans le groupe **Paragraphe**, le bouton **Bordures** propose des choix rapides : **Bordure inférieure**, **Bordure supérieure**, **Bordure gauche**, **Bordure droite**, **Toutes les bordures** et **Aucune bordure**. Un choix rapide trace un trait simple, d'un demi-point d'épaisseur, à un point du texte. Un choix pour un seul côté laisse les autres côtés tels qu'ils étaient.

**Bordure et trame** au bas de cette liste ouvre une boîte. Cochez les côtés qui reçoivent un trait, ou cliquez sur **Encadrement** ou **Aucun**. Cochez **Entre** pour tracer un trait entre les paragraphes sélectionnés qui ont les mêmes bordures. Choisissez ensuite le **Style** (simple, double, épais, pointillé, tireté, tirets courts, tiret-point ou tiret-point-point), la **Largeur**, la **Couleur** et la **Distance du texte**. **Trame** remplit les paragraphes d'une couleur. Le bouton **Trame** à côté de **Bordures** ne règle que le remplissage, avec les mêmes couleurs que **Couleur de police**.

Les traits et le remplissage constituent une seule étape que **Annuler** retire, et ils sont conservés lorsque vous enregistrez un fichier .docx ou .odt. Si un paragraphe provient d'un fichier qui avait sa propre bordure ou sa propre trame, votre choix le remplace.

## Tracer une ligne horizontale

Cliquez sur **Ligne horizontale** dans le groupe **Paragraphe**. JeffWrite ajoute un paragraphe distinct, avec un trait fin en dessous, après le paragraphe où se trouve le curseur. Le curseur passe à un paragraphe simple situé sous la ligne. Pour supprimer la ligne, sélectionnez son paragraphe et supprimez-le.

## Voir ce qui est défini à l'emplacement du curseur

Appuyez sur **Maj+F1**, ou utilisez **Révéler la mise en forme**. Une boîte liste les styles, la police, la taille, l'aspect des lettres, la couleur, ainsi que l'alignement, les retraits et l'espacement du paragraphe à l'emplacement du curseur. Elle se contente de les afficher. Pour les modifier, utilisez les boîtes **Police** et **Paragraphe**.
