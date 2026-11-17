# Modifier la casse, trier et coller autrement

<!-- keywords: bouton coller, couper-coller intelligent, espaces au collage, ligne horizontale, filet, révéler la mise en forme, volet Styles, maj+F1, tabulations, taquets de tabulation, taquet barre, tabulation de barre, taquets par défaut, distance de tabulation, points de suite, conserver avec le suivant, lignes solidaires, saut de page avant, veuve, orpheline, niveau du plan, sauts de ligne et de page, changer la casse, majuscules, minuscules, casse de phrase, majuscule initiale, inverser la casse, maj+F3, trier, trier les paragraphes, ordre alphabétique, coller, collage spécial, continuer la liste, nouvelle liste, fusionner la liste, imbriquer le tableau, cellules de tableur, conserver uniquement le texte, fusionner la mise en forme, rtf, texte enrichi, texte mis en forme, glisser, texte sans mise en forme, image, presse-papiers -->

Ces outils se trouvent sous l'onglet **Accueil**. Ils agissent sur le texte sélectionné.

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

Appuyez sur **Maj+F3** pour parcourir trois de ces options dans l'ordre : **Majuscule à chaque mot**, puis **MAJUSCULES**, puis **minuscules**.

La mise en forme des lettres ne change pas. Le gras reste du gras. **Ctrl+Z** annule la modification en une seule étape. Si le suivi des modifications est activé, la modification apparaît comme une suppression et une insertion.

## Trier des paragraphes

Le tri met des paragraphes en ordre, par exemple de A à Z. Utilisez-le pour une liste de noms, ou pour des lignes de tableau créées avec des tabulations ou des virgules.

1. Sélectionnez deux paragraphes ou plus.
2. Dans le groupe **Paragraphe**, cliquez sur **Trier**.
3. Choisissez le critère de tri. Chaque choix comporte un **champ**, un **type** et un **ordre**.
   - Le **champ** est une partie du paragraphe. Le champ 1 est la première partie. Indiquez si les parties sont séparées par des virgules ou des tabulations, dans la zone située sous les clés. Si vous laissez **Paragraphes**, le paragraphe entier constitue le seul champ.
   - Le **type** est **Texte**, **Nombre** ou **Date**. Un nombre se trie d'après sa valeur, donc 9 vient avant 10.
   - L'**ordre** est **Croissant** (de A à Z, du plus petit au plus grand, du plus ancien au plus récent) ou **Décroissant**.
4. Utilisez **Puis par** pour départager les ex æquo.
5. Cochez **La première ligne est un en-tête** pour la laisser à sa place.
6. Cliquez sur **OK**.

Chaque paragraphe conserve sa propre mise en forme. Le tri ne peut pas être utilisé pendant que le suivi des modifications est activé, car un tri n'est pas une modification à afficher. Désactivez le suivi des modifications, triez, puis réactivez-le.

## Coller autrement

Cliquez sur la petite flèche sous **Coller** pour afficher d'autres choix :

- Lorsque le texte provient d'un autre programme qui place du texte enrichi (RTF) dans le presse-papiers, **Ctrl+V** conserve sa mise en forme : polices, tailles, couleurs, gras, listes, tableaux, liens et images. Si le programme n'a fourni aucun texte enrichi, JeffWrite utilise sa forme de page web (HTML), puis le texte brut. Le texte enrichi de plus de 16 Mo n'est pas lu, et le collage utilise la forme suivante. Faire glisser un texte mis en forme depuis un autre programme vers la page produit le même résultat.
- **Fusionner la mise en forme** colle le texte avec la mise en forme de l'endroit où il est placé. Seuls le gras, l'italique, le soulignement et le barré sont conservés.
- **Conserver uniquement le texte** colle les mots sans aucune mise en forme.
- **Image** colle une image du presse-papiers. La commande est grisée lorsque le presse-papiers ne contient pas d'image.
- **Collage spécial** (**Ctrl+Alt+V**) affiche toutes les façons de coller le contenu actuel du presse-papiers, et vous permet d'en choisir une, par exemple **Texte mis en forme (RTF)**, **Texte sans mise en forme** ou **HTML**.

Un choix est grisé lorsque le presse-papiers ne contient rien qu'il puisse utiliser.

Juste après un collage, un petit bouton **Options de collage** apparaît à côté du curseur. Cliquez dessus pour choisir **Conserver la mise en forme source**, **Fusionner la mise en forme** ou **Conserver uniquement le texte**. JeffWrite annule le collage et le refait de cette manière. Le bouton disparaît dès que vous tapez ou déplacez le curseur.

Lorsque le collage apporte des paragraphes de liste à côté d'une liste, le bouton propose aussi **Continuer la liste** (la numérotation reprend celle de la liste voisine), **Nouvelle liste** (la numérotation recommence à 1) et **Fusionner la liste** (les éléments rejoignent la liste voisine et prennent sa mise en forme). Le **Ctrl+V** simple insère les éléments tels quels, et un choix modifie ce comportement. Les mêmes choix figurent comme commandes dans le menu Coller de l'onglet Accueil et dans **Collage spécial**, de sorte que le clavier y accède également.

Lorsque le presse-papiers contient des cellules de tableur (copiées depuis JeffCrunch ou un autre programme, sous forme de tableau ou de lignes avec des tabulations entre les cellules), le **Ctrl+V** simple les colle sous forme de texte. Pour les coller en tant que cellules, choisissez **Tableau** dans le bouton, dans le menu Coller ou dans **Collage spécial**. Lorsque le curseur se trouve dans un tableau, le bouton propose **Imbriquer le tableau** (un tableau dans la cellule), **Fusionner le tableau** (les cellules rejoignent le tableau, qui reçoit les colonnes nécessaires), **Insérer comme nouvelles lignes** et **Remplacer les cellules** (les cellules remplacent celles du tableau, à partir de la cellule du curseur). Un tableau collé comporte au maximum 500 lignes et 63 colonnes, et une cellule contient au maximum 4 000 caractères ; ce qui dépasse est omis, et un message l'indique. Un tableau imbriqué dans un tableau peut comporter au maximum 8 niveaux, et un tableau imbriqué dont les colonnes seraient trop étroites pour la cellule est refusé. Chaque collage constitue une seule étape : **Annuler** les annule tous, et **Rétablir** les restaure.

Le **couper-coller intelligent** conserve des espaces corrects lorsque vous déplacez des mots. Si vous coupez des mots entiers situés entre deux espaces, JeffWrite supprime également l'un de ces espaces, pour éviter un double espace. Si vous collez un mot directement contre un autre mot, JeffWrite ajoute un espace entre eux. Il ne modifie pas le milieu d'un mot. Pour désactiver cette fonction, cliquez sur **Fichier > Options > Avancé** et décochez **Couper-coller intelligent**.

## Sauts de ligne et de page, et tabulations

Cliquez sur le petit bouton situé dans le coin du groupe **Paragraphe** pour ouvrir la boîte de dialogue **Paragraphe**. Sous les options d'espacement, on trouve :

- **Contrôle des veuves et des orphelines** empêche qu'une ligne isolée d'un paragraphe reste seule en haut ou en bas d'une page.
- **Paragraphes solidaires** conserve le paragraphe sur la même page que le suivant. Utilisez-le pour un titre.
- **Lignes solidaires** conserve toutes les lignes du paragraphe sur une même page.
- **Masquer les numéros de ligne** exclut ce paragraphe de la numérotation des lignes (voir Mise en page > Numéros de ligne).
- **Saut de page avant** place le paragraphe en haut d'une nouvelle page.
- **Ne pas diviser les mots** empêche les mots du paragraphe d'être coupés par un trait d'union en fin de ligne.
- **Ne pas ajouter d'espace entre les paragraphes de même style** supprime l'espace entre deux paragraphes ayant le même style.
- **Niveau du plan** indique la position du paragraphe dans le plan du document : **Corps de texte**, ou un niveau de 1 à 9.

Cliquez sur **Tabulations** dans la boîte de dialogue pour définir des taquets de tabulation. Un taquet de tabulation est une position vers laquelle le texte se déplace lorsque vous appuyez sur **Tab**. Saisissez sa position, choisissez l'alignement du texte à cet endroit (à gauche, centré, à droite ou décimal), puis choisissez des points de suite. Un point de suite est la rangée de points, de tirets ou la ligne qui remplit l'espace situé avant le taquet. Cliquez sur **Définir** pour l'ajouter. Pour en supprimer un, sélectionnez-le dans la liste et cliquez sur **Effacer**. **Tout effacer** les supprime tous.

Choisissez **Barre** pour définir une tabulation de barre. Une tabulation de barre ne stoppe pas le texte. Elle trace un trait fin vertical à cet endroit, sur chaque ligne du paragraphe. C'est un moyen de placer un filet entre des colonnes de texte. Une tabulation de barre n'a pas de point de suite. La règle n'affiche pas les tabulations de barre. Un paragraphe en conserve jusqu'à 64. Point non encore pris en charge : dans un paragraphe lu de droite à gauche, les tabulations de barre sont toujours mesurées depuis le bord gauche. Définissez-les et effacez-les dans la boîte de dialogue **Tabulations**.

**Taquets par défaut** correspond à la distance entre les taquets qu'un paragraphe reçoit lorsqu'il n'en possède aucun, et au-delà du dernier taquet qui lui appartient. La distance habituelle est de 1,25 cm. Ce réglage s'applique à l'ensemble du document, et pas seulement au paragraphe où vous vous trouvez. La nouvelle distance s'applique à tous les paragraphes dès que vous cliquez sur **OK**.

## Tracer une ligne horizontale

Cliquez sur **Ligne horizontale** dans le groupe **Paragraphe**. JeffWrite ajoute un paragraphe distinct, avec un trait fin sous celui-ci, après le paragraphe où se trouve le curseur. Le curseur passe à un paragraphe ordinaire situé sous la ligne. Pour supprimer la ligne, sélectionnez son paragraphe et supprimez-le.

## Voir la mise en forme à l'emplacement du curseur

Appuyez sur **Maj+F1**, ou utilisez **Révéler la mise en forme**. Une boîte affiche les styles, la police, la taille, l'apparence des lettres, la couleur, ainsi que l'alignement, les retraits et l'espacement du paragraphe à l'emplacement du curseur. Elle se contente de les afficher. Pour les modifier, utilisez les boîtes de dialogue **Police** et **Paragraphe**.
