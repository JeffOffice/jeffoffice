# Tableaux
<!-- keywords: supprimer des cellules, décaler les cellules vers la gauche, habillage du texte autour d'un tableau, habillage du texte, tableau flottant, tableau, tableaux, insérer un tableau, lignes, colonnes, cellule, cellules, fusionner les cellules, fractionner les cellules, bordures, trame, remplissage, taille de la cellule, largeur de colonne, hauteur de ligne, ajuster, distribuer les lignes, distribuer les colonnes, propriétés du tableau, ligne d'en-tête, répéter l'en-tête, convertir du texte en tableau, convertir un tableau en texte, scinder le tableau, trier, trier les lignes, formule, formules, somme, afficher le quadrillage, style de tableau, styles de tableau, lignes alternées, alignement, aligner le texte dans une cellule, supprimer une ligne, supprimer une colonne, grille, quadrillage, clic droit, disposition du tableau, création de tableau, dessiner un tableau, crayon, gomme, sens du texte, faire pivoter le texte, texte vertical -->

Un **tableau** est une grille de **cellules** disposées en **lignes** et en **colonnes**. Chaque cellule contient du texte, que vous saisissez comme n'importe où sur la page. Cette rubrique explique comment insérer un tableau, modifier sa forme et son apparence, et le préparer pour les pages sur lesquelles il s'étend.

## Insérer un tableau

- Cliquez sur **Insertion > Tableau**, puis pointez la grille et cliquez sur la taille souhaitée. Le premier nombre correspond au nombre de colonnes en largeur, et le second au nombre de lignes en hauteur.
- Ou cliquez sur **Insertion > Tableau > Insérer un tableau** pour saisir le nombre de colonnes et de lignes, et indiquer comment les colonnes sont ajustées. **Largeur de colonne fixe** conserve à chaque colonne la largeur que vous lui donnez (ou, avec *Auto*, répartit la largeur de la zone de texte entre les colonnes). **Ajuster au contenu** dimensionne les colonnes selon les mots qu'elles contiennent. **Ajuster à la fenêtre** donne au tableau la largeur de la zone de texte. Cochez **Mémoriser les dimensions pour les nouveaux tableaux** pour que le tableau suivant reprenne les mêmes valeurs.

Le tableau est placé à l'emplacement du curseur. Si le curseur se trouve au milieu d'un paragraphe, celui-ci est coupé en deux et le tableau s'insère entre les deux parties. Le curseur se place dans la première cellule. Appuyez sur **Tab** pour passer à la cellule suivante, et sur **Maj+Tab** pour revenir en arrière. **Tab** dans la dernière cellule ajoute une ligne. Il n'est pas possible d'insérer un tableau dans une cellule de cette manière.

## Tableaux prédéfinis

Cliquez sur **Insertion > Tableau > Tableaux prédéfinis**, puis choisissez-en un. Un calendrier affiche les jours du mois en cours. Une matrice comporte une ligne supérieure et une colonne de gauche à compléter. Une liste comporte quatre colonnes : article, description, quantité et prix. Un planning affiche les jours d'une semaine de travail et les heures d'une journée. Le tableau contient déjà ses mots, que vous pouvez remplacer en tapant par-dessus.

## Créer un tableau à partir de texte

Sélectionnez les paragraphes, puis cliquez sur **Insertion > Tableau > Convertir du texte en tableau**. Indiquez ce qui marque la fin d'une cellule et le début de la suivante :

- **Tabulations**, **Virgules** ou **Autre** (saisissez le caractère, par exemple un point-virgule). Chaque paragraphe devient une ligne, et le texte situé entre les marques devient les cellules de cette ligne. Le tableau comporte autant de colonnes que la ligne la plus longue, et une ligne plus courte reçoit des cellules vides à la fin.
- **Paragraphes**. Chaque paragraphe devient une cellule. Saisissez le nombre de colonnes souhaité : les cellules remplissent une ligne à la fois.

Les mots conservent leur gras, leur italique et leurs autres attributs. Une étape d'**Annuler** transforme le tableau de nouveau en paragraphes.

## Créer du texte à partir d'un tableau

Placez le curseur dans le tableau, puis cliquez sur **Disposition du tableau > Convertir en texte**. Choisissez ce qui sépare les mots des cellules d'une même ligne : une tabulation, une virgule ou un autre caractère. Vous pouvez aussi choisir **Marques de paragraphe** pour faire de chaque paragraphe de chaque cellule un paragraphe distinct. Un tableau qui contient un autre tableau dans l'une de ses cellules doit être converti de l'intérieur vers l'extérieur : convertissez d'abord le tableau intérieur.

## Les onglets de tableau

Lorsque le curseur se trouve dans un tableau, deux onglets apparaissent à droite du ruban, sous le nom **Outils de tableau** : **Création de tableau** et **Disposition du tableau**. Ils disparaissent lorsque le curseur quitte le tableau. Vous pouvez également cliquer avec le bouton droit dans une cellule pour accéder aux mêmes commandes dans un menu.

## Sélectionner des cellules

Cliquez dans une cellule, ou faites glisser le pointeur d'une cellule à une autre pour sélectionner un bloc de cellules. **Disposition du tableau > Sélectionner** sélectionne le tableau entier, la ligne, la colonne ou la cellule où se trouve le curseur. La plupart des commandes ci-dessous s'appliquent aux cellules sélectionnées, ou à la cellule où se trouve le curseur si aucune cellule n'est sélectionnée.

## Ajouter et supprimer des lignes et des colonnes

Dans **Disposition du tableau**, utilisez **Insérer au-dessus**, **Insérer en dessous**, **Insérer à gauche** et **Insérer à droite**. La nouvelle ligne ou colonne reprend autant de lignes ou de colonnes que vous en avez sélectionnées. **Supprimer** enlève les lignes, les colonnes ou le tableau entier. **Supprimer > Supprimer les cellules et décaler vers la gauche** enlève uniquement les cellules sélectionnées, avec leur contenu, et déplace les autres cellules de la ligne vers la gauche. La ligne se termine alors par un espace vide à l'emplacement de la dernière cellule. Cette commande est grisée lorsqu'une cellule est fusionnée avec des cellules situées en dessous, ou lorsqu'elle supprimerait toutes les cellules d'une ligne (utilisez alors **Supprimer les lignes** dans ce cas). **Supprimer > Supprimer les cellules et décaler vers le haut** enlève le contenu des cellules sélectionnées et remonte le contenu des cellules situées en dessous, dans les mêmes colonnes. Les dernières cellules de ces colonnes restent vides. Les commandes **Supprimer les cellules et décaler vers la gauche** et **Supprimer les cellules et décaler vers le haut** déplacent le texte sans marquer de modification : lorsque le suivi des modifications est activé, elles ne font rien, et la barre d'état vous invite à le désactiver d'abord. Les cellules conservent leur emplacement, leur largeur et leur mise en forme ; seul le texte se déplace. Cette commande est grisée lorsqu'une cellule située en dessous, dans ces colonnes, est fusionnée avec d'autres cellules, lorsqu'elle contient un tableau, ou lorsque les lignes n'ont pas leurs cellules dans les mêmes colonnes. Si vous supprimez toutes les lignes, le tableau disparaît.

## Fusionner et fractionner des cellules

- **Fusionner les cellules** réunit les cellules sélectionnées en une seule. Le contenu de chaque cellule est conservé, l'un après l'autre, dans la cellule fusionnée. Une cellule vide n'ajoute rien. Les cellules fusionnées horizontalement forment une cellule large ; celles fusionnées verticalement forment une cellule haute.
- **Fractionner les cellules** demande en combien de colonnes et de lignes diviser la cellule où se trouve le curseur. Une cellule fractionnée en plus de colonnes qu'elle n'en occupe crée de nouvelles colonnes dans la grille, et les cellules situées au-dessus et en dessous s'élargissent pour les couvrir. Une cellule fractionnée en lignes ajoute des lignes sous elle, et les cellules voisines se prolongent dans les nouvelles lignes. Une cellule fusionnée verticalement peut être fractionnée en autant de lignes qu'elle en occupe, ou en colonnes qui restent fusionnées.

- **Scinder le tableau** coupe le tableau en deux, au-dessus de la ligne où se trouve le curseur. Un paragraphe vide est inséré entre les deux tableaux, afin que vous puissiez écrire à cet endroit. Il est impossible de couper au-dessus de la première ligne, ou dans une cellule fusionnée verticalement.

Chacune de ces opérations s'annule en une seule étape.

## Trier les lignes

**Disposition du tableau > Trier** range les lignes dans l'ordre. Choisissez la colonne de tri, indiquez si ses cellules contiennent du **Texte**, un **Nombre** ou une **Date**, puis si le tri est croissant (**Croissant**, par exemple de A à Z ou du plus petit au plus grand) ou décroissant (**Décroissant**). Ajoutez jusqu'à deux autres colonnes pour départager les égalités. Si vous sélectionnez au préalable deux lignes ou plus, seules ces lignes sont triées. Cochez **La première ligne est une ligne d'en-tête** pour la conserver en haut.

Les mots d'une ligne se déplacent ensemble. Les traits et la trame propres à une cellule restent à leur place. Un tableau dont les cellules sont fusionnées verticalement, ou qui contient un tableau dans une cellule, ne peut pas être trié.

## Additionner des nombres avec une formule

Placez le curseur dans une cellule, puis cliquez sur **Disposition du tableau > Formule**. La zone commence par **SOMME(AU-DESSUS)**, qui additionne les nombres des cellules situées au-dessus, ou **SOMME(GAUCHE)** lorsque les nombres se trouvent à gauche. Cliquez sur **OK** : le résultat s'affiche dans la cellule. Il s'agit d'un champ : vous pouvez cliquer dessus puis appuyer sur **F9** pour le recalculer après avoir modifié un nombre.

Dans une formule, vous pouvez désigner des cellules :

- **A1** désigne la cellule de la colonne A, ligne 1. Les colonnes sont désignées par des lettres à partir de la gauche, et les lignes sont numérotées à partir du haut.
- **A1:B3** désigne un bloc de cellules.
- **AU-DESSUS**, **EN-DESSOUS**, **GAUCHE** et **DROITE** désignent les cellules de cette direction qui contiennent des nombres. La liste s'arrête à la première cellule vide.

Utilisez-les dans **SOMME**, **MOYENNE**, **NB**, **MIN**, **MAX**, **PRODUIT** et les autres fonctions, ou dans une somme comme **A1 + B2 * 2**. Les nombres peuvent comporter un signe euro, des espaces ou des virgules ainsi qu'un signe pourcentage, et un nombre entre parenthèses est négatif. **Format du nombre** affiche le résultat comme vous le souhaitez, par exemple **0,00** pour deux décimales, ou **#,##0.00 €** pour une somme d'argent.

## Bordures et trame

- **Création de tableau > Bordures** ajoute ou retire un trait sur les côtés des cellules sélectionnées : en bas, en haut, à gauche, à droite, toutes les bordures, les bordures extérieures, les bordures intérieures, ou aucune. Un trait placé sur le bord du bloc est également posé sur le côté de la cellule voisine, afin que les deux cellules concordent.
- **Création de tableau > Trame** remplit les cellules sélectionnées d'une couleur, ou avec **Aucune couleur**.
- **Bordure et trame** (dans le menu Bordures, ou dans le menu du clic droit) ouvre une fenêtre permettant de choisir le style du trait (simple, double, pointillé, tireté ou triple), son épaisseur et sa couleur, les côtés qui le reçoivent, ainsi que le remplissage. Choisissez de l'appliquer aux cellules sélectionnées ou au tableau.

## Tracer des bordures avec le pinceau de bordures

1. Dans **Création de tableau > Tracer les bordures**, choisissez le **Style de stylet** (simple, double, triple, pointillé ou tireté), l'**Épaisseur du stylet** et la **Couleur du stylet**.
2. Cliquez sur **Pinceau de bordures**. Le pointeur devient une croix, et le reste jusqu'à ce que vous appuyiez sur **Échap** ou que vous cliquiez de nouveau sur le bouton.
3. Faites glisser le pointeur le long des traits du tableau, ou le long de son bord extérieur. Chaque côté de cellule le long duquel votre glissement passe sur au moins la moitié de sa longueur reçoit le trait. Un glissement qui traverse le milieu d'une cellule, ou qui ne fait que croiser un trait, ne modifie rien.

Pour effacer des bordures, réglez le **Style de stylet** sur **Aucune bordure**, puis faites glisser le pointeur le long des traits de la même manière. Les cellules situées de part et d'autre d'un trait s'accordent : le trait est tracé ou effacé des deux côtés. Chaque glissement s'annule en une seule étape.

Une cellule fusionnée sur plusieurs lignes a un côté aussi haut que toutes ces lignes : un glissement le long d'au moins la moitié de ce côté trace ou efface son trait sur toute la longueur, tandis qu'un glissement plus court laisse le trait de cette cellule inchangé, ce qui peut le faire réapparaître après que vous avez effacé une partie de celui-ci.

Les traits sont conservés dans les fichiers .docx, .odt et .rtf. Une modification des bordures n'est pas marquée comme modification suivie, même lorsque le suivi des modifications est activé, comme pour le menu Bordures.

## Créer votre propre style de tableau

Ouvrez la galerie dans **Création de tableau** et cliquez sur **Nouveau style de tableau**. Donnez-lui un nom, choisissez le style dont il part, puis définissez les traits, l'aspect de la ligne d'en-tête, de la ligne de total et des première et dernière colonnes, ainsi que les remplissages des lignes et des colonnes alternées. Le nouveau style s'applique au tableau dans lequel vous vous trouvez. Pour modifier un style, cliquez dans un tableau qui le possède, puis cliquez sur **Modifier le style de tableau**. Tous les tableaux qui utilisent ce style changent avec lui. La ligne d'en-tête et les autres parties apparaissent lorsque **Options de style de tableau** les active. La modification d'un style compte pour une étape, et **Annuler** rétablit l'ancien.

## Afficher les bords des cellules

**Disposition du tableau > Afficher le quadrillage** trace de fins pointillés le long des bords des cellules, afin que vous puissiez voir un tableau sans bordures. Ces lignes ne s'affichent qu'à l'écran. Elles ne sont jamais imprimées ni enregistrées dans le fichier.

## Styles de tableau

**Création de tableau > Styles de tableau** affiche des aperçus de mises en forme prédéfinies : tableaux simples, quadrillés et en liste, certains en couleurs. Cliquez sur l'un d'eux pour appliquer cet aspect au tableau. Le style définit les traits, les remplissages et le texte en gras pour les différentes parties du tableau. **Effacer** rétablit un tableau sans style.

Les options à gauche de la galerie indiquent les parties auxquelles le style s'applique : **Ligne d'en-tête** et **Ligne de total** (première et dernière lignes), **Première colonne** et **Dernière colonne**, ainsi que **Lignes alternées** et **Colonnes alternées** (chaque ligne ou colonne sur deux reçoit le remplissage du style). Les traits et la trame que vous avez appliqués vous-même aux cellules restent tels que vous les avez faits.

## Taille

Dans **Disposition du tableau** :

- Saisissez une valeur dans **Hauteur** pour régler la hauteur des lignes où se trouve le curseur, et dans **Largeur** pour régler la largeur de sa colonne. Une hauteur est une hauteur minimale : la ligne s'agrandit avec son contenu. Pour une hauteur fixe, utilisez les propriétés du tableau.
- **Ajuster** dimensionne les colonnes selon leur contenu, selon la fenêtre, ou les fixe à leur emplacement actuel.
- **Distribuer les lignes** et **Distribuer les colonnes** rendent les lignes sélectionnées de même hauteur, ou les colonnes sélectionnées de même largeur. Si une seule ligne ou colonne est sélectionnée, la commande s'applique à tout le tableau.

### Déplacer les traits à la souris

- Pointez le trait situé à droite d'une colonne. Le pointeur devient deux flèches. Appuyez, tirez latéralement, puis relâchez. La colonne prend cette largeur. Une ligne en pointillés indique où se placera le bord.
- Pointez le trait situé sous une ligne, puis tirez vers le haut ou vers le bas. La ligne prend au moins cette hauteur.
- Cliquez dans un tableau. Un petit carré apparaît dans son coin inférieur droit. Faites-le glisser latéralement pour élargir ou rétrécir l'ensemble du tableau. Chaque colonne grandit ou rétrécit dans la même proportion.

Chaque déplacement compte pour une étape. **Annuler** le défait.

## Déplacer un tableau

- Cliquez dans un tableau. Un petit carré surmonté d'une croix apparaît dans son coin supérieur gauche. Cliquez dessus pour sélectionner l'ensemble du tableau. Faites-le glisser vers un autre emplacement : une ligne indique où le tableau sera placé. Relâchez : le tableau se place juste avant le paragraphe situé à cet endroit. Si vous relâchez à l'intérieur d'un tableau, ou si vous appuyez sur **Échap**, rien ne bouge.
- Ou utilisez **Disposition du tableau > Déplacer le tableau vers le haut** et **Déplacer le tableau vers le bas**, ou **Déplacer le tableau** dans le menu du clic droit. Le tableau passe alors avant ou après le paragraphe voisin.

Un déplacement compte pour une étape. Le texte, la trame et les bordures des cellules suivent le tableau. Un déplacement n'est pas marqué comme modification suivie.

Un tableau ne peut pas être placé directement contre un autre tableau : gardez un paragraphe entre les deux. Les commandes de déplacement sont grisées lorsque le tableau ne peut pas aller plus loin : au-dessus du premier paragraphe, sous le dernier, ou au-delà d'un paragraphe qui laisserait deux tableaux côte à côte. Un tableau placé dans une cellule d'un autre tableau ne peut pas être déplacé de cette manière. La poignée n'apparaît pas dans un document en lecture seule ou protégé. Chaque paragraphe qui entoure le tableau conserve sa propre mise en forme.

## Copier et coller un tableau

Sélectionnez un tableau entier, avec le paragraphe qui le précède ou le suit, ou de sa première à sa dernière cellule, puis copiez-le. Le collage insère un tableau : lignes, colonnes, largeurs, bordures, trame, cellules fusionnées, ligne d'en-tête, tableaux imbriqués dans des cellules et style de tableau sont conservés. Vous pouvez coller dans le même document ou dans un autre. Si l'autre document ne possède pas de style de tableau portant ce nom, JeffWrite l'ajoute. S'il en possède un de même nom mais d'aspect différent, son style et ses tableaux restent inchangés, et le tableau collé reçoit le style qui l'accompagnait, ajouté sous le même nom suivi d'un numéro (« Quadrillage 2 »). Coller de nouveau le même tableau réutilise ce style. Un collage compte pour une étape : **Annuler** retire donc le tableau entier.

- Lorsque le **suivi des modifications** est activé, un tableau collé est une insertion suivie : ses lignes et ses mots sont marqués comme insérés. **Tout refuser** supprime donc le tableau entier, et **Tout accepter** le conserve.
- Si vous ne sélectionnez qu'une partie d'un tableau, seuls ses mots sont copiés, un paragraphe par cellule.
- Un tableau contenant des modifications suivies est copié sous forme de mots, de même qu'un tableau situé dans une sélection qui ne contient qu'une partie d'un contrôle de contenu, ou lorsque la sélection comporte plus de 100 tableaux ou 20 000 cellules. Dans chaque cas, JeffWrite vous en informe après la copie.
- Collé dans une cellule, un en-tête, un pied de page ou une note, un tableau n'est inséré que sous forme de mots.
- Les tableaux contenus dans un texte inséré depuis un fichier avec **Insertion > Texte à partir d'un fichier**, ainsi que dans du RTF collé depuis un autre programme, sont conservés comme tableaux, selon les mêmes règles et les mêmes limites.

## Dessiner un tableau

Cliquez sur **Insertion > Tableau > Dessiner un tableau**, ou sur **Disposition du tableau > Dessiner un tableau**. Le pointeur devient une croix, et le reste jusqu'à ce que vous appuyiez sur **Échap** ou que vous cliquiez de nouveau sur le bouton.

- Faites glisser un rectangle dans le texte : un tableau d'une seule cellule, de la taille du rectangle, apparaît.
- Faites glisser une ligne à travers une cellule : la cellule est divisée. Une ligne tracée surtout horizontalement crée deux lignes. Une ligne tracée surtout verticalement crée deux colonnes. Les deux parties sont toujours égales, quel que soit l'endroit où vous avez tracé la ligne.
- Cliquez sur **Disposition du tableau > Gomme**, puis faites glisser le pointeur d'une cellule à la suivante. Les deux cellules se réunissent, et les mots des deux sont conservés.

Chaque glissement compte pour une étape d'**Annuler**.

## Position du texte dans une cellule

Les neuf boutons de **Disposition du tableau > Alignement** placent le texte en haut, au milieu ou en bas de la cellule, et à gauche, au centre ou à droite.

## Orienter le texte d'une cellule

Cliquez sur **Disposition du tableau > Sens du texte**. **Faire pivoter le texte vers le bas** oriente le texte de haut en bas de la cellule, et **Faire pivoter le texte vers le haut** l'oriente de bas en haut. **Texte horizontal** rétablit l'orientation normale. La ligne s'agrandit jusqu'à la longueur des mots, dans la limite d'environ 7 cm, afin qu'une courte étiquette comme « Total » tienne sur une seule ligne. Cliquez dans une cellule orientée et saisissez comme dans n'importe quelle autre. Le même choix figure dans **Propriétés du tableau**, sous l'onglet **Cellule**.

## Propriétés du tableau

Cliquez sur **Propriétés du tableau** dans **Disposition du tableau**, ou dans le menu du clic droit. La boîte de dialogue comporte un onglet par partie :

- **Tableau** : position du tableau (à gauche, au centre ou à droite), distance par rapport à la marge de gauche, largeur préférée (automatique, en points ou en pourcentage de la zone de texte), indication si les colonnes sont fixes, **Habiller le texte autour du tableau** (le tableau reste à sa place et les mots des paragraphes suivants s'écoulent à côté de lui, au lieu de passer en dessous), l'espace intérieur de chaque cellule et l'espace entre les cellules.
- **Ligne** : la hauteur et l'indication qu'elle est minimale ou exacte, **Ne pas laisser la ligne se couper entre deux pages**, et **Répéter comme ligne d'en-tête en haut de chaque page**. Une ligne d'en-tête réapparaît en haut de chaque page sur laquelle s'étend un tableau long. Le bouton **Disposition du tableau > Répéter les lignes d'en-tête** a le même effet pour les lignes sélectionnées.
- **Colonne** : la largeur de la colonne, en points ou en pourcentage du tableau. Les deux zones se répercutent l'une sur l'autre.
- **Cellule** : position verticale et horizontale des mots, sens d'écriture, et indication si les mots peuvent passer à la ligne.
- **Texte de remplacement** : un titre et une description du tableau destinés aux personnes qui ne peuvent pas le voir.

## Ce que JeffWrite conserve

Un tableau que vous créez ou modifiez est enregistré dans le fichier avec ses largeurs, ses traits, ses remplissages, ses fusions et ses options, et il s'ouvre de la même façon dans JeffWrite et dans les autres traitements de texte. Les parties d'un tableau que le fichier contenait et que JeffWrite n'affiche pas sont conservées telles quelles.

Dans un fichier `.odt`, un tableau conserve ses traits, ses remplissages et ses largeurs, et les remplissages que son style donne à la ligne d'en-tête et aux lignes alternées sont écrits directement sur les cellules. Une formule est conservée, mais pas son format de nombre, et un style n'est pas conservé en tant que style : le tableau a le même aspect à la réouverture, mais il ne porte plus de nom de style.

[Retour au sommaire](index)
