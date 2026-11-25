# Modifier la casse, trier et coller autrement

<!-- keywords: bouton options de collage, couper-coller intelligent, espaces au collage, ligne horizontale, filet, afficher la mise en forme, inspecteur de styles, maj+f1, tabulations, taquets de tabulation, taquet à barre, barre de tabulation, taquets par défaut, distance de tabulation, points de suite, reste avec le paragraphe suivant, garder les lignes ensemble, saut de page avant, veuve, orpheline, niveau de plan, sauts de ligne et de page, changer la casse, majuscules, minuscules, phrase, majuscule initiale, inverser la casse, maj+f3, trier, trier les paragraphes, alphabétique, ordre, coller, collage spécial, continuer la liste, nouvelle liste, fusionner la liste, imbriquer le tableau, cellules de tableur, conserver uniquement le texte, fusionner la mise en forme, rtf, texte enrichi, texte mis en forme, glisser, texte sans mise en forme, image, presse-papiers -->

Ces outils se trouvent sous l'onglet **Accueil**. Ils agissent sur le texte que vous sélectionnez.

## Modifier la casse des lettres

La **casse** d'une lettre indique si c'est une majuscule (A) ou une minuscule (a).

1. Sélectionnez le texte. Si rien n'est sélectionné, JeffWrite modifie le mot où se trouve le curseur.
2. Dans le groupe **Police**, cliquez sur **Changer la casse**.
3. Choisissez l'une des options :
   - **Phrase** met en majuscule la première lettre de chaque phrase. Les autres lettres sont en minuscules.
   - **minuscules** met toutes les lettres en minuscules.
   - **MAJUSCULES** met toutes les lettres en majuscules.
   - **Majuscule à chaque mot** met en majuscule la première lettre de chaque mot.
   - **iNVERSER lA cASSE** transforme chaque majuscule en minuscule, et chaque minuscule en majuscule.

Appuyez sur **Maj+F3** pour parcourir trois de ces options tour à tour : **Majuscule à chaque mot**, puis **MAJUSCULES**, puis **minuscules**.

L'aspect des lettres ne change pas. Le gras reste du gras. **Ctrl+Z** annule la modification en une seule étape. Si le suivi des modifications est activé, la modification apparaît sous forme de suppression et d'insertion.

## Trier des paragraphes

Le tri met les paragraphes en ordre, par exemple de A à Z. Utilisez-le pour une liste de noms, ou pour les lignes d'un tableau créé avec des tabulations ou des virgules.

1. Sélectionnez deux paragraphes ou plus.
2. Dans le groupe **Paragraphe**, cliquez sur **Trier**.
3. Choisissez le critère de tri. Chaque choix comporte un **champ**, un **type** et un **ordre**.
   - Le **champ** est une partie du paragraphe. Le champ 1 est la première partie. Indiquez si les parties sont séparées par des virgules ou des tabulations, dans la zone située sous les clés. Si vous laissez **Paragraphes**, le paragraphe entier constitue le seul champ.
   - Le **type** est **Texte**, **Nombre** ou **Date**. Un nombre se trie selon sa valeur, donc 9 vient avant 10.
   - L'**ordre** est **Croissant** (de A à Z, du plus petit au plus grand, de la plus ancienne à la plus récente) ou **Décroissant**.
4. Utilisez **Puis par** pour départager les ex æquo.
5. Cochez **La première ligne est un en-tête** pour la laisser à sa place.
6. Cliquez sur **OK**.

Chaque paragraphe conserve sa propre mise en forme. Le tri ne peut pas être utilisé lorsque le suivi des modifications est activé, car un tri n'est pas une modification à signaler. Désactivez le suivi des modifications, triez, puis réactivez-le.

## Coller autrement

Cliquez sur la petite flèche sous **Coller** pour afficher d'autres choix :

- Lorsque le texte provient d'un autre programme qui place du texte enrichi (RTF) dans le presse-papiers, **Ctrl+V** conserve son aspect : polices, tailles, couleurs, gras, listes, tableaux, liens et images. Si le programme n'a fourni aucun texte enrichi, JeffWrite utilise sa forme de page web (HTML), puis le texte seul. Le texte enrichi de plus de 16 Mo n'est pas lu : le collage utilise alors la forme suivante. Faire glisser un texte mis en forme depuis un autre programme vers la page produit le même résultat.
- **Fusionner la mise en forme** colle le texte avec l'aspect de l'endroit où il est inséré. Seuls le gras, l'italique, le soulignement et le barré sont conservés.
- **Conserver uniquement le texte** colle les mots sans aucune mise en forme.
- **Image** colle une image du presse-papiers. Cette commande est grisée lorsque le presse-papiers ne contient pas d'image.
- **Collage spécial** (**Ctrl+Alt+V**) affiche tous les formats dans lesquels le contenu du presse-papiers peut être collé à cet instant, et vous permet d'en choisir un, comme **Texte mis en forme (RTF)**, **Texte sans mise en forme** ou **HTML**.

Un choix est grisé lorsque le presse-papiers ne contient rien qu'il puisse utiliser.

Juste après un collage, un petit bouton **Options de collage** apparaît à côté du curseur. Cliquez dessus pour choisir **Conserver la mise en forme source**, **Fusionner la mise en forme** ou **Conserver uniquement le texte**. JeffWrite annule alors le collage et le refait de cette manière. La modification forme une seule étape : un seul **Annuler** annule tout le collage, quel que soit le nombre de fois où vous avez changé d'avis. Au clavier, appuyez sur la touche **Ctrl** (la touche **Commande** sur un Mac) après un collage pour ouvrir le menu du bouton, puis utilisez les touches fléchées et **Entrée** pour choisir. Le bouton disparaît dès que vous saisissez du texte, déplacez le curseur ou appuyez sur **Échap**, et il n'est pas proposé dans un document que vous ne pouvez pas modifier.

**Définir le collage par défaut** se trouve dans la page **Avancé** des **Options**. Elle comporte une zone pour le collage au sein du même document, une pour le collage entre documents, et une pour le collage depuis d'autres programmes. Chacune peut être réglée sur **Conserver la mise en forme source** (comportement d'origine), **Fusionner la mise en forme** ou **Conserver uniquement le texte**. **Coller** (**{key:Paste}**) applique le réglage défini pour l'origine du contenu. Exemple : réglez le collage depuis d'autres programmes sur **Conserver uniquement le texte** ; les mots copiés depuis une page web prendront alors la mise en forme de l'endroit où ils sont insérés. Vous pouvez toujours choisir une autre forme pour un collage donné avec le bouton **Options de collage**.

Lorsque le collage apporte des paragraphes de liste à côté d'une liste, le bouton propose aussi **Continuer la liste** (la numérotation se poursuit à partir de la liste voisine), **Nouvelle liste** (la numérotation repart de 1) et **Fusionner la liste** (les éléments rejoignent la liste voisine et prennent son aspect). **Ctrl+V** simple insère les éléments tels qu'ils étaient, et un choix modifie ce comportement. Les mêmes choix sont disponibles comme commandes dans le menu Coller de l'onglet Accueil et dans **Collage spécial**, donc accessibles au clavier également.

Lorsque le presse-papiers contient des cellules de tableur (copiées depuis JeffCrunch ou un autre programme, sous forme de tableau ou de lignes avec des tabulations entre les cellules), **Ctrl+V** simple les colle sous forme de texte. Pour les coller comme cellules, choisissez **Tableau** dans le bouton, dans le menu Coller ou dans **Collage spécial**. Lorsque le curseur se trouve dans un tableau, le bouton propose **Imbriquer le tableau** (un tableau dans la cellule), **Fusionner le tableau** (les cellules rejoignent le tableau, qui reçoit les colonnes nécessaires), **Insérer comme nouvelles lignes**, **Insérer comme nouvelles colonnes** (les cellules sont placées dans de nouvelles colonnes après la cellule du curseur, à partir de sa ligne vers le bas ; un tableau peut comporter au maximum 256 colonnes et 50 000 cellules, et le collage est refusé avec un message lorsqu'une cellule fusionnée s'étend sur l'emplacement des colonnes) et **Écraser les cellules** (les cellules remplacent celles du tableau, à partir de la cellule du curseur). Un tableau collé comporte au maximum 500 lignes et 63 colonnes, et une cellule contient au maximum 4 000 caractères ; ce qui dépasse est omis, et un message l'indique. Un tableau imbriqué dans un tableau peut atteindre au maximum 8 niveaux d'imbrication, et un tableau imbriqué dont les colonnes seraient trop étroites pour la cellule est refusé. Chaque collage forme une seule étape : Annuler le retire en entier, et Rétablir le restitue.

Le **couper-coller intelligent** conserve les espaces justes lorsque vous déplacez des mots. Si vous coupez des mots entiers situés entre deux espaces, JeffWrite supprime aussi l'une de ces espaces, pour éviter un double espace. Si vous collez un mot directement contre un autre mot, JeffWrite ajoute une espace entre les deux. Il ne modifie pas le milieu d'un mot. Pour le désactiver, cliquez sur **Fichier > Options > Avancé**, puis décochez **Couper-coller intelligent**.

## Sauts de ligne et de page, et tabulations

Cliquez sur le petit bouton situé dans le coin du groupe **Paragraphe** pour ouvrir la boîte de dialogue **Paragraphe**. Sous les options d'espacement, on trouve :

- **Contrôle des veuves et des orphelines** empêche qu'une ligne isolée d'un paragraphe reste seule en haut ou en bas d'une page.
- **Lier au paragraphe suivant** conserve le paragraphe sur la même page que le suivant. Utilisez-le pour un titre.
- **Garder les lignes ensemble** conserve toutes les lignes du paragraphe sur une même page.
- **Masquer les numéros de ligne** exclut ce paragraphe de la numérotation des lignes lorsque les lignes de la page sont numérotées (voir Mise en page > Numéros de ligne).
- **Saut de page avant** commence le paragraphe sur une nouvelle page.
- **Ne pas diviser les mots** empêche la coupure par trait d'union, en fin de ligne, des mots du paragraphe.
- **Ne pas ajouter d'espace entre les paragraphes du même style** supprime l'espace entre deux paragraphes ayant le même style.
- **Niveau de plan** indique la place du paragraphe dans le plan du document : **Corps de texte**, ou un niveau de 1 à 9.

Cliquez sur **Tabulations** dans la boîte pour définir des taquets de tabulation. Un taquet de tabulation est un emplacement où le texte se déplace lorsque vous appuyez sur **Tab**. Saisissez sa position, choisissez l'alignement du texte à cet endroit (à gauche, centré, à droite ou décimal), puis choisissez un remplissage. Le remplissage est la série de points, de tirets ou le trait qui comble l'espace avant le taquet. Cliquez sur **Définir** pour l'ajouter. Pour en supprimer un, sélectionnez-le dans la liste et cliquez sur **Effacer**. **Tout effacer** les supprime tous.

Choisissez **Barre** pour définir une barre de tabulation. Une barre n'arrête pas le texte. Elle trace un fin trait vertical, de haut en bas, à cet emplacement sur chaque ligne du paragraphe. C'est un moyen de placer un filet entre des colonnes de texte. Une barre n'a pas de remplissage. La règle n'affiche pas les barres. Un paragraphe peut en comporter jusqu'à 64. Limitation actuelle : dans un paragraphe qui se lit de droite à gauche, les barres sont encore mesurées depuis le bord gauche. Définissez-les et effacez-les dans la boîte **Tabulations**.

**Distance des taquets par défaut** est la distance entre les taquets qu'un paragraphe reçoit lorsqu'il n'en a pas de propres, et au-delà du dernier taquet qu'il possède. La distance habituelle est de 1,27 cm (un demi-pouce). Ce réglage concerne l'ensemble du document, et non seulement le paragraphe où se trouve le curseur. La nouvelle distance s'applique à tous les paragraphes dès que vous cliquez sur **OK**.

## Bordures et trames des paragraphes

Sélectionnez un ou plusieurs paragraphes. Dans le groupe **Paragraphe**, le bouton **Bordures** propose des choix rapides : **Bordure inférieure**, **Bordure supérieure**, **Bordure gauche**, **Bordure droite**, **Toutes les bordures** et **Aucune bordure**. Un choix rapide trace un trait simple, d'une demi-point d'épaisseur, à un point du texte. Un choix pour un côté laisse les autres côtés tels qu'ils étaient.

**Bordure et trame** en bas de cette liste ouvre une boîte de dialogue. Cochez les côtés qui reçoivent un trait, ou cliquez sur **Encadré** ou **Aucun**. Cochez **Entre** pour tracer un trait entre les paragraphes sélectionnés qui ont les mêmes bordures. Choisissez ensuite le **Style** (simple, double, épais, pointillé, tireté, tirets courts, tiret-point ou tiret-point-point), la **Largeur**, la **Couleur** et la **Distance du texte**. **Trame** remplit les paragraphes d'une couleur. Le bouton **Trame** situé à côté de **Bordures** ne règle que le remplissage, avec les mêmes couleurs que **Couleur de police**.

Les traits et le remplissage forment une seule étape que **Annuler** retire, et ils sont conservés lorsque vous enregistrez un fichier .docx ou .odt. Si un paragraphe provient d'un fichier qui possède sa propre bordure ou trame, votre choix la remplace.

## Tracer une ligne horizontale

Cliquez sur **Ligne horizontale** dans le groupe **Paragraphe**. JeffWrite ajoute un paragraphe distinct, avec un fin trait sous lui, après le paragraphe où se trouve le curseur. Le curseur passe à un paragraphe ordinaire situé sous le trait. Pour supprimer le trait, sélectionnez son paragraphe et supprimez-le.

## Voir la mise en forme à l'emplacement du curseur

Appuyez sur **Maj+F1**, ou utilisez **Afficher la mise en forme**. Une boîte liste les styles, la police, la taille, l'aspect des lettres, la couleur, ainsi que l'alignement, les retraits et l'espacement du paragraphe à l'emplacement du curseur. Elle se contente d'afficher ces éléments. Pour les modifier, utilisez les boîtes **Police** et **Paragraphe**.

## Le volet Presse-papiers

Le volet Presse-papiers mémorise les 24 derniers éléments copiés ou coupés dans le programme de cette fenêtre, afin que vous puissiez coller un élément plus ancien sans le copier à nouveau. Cliquez sur la petite flèche située dans le coin du groupe **Presse-papiers** de l'onglet **Accueil** (ou appuyez sur **Alt**, puis **H**, puis **FO**) pour l'ouvrir. Un panneau apparaît à côté de la page.

Chaque élément affiche un petit aperçu et le type de contenu : texte, texte mis en forme, cellules, image, formes ou diapositives. Le plus récent se trouve en haut. Cliquez sur un élément pour le coller à l'emplacement du curseur ou de la sélection. Le collage se fait comme tout autre collage : une étape d'**Annuler** le retire, et le bouton Options de collage apparaît comme d'habitude.

- **Tout coller** colle tous les éléments, du plus ancien au plus récent, afin qu'ils se lisent dans l'ordre où vous les avez copiés.
- **Supprimer** retire l'élément sélectionné. **Tout effacer** les retire tous.
- Le volet ne collecte que lorsqu'il est ouvert. Pour continuer la collecte après l'avoir fermé, cochez **Continuer la collecte lorsque ce volet est fermé**. Ce choix est mémorisé, et la collecte commence au lancement du programme, avant que vous n'ouvriez le volet.
- Pour collecter aussi ce que vous copiez dans d'autres programmes, cochez **Collecter aussi ce qui est copié dans d'autres programmes**. Seuls le texte brut et les images sont conservés, et cette option est désactivée à chaque ouverture du programme. Avec cette option, une copie faite dans un autre programme est lue en entier avant que ce programme décide de la conserver ; une copie très volumineuse peut donc faire patienter le programme un instant et utiliser autant de mémoire que la taille de la copie. Une seule forme est lue : son texte lorsqu'il en comporte, sinon une seule image. Lorsque cette option est désactivée, rien de ce qui est copié dans d'autres programmes n'est lu.

Les éléments ne sont conservés que dans la mémoire du programme. Rien n'est enregistré dans un fichier, et la liste est vidée lorsque vous fermez la dernière fenêtre du programme. Un élément reste dans la liste après la fermeture du document dont il provient. Un élément de plus de 8 Mo n'est pas conservé, et le volet vous en informe ; une image compte 4 octets par point (pixel), donc une image de plus de 2 millions de pixels environ est trop grande. L'ensemble des éléments est limité à 64 Mo, et les plus anciens sont supprimés en premier. Tout ce qu'un gestionnaire de mots de passe marque comme secret n'est jamais conservé.
