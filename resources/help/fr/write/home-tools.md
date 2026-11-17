# Changer la casse, trier et coller autrement

<!-- keywords: bouton coller, couper et coller intelligent, espaces au collage, ligne horizontale, filet, afficher la mise en forme, volet d'inspection des styles, maj+F1, tabulations, taquets de tabulation, points de suite, rester avec le suivant, garder les lignes ensemble, saut de page avant, veuve, orpheline, niveau du plan, sauts de ligne et de page, changer la casse, majuscules, minuscules, casse de phrase, capitales, inverser la casse, maj+F3, trier, trier les paragraphes, ordre alphabétique, ordre, coller, collage spécial, poursuivre la liste, nouvelle liste, fusionner la liste, imbriquer le tableau, cellules de tableur, conserver uniquement le texte, fusionner la mise en forme, rtf, texte enrichi, texte mis en forme, glisser, glisser-déplacer du texte, texte sans mise en forme, image, presse-papiers -->

Ces outils se trouvent sous l'onglet **Accueil**. Ils agissent sur le texte que vous sélectionnez.

## Changer la casse des lettres

La **casse** d'une lettre indique s'il s'agit d'une majuscule (A) ou d'une minuscule (a).

1. Sélectionnez le texte. Si rien n'est sélectionné, JeffWrite modifie le mot où se trouve le curseur.
2. Dans le groupe **Police**, cliquez sur **Modifier la casse**.
3. Choisissez l'une des options :
   - **Phrase** met en majuscule la première lettre de chaque phrase. Les autres lettres sont en minuscules.
   - **minuscules** met toutes les lettres en minuscules.
   - **MAJUSCULES** met toutes les lettres en majuscules.
   - **Majuscule à chaque mot** met en majuscule la première lettre de chaque mot.
   - **iNVERSER lA cASSE** transforme chaque majuscule en minuscule, et chaque minuscule en majuscule.

Appuyez sur **Maj+F3** pour parcourir trois de ces options à tour de rôle : **Majuscule à chaque mot**, puis **MAJUSCULES**, puis **minuscules**.

L'apparence des lettres reste la même. Le gras reste du gras. **Ctrl+Z** annule la modification en une seule étape. Si le suivi des modifications est activé, le changement apparaît comme une suppression et une insertion.

## Trier des paragraphes

Le tri classe les paragraphes dans un ordre, par exemple de A à Z. Utilisez-le pour une liste de noms ou pour des lignes de tableau créées avec des tabulations ou des virgules.

1. Sélectionnez deux paragraphes ou plus.
2. Dans le groupe **Paragraphe**, cliquez sur **Trier**.
3. Choisissez le critère de tri. Chaque choix comporte un **champ**, un **type** et un **ordre**.
   - Le **champ** est une partie du paragraphe. Le champ 1 est la première partie. Indiquez si les parties sont séparées par des virgules ou des tabulations, dans la zone située sous les clés. Si vous laissez **Paragraphes**, le paragraphe entier constitue le seul champ.
   - Le **type** est **Texte**, **Nombre** ou **Date**. Un nombre se trie d'après sa valeur, donc 9 vient avant 10.
   - L'**ordre** est **Croissant** (de A à Z, du plus petit au plus grand, du plus ancien au plus récent) ou **Décroissant**.
4. Utilisez **Puis par** pour départager les égalités.
5. Cochez **La première ligne est un en-tête** pour la laisser à sa place.
6. Cliquez sur **OK**.

Chaque emplacement conserve la mise en forme de son propre paragraphe. Le tri ne peut pas être utilisé lorsque le suivi des modifications est activé, car un tri n'est pas une modification à afficher. Désactivez le suivi des modifications, triez, puis réactivez-le.

## Coller autrement

Cliquez sur la petite flèche sous **Coller** pour afficher d'autres choix :

- Lorsque le texte provient d'un autre programme qui place du texte enrichi (RTF) dans le presse-papiers, **Ctrl+V** conserve son aspect : polices, tailles, couleurs, gras, listes, tableaux, liens et images. Si le programme n'a fourni aucun texte enrichi, JeffWrite utilise sa forme de page web (HTML), puis le texte brut. Le texte enrichi de plus de 16 Mo n'est pas lu, et le collage utilise la forme suivante. Faire glisser du texte mis en forme depuis un autre programme vers la page produit le même résultat.
- **Fusionner la mise en forme** colle le texte avec l'aspect de l'endroit où il est placé. Seuls le gras, l'italique, le soulignement et le barré sont conservés.
- **Conserver uniquement le texte** colle les mots sans aucune mise en forme.
- **Image** colle une image du presse-papiers. Cette commande est grisée lorsque le presse-papiers ne contient aucune image.
- **Collage spécial** (**Ctrl+Alt+V**) affiche toutes les façons dont le presse-papiers peut être collé à ce moment, et vous permet d'en choisir une, par exemple **Texte mis en forme (RTF)**, **Texte sans mise en forme** ou **HTML**.

Un choix est grisé lorsque le presse-papiers ne contient rien qu'il puisse utiliser.

Juste après le collage, un petit bouton **Options de collage** apparaît près du curseur. Cliquez dessus pour choisir **Conserver la mise en forme source**, **Fusionner la mise en forme** ou **Conserver uniquement le texte**. JeffWrite annule le collage et le refait de cette manière. Le bouton disparaît dès que vous tapez ou déplacez le curseur.

Lorsque le collage apporte des paragraphes de liste à côté d'une liste, le bouton propose aussi **Poursuivre la liste** (la numérotation continue depuis la liste voisine), **Nouvelle liste** (la numérotation repart de 1) et **Fusionner la liste** (les éléments rejoignent la liste voisine et prennent son aspect). Le **Ctrl+V** simple insère les éléments tels quels, et un choix modifie ce comportement. Les mêmes choix sont disponibles comme commandes dans le menu Coller de l'onglet Accueil et dans **Collage spécial**, de sorte que le clavier y accède aussi.

Lorsque le presse-papiers contient des cellules de tableur (copiées depuis JeffCrunch ou un autre programme, sous forme de tableau ou de lignes avec des tabulations entre les cellules), le **Ctrl+V** simple les colle sous forme de texte. Pour les coller en tant que cellules, choisissez **Tableau** dans le bouton, dans le menu Coller ou dans **Collage spécial**. Lorsque le curseur se trouve dans un tableau, le bouton propose **Imbriquer le tableau** (un tableau dans la cellule), **Fusionner le tableau** (les cellules rejoignent le tableau, qui reçoit les colonnes nécessaires), **Insérer comme nouvelles lignes** et **Remplacer les cellules** (les cellules remplacent celles du tableau, à partir de la cellule du curseur). Un tableau collé comporte au maximum 500 lignes et 63 colonnes, et une cellule contient au maximum 4 000 caractères ; ce qui dépasse est omis, et un message l'indique. Un tableau imbriqué dans un tableau peut comporter au maximum 8 niveaux, et un tableau imbriqué dont les colonnes seraient trop étroites pour la cellule est refusé. Chaque collage constitue une seule étape : **Annuler** les retire tous, et **Rétablir** les remet.

**Couper et coller intelligent** conserve les espaces correctes lorsque vous déplacez des mots. Si vous coupez des mots entiers situés entre deux espaces, JeffWrite supprime aussi l'une de ces espaces, pour éviter un double espace. Si vous collez un mot directement contre un autre mot, JeffWrite ajoute une espace entre les deux. Il ne modifie pas le milieu d'un mot. Pour désactiver cette fonction, cliquez sur **Fichier > Options > Avancé**, puis décochez **Couper et coller intelligent**.

## Sauts de ligne et de page, et tabulations

Cliquez sur le petit bouton situé dans le coin du groupe **Paragraphe** pour ouvrir la boîte de dialogue **Paragraphe**. Sous les choix d'espacement, vous trouverez les options suivantes :

- **Contrôle des veuves et des orphelines** empêche qu'une seule ligne d'un paragraphe reste isolée en haut ou en bas d'une page.
- **Paragraphes solidaires** conserve le paragraphe sur la même page que le suivant. Utilisez-le pour un titre.
- **Garder les lignes ensemble** conserve toutes les lignes du paragraphe sur une même page.
- **Masquer les numéros de ligne** exclut ce paragraphe de la numérotation des lignes de la page (voir Disposition > Numéros de ligne).
- **Saut de page avant** place le paragraphe en haut d'une nouvelle page.
- **Ne pas diviser les mots** empêche les mots du paragraphe d'être coupés par un trait d'union en fin de ligne.
- **Ne pas ajouter d'espace entre les paragraphes du même style** supprime l'espace entre deux paragraphes qui ont le même style.
- **Niveau du plan** indique la position du paragraphe dans le plan du document : **Corps de texte**, ou un niveau de 1 à 9.

Cliquez sur **Tabulations** dans la boîte pour définir des taquets de tabulation. Un taquet de tabulation est un emplacement où le texte se déplace lorsque vous appuyez sur **Tab**. Saisissez sa position, choisissez l'alignement du texte à cet endroit (à gauche, centré, à droite ou décimal), puis choisissez un remplissage. Le remplissage est la rangée de points, de tirets ou la ligne qui comble l'espace avant le taquet. Cliquez sur **Définir** pour l'ajouter. Pour en supprimer un, sélectionnez-le dans la liste et cliquez sur **Effacer**. **Tout effacer** les supprime tous.

## Tracer une ligne horizontale

Cliquez sur **Ligne horizontale** dans le groupe **Paragraphe**. JeffWrite ajoute un paragraphe propre, muni d'un trait fin sous celui-ci, après le paragraphe où se trouve le curseur. Le curseur passe dans un paragraphe normal situé sous la ligne. Pour supprimer la ligne, sélectionnez son paragraphe et supprimez-le.

## Voir la mise en forme à l'emplacement du curseur

Appuyez sur **Maj+F1**, ou utilisez **Afficher la mise en forme**. Une boîte liste les styles, la police, la taille, l'aspect des lettres, la couleur, ainsi que l'alignement, les retraits et l'espacement du paragraphe à l'emplacement du curseur. Elle se contente d'afficher ces informations. Pour les modifier, utilisez les boîtes **Police** et **Paragraphe**.
