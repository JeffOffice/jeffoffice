# Noms de cellules et de valeurs
<!-- keywords: nom, noms, nom défini, nouveau nom, plage nommée, portée, commentaire, nom à partir des cellules, insérer un nom, Ctrl+F3, fait référence à -->

Un **nom** est un mot qui représente une cellule, une plage de cellules, un nombre ou une formule. Vous pouvez ainsi écrire =Prix*Quantité au lieu de =B2*C2. Un nom s'applique à tout le classeur ou à une seule feuille. Sur une feuille, un nom propre à cette feuille masque le nom du classeur qui s'écrit de la même façon.

## Règles applicables aux noms

- Un nom commence par une lettre, un trait de soulignement ou une barre oblique inverse, puis ne contient que des lettres, des chiffres, des traits de soulignement et des points. Il ne contient pas d'espace.
- Il comporte au maximum 255 caractères.
- Ce n'est pas l'adresse d'une cellule, comme **A1** ou **R2C3**.
- Aucun autre nom de la même feuille (ou du classeur) ne s'écrit de la même façon, en majuscules ou non.

Pendant la saisie, la boîte de dialogue indique le problème, et le bouton **OK** reste indisponible.

## Créer un nom

1. Sélectionnez les cellules, puis cliquez sur **Nouveau nom** sous l'onglet **Formules**.
2. Tapez le nom, choisissez sa portée et indiquez ce à quoi il fait référence. Les cellules sélectionnées sont déjà renseignées.

**Nom à partir des cellules** crée plusieurs noms à la fois à partir des étiquettes qui entourent une plage : si les en-têtes se trouvent dans la ligne du haut et les nombres en dessous, chaque en-tête devient le nom des cellules situées sous lui. Choisissez l'emplacement des étiquettes (ligne du haut, colonne de gauche, ligne du bas ou colonne de droite). Dans une étiquette, les espaces et les caractères interdits dans un nom sont remplacés par des traits de soulignement. Une étiquette vide, en double ou correspondant à un nom qui existe déjà dans le classeur est ignorée, et la barre d'état indique combien d'étiquettes l'ont été.

## Afficher tous les noms

Cliquez sur **Noms** sous l'onglet **Formules**, ou appuyez sur **Ctrl+F3**. La liste affiche chaque nom, sa valeur actuelle, ce à quoi il fait référence, sa portée et son commentaire. Tapez dans le filtre pour n'afficher que les noms correspondants. **Nouveau**, **Modifier** et **Supprimer** permettent de créer, de modifier et de supprimer un nom. Si un classeur contient plus de 20 000 noms, seuls les 20 000 premiers sont listés.

Lorsque vous renommez un nom, il est modifié en une seule opération dans toutes les formules qui l'utilisent, sur toutes les feuilles. Si vous supprimez un nom, les formules qui l'utilisaient affichent **#NAME?**. Chaque modification peut être annulée.

## Insérer un nom dans une formule

Pendant la saisie d'une formule, les noms correspondants s'affichent dans la liste sous la cellule. **Insérer un nom** sous l'onglet **Formules** affiche la liste des noms disponibles.
