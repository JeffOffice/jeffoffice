# Tableaux
<!-- keywords: tableau, tableaux, supprimer les doublons, doublons, synthétiser, effacer le style, référence structurée, ligne d'en-tête, ligne de total, colonne calculée, bandes, lignes alternées, style, apparences de tableau, nouveau style de tableau, style de tableau personnalisé, boutons de filtre, Ctrl+T, convertir, cellules ordinaires, redimensionner, nom de colonne, [@Colonne], Tableau1[Colonne] -->

Un tableau est un bloc de cellules dont la première ligne contient les noms des colonnes, traité comme un seul objet : il a un nom, un aspect, des boutons pour filtrer et trier ses lignes, et il s'agrandit quand vous y ajoutez des données. Les formules peuvent appeler ses colonnes par leur nom au lieu de leur adresse de cellule.

## Créer un tableau

Sélectionnez les cellules, ou cliquez dans une cellule des données, puis choisissez **Tableau** dans l'onglet **Insertion** (**Ctrl+T**). JeffCrunch demande quelles cellules le tableau couvre et si la première ligne contient les noms des colonnes. Les noms qui sont des nombres, des dates ou des formules deviennent le texte qu'ils affichent, les noms vides deviennent Colonne1, Colonne2, et ainsi de suite, et deux colonnes portant le même nom sont numérotées. S'il n'y a pas de noms, le tableau n'a pas de ligne de noms, et ses colonnes s'appellent Colonne1, Colonne2, et ainsi de suite dans les formules.

La galerie **Apparences de tableau** (onglet **Accueil**, ou onglet **Tableau**) présente les apparences : claire, moyenne et foncée, chacune en noir et dans les six couleurs du thème du classeur. Choisir une apparence pour des cellules qui ne forment pas un tableau les transforme en tableau. L'apparence est dessinée sous ce qu'une cellule possède déjà : un remplissage, des bordures, une couleur de texte ou du gras que vous avez défini sur une cellule est conservé.

Pour créer votre propre apparence, choisissez **Nouveau style de tableau** en bas de la galerie. Saisissez un nom. Choisissez ensuite une couleur de remplissage, une couleur de texte et le gras pour chaque partie que vous voulez modifier : la ligne d'en-tête, les lignes alternées, la ligne des totaux et la première colonne. La nouvelle apparence apparaît dans un groupe **Personnalisé** en haut de la galerie, et elle est appliquée au tableau dans lequel vous vous trouvez. Elle est enregistrée dans le fichier, donc elle reste disponible quand vous rouvrez le fichier.

## L'onglet Tableau

L'onglet **Tableau** s'affiche lorsque le curseur se trouve dans un tableau.

- **Nom** est le nom par lequel les formules appellent le tableau. Toutes les formules qui utilisent l'ancien nom suivent le changement. Un nom comporte des lettres, des chiffres, des traits de soulignement et des points, commence par une lettre ou un trait de soulignement, et n'est ni celui d'un autre tableau, ni un nom du classeur, ni une adresse de cellule comme A1.
- **Ligne d'en-tête**, **Ligne des totaux**, **Lignes alternées**, **Colonnes alternées**, **Première colonne**, **Dernière colonne** et **Boutons de filtre** sont cochés ou décochés pour le tableau.
- **Redimensionner le tableau…** demande les cellules que couvre le tableau. Il conserve sa première cellule ; des lignes et des colonnes peuvent être ajoutées en bas et à droite, ou retirées.
- **Synthétiser avec un tableau croisé dynamique** crée un tableau croisé dynamique à partir des cellules du tableau (voir [Tableaux croisés dynamiques](pivot-tables)). Le tableau croisé dynamique lit le tableau par son nom, il suit donc le tableau quand celui-ci s'agrandit.
- **Supprimer les doublons…** retire les lignes qui se répètent. Une ligne se répète lorsqu'elle affiche les mêmes valeurs, dans toutes les colonnes que vous cochez, qu'une ligne située au-dessus. Les majuscules et les minuscules sont considérées comme identiques. La première des lignes est conservée, et les lignes situées sous une ligne supprimée remontent. Le tableau devient plus court, et sa ligne des totaux remonte avec lui. Un seul **Annuler** les rétablit toutes. Les cellules situées sous le tableau restent à leur place. Les formules qui pointent vers les cellules du tableau, sur n'importe quelle feuille, suivent les cellules comme lors de la suppression de lignes ; une formule qui pointait vers une ligne supprimée affiche #REF!. Cette fonction ne fonctionne pas sur un tableau dont un filtre est activé.
- **Convertir en plage de cellules** transforme le tableau en cellules ordinaires. Elles conservent leur contenu. Une formule qui désignait le tableau ou ses colonnes désigne désormais les cellules.

## Le menu des cellules

Faites un clic droit sur une cellule d'un tableau et choisissez **Tableau** : **Insérer une ligne au-dessus**, **Insérer une ligne en dessous**, **Insérer une colonne à gauche**, **Insérer une colonne à droite**, **Supprimer les lignes du tableau**, **Supprimer les colonnes du tableau**, **Ligne des totaux** et **Convertir en plage de cellules**. Ces commandes ajoutent ou retirent des lignes et des colonnes entières de la feuille, comme Insérer et Supprimer ; une ligne ajoutée juste après la fin du tableau est intégrée par celui-ci.

## Ligne des totaux

Activez **Ligne des totaux** : la ligne située sous le tableau contient le mot Total dans sa première colonne et un total dans sa dernière. Chaque cellule de la ligne possède un bouton : choisissez ce que affiche le total de la colonne (la moyenne, un nombre, le nombre de valeurs numériques, la plus grande valeur, la plus petite, la somme, l'écart type, la variance ou **Formule personnalisée**). Un total est un SOUS.TOTAL de sa colonne ; il exclut donc les lignes masquées par un filtre.

## Ajouter des données à un tableau

- Saisissez dans la ligne située sous un tableau, ou dans la colonne qui le jouxte, et le tableau s'agrandit pour intégrer la cellule. **Annuler** supprime cet agrandissement et conserve ce que vous avez saisi.
- Une formule saisie dans une cellule d'une colonne par ailleurs vide est recopiée dans toutes les cellules de cette colonne : c'est une colonne calculée. Utilisez **Annuler** juste après pour ne conserver la formule que dans une seule cellule. Une formule saisie sur une colonne calculée modifie toute la colonne si les autres cellules contiennent l'ancienne formule ; si certaines ne la contiennent pas, la cellule reste isolée.
- Les lignes insérées à l'intérieur d'un tableau reçoivent les formules des colonnes calculées. Les colonnes insérées à l'intérieur du tableau sont nommées dans la ligne d'en-tête. Supprimer des lignes ou des colonnes à l'intérieur du tableau le rend plus petit ; la ligne d'en-tête et chaque colonne ne peuvent pas toutes être supprimées.

## Filtrer et trier un tableau

Les boutons situés sur les noms filtrent et trient uniquement les lignes du tableau. Ils fonctionnent comme les boutons du filtre d'une feuille (voir [Trier et filtrer](sort-filter)). Un tableau ne peut pas être filtré pendant qu'un filtre de la feuille, ou d'un autre tableau, porte sur les mêmes lignes, car chacun devrait savoir quelles lignes lui appartiennent.

## Formules qui désignent un tableau

- `Ventes[Montant]` désigne les cellules de la colonne Montant, sans son nom ni son total.
- `[@Montant]` désigne la cellule de la ligne de la formule elle-même ; dans le fichier, elle est écrite `Ventes[[#Cette ligne],[Montant]]`.
- `Ventes[#En-têtes]`, `Ventes[#Données]`, `Ventes[#Totaux]` et `Ventes[#Tout]` désignent les noms, les données, les totaux et le tableau entier, et `Ventes[[#En-têtes],[Montant]]` désigne une colonne de ces éléments.
- `Ventes[[Région]:[Montant]]` désigne les colonnes de Région à Montant.

Saisissez `[` après le nom d'un tableau : la liste de ses colonnes apparaît. Pendant la saisie d'une formule, un clic sur une cellule d'un tableau écrit une référence comme `Ventes[@Montant]` ou `Ventes[Montant]` (un clic sur une colonne entière de données écrit la colonne). Renommer une colonne dans son en-tête la modifie dans toutes les formules ; insérer et supprimer des colonnes conserve aux formules les colonnes qu'elles désignaient, et une colonne supprimée laisse #REF! dans les formules qui la désignaient.
