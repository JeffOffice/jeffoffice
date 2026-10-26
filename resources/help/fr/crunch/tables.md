# Tableaux
<!-- keywords: tableau, tableaux, référence structurée, ligne d'en-tête, ligne des totaux, ligne Total, colonne calculée, bandes, lignes à bandes, style, style de tableau, apparences de tableau, boutons de filtre, Ctrl+T, convertir, plage de cellules, redimensionner, nom de colonne, [@Colonne], Tableau1[Colonne] -->

Un tableau est une plage de cellules surmontée d'une ligne de noms de colonnes, traitée comme un seul élément : il a un nom, une apparence, des boutons pour filtrer et trier ses lignes, et il s'agrandit lorsque vous y ajoutez des données. Les formules peuvent désigner ses colonnes par leur nom plutôt que par l'adresse des cellules.

## Créer un tableau

Sélectionnez les cellules, ou cliquez dans une cellule des données, puis choisissez **Tableau** dans l'onglet **Insertion** (**Ctrl+T**). JeffCrunch demande quelles cellules le tableau couvre et si la première ligne contient les noms des colonnes. Les noms qui sont des nombres, des dates ou des formules deviennent le texte qu'ils affichent, les noms vides deviennent Colonne1, Colonne2, etc., et deux colonnes portant le même nom sont numérotées. S'il n'y a pas de noms, le tableau n'a pas de ligne d'en-tête, et ses colonnes s'appellent Colonne1, Colonne2, etc. dans les formules.

La galerie **Apparences de tableau** (onglet **Accueil** ou onglet **Tableau**) présente les apparences : claires, moyennes et foncées, chacune en noir et dans les six couleurs du thème du classeur. En choisir une pour des cellules qui ne forment pas un tableau les convertit en tableau. L'apparence est appliquée sous la mise en forme propre à chaque cellule : un remplissage, des bordures, une couleur de texte ou du gras définis sur une cellule sont conservés.

## L'onglet Tableau

L'onglet **Tableau** s'affiche lorsque le curseur se trouve dans un tableau.

- **Nom** est le nom par lequel les formules désignent le tableau. Toutes les formules qui utilisent l'ancien nom sont mises à jour. Un nom se compose de lettres, de chiffres, de traits de soulignement et de points, commence par une lettre ou un trait de soulignement, et ne peut être ni le nom d'un autre tableau, ni un nom du classeur, ni une adresse de cellule comme A1.
- **Ligne d'en-tête**, **Ligne des totaux**, **Lignes à bandes**, **Colonnes à bandes**, **Première colonne**, **Dernière colonne** et **Boutons de filtre** s'activent ou se désactivent pour le tableau.
- **Redimensionner le tableau…** demande les cellules que couvre le tableau. Sa première cellule ne change pas ; vous pouvez ajouter ou retirer des lignes en bas et des colonnes à droite.
- **Convertir en plage de cellules** transforme le tableau en cellules ordinaires. Elles conservent leur contenu. Une formule qui désignait le tableau ou ses colonnes désigne alors les cellules.

## Le menu contextuel des cellules

Cliquez avec le bouton droit sur une cellule d'un tableau et choisissez **Tableau** : **Insérer une ligne au-dessus**, **Insérer une ligne en dessous**, **Insérer une colonne à gauche**, **Insérer une colonne à droite**, **Supprimer les lignes du tableau**, **Supprimer les colonnes du tableau**, **Ligne des totaux** et **Convertir en plage de cellules**. Ces commandes insèrent ou suppriment des lignes et des colonnes entières de la feuille, comme Insérer et Supprimer ; une ligne ou une colonne insérée juste après la fin du tableau est intégrée au tableau.

## Ligne des totaux

Activez **Ligne des totaux** : la ligne sous le tableau affiche le mot Total dans sa première colonne et un total dans sa dernière. Chaque cellule de cette ligne comporte un bouton qui permet de choisir ce qu'affiche le total de la colonne (la moyenne, le nombre de valeurs, le nombre de valeurs numériques, le maximum, le minimum, la somme, l'écart type, la variance ou **Formule personnalisée…**). Un total est un SOUS.TOTAL de sa colonne : il ne tient donc pas compte des lignes masquées par un filtre.

## Ajouter des données à un tableau

- Tapez dans la ligne située sous un tableau, ou dans la colonne voisine, et le tableau s'agrandit pour inclure la cellule. **Annuler** supprime l'agrandissement et conserve ce que vous avez tapé.
- Une formule tapée dans une cellule d'une colonne vide par ailleurs est recopiée dans toutes les cellules de la colonne : c'est une colonne calculée. Choisissez **Annuler** juste après pour ne garder la formule que dans cette cellule. Une formule tapée dans une colonne calculée modifie toute la colonne si les autres cellules contiennent l'ancienne formule ; sinon, seule cette cellule change.
- Les lignes insérées dans un tableau reçoivent les formules des colonnes calculées. Les colonnes insérées dans un tableau reçoivent un nom dans la ligne d'en-tête. Supprimer des lignes ou des colonnes d'un tableau le réduit ; vous ne pouvez pas supprimer à la fois la ligne d'en-tête et toutes les colonnes.

## Filtrer et trier un tableau

Les boutons de la ligne d'en-tête filtrent et trient uniquement les lignes du tableau. Ils fonctionnent comme les boutons de filtre d'une feuille (voir [Trier et filtrer](sort-filter)). Un tableau ne peut pas être filtré si un filtre de la feuille, ou d'un autre tableau, couvre les mêmes lignes, car chacun devrait savoir quelles lignes lui appartiennent.

## Formules qui font référence à un tableau

- `Sales[Amount]` désigne les cellules de la colonne Amount, sans son nom ni son total.
- `[@Amount]` désigne la cellule de la colonne située sur la ligne de la formule ; elle s'écrit `Sales[[#This Row],[Amount]]` dans le fichier.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]` et `Sales[#All]` désignent les en-têtes, les données, les totaux et le tableau entier, et `Sales[[#Headers],[Amount]]` désigne une seule colonne de ces éléments.
- `Sales[[Region]:[Amount]]` désigne les colonnes de Region à Amount.

Tapez `[` après le nom d'un tableau pour afficher la liste de ses colonnes. Pendant la saisie d'une formule, cliquer sur une cellule d'un tableau insère une référence comme `Sales[@Amount]`, ou `Sales[Amount]` si vous sélectionnez toute une colonne de données. Renommer une colonne dans son en-tête la renomme dans toutes les formules ; l'insertion et la suppression de colonnes laissent les formules pointer vers les colonnes qu'elles désignaient, et une colonne supprimée laisse #REF! dans les formules qui la désignaient.
