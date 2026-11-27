# Changer la casse, trier et coller autrement

<!-- keywords: bouton options de collage, couper-coller intelligent, espaces au collage, ligne horizontale, filet, afficher la mise en forme, inspecteur de styles, maj+F1, tabulations, taquets de tabulation, taquet à barre, barre de tabulation, taquets de tabulation par défaut, distance de tabulation, points de suite, paragraphes solidaires, lignes solidaires, saut de page avant, veuve, orpheline, niveau de plan, sauts de ligne et de page, changer la casse, majuscules, minuscules, phrase, majuscule, inverser la casse, maj+F3, trier, trier les paragraphes, alphabétique, ordre, coller, collage spécial, continuer la liste, nouvelle liste, fusionner la liste, imbriquer le tableau, cellules de tableur, conserver uniquement le texte, fusionner la mise en forme, rtf, texte enrichi, texte mis en forme, glisser, texte sans mise en forme, image, presse-papiers -->

Ces outils se trouvent sous l'onglet **Accueil**. Ils s'appliquent au texte que vous sélectionnez.

## Changer la casse des lettres

La **casse** d'une lettre indique s'il s'agit d'une majuscule (A) ou d'une minuscule (a).

1. Sélectionnez le texte. Si rien n'est sélectionné, JeffWrite modifie le mot où se trouve le curseur.
2. Dans le groupe **Police**, cliquez sur **Modifier la casse**.
3. Choisissez l'une des options :
   - **Phrase** met en majuscule la première lettre de chaque phrase. Les autres lettres restent en minuscules.
   - **minuscules** met toutes les lettres en minuscules.
   - **MAJUSCULES** met toutes les lettres en majuscules.
   - **Majuscule à chaque mot** met en majuscule la première lettre de chaque mot.
   - **iNVERSER lA cASSE** transforme chaque majuscule en minuscule, et chaque minuscule en majuscule.

Appuyez sur **Maj+F3** pour parcourir trois de ces options à tour de rôle : **Majuscule à chaque mot**, puis **MAJUSCULES**, puis **minuscules**.

L'aspect des lettres ne change pas. Le gras reste du gras. **Ctrl+Z** annule la modification en une seule étape. Si le suivi des modifications est activé, la modification apparaît comme une suppression et une insertion.

## Trier les paragraphes

Le tri range les paragraphes dans un ordre, par exemple de A à Z. Utilisez-le pour une liste de noms, ou pour les lignes d'un tableau créé avec des tabulations ou des virgules.

1. Sélectionnez deux paragraphes ou plus.
2. Dans le groupe **Paragraphe**, cliquez sur **Trier**.
3. Choisissez le critère de tri. Chaque choix comporte un **champ**, un **type** et un **ordre**.
   - Le **champ** est une partie du paragraphe. Le champ 1 est la première partie. Indiquez si les parties sont séparées par des virgules ou des tabulations, dans la zone située sous les critères. Si vous laissez **Paragraphes**, le paragraphe entier constitue le seul champ.
   - Le **type** est **Texte**, **Nombre** ou **Date**. Un nombre se trie selon sa valeur, donc 9 vient avant 10.
   - L'**ordre** est **Croissant** (de A à Z, du plus petit au plus grand, de la plus ancienne à la plus récente) ou **Décroissant**.
4. Utilisez **Puis par** pour départager les éléments égaux.
5. Cochez **La première ligne est un en-tête** pour la laisser à sa place.
6. Cliquez sur **OK**.

Chaque paragraphe conserve sa propre mise en forme. Lorsque le suivi des modifications est activé, le tri est effectué normalement, sans être enregistré, et la barre d'état indique qu'il n'a pas été enregistré comme modification suivie. Le tri des lignes d'un tableau comportant une ligne insérée ou supprimée n'est pas effectué : acceptez ou refusez d'abord ces modifications.

## Coller autrement

Cliquez sur la petite flèche sous **Coller** pour afficher d'autres choix :

- Lorsque le texte provient d'un autre programme qui place du texte enrichi (RTF) dans le presse-papiers, **Ctrl+V** conserve sa mise en forme : polices, tailles, couleurs, gras, listes, tableaux, liens et images. Si le programme n'a fourni aucun texte enrichi, JeffWrite utilise sa forme de page web (HTML), puis le texte brut. Le texte enrichi de plus de 16 Mo n'est pas lu : le collage utilise alors la forme suivante. Faire glisser un texte mis en forme depuis un autre programme vers la page produit le même résultat.
- **Fusionner la mise en forme** colle le texte dans la mise en forme de l'emplacement de destination. Seuls le gras, l'italique, le soulignement et le barré sont conservés.
- **Conserver uniquement le texte** colle les mots sans aucune mise en forme.
- **Image** colle une image du presse-papiers. Cette option est grisée lorsque le presse-papiers ne contient pas d'image.
- **Collage spécial** (**Ctrl+Alt+V**) affiche toutes les façons de coller le contenu du presse-papiers à cet instant, et vous permet d'en choisir une, par exemple **Texte mis en forme (RTF)**, **Texte sans mise en forme** ou **HTML**.

Un choix est grisé lorsque le presse-papiers ne contient rien qu'il puisse utiliser.

Juste après un collage, un petit bouton **Options de collage** apparaît à côté du curseur. Cliquez dessus pour choisir **Conserver la mise en forme source**, **Fusionner la mise en forme** ou **Conserver uniquement le texte**. JeffWrite annule le collage et le refait de cette manière. La modification ne compte que pour une étape : un seul **Annuler** annule l'ensemble du collage, quel que soit le nombre de fois où vous avez changé d'avis. Au clavier, appuyez sur la touche **Ctrl** (la touche **Contrôle** sur un Mac) après un collage pour ouvrir le menu du bouton, puis utilisez les touches fléchées et **Entrée** pour choisir. Le bouton disparaît dès que vous saisissez du texte, déplacez le curseur ou appuyez sur **Échap**, et il n'est pas proposé dans un document que vous ne pouvez pas modifier.

**Définir le collage par défaut** se trouve dans la page **Avancé** de **Options**. Elle comporte une zone pour le collage au sein d'un même document, une pour le collage entre documents et une pour le collage depuis d'autres programmes. Chacune peut être réglée sur **Conserver la mise en forme source** (comportement d'origine), **Fusionner la mise en forme** ou **Conserver uniquement le texte**. **Coller** (**{key:Paste}**) applique le réglage correspondant à l'origine du contenu. Exemple : réglez le collage depuis d'autres programmes sur **Conserver uniquement le texte**, et les mots copiés depuis une page web prendront la mise en forme de l'endroit où ils sont collés. Vous pouvez toujours choisir une autre forme pour un collage donné avec le bouton **Options de collage**.

Deux autres zones de cette page définissent ce que fait **Coller** avec les éléments de liste collés à côté d'une liste (**Tels quels**, **Continuer la liste**, **Nouvelle liste** ou **Fusionner la liste**), et avec les cellules de tableur (**Comme des mots**, **Tableau**, **Imbriquer le tableau**, **Fusionner le tableau**, **Insérer comme nouvelles lignes** ou **Écraser les cellules**). Un choix n'est utilisé que lorsqu'il convient : **Imbriquer le tableau** avec le curseur hors d'un tableau, par exemple, colle les cellules comme des mots.

Lorsque le collage apporte des paragraphes de liste à côté d'une liste, le bouton propose aussi **Continuer la liste** (la numérotation reprend à partir de la liste voisine), **Nouvelle liste** (la numérotation recommence à 1) et **Fusionner la liste** (les éléments rejoignent la liste voisine et en prennent la mise en forme). **Ctrl+V** simple insère les éléments tels quels, et un choix modifie ce comportement. Les mêmes choix sont disponibles sous forme de commandes dans le menu Coller de l'onglet Accueil et dans **Collage spécial**, de sorte que le clavier y accède aussi.

Lorsque le presse-papiers contient des cellules de tableur (copiées depuis JeffCrunch ou un autre programme sous forme de tableau ou de lignes dont les cellules sont séparées par des tabulations), **Ctrl+V** simple les colle sous forme de mots. Pour les coller sous forme de cellules, choisissez **Tableau** dans le bouton, dans le menu Coller ou dans **Collage spécial**. Lorsque le curseur se trouve dans un tableau, le bouton propose **Imbriquer le tableau** (un tableau dans la cellule), **Fusionner le tableau** (les cellules rejoignent le tableau, qui reçoit les colonnes nécessaires), **Insérer comme nouvelles lignes**, **Insérer en tant que nouvelles colonnes** (les cellules sont placées dans de nouvelles colonnes après la cellule du curseur, à partir de sa ligne vers le bas ; un tableau peut comporter au maximum 256 colonnes et 50 000 cellules, et le collage est refusé, avec un message, lorsqu'une cellule fusionnée traverse l'emplacement des colonnes) et **Écraser les cellules** (les cellules remplacent celles du tableau, à partir de la cellule du curseur). Un tableau collé comporte au maximum 500 lignes et 63 colonnes, et une cellule contient au maximum 4 000 caractères ; ce qui dépasse est omis, et un message le signale. Un tableau imbriqué dans un tableau peut avoir au maximum 8 niveaux, et un tableau imbriqué dont les colonnes seraient trop étroites pour la cellule est refusé. Chaque collage ne compte que pour une étape : **Annuler** les annule tous, et **Rétablir** les remet en place.

Le **couper-coller intelligent** conserve les espaces correctes lorsque vous déplacez des mots. Si vous coupez des mots entiers situés entre deux espaces, JeffWrite supprime aussi l'une de ces espaces, pour éviter un double espace. Si vous collez un mot directement contre un autre mot, il ajoute une espace entre les deux. Il ne modifie pas le milieu d'un mot. Pour le désactiver, cliquez sur **Fichier > Options > Avancé** et décochez **Couper-coller intelligent**.

## Sauts de ligne et de page, et tabulations

Cliquez sur le petit bouton situé dans le coin du groupe **Paragraphe** pour ouvrir la boîte **Paragraphe**. Sous les choix d'espacement figurent les options suivantes :

- **Contrôle des veuves et des orphelines** évite qu'une ligne isolée d'un paragraphe reste seule en haut ou en bas d'une page.
- **Paragraphes solidaires** conserve le paragraphe sur la même page que le suivant. Utilisez-le pour un titre.
- **Lignes solidaires** conserve toutes les lignes du paragraphe sur une même page.
- **Masquer les numéros de ligne** exclut ce paragraphe de la numérotation des lignes de la page (voir Mise en page > Numéros de ligne).
- **Saut de page avant** place le paragraphe en haut d'une nouvelle page.
- **Ne pas diviser les mots** empêche la coupure des mots du paragraphe par un trait d'union en fin de ligne.
- **Ne pas ajouter d'espace entre les paragraphes du même style** supprime l'espace entre deux paragraphes ayant le même style.
- **Niveau de plan** indique la place du paragraphe dans le plan du document : **Corps de texte**, ou un niveau de 1 à 9.

Cliquez sur **Tabulations** dans la boîte pour définir des taquets de tabulation. Un taquet de tabulation est un emplacement où le texte saute lorsque vous appuyez sur **Tab**. Saisissez sa position, choisissez l'alignement du texte à cet endroit (à gauche, centré, à droite ou décimal), puis choisissez un point de suite. Un point de suite est la suite de points, de tirets ou de trait qui remplit l'espace situé avant le taquet. Cliquez sur **Définir** pour l'ajouter. Pour en supprimer un, sélectionnez-le dans la liste et cliquez sur **Effacer**. **Tout effacer** les supprime tous.

Choisissez **Barre** pour définir une barre de tabulation. Une barre n'arrête pas le texte. Elle trace un fin trait vertical à cet endroit sur chaque ligne du paragraphe. C'est un moyen de placer un filet entre des colonnes de texte. Une barre n'a pas de point de suite. La règle n'affiche pas les barres. Un paragraphe peut en comporter jusqu'à 64. Pas encore : dans un paragraphe qui se lit de droite à gauche, les barres sont encore mesurées à partir du bord gauche. Vous définissez et effacez les barres dans la boîte **Tabulations**.

**Taquets de tabulation par défaut** correspond à la distance entre les taquets qu'un paragraphe reçoit lorsqu'il n'en a pas de propres, et au-delà du dernier taquet qu'il possède. La distance habituelle est de 1,27 cm (un demi-pouce). Ce réglage vaut pour l'ensemble du document, et non seulement pour le paragraphe où vous vous trouvez. La nouvelle distance s'applique à tous les paragraphes dès que vous cliquez sur **OK**.

## Bordures et trame des paragraphes

Sélectionnez un ou plusieurs paragraphes. Dans le groupe **Paragraphe**, le bouton **Bordures** propose des choix rapides : **Bordure inférieure**, **Bordure supérieure**, **Bordure gauche**, **Bordure droite**, **Toutes les bordures** et **Aucune bordure**. Un choix rapide trace un trait simple, d'une demi-point d'épaisseur, à un point du texte. Un choix pour un seul côté laisse les autres côtés tels qu'ils étaient.

**Bordure et trame** en bas de cette liste ouvre une boîte. Cochez les côtés qui reçoivent un trait, ou cliquez sur **Encadré** ou **Aucun**. Cochez **Entre** pour placer un trait entre les paragraphes sélectionnés qui ont les mêmes bordures. Choisissez ensuite le **Style** (simple, double, épais, pointillé, tireté, tirets courts, tiret-point ou tiret-point-point), l'**Épaisseur**, la **Couleur** et la **Distance du texte**. **Trame** remplit les paragraphes d'une couleur. Le bouton **Trame** à côté de **Bordures** ne règle que le remplissage, avec les mêmes couleurs que **Couleur de police**.

Les traits et le remplissage constituent une seule étape, que **Annuler** peut retirer, et ils sont conservés lorsque vous enregistrez un fichier .docx ou .odt. Si un paragraphe provient d'un fichier qui avait sa propre bordure ou sa propre trame, votre choix le remplace.

## Tracer une ligne horizontale

Cliquez sur **Ligne horizontale** dans le groupe **Paragraphe**. JeffWrite ajoute un paragraphe distinct, muni d'un fin trait en dessous, après le paragraphe où se trouve le curseur. Le curseur passe à un paragraphe ordinaire situé sous la ligne. Pour supprimer la ligne, sélectionnez son paragraphe et supprimez-le.

## Voir la mise en forme à l'emplacement du curseur

Appuyez sur **Maj+F1**, ou utilisez **Afficher la mise en forme**. Une boîte présente les styles, la police, la taille, l'aspect des lettres, la couleur, ainsi que l'alignement, les retraits et l'espacement du paragraphe à l'emplacement du curseur. Elle ne fait qu'afficher ces éléments. Pour les modifier, utilisez les boîtes **Police** et **Paragraphe**.

## Le volet Presse-papiers

Le volet Presse-papiers conserve les 24 derniers éléments copiés ou coupés dans le programme de cette fenêtre, afin de coller un élément plus ancien sans le copier de nouveau. Cliquez sur la petite flèche située dans le coin du groupe **Presse-papiers** de l'onglet **Accueil** (ou appuyez sur **Alt**, puis **H**, puis **FO**) pour l'ouvrir. Un panneau apparaît à côté de la page.

Chaque élément affiche un petit aperçu et le type de contenu : texte, texte mis en forme, cellules, image, formes ou diapositives. Le plus récent figure en haut. Cliquez sur un élément pour le coller à l'endroit où se trouvent le curseur ou la sélection. Le collage se fait comme tout autre collage : une étape d'**Annuler** le retire, et le bouton Options de collage apparaît comme d'habitude.

- **Tout coller** colle tous les éléments, du plus ancien au plus récent, afin qu'ils se lisent dans l'ordre où vous les avez copiés.
- **Supprimer** retire l'élément sélectionné. **Tout effacer** les retire tous.
- Le volet ne collecte les éléments que lorsqu'il est ouvert. Pour continuer la collecte après l'avoir fermé, cochez **Continuer la collecte lorsque ce volet est fermé**. Ce choix est mémorisé, et la collecte commence au lancement du programme, avant que vous n'ouvriez le volet.
- Pour collecter aussi ce qui est copié dans d'autres programmes, cochez **Collecter aussi ce qui est copié dans d'autres programmes**. Seuls le texte brut et les images sont conservés, et cette option est désactivée à chaque ouverture du programme. Avec cette option activée, une copie faite dans un autre programme est lue intégralement avant que ce programme décide de la conserver ; une copie très volumineuse peut donc faire pause le programme un instant et utiliser autant de mémoire que la copie. Une seule forme est lue : son texte lorsqu'il en contient, sinon une image. Avec cette option désactivée, rien de ce qui est copié dans d'autres programmes n'est lu.

Les éléments ne sont conservés que dans la mémoire du programme. Rien n'est enregistré dans un fichier, et la liste est vidée lorsque vous fermez la dernière fenêtre du programme. Un élément reste dans la liste après la fermeture du document dont il provient. Un élément de plus de 8 Mo n'est pas conservé, et le volet vous en avertit ; une image compte 4 octets par point (pixel), donc une image de plus de 2 millions de pixels environ est trop volumineuse. L'ensemble des éléments est limité à 64 Mo, et les plus anciens sont retirés en premier. Tout élément qu'un gestionnaire de mots de passe marque comme secret n'est jamais conservé.
