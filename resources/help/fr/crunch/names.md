# Noms des cellules et des valeurs
<!-- keywords: nom, noms, nom défini, nouveau nom, plage nommée, portée, commentaire, nom à partir des cellules, insérer un nom, Ctrl+F3, fait référence à -->

Un **nom** est un mot qui désigne une cellule, une plage de cellules, un nombre ou une formule, de sorte que =Prix*Quantité peut être écrit à la place de =B2*C2. Un nom appartient à l'ensemble du classeur ou à une seule feuille. Un nom défini sur une feuille masque un nom du classeur qui s'écrit de la même façon, sur cette feuille.

## Les règles applicables à un nom

- Il commence par une lettre, un trait de soulignement ou une barre oblique inverse, et ne comporte ensuite que des lettres, des chiffres, des traits de soulignement et des points. Il ne contient aucun espace.
- Il comporte 255 caractères au maximum.
- Il ne correspond pas à l'adresse d'une cellule, comme **A1** ou **L2C3**.
- Aucun autre nom de la même feuille (ou du classeur) ne s'écrit de la même façon, en majuscules ou non.

Pendant la saisie, la boîte de dialogue indique ce qui ne va pas, et le bouton **OK** reste inactif.

## Créer un nom

1. Sélectionnez les cellules, puis cliquez sur **Nouveau nom** dans l'onglet **Formules**.
2. Saisissez le nom, choisissez la portée et indiquez ce qu'il désigne. Les cellules sélectionnées sont reprises automatiquement.

Il existe un moyen plus rapide pour un nom de l'ensemble du classeur. Sélectionnez les cellules, cliquez dans la **zone Nom** (à gauche de la barre de formule), saisissez le nom, puis appuyez sur **Entrée**. Si aucun nom de ce type n'existe encore, il est créé pour les cellules sélectionnées. S'il existe, **Entrée** vous amène à ses cellules.

**Nom à partir des cellules** crée plusieurs noms à la fois à partir des étiquettes situées autour d'une plage : si les intitulés figurent dans la ligne du haut et les nombres en dessous, chaque intitulé désigne les cellules situées sous lui. Choisissez où se trouvent les étiquettes (ligne du haut, colonne de gauche, ligne du bas ou colonne de droite). Une étiquette qui contient des espaces ou des caractères interdits dans un nom est transformée : ces espaces sont remplacés par des traits de soulignement. Une étiquette vide, en double ou correspondant à un nom déjà présent dans le classeur est ignorée, et la barre d'état indique le nombre de noms ainsi ignorés.

## Afficher tous les noms

Cliquez sur **Noms** dans l'onglet **Formules**, ou appuyez sur **Ctrl+F3**. La liste affiche chaque nom, sa valeur actuelle, ce qu'il désigne, sa portée et son commentaire. Saisissez du texte dans le filtre pour n'afficher que les noms correspondants. Les boutons **Nouveau**, **Modifier** et **Supprimer** permettent de créer un nom, d'en modifier un ou de le supprimer. Un classeur comportant plus de 20 000 noms n'affiche que les 20 000 premiers.

La modification du nom d'un nom le change dans toutes les formules qui l'utilisent, sur toutes les feuilles, en une seule opération. La suppression d'un nom laisse les formules qui l'utilisaient affichant **#NOM ?**. Chaque modification peut être annulée.

## Insérer un nom dans une formule

Pendant la saisie d'une formule, les noms correspondants s'affichent dans la liste sous la cellule. Le bouton **Insérer un nom** de l'onglet **Formules** affiche la liste des noms à choisir.
