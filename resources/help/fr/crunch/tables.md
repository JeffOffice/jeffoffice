# Tableaux
<!-- keywords: tableau, tableaux, supprimer les doublons, doublons, synthétiser, effacer le style, référence structurée, ligne d'en-tête, ligne des totaux, colonne calculée, à bandes, rayé, style, apparences de tableau, boutons de filtre, Ctrl+T, convertir, plage de cellules, redimensionner, nom de colonne, [@Colonne], Tableau1[Colonne] -->

Un tableau est une plage de cellules dont la première ligne contient les noms de ses colonnes, traitée comme un seul élément : il a un nom, une apparence, des boutons pour filtrer et trier ses lignes, et il s'agrandit quand vous y ajoutez des données. Les formules peuvent appeler ses colonnes par leur nom au lieu de leur adresse de cellule.

## Créer un tableau

Sélectionnez les cellules, ou cliquez dans une cellule des données, puis choisissez **Tableau** dans l'onglet **Insertion** (**Ctrl+T**). JeffCrunch demande quelles cellules le tableau couvre et si la première ligne contient les noms des colonnes. Les noms qui sont des nombres, des dates ou des formules deviennent le texte qu'ils affichent ; les noms vides deviennent Colonne1, Colonne2, et ainsi de suite ; deux colonnes portant le même nom sont numérotées. S'il n'y a pas de noms, le tableau n'a pas de ligne d'en-tête, et ses colonnes s'appellent Colonne1, Colonne2, et ainsi de suite dans les formules.

La galerie **Apparences de tableau** (onglet **Accueil**, ou onglet **Tableau**) présente les apparences : **Effacer**, sous la galerie dans l'onglet **Tableau**, retire l'apparence du tableau. Vous pouvez choisir une apparence claire, moyenne ou foncée, chacune en noir et dans les six couleurs du thème du classeur. Si vous en choisissez une pour des cellules qui ne forment pas un tableau, celles-ci deviennent un tableau. L'apparence s'applique sous la mise en forme propre à une cellule : un remplissage, des bordures, une couleur de texte ou du gras que vous avez définis sur une cellule restent en place.

## L'onglet Tableau

L'onglet **Tableau** s'affiche lorsque le curseur se trouve dans un tableau.

- **Nom** est le nom par lequel les formules appellent le tableau. Toute formule qui utilise l'ancien nom suit la modification. Un nom comporte des lettres, des chiffres, des traits de soulignement et des points, commence par une lettre ou un trait de soulignement, et n'est ni celui d'un autre tableau, ni un nom du classeur, ni une adresse de cellule telle que A1.
- **Ligne d'en-tête**, **Ligne des totaux**, **Lignes à bandes**, **Colonnes à bandes**, **Première colonne**, **Dernière colonne** et **Boutons de filtre** sont cochés ou décochés pour le tableau.
- **Redimensionner le tableau…** demande les cellules que couvre le tableau. Il conserve sa première cellule ; des lignes et des colonnes peuvent être ajoutées en bas et à droite, ou retirées.
- **Synthétiser avec un tableau croisé dynamique** crée un tableau croisé dynamique à partir des cellules du tableau (voir [Tableaux croisés dynamiques](pivot-tables)). Le tableau croisé dynamique lit le tableau par son nom, il suit donc le tableau quand celui-ci s'agrandit.
- **Supprimer les doublons…** retire les lignes qui se répètent. Une ligne se répète lorsqu'elle affiche les mêmes valeurs, dans toutes les colonnes que vous cochez, qu'une ligne située au-dessus. Les majuscules et les minuscules sont considérées comme identiques. La première des lignes reste, et les lignes situées sous une ligne supprimée remontent. Le tableau devient plus court, et sa ligne des totaux remonte avec lui. Une seule commande **Annuler** les rétablit toutes. Les cellules situées sous le tableau restent à leur place. Les formules qui pointent vers les cellules du tableau, sur n'importe quelle feuille, suivent les cellules comme lorsque vous supprimez des lignes ; une formule qui pointait vers une ligne supprimée affiche #REF!. Cette fonction ne fonctionne pas sur un tableau dont un filtre est activé.
- **Convertir en plage de cellules** transforme le tableau en cellules ordinaires. Elles conservent leur contenu. Une formule qui désignait le tableau ou ses colonnes désigne désormais les cellules.

## Le menu des cellules

Cliquez avec le bouton droit sur une cellule d'un tableau, puis choisissez **Tableau** : **Insérer une ligne au-dessus**, **Insérer une ligne en dessous**, **Insérer une colonne à gauche**, **Insérer une colonne à droite**, **Supprimer les lignes du tableau**, **Supprimer les colonnes du tableau**, **Ligne des totaux** et **Convertir en plage de cellules**. Ces commandes ajoutent ou retirent des lignes et des colonnes entières de la feuille, comme le font Insertion et Supprimer ; une ligne ajoutée juste après la fin du tableau est intégrée au tableau.

## Ligne des totaux

Activez **Ligne des totaux** : la ligne située sous le tableau affiche le mot Total dans sa première colonne et un total dans sa dernière. Chaque cellule de la ligne a un bouton : choisissez ce qu'affiche le total de la colonne (la moyenne, un nombre, le nombre de valeurs numériques, la plus grande valeur, la plus petite, la somme, l'écart type, la variance, ou **Formule personnalisée**). Un total est un SOUS.TOTAL de sa colonne, il ignore donc les lignes masquées par un filtre.

## Ajouter des données à un tableau

- Saisissez dans la ligne située sous un tableau, ou dans la colonne qui le jouxte, et le tableau s'agrandit pour intégrer la cellule. **Annuler** supprime l'agrandissement et conserve ce que vous avez saisi.
- Une formule saisie dans une cellule d'une colonne par ailleurs vide est recopiée dans toutes les cellules de cette colonne : c'est une colonne calculée. Utilisez **Annuler** juste après pour que la formule ne reste que dans une seule cellule. Une formule saisie par-dessus une colonne calculée modifie toute la colonne si les autres cellules contiennent l'ancienne formule ; si certaines ne la contiennent pas, la cellule reste isolée.
- Les lignes insérées à l'intérieur d'un tableau reçoivent les formules des colonnes calculées. Les colonnes insérées à l'intérieur sont nommées dans la ligne d'en-tête. Supprimer des lignes ou des colonnes à l'intérieur d'un tableau le rend plus petit ; la ligne d'en-tête et toutes les colonnes ne peuvent pas être supprimées ensemble.

## Filtrer et trier un tableau

Les boutons situés sur les noms ne filtrent et ne trient que les lignes du tableau. Ils fonctionnent comme les boutons du filtre d'une feuille (voir [Trier et filtrer](sort-filter)). Un tableau ne peut pas être filtré pendant qu'un filtre de la feuille, ou d'un autre tableau, couvre les mêmes lignes, car chacun devrait savoir quelles lignes lui appartiennent.

## Formules qui désignent un tableau

- `Ventes[Montant]` désigne les cellules de la colonne Montant, sans son nom ni son total.
- `[@Montant]` désigne la cellule de la ligne de la formule elle-même ; elle s'écrit `Ventes[[#Cette ligne],[Montant]]` dans le fichier.
- `Ventes[#En-têtes]`, `Ventes[#Données]`, `Ventes[#Totaux]` et `Ventes[#Tout]` désignent les noms, les données, les totaux et l'ensemble du tableau, et `Ventes[[#En-têtes],[Montant]]` désigne une colonne de ceux-ci.
- `Ventes[[Région]:[Montant]]` désigne les colonnes allant de Région à Montant.

Saisissez `[` après le nom d'un tableau, et la liste de ses colonnes apparaît. Pendant la saisie d'une formule, cliquer sur une cellule d'un tableau insère une référence telle que `Ventes[@Montant]` ou `Ventes[Montant]` (cliquer sur une colonne entière de ses données). Renommer une colonne dans son en-tête modifie son nom dans toutes les formules ; insérer et supprimer des colonnes maintient les formules pointées vers les colonnes qu'elles désignaient, et une colonne supprimée laisse #REF! dans les formules qui la désignaient.
