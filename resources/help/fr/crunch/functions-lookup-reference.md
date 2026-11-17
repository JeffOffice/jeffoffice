# Fonctions de recherche et de référence
<!-- keywords: fonctions, ADRESSE, ZONES, COLONNE, COLONNES, FILTRE, FORMULETEXTE, LIREDONNEESTABLEAUCROISE, RECHERCHEH, LIEN_HYPERTEXTE, INDEX, INDIRECT, RECHERCHE, EQUIV, DECALER, LIGNE, LIGNES, TRI, TRIPAR, TRANSPOSER, UNIQUE, RECHERCHEV, RECHERCHEX, EQUIVX, recherche, référence, correspondance, recherche verticale, recherche horizontale, tableau croisé dynamique -->

Voici les fonctions de recherche et de référence que JeffCrunch peut calculer. Chacune indique son écriture, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires du dernier type que nécessaire. Pour essayer l'une d'elles, reportez-vous à [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments, et un clic sur le nom de la fonction dans cette info-bulle ouvre sa fiche ici.

## ADRESSE

`ADRESSE(no_lig, no_col, [abs_num], [a1], [nom_feuille])`

Écrit l'adresse d'une cellule sous forme de texte, à partir de son numéro de ligne et de colonne.

- `no_lig` : Le numéro de la ligne dans la plage, la première ligne étant comptée comme 1.
- `no_col` : Le numéro de la colonne dans la plage, la première colonne étant comptée comme 1.
- `abs_num` (facultatif) : La part de l'adresse qui est fixée : 1 toute l'adresse, 2 la ligne seulement, 3 la colonne seulement, 4 aucune partie.
- `a1` (facultatif) : VRAI ou omis pour lire l'adresse sous forme de lettre de colonne et de numéro de ligne ; FAUX pour la lire sous forme de numéros de ligne et de colonne.
- `nom_feuille` (facultatif) : Le nom de la feuille à placer devant l'adresse.

## ZONES

`ZONES(référence)`

Donne le nombre de plages distinctes de cellules dans une référence.

- `référence` : Une cellule ou une plage de cellules.

## COLONNE

`COLONNE([référence])`

Donne le numéro de colonne d'une cellule.

- `référence` (facultatif) : Une cellule ou une plage de cellules.

## COLONNES

`COLONNES(matrice)`

Compte les colonnes d'une plage de cellules.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.

## FILTRE

`FILTRE(matrice, inclure, [si_vide])`

Donne les lignes ou les colonnes d'une plage qui satisfont un critère.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `inclure` : Une colonne ou une ligne de valeurs VRAI et FAUX, une pour chaque ligne ou colonne de la plage ; VRAI la conserve.
- `si_vide` (facultatif) : Ce qu'il faut donner lorsqu'aucune ligne ou colonne ne satisfait le critère (l'erreur #CALC! si omis).

## FORMULETEXTE

`FORMULETEXTE(référence)`

Donne la formule d'une cellule sous forme de texte.

- `référence` : Une cellule ou une plage de cellules.

## LIREDONNEESTABLEAUCROISE

`LIREDONNEESTABLEAUCROISE(champ_données, tableau_croisé, [champ1], [élément1], ...)`

Donne la valeur qu'affiche un tableau croisé dynamique pour une valeur et pour les éléments de ses lignes et de ses colonnes. Une cellule du tableau est le deuxième argument ; les champs indiqués doivent être les premiers des lignes ou des colonnes.

- `champ_données` : Le nom de la valeur à donner, entre guillemets, tel que le tableau croisé l'affiche (« Somme de Montant »).
- `tableau_croisé` : Une cellule quelconque du tableau croisé dynamique.
- `champ1` (facultatif) : Le nom d'un champ des lignes ou des colonnes, entre guillemets (peut être omis avec son élément).
- `élément1` (facultatif) : L'élément de ce champ pour lequel donner la valeur, entre guillemets.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## RECHERCHEH

`RECHERCHEH(valeur_cherchée, matrice_table, no_index_lig, [valeur_proche])`

Recherche une valeur dans la première ligne d'un tableau et donne une valeur de la même colonne.

- `valeur_cherchée` : La valeur à rechercher.
- `matrice_table` : La plage de cellules dans laquelle chercher ; la valeur recherchée doit se trouver dans sa première colonne.
- `no_index_lig` : Le numéro de la ligne du tableau dont on donne une valeur, la première ligne étant comptée comme 1.
- `valeur_proche` (facultatif) : FAUX trouve une correspondance exacte ; VRAI ou omis trouve la valeur la plus proche située en dessous (la première colonne doit être triée par ordre croissant).

## LIEN_HYPERTEXTE

`LIEN_HYPERTEXTE(emplacement_lien, [nom_convivial])`

Crée une cellule qui ouvre un lien lorsqu'on clique dessus.

- `emplacement_lien` : La destination du lien : une adresse web, un fichier ou un emplacement de ce classeur.
- `nom_convivial` (facultatif) : Le texte à afficher dans la cellule (le lien lui-même si omis).

## INDEX

`INDEX(matrice, no_lig, [no_col], [no_zone])`

Donne la valeur située à une ligne et une colonne d'une plage de cellules.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `no_lig` : Le numéro de la ligne dans la plage, la première ligne étant comptée comme 1.
- `no_col` (facultatif) : Le numéro de la colonne dans la plage, la première colonne étant comptée comme 1.
- `no_zone` (facultatif) : La plage à utiliser lorsque le premier argument en désigne plusieurs (1 pour la première).

## INDIRECT

`INDIRECT(texte_réf, [a1])`

Donne la cellule vers laquelle pointe une adresse écrite sous forme de texte.

- `texte_réf` : Une adresse écrite sous forme de texte, par exemple « B5 ».
- `a1` (facultatif) : VRAI ou omis pour lire l'adresse sous forme de lettre de colonne et de numéro de ligne ; FAUX pour la lire sous forme de numéros de ligne et de colonne.

## RECHERCHE

`RECHERCHE(valeur_cherchée, vecteur_recherche, [vecteur_résultat])`

Recherche une valeur dans une ligne ou une colonne et donne la valeur située au même endroit dans une autre.

- `valeur_cherchée` : La valeur à rechercher.
- `vecteur_recherche` : La seule ligne ou la seule colonne de cellules dans laquelle chercher.
- `vecteur_résultat` (facultatif) : La seule ligne ou la seule colonne de cellules dans laquelle prendre le résultat.

## EQUIV

`EQUIV(valeur_cherchée, matrice_recherche, [type_correspondance])`

Donne la position d'une valeur dans une liste.

- `valeur_cherchée` : La valeur à rechercher.
- `matrice_recherche` : La plage de cellules, une seule ligne ou une seule colonne, dans laquelle chercher.
- `type_correspondance` (facultatif) : 1 ou omis trouve la plus grande valeur qui ne dépasse pas la valeur cherchée (la liste doit être triée par ordre croissant) ; 0 trouve une correspondance exacte ; -1 trouve la plus petite valeur qui n'est pas inférieure à la valeur cherchée (la liste doit être triée par ordre décroissant).

## DECALER

`DECALER(référence, lignes, colonnes, [hauteur], [largeur])`

Donne les cellules situées à un nombre donné de lignes et de colonnes d'une cellule de départ.

- `référence` : Une cellule ou une plage de cellules.
- `lignes` : Le nombre de lignes de décalage : positif vers le bas, négatif vers le haut.
- `colonnes` : Le nombre de colonnes de décalage : positif vers la droite, négatif vers la gauche.
- `hauteur` (facultatif) : Le nombre de lignes que compte le résultat.
- `largeur` (facultatif) : Le nombre de colonnes que compte le résultat.

## LIGNE

`LIGNE([référence])`

Donne le numéro de ligne d'une cellule.

- `référence` (facultatif) : Une cellule ou une plage de cellules.

## LIGNES

`LIGNES(matrice)`

Compte les lignes d'une plage de cellules.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.

## TRI

`TRI(matrice, [index_tri], [ordre_tri], [par_col])`

Donne une plage dont les lignes ou les colonnes sont mises en ordre.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `index_tri` (facultatif) : Le numéro de la colonne (ou de la ligne) selon laquelle trier (1 si omis).
- `ordre_tri` (facultatif) : 1 ou omis pour un ordre croissant ; -1 pour un ordre décroissant.
- `par_col` (facultatif) : VRAI pour travailler sur les colonnes ; FAUX ou omis pour travailler sur les lignes.

## TRIPAR

`TRIPAR(matrice, matrice_tri1, [ordre_tri1], ...)`

Donne une plage placée dans l'ordre d'une ou de plusieurs autres plages.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `matrice_tri1` : La plage de cellules selon laquelle classer la première plage ; de même longueur que la première.
- `ordre_tri1` (facultatif) : 1 ou omis pour un ordre croissant ; -1 pour un ordre décroissant.
- `...` : Autant d'autres arguments que nécessaire, du même type que le précédent.

## TRANSPOSER

`TRANSPOSER(matrice)`

Transforme les lignes en colonnes et les colonnes en lignes.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.

## UNIQUE

`UNIQUE(matrice, [par_col], [une_seule_fois])`

Donne les lignes ou les colonnes différentes d'une plage.

- `matrice` : Une plage de cellules, ou une liste de valeurs écrite entre accolades.
- `par_col` (facultatif) : VRAI pour travailler sur les colonnes ; FAUX ou omis pour travailler sur les lignes.
- `une_seule_fois` (facultatif) : VRAI pour ne donner que les éléments qui n'apparaissent qu'une fois ; FAUX ou omis pour donner chaque élément différent.

## RECHERCHEV

`RECHERCHEV(valeur_cherchée, matrice_table, no_index_col, [valeur_proche])`

Recherche une valeur dans la première colonne d'un tableau et donne une valeur de la même ligne.

- `valeur_cherchée` : La valeur à rechercher.
- `matrice_table` : La plage de cellules dans laquelle chercher ; la valeur recherchée doit se trouver dans sa première colonne.
- `no_index_col` : Le numéro de la colonne du tableau dont on donne une valeur, la première colonne étant comptée comme 1.
- `valeur_proche` (facultatif) : FAUX trouve une correspondance exacte ; VRAI ou omis trouve la valeur la plus proche située en dessous (la première colonne doit être triée par ordre croissant).

## RECHERCHEX

`RECHERCHEX(valeur_cherchée, matrice_recherche, matrice_renvoi, [si_introuvable], [mode_correspondance], [mode_recherche])`

Recherche une valeur dans une liste et donne l'élément correspondant d'une autre liste.

- `valeur_cherchée` : La valeur à rechercher.
- `matrice_recherche` : La plage de cellules, une seule ligne ou une seule colonne, dans laquelle chercher.
- `matrice_renvoi` : La plage de cellules dans laquelle prendre le résultat de la correspondance.
- `si_introuvable` (facultatif) : Ce qu'il faut donner lorsqu'il n'y a pas de correspondance (l'erreur #N/A si omis).
- `mode_correspondance` (facultatif) : 0 ou omis pour une correspondance exacte ; -1 pour la valeur immédiatement inférieure ; 1 pour la valeur immédiatement supérieure ; 2 pour utiliser des caractères génériques.
- `mode_recherche` (facultatif) : 1 ou omis pour chercher depuis le premier élément ; -1 depuis le dernier ; 2 ou -2 pour une recherche dichotomique dans une liste triée.

## EQUIVX

`EQUIVX(valeur_cherchée, matrice_recherche, [mode_correspondance], [mode_recherche])`

Donne la position d'une valeur dans une liste, avec des choix sur la façon de faire correspondre et sur le sens de la recherche.

- `valeur_cherchée` : La valeur à rechercher.
- `matrice_recherche` : La plage de cellules, une seule ligne ou une seule colonne, dans laquelle chercher.
- `mode_correspondance` (facultatif) : 0 ou omis pour une correspondance exacte ; -1 pour la valeur immédiatement inférieure ; 1 pour la valeur immédiatement supérieure ; 2 pour utiliser des caractères génériques.
- `mode_recherche` (facultatif) : 1 ou omis pour chercher depuis le premier élément ; -1 depuis le dernier ; 2 ou -2 pour une recherche dichotomique dans une liste triée.
