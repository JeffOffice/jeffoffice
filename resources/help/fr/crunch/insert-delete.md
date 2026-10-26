# Insérer et supprimer des lignes, des colonnes et des cellules
<!-- keywords: insérer, insertion, ajouter, supprimer, effacer, retirer, ligne, colonne, cellule, décaler, déplacer, vide, Ctrl+plus, Ctrl+moins, clic droit, en-tête, annuler, référence, #REF!, formule, nom, fusionnées, tableau, matricielle, bord -->

Vous pouvez ajouter des lignes, des colonnes ou des cellules vides au milieu de vos données, et retirer des lignes, des colonnes ou des cellules. Tout ce qui suit se décale pour faire de la place ou pour combler le vide, et les formules qui font référence aux cellules déplacées continuent d'y faire référence.

## Insérer

1. Sélectionnez les numéros de ligne, les lettres de colonne ou les cellules à l'endroit où vous voulez les nouvelles. Pour ajouter trois lignes, sélectionnez trois lignes.
2. Appuyez sur **Ctrl et la touche plus** (**Ctrl et la touche moins** supprime), ou utilisez **Accueil > Insérer**, ou cliquez avec le bouton droit sur la sélection et choisissez **Insérer**.

Si des lignes ou des colonnes entières sont sélectionnées, les nouvelles sont insérées immédiatement, au-dessus des lignes ou à gauche des colonnes. Si des cellules sont sélectionnées, une petite fenêtre vous demande comment décaler les cellules qui suivent :

- **Décaler les cellules vers la droite** ou **Décaler les cellules vers le bas** ne déplace que les cellules des colonnes (ou des lignes) de votre sélection et laisse le reste de la feuille en place.
- **Ligne entière** ou **Colonne entière** ajoute des lignes ou des colonnes entières.

## Supprimer

Sélectionnez ce que vous voulez retirer et appuyez sur **Ctrl et la touche moins**, ou utilisez **Accueil > Supprimer**. Si des cellules sont sélectionnées, la fenêtre vous demande s'il faut **décaler les cellules vers la gauche**, **décaler les cellules vers le haut**, ou supprimer la **ligne entière** ou la **colonne entière**. La suppression se fait en une seule étape, et **Annuler** rétablit tout ce qu'elle a retiré, formules comprises.

## Conséquences

Une formule qui fait référence à des cellules déplacées y fait référence à leur nouvel emplacement. Exemple : la cellule B1 contient **=SOMME(A1:A5)**. Insérez une ligne au-dessus de la ligne 3 : la formule devient **=SOMME(A1:A6)**, car la plage s'est agrandie pour inclure la nouvelle ligne. Si vous supprimez la ligne 3, elle devient **=SOMME(A1:A4)**. Une formule qui faisait référence à une cellule supprimée affiche **#REF!**, ce qui signifie que la cellule dont elle a besoin n'existe plus. **Annuler** rétablit la cellule et la formule.

Les éléments suivants suivent aussi les cellules : les formules de toutes les feuilles, les noms (comme les zones d'impression et les titres à imprimer), les cellules fusionnées, les cellules concernées par les règles de validation et les mises en forme conditionnelles, les tableaux, les graphiques, les images, les notes, les liens hypertexte et les sauts de page.

## Opérations impossibles

- Impossible d'insérer ou de supprimer des cellules à travers une partie d'une **formule matricielle** ou d'une table de données. Déplacez-les en entier, ou n'y touchez pas. Une cellule lue par une table de données comme valeur d'entrée ne peut pas non plus être supprimée.
- Un **tableau croisé dynamique** ne peut pas encore être déplacé ni coupé : impossible d'insérer ou de supprimer des lignes, des colonnes ou des cellules là où cela le déplacerait ou le modifierait, et impossible de couper des cellules depuis ce tableau ou vers lui. Le reste de la feuille se modifie normalement.
- Un **tableau** ne peut pas être coupé en travers : impossible d'y insérer une colonne, de supprimer sa ligne d'en-tête ou toutes ses lignes. Insérez ou supprimez des lignes à l'intérieur, ou modifiez les cellules qui l'entourent.
- Sur une **feuille protégée**, impossible d'insérer ou de supprimer des lignes, des colonnes ou des cellules. Pour le faire, choisissez Ôter la protection de la feuille dans l'onglet Révision.
- Les cellules non vides situées au bout de la feuille ne peuvent pas être repoussées au-delà du bord. Supprimez-les d'abord.
- Après un enregistrement, **Annuler** revient toujours sur ce que vous avez fait auparavant, y compris les lignes et colonnes insérées (l'enregistrement suivant les retire de nouveau). La seule chose qu'il ne peut pas rétablir, ce sont les lignes, colonnes ou cellules **supprimées** : le fichier enregistré a perdu leur contenu, si bien qu'**Annuler** s'arrête à la dernière suppression enregistrée, et la ligne en bas de la fenêtre vous le signale lors de l'enregistrement.
- Une opération qui déplace ou supprime tant de cellules qu'**Annuler** ne pourrait pas la conserver (plus de 128 Mo environ) demande d'abord confirmation, comme le font les tableurs que vous connaissez : « Cette action est irréversible. » Si vous continuez, l'opération est effectuée et **Annuler** ne peut revenir ni sur elle ni sur ce qui la précède.
