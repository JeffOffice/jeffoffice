# Noms de cellules et de valeurs
<!-- keywords: nom, noms, nom défini, nouveau nom, plage nommée, portée, commentaire, nom à partir des cellules, insérer un nom, Ctrl+F3, fait référence à, trier les noms, nom d'erreur, avertissement -->

Un **nom** est un mot qui désigne une cellule, une plage de cellules, un nombre ou une formule, de sorte que =Prix*Quantité peut être écrit à la place de =B2*C2. Un nom appartient à l'ensemble du classeur ou à une seule feuille. Un nom défini sur une feuille masque le nom du classeur qui s'écrit de la même façon, sur cette feuille.

## Les règles d'un nom

- Il commence par une lettre, un trait de soulignement ou une barre oblique inverse, et ne comprend ensuite que des lettres, des chiffres, des traits de soulignement et des points. Il ne contient pas d'espaces.
- Il comporte au maximum 255 caractères.
- Ce n'est pas l'adresse d'une cellule, comme **A1** ou **L2C3**.
- Aucun autre nom de la même feuille (ou du classeur) ne s'écrit de la même façon, en majuscules ou non.

Pendant la saisie, la boîte de dialogue indique ce qui ne va pas, et le bouton **OK** reste inactif.

## Créer un nom

1. Sélectionnez les cellules, puis cliquez sur **Nouveau nom** dans l'onglet **Formules**.
2. Saisissez le nom, choisissez où il s'applique et indiquez ce qu'il désigne. Les cellules sélectionnées sont saisies pour vous.

Il existe un moyen plus rapide pour un nom valable pour tout le classeur. Sélectionnez les cellules, cliquez dans la **zone Nom** (à gauche de la barre de formule), saisissez le nom, puis appuyez sur **Entrée**. Si aucun nom ne s'écrit ainsi, il est créé pour les cellules sélectionnées. Sinon, **Entrée** vous mène à ses cellules.

**Nom à partir des cellules** crée d'un seul coup plusieurs noms à partir des étiquettes situées autour d'une plage : si les intitulés figurent dans la ligne du haut et les nombres en dessous, chaque intitulé désigne les cellules situées sous lui. Choisissez où se trouvent les étiquettes (ligne du haut, colonne de gauche, ligne du bas ou colonne de droite). Une étiquette qui contient des espaces ou des signes interdits dans un nom est corrigée : ces caractères sont remplacés par des traits de soulignement. Une étiquette vide, en double, ou qui est déjà un nom du classeur est ignorée, et la barre d'état indique combien il y en a.

## Afficher tous les noms

Cliquez sur **Noms** dans l'onglet **Formules**, ou appuyez sur **Ctrl+F3**. La liste affiche chaque nom, la valeur qu'il a actuellement, ce qu'il désigne, l'endroit où il s'applique et son commentaire. Saisissez dans le filtre pour n'afficher que les noms correspondants. **Nouveau**, **Modifier** et **Supprimer** créent un nom, en modifient un ou en retirent un. Cliquez sur un en-tête pour trier la liste selon cette colonne, et cliquez à nouveau pour inverser l'ordre. Un nom qui pointe vers une erreur, par exemple un emplacement supprimé, est signalé par un petit triangle d'avertissement. Un classeur comportant plus de 20 000 noms n'affiche que les 20 000 premiers.

Modifier le nom d'un nom le change dans chaque formule qui l'utilise, sur toutes les feuilles, en une seule opération. Supprimer un nom laisse les formules qui l'utilisaient afficher **#NOM ?**. Chaque modification peut être annulée.

## Mettre un nom dans une formule

Pendant la saisie d'une formule, les noms correspondants apparaissent dans la liste située sous la cellule. **Insérer un nom** dans l'onglet **Formules** affiche la liste des noms à choisir.
