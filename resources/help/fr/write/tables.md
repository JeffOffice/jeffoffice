# Tableaux
<!-- keywords: supprimer des cellules, décaler les cellules vers la gauche, habillage du texte autour d'un tableau, renvoi à la ligne, habillage, tableau flottant, tableau, tableaux, insérer un tableau, lignes, colonnes, cellule, cellules, fusionner les cellules, fractionner les cellules, bordures, trame, remplissage, taille de la cellule, largeur de colonne, hauteur de ligne, ajuster, répartir les lignes, répartir les colonnes, propriétés du tableau, ligne d'en-tête, répéter l'en-tête, convertir du texte en tableau, convertir un tableau en texte, scinder le tableau, trier, trier les lignes, formule, formules, somme, afficher le quadrillage, style de tableau, styles de tableau, lignes à bandes, alignement, aligner le texte dans une cellule, supprimer une ligne, supprimer une colonne, quadrillage, quadrillage, clic droit, disposition du tableau, création de tableau, dessiner un tableau, crayon, gomme, sens du texte, faire pivoter le texte, texte vertical -->

Un **tableau** est une grille de **cellules** organisées en **lignes** et en **colonnes**. Chaque cellule contient du texte, que vous saisissez comme n'importe où dans la page. Cette rubrique explique comment ajouter un tableau, modifier sa forme et son apparence, et le préparer pour les pages qu'il traverse.

## Ajouter un tableau

- Cliquez sur **Insertion > Tableau**, puis pointez la grille et cliquez sur la taille souhaitée. Le premier nombre indique le nombre de colonnes en largeur, et le second le nombre de lignes en hauteur.
- Ou cliquez sur **Insertion > Tableau > Insérer un tableau** pour saisir le nombre de colonnes et de lignes, et choisir la manière dont les colonnes sont ajustées. **Largeur de colonne fixe** conserve à chaque colonne la largeur que vous lui donnez (ou, avec *Auto*, répartit la largeur de la zone de texte entre les colonnes). **Ajuster au contenu** dimensionne les colonnes selon les mots qu'elles contiennent. **Ajuster à la fenêtre** donne au tableau la largeur de la zone de texte. Cochez **Mémoriser les dimensions pour les nouveaux tableaux** pour que le tableau suivant reprenne les mêmes nombres.

Le tableau est inséré à l'emplacement du curseur. Si le curseur se trouve au milieu d'un paragraphe, celui-ci est coupé en deux et le tableau est placé entre les deux parties. Le curseur se place dans la première cellule. Appuyez sur **Tab** pour passer à la cellule suivante, et sur **Maj+Tab** pour revenir en arrière. **Tab** dans la dernière cellule ajoute une ligne. Il n'est pas possible d'insérer un tableau dans une cellule de cette manière.

## Tableaux prédéfinis

Cliquez sur **Insertion > Tableau > Tableaux prédéfinis** et choisissez-en un. Un calendrier affiche les jours du mois en cours. Une matrice comporte une ligne d'en-tête et une colonne de gauche à compléter. Une liste comporte quatre colonnes : article, description, quantité et prix. Un planning affiche les jours d'une semaine de travail et les heures d'une journée. Le tableau contient déjà ses libellés, que vous pouvez remplacer par votre saisie.

## Créer un tableau à partir de texte

Sélectionnez les paragraphes, puis cliquez sur **Insertion > Tableau > Convertir du texte en tableau**. Indiquez ce qui marque la fin d'une cellule et le début de la suivante :

- **Tabulations**, **Virgules** ou **Autre** (saisissez le caractère, par exemple un point-virgule). Chaque paragraphe devient une ligne, et le texte situé entre les marques devient les cellules de cette ligne. Le tableau comporte autant de colonnes que la ligne la plus longue, et une ligne plus courte reçoit des cellules vides à sa fin.
- **Paragraphes**. Chaque paragraphe devient une cellule. Saisissez le nombre de colonnes souhaité : les cellules remplissent une ligne après l'autre.

Les mots conservent leur gras, leur italique et les autres attributs. Une seule action **Annuler** transforme le tableau de nouveau en paragraphes.

## Créer du texte à partir d'un tableau

Placez le curseur dans le tableau et cliquez sur **Disposition du tableau > Convertir en texte**. Choisissez ce qui sépare les mots des cellules d'une même ligne : une tabulation, une virgule ou un autre caractère. Ou choisissez **Marques de paragraphe** pour que chaque paragraphe de chaque cellule devienne un paragraphe indépendant. Un tableau contenant un autre tableau dans l'une de ses cellules doit être converti de l'intérieur vers l'extérieur : convertissez d'abord le tableau intérieur.

## Les onglets du tableau

Lorsque le curseur se trouve dans un tableau, deux onglets apparaissent à droite du ruban, sous le nom **Outils de tableau** : **Création de tableau** et **Disposition du tableau**. Ils disparaissent lorsque le curseur quitte le tableau. Vous pouvez également cliquer avec le bouton droit dans une cellule pour accéder aux mêmes commandes dans un menu.

## Sélectionner des cellules

Cliquez dans une cellule, ou faites glisser le pointeur d'une cellule à une autre pour sélectionner un bloc de cellules. **Disposition du tableau > Sélectionner** sélectionne le tableau entier, la ligne, la colonne ou la cellule où se trouve le curseur. La plupart des commandes ci-dessous s'appliquent aux cellules sélectionnées, ou à la cellule où se trouve le curseur si aucune n'est sélectionnée.

## Ajouter et supprimer des lignes et des colonnes

Dans **Disposition du tableau**, utilisez **Insérer au-dessus**, **Insérer en dessous**, **Insérer à gauche** et **Insérer à droite**. La nouvelle ligne ou colonne prend autant de lignes ou de colonnes que vous en avez sélectionné. **Supprimer** retire les lignes, les colonnes ou le tableau entier. **Supprimer > Supprimer les cellules et décaler vers la gauche** retire uniquement les cellules sélectionnées, avec leur contenu, et décale les autres cellules de la ligne vers la gauche. La ligne se termine alors par un espace vide à l'emplacement de la dernière cellule. Cette commande est grisée lorsqu'une cellule est fusionnée avec des cellules situées en dessous, ou lorsqu'elle supprimerait toutes les cellules d'une ligne (utilisez **Supprimer les lignes** dans ce cas). Si vous supprimez toutes les lignes, le tableau disparaît.

## Fusionner et fractionner des cellules

- **Fusionner les cellules** réunit les cellules sélectionnées en une seule. Le contenu de chaque cellule est conservé, les uns à la suite des autres, dans la cellule fusionnée. Une cellule vide n'ajoute rien. Les cellules fusionnées horizontalement forment une cellule large ; les cellules fusionnées verticalement forment une cellule haute.
- **Fractionner les cellules** demande en combien de colonnes et de lignes diviser la cellule où se trouve le curseur. Une cellule fractionnée en davantage de colonnes qu'elle n'en occupait crée de nouvelles colonnes dans la grille, et les cellules situées au-dessus et en dessous s'élargissent pour les couvrir. Une cellule fractionnée en lignes ajoute des lignes sous elle, et les cellules voisines se prolongent dans les nouvelles lignes. Une cellule fusionnée verticalement peut être fractionnée en autant de lignes qu'elle en couvre, ou en colonnes qui restent fusionnées.

- **Scinder le tableau** coupe le tableau en deux, au-dessus de la ligne où se trouve le curseur. Un paragraphe vide est inséré entre les deux tableaux afin que vous puissiez y écrire. Il n'est pas possible de scinder au-dessus de la première ligne, ni à travers une cellule fusionnée verticalement.

Chacune de ces opérations s'annule en une seule action.

## Trier les lignes

**Disposition du tableau > Trier** met les lignes en ordre. Choisissez la colonne de tri, indiquez si ses cellules contiennent du **Texte**, des **Nombres** ou des **Dates**, et si le tri doit être **Croissant** (de A à Z, ou du plus petit au plus grand) ou **Décroissant**. Ajoutez jusqu'à deux colonnes supplémentaires pour départager les valeurs identiques. Si vous sélectionnez au préalable deux lignes ou plus, seules ces lignes sont triées. Cochez **La première ligne est une ligne d'en-tête** pour la conserver en haut.

Les mots d'une ligne se déplacent ensemble. Les traits et les trames propres à une cellule restent à leur place. Un tableau dont des cellules sont fusionnées verticalement, ou qui contient un tableau dans une cellule, ne peut pas être trié.

## Additionner des nombres avec une formule

Placez le curseur dans une cellule, puis cliquez sur **Disposition du tableau > Formule**. La zone affiche d'abord **SOMME(AU-DESSUS)**, qui additionne les nombres des cellules situées au-dessus, ou **SOMME(GAUCHE)** lorsque les nombres se trouvent à gauche. Appuyez sur **OK** : le résultat s'affiche dans la cellule. Il s'agit d'un champ : vous pouvez cliquer dessus puis appuyer sur **F9** pour le recalculer après avoir modifié un nombre.

Dans une formule, vous pouvez désigner des cellules :

- **A1** désigne la cellule de la colonne A, ligne 1. Les colonnes sont désignées par des lettres à partir de la gauche, et les lignes sont numérotées à partir du haut.
- **A1:B3** désigne un bloc de cellules.
- **AU-DESSUS**, **EN-DESSOUS**, **GAUCHE** et **DROITE** désignent les cellules de cette direction qui contiennent des nombres. La liste s'arrête à la première cellule vide.

Utilisez-les avec **SOMME**, **MOYENNE**, **NB**, **MIN**, **MAX**, **PRODUIT** et les autres fonctions, ou dans un calcul tel que **A1 + B2 * 2**. Les nombres peuvent comporter un signe euro, des virgules ou un signe pourcentage, et un nombre entre parenthèses est négatif. **Format du nombre** affiche le résultat comme vous le souhaitez, par exemple **0,00** pour deux décimales, ou **#.##0,00 €** pour une somme d'argent.

## Bordures et trames

- **Création de tableau > Bordures** ajoute ou supprime un trait sur les côtés des cellules sélectionnées : en bas, en haut, à gauche ou à droite, tous les traits, les traits extérieurs, les traits intérieurs, ou aucun. Un trait placé sur le bord du bloc est également posé sur le côté de la cellule voisine, afin que les deux cellules concordent.
- **Création de tableau > Trame** remplit les cellules sélectionnées d'une couleur, ou applique **Aucune couleur**.
- **Bordure et trame** (dans le menu Bordures, ou dans le menu du clic droit) ouvre une fenêtre pour choisir le style du trait (simple, double, pointillé, tireté ou triple), son épaisseur et sa couleur, les côtés concernés, et le remplissage. Choisissez de l'appliquer aux cellules sélectionnées ou au tableau.

## Tracer des bordures avec le traceur de bordures

1. Dans **Création de tableau > Traçage des bordures**, choisissez **Style de stylo** (simple, double, triple, pointillé ou tireté), **Épaisseur du stylo** et **Couleur du stylo**.
2. Cliquez sur **Traceur de bordures**. Le pointeur devient une croix, et le reste jusqu'à ce que vous appuyiez sur **Échap** ou que vous cliquiez de nouveau sur le bouton.
3. Faites glisser le pointeur le long des traits du tableau, ou le long de son bord extérieur. Chaque côté de cellule que votre glissement longe sur au moins la moitié de sa longueur reçoit le trait. Un glissement qui traverse le milieu d'une cellule, ou qui ne fait que croiser un trait, ne modifie rien.

Pour effacer des bordures, réglez **Style de stylo** sur **Aucune bordure**, puis faites glisser le pointeur le long des traits de la même manière. Les cellules situées de part et d'autre d'un trait se correspondent : le trait est tracé ou effacé des deux côtés. Chaque glissement s'annule en une seule action.

Une cellule fusionnée sur plusieurs lignes a un côté aussi haut que toutes ces lignes : un glissement le long d'au moins la moitié de ce côté trace ou efface son trait sur toute la longueur, tandis qu'un glissement plus court laisse le trait de cette cellule inchangé, ce qui peut le faire apparaître après que vous avez effacé une partie de celui-ci.

Les traits sont conservés dans les fichiers .docx, .odt et .rtf. Une modification des bordures n'est pas signalée comme une modification suivie, même lorsque le suivi des modifications est activé, comme pour le menu Bordures.

## Créer votre propre style de tableau

Ouvrez la galerie dans **Création de tableau** et cliquez sur **Nouveau style de tableau**. Donnez-lui un nom, choisissez le style dont il part, puis définissez les traits, l'aspect de la ligne d'en-tête, de la ligne de total, de la première et de la dernière colonne, ainsi que les remplissages des lignes et des colonnes à bandes. Le nouveau style est appliqué au tableau où vous vous trouvez. Pour modifier un style, cliquez dans un tableau qui l'utilise, puis cliquez sur **Modifier le style de tableau**. Tous les tableaux qui utilisent ce style sont alors modifiés. La ligne d'en-tête et les autres parties s'affichent lorsque **Options de style de tableau** les active. La modification d'un style s'effectue en une étape, et **Annuler** rétablit l'ancien style.

## Afficher les bords des cellules

**Disposition du tableau > Afficher le quadrillage** trace de fines lignes pointillées le long des bords des cellules, afin de voir un tableau dépourvu de bordures. Ces lignes ne s'affichent qu'à l'écran. Elles ne sont jamais imprimées ni enregistrées dans le fichier.

## Styles de tableau

**Création de tableau > Styles de tableau** présente des aperçus de présentations prédéfinies : tableaux simples, quadrillés et en liste, certains en couleurs. Cliquez sur l'un d'eux pour appliquer cette présentation au tableau. Le style définit les traits, les remplissages et le gras des différentes parties du tableau. **Effacer** rétablit un tableau sans style.

Les options à gauche de la galerie indiquent les parties auxquelles le style s'applique : **Ligne d'en-tête** et **Ligne de total** (première et dernière lignes), **Première colonne** et **Dernière colonne**, et **Lignes à bandes** et **Colonnes à bandes** (une ligne ou une colonne sur deux reçoit le remplissage du style). Les traits et les trames que vous avez appliqués vous-même aux cellules restent tels que vous les avez faits.

## Dimensions

Dans **Disposition du tableau** :

- Saisissez dans **Hauteur** la hauteur des lignes où se trouve le curseur, et dans **Largeur** la largeur de sa colonne. Une hauteur est une hauteur minimale : la ligne s'agrandit avec son contenu. Pour une hauteur fixe, utilisez Propriétés du tableau.
- **Ajuster** dimensionne les colonnes selon leur contenu, selon la fenêtre, ou les fixe à leurs dimensions actuelles.
- **Répartir les lignes** et **Répartir les colonnes** rendent les lignes sélectionnées aussi hautes, ou les colonnes sélectionnées aussi larges, les unes que les autres. Si une seule ligne ou colonne est sélectionnée, la commande s'applique à tout le tableau.

### Déplacer les traits à la souris

- Pointez le trait situé à droite d'une colonne. Le pointeur devient une double flèche. Maintenez le bouton enfoncé, tirez latéralement, puis relâchez. La colonne prend cette largeur. Un trait en pointillés indique où se placera le bord.
- Pointez le trait situé sous une ligne, puis tirez vers le haut ou vers le bas. La ligne prend au moins cette hauteur.
- Cliquez dans un tableau. Un petit carré apparaît dans son coin inférieur droit. Faites-le glisser latéralement pour élargir ou réduire l'ensemble du tableau. Chaque colonne s'agrandit ou se réduit dans la même proportion.

Chaque déplacement correspond à une étape. **Annuler** le supprime.

## Déplacer un tableau

- Cliquez dans un tableau. Un petit carré portant une croix apparaît dans son coin supérieur gauche. Cliquez dessus pour sélectionner le tableau entier. Faites-le glisser vers un autre emplacement : une ligne indique où le tableau sera placé. Relâchez, et le tableau se place juste avant le paragraphe situé à cet endroit. Si vous relâchez à l'intérieur d'un tableau, ou si vous appuyez sur **Échap**, rien ne bouge.
- Ou utilisez **Disposition du tableau > Déplacer le tableau vers le haut** et **Déplacer le tableau vers le bas**, ou **Déplacer le tableau** dans le menu du clic droit. Le tableau passe au-delà du paragraphe qui le précède ou qui le suit.

Un déplacement correspond à une étape. Les mots, les trames et les bordures des cellules suivent le tableau. Un déplacement n'est pas signalé comme une modification suivie.

Un tableau ne peut pas être placé directement contre un autre tableau : laissez donc un paragraphe entre eux. Les commandes de déplacement sont grisées lorsque le tableau ne peut pas aller plus loin : au-dessus du premier paragraphe, en dessous du dernier, ou au-delà d'un paragraphe qui laisserait deux tableaux côte à côte. Un tableau placé dans une cellule d'un autre tableau ne peut pas être déplacé de cette manière. La poignée n'apparaît pas dans un document en lecture seule ou protégé. Chaque paragraphe qui entoure le tableau conserve sa propre mise en forme.

## Dessiner un tableau

Cliquez sur **Insertion > Tableau > Dessiner un tableau**, ou sur **Disposition du tableau > Dessiner un tableau**. Le pointeur devient une croix, et le reste jusqu'à ce que vous appuyiez sur **Échap** ou que vous cliquiez de nouveau sur le bouton.

- Faites glisser un rectangle dans le texte : un tableau d'une seule cellule, de la taille du rectangle, apparaît.
- Faites glisser une ligne à travers une cellule : la cellule est divisée. Une ligne tracée surtout horizontalement crée deux lignes. Une ligne tracée surtout verticalement crée deux colonnes. Les deux parties sont toujours égales, quel que soit l'endroit où vous avez tracé la ligne.
- Cliquez sur **Disposition du tableau > Gomme**, puis faites glisser le pointeur d'une cellule à la suivante. Les deux cellules se réunissent, et le contenu des deux est conservé.

Chaque glissement correspond à une étape. **Annuler** le supprime.

## Position du texte dans une cellule

Les neuf boutons de **Disposition du tableau > Alignement** placent les mots en haut, au milieu ou en bas de la cellule, et à gauche, au centre ou à droite.

## Orienter le texte d'une cellule

Cliquez sur **Disposition du tableau > Sens du texte**. **Faire pivoter le texte vers le bas** oriente les mots de haut en bas de la cellule, et **Faire pivoter le texte vers le haut** les oriente de bas en haut. **Texte horizontal** les remet à plat. La ligne s'agrandit en fonction de la longueur des mots, jusqu'à environ 200 points, afin qu'une courte étiquette telle que « Total » tienne sur une seule ligne. Cliquez dans une cellule orientée et saisissez le texte comme dans n'importe quelle autre cellule. Le même choix figure dans **Propriétés du tableau**, sous l'onglet **Cellule**.

## Propriétés du tableau

Cliquez sur **Propriétés du tableau** dans **Disposition du tableau**, ou dans le menu du clic droit. La fenêtre comporte un onglet par élément :

- **Tableau** : position du tableau (à gauche, au centre ou à droite), distance par rapport à la marge gauche, largeur préférée (automatique, en points ou en pourcentage de la zone de texte), indication des colonnes fixes, **Habillage du texte autour du tableau** (le tableau reste à sa place et le texte des paragraphes suivants s'écoule à côté de lui, au lieu de passer en dessous), l'espace à l'intérieur de chaque cellule et l'espace entre les cellules.
- **Ligne** : la hauteur et l'indication d'une hauteur minimale ou exacte, **Ne pas laisser la ligne se couper entre deux pages**, et **Répéter comme ligne d'en-tête en haut de chaque page**. Une ligne d'en-tête réapparaît en haut de chaque page sur laquelle se prolonge un tableau long. Le bouton **Disposition du tableau > Répéter les lignes d'en-tête** fait de même pour les lignes sélectionnées.
- **Colonne** : la largeur de la colonne, en points ou en pourcentage du tableau. Les deux zones se mettent à jour l'une l'autre.
- **Cellule** : alignement vertical du texte, sens du texte, et possibilité ou non de renvoyer le texte à la ligne.
- **Texte de remplacement** : un titre et une description du tableau pour les personnes qui ne peuvent pas le voir.

## Ce que JeffWrite conserve

Un tableau que vous créez ou modifiez est enregistré dans le fichier avec ses largeurs, ses traits, ses remplissages, ses fusions et ses options, et il s'ouvre de la même manière dans JeffWrite et dans d'autres traitements de texte. Les parties d'un tableau que le fichier contenait et que JeffWrite n'affiche pas sont conservées telles quelles.

Dans un fichier `.odt`, un tableau conserve ses traits, ses remplissages et ses largeurs. Les remplissages que son style applique à la ligne d'en-tête et aux bandes sont inscrits dans les cellules. Une formule est conservée, mais pas son format de nombre, et un style n'est pas conservé en tant que style : le tableau présente le même aspect à la réouverture, mais il ne porte plus de nom de style.

[Retour au sommaire](index)
