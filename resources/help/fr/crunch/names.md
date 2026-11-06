# Noms des cellules et des valeurs
<!-- keywords: nom, noms, nom défini, nouveau nom, plage nommée, portée, commentaire, nom à partir des cellules, insérer un nom, appliquer les noms, Ctrl+F3, fait référence à, trier les noms, nom d'erreur, avertissement -->

Un **nom** est un mot qui représente une cellule, une plage de cellules, un nombre ou une formule, de sorte que =Prix*Quantité peut être écrit à la place de =B2*C2. Un nom appartient à l'ensemble du classeur ou à une seule feuille. Un nom défini sur une feuille masque un nom du classeur qui s'écrit de la même façon, sur cette feuille.

## Les règles d'un nom

- Il commence par une lettre, un trait de soulignement ou une barre oblique inverse, et ne comporte ensuite que des lettres, des chiffres, des traits de soulignement et des points. Il ne contient aucun espace.
- Il comporte au maximum 255 caractères.
- Ce n'est pas l'adresse d'une cellule, comme **A1** ou **L2C3**.
- Aucun autre nom de la même feuille (ou du classeur) ne s'écrit de la même façon, en majuscules ou non.

Pendant la saisie, la boîte de dialogue indique ce qui ne va pas, et le bouton **OK** reste inactif.

## Créer un nom

1. Sélectionnez les cellules, puis cliquez sur **Nouveau nom** dans l'onglet **Formules**.
2. Saisissez le nom, choisissez l'endroit où il s'applique et indiquez ce qu'il représente. Les cellules sélectionnées sont saisies automatiquement.

Il existe un moyen plus rapide pour un nom de l'ensemble du classeur. Sélectionnez les cellules, cliquez dans la **Zone Nom** (à gauche de la barre de formule), saisissez le nom, puis appuyez sur **Entrée**. Si aucun nom de ce type n'existe encore, il est créé pour les cellules sélectionnées. Sinon, **Entrée** vous amène à ses cellules.

**Nom à partir des cellules** crée plusieurs noms en une seule fois à partir des étiquettes entourant une plage : si les intitulés figurent dans la ligne du haut et les nombres en dessous, chaque intitulé nomme les cellules situées sous lui. Choisissez l'emplacement des étiquettes (ligne du haut, colonne de gauche, ligne du bas ou colonne de droite). Une étiquette contenant des espaces ou des caractères interdits dans un nom est corrigée : ces espaces sont remplacés par des traits de soulignement. Une étiquette vide, en double, ou qui est déjà un nom du classeur est ignorée, et la barre d'état indique combien de noms ont été ignorés.

## Afficher tous les noms

Cliquez sur **Noms** dans l'onglet **Formules**, ou appuyez sur **Ctrl+F3**. La liste affiche chaque nom, la valeur qu'il a actuellement, ce qu'il représente, l'endroit où il s'applique et son commentaire. Saisissez dans le filtre pour n'afficher que les noms correspondants. Les boutons **Nouveau**, **Modifier** et **Supprimer** permettent de créer un nom, d'en modifier un ou d'en supprimer un. Cliquez sur un en-tête pour trier la liste selon cette colonne, puis cliquez de nouveau pour inverser l'ordre. Un nom qui renvoie à une erreur, par exemple à un emplacement supprimé, est signalé par un petit triangle d'avertissement. Un classeur comportant plus de 20 000 noms n'affiche que les 20 000 premiers.

La modification du nom d'un nom le change dans toutes les formules qui l'utilisent, sur toutes les feuilles, en une seule opération. La suppression d'un nom laisse les formules qui l'utilisaient afficher **#NOM?**. Chaque modification peut être annulée.

## Insérer un nom dans une formule

Pendant la saisie d'une formule, les noms correspondants s'affichent dans la liste sous la cellule. La commande **Insérer un nom**, dans l'onglet **Formules**, affiche la liste des noms à choisir.

## Mettre des noms dans des formules déjà écrites

Supposons que vous ayez créé le nom Taux pour $A$1 alors que de nombreuses formules utilisent déjà $A$1. Cliquez sur **Appliquer les noms** dans l'onglet **Formules** pour écrire Taux à leur place. Sélectionnez d'abord les cellules à examiner, ou sélectionnez une seule cellule pour examiner toute la feuille. Choisissez les noms à utiliser. Une référence n'est modifiée que si elle désigne exactement les cellules du nom, avec ou sans signes dollar : A1:B2 n'est pas remplacée par un nom désignant A1. Seuls les noms désignant une plage de cellules d'une feuille sont proposés, ainsi que les noms de l'ensemble du classeur. La modification se fait en une seule étape, donc **Annuler** la rétablit.
