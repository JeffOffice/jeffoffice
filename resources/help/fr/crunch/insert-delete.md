# Insérer et supprimer des lignes, des colonnes et des cellules
<!-- keywords: insérer cellules coupées, insérer cellules copiées, décaler vers la droite, décaler vers le bas, insérer, supprimer, retirer, effacer, ligne, colonne, cellule, décaler, déplacer, ajouter, vide, Ctrl+plus, Ctrl+moins, clic droit, en-tête, annuler, référence, #REF!, formule, nom, fusionnée, tableau, matricielle, bord -->

Vous pouvez ajouter des lignes, des colonnes ou des cellules vides au milieu de vos données, et en retirer. Tout ce qui se trouve après se déplace pour faire de la place ou pour combler le vide, et les formules qui pointent vers les cellules déplacées continuent de pointer vers elles.

## Insérer

1. Sélectionnez les numéros de lignes, les lettres de colonnes ou les cellules à l'endroit où vous voulez les nouvelles. Pour ajouter trois lignes, sélectionnez trois lignes.
2. Appuyez sur **Ctrl et la touche plus** (**Ctrl et la touche moins** supprime), ou utilisez **Accueil > Insérer**, ou faites un clic droit sur la sélection et choisissez **Insérer**.

Lorsque des lignes ou des colonnes entières sont sélectionnées, les nouvelles sont insérées en une fois, au-dessus des lignes ou à gauche des colonnes. Lorsque des cellules sont sélectionnées, une petite fenêtre vous demande comment les cellules suivantes doivent se déplacer :

- **Décaler les cellules vers la droite** ou **Décaler les cellules vers le bas** ne déplace que les cellules des colonnes (ou des lignes) de votre sélection, et laisse le reste de la feuille à sa place.
- **Ligne entière** ou **Colonne entière** ajoute des lignes ou des colonnes complètes.

## Insérer des cellules coupées ou copiées

Pour placer des cellules que vous avez coupées ou copiées entre d'autres cellules, au lieu de les mettre par-dessus, commencez par les couper ou les copier (**Ctrl+X** ou **Ctrl+C**). Sélectionnez ensuite la cellule où elles doivent aller, et utilisez **Accueil > Insérer > Insérer les cellules coupées ou copiées** (la commande affiche **Insérer les cellules coupées** ou **Insérer les cellules copiées** dès qu'il y a quelque chose à placer), ou faites un clic droit sur la cellule et choisissez-la. Une petite fenêtre vous demande si les cellules présentes doivent **décaler vers la droite** ou **décaler vers le bas** pour faire de la place.

Le bouton **Insérer** et **Ctrl et la touche plus** (ainsi que **Ctrl+Maj et la touche plus**) font de même, mais seulement lorsque l'intention est claire :

- lorsque des cellules sont **coupées** et que la coupe est toujours en attente (sa bordure en pointillés est visible) sur la feuille d'origine, et
- lorsque des cellules sont **copiées** et que vous n'avez rien modifié depuis (pas de saisie, pas d'autre modification, ni Annuler ni Rétablir).

Dans tous les autres cas, le bouton et les touches insèrent des cellules vides, comme d'habitude. Après une modification, la copie reste disponible pour **Coller** et pour la commande du menu. **Échap** met fin à une coupe ou à une copie : la bordure disparaît, et le bouton et les touches n'insèrent de nouveau que des cellules vides. Si des cellules sont coupées sur une feuille et que vous appuyez sur les touches sur une autre feuille, des cellules vides sont insérées ; la commande du menu indique que les cellules coupées ne peuvent être placées entre d'autres cellules que sur leur propre feuille. Lorsque des lignes ou des colonnes entières sont sélectionnées, la commande est désactivée, et Insérer ajoute des lignes ou des colonnes comme auparavant.

- Les cellules coupées sont **déplacées** : elles quittent leur ancien emplacement, et ce qui pointe vers elles les suit. C'est le même effet que de maintenir **Maj** en faisant glisser la bordure de la sélection, et c'est tout ou rien : si une partie est refusée, rien ne change.
- Les cellules copiées sont **placées** sous forme de copie, comme le ferait **Coller** (selon le paramètre de **Définir le collage par défaut**, et avec les formules adaptées à leur nouvel emplacement). La copie reste prête, de sorte que vous pouvez la placer à nouveau ailleurs.
- Les cellules ne sont insérées qu'une fois, dans la cellule en haut à gauche de votre sélection, même si vous avez sélectionné plus de cellules que vous n'en avez copié.
- Si la liste est filtrée, les cellules sont placées dans les lignes visibles, comme pour un collage, et l'espace créé comprend les lignes masquées par le filtre entre elles. Une ligne que vous avez masquée manuellement est traitée comme n'importe quelle autre ligne.
- Chaque opération correspond à une étape que vous pouvez annuler. Les cellules coupées ne peuvent être placées entre d'autres cellules que sur la feuille d'où elles viennent ; pour les déplacer vers une autre feuille, utilisez **Coller**.
- Une cellule ne peut pas être placée dans son propre chemin (des cellules coupées de A3:A4 ne peuvent pas être placées entre A3 et A4), et une feuille protégée le refuse. JeffCrunch indique la raison et ne change rien.

## Supprimer

Sélectionnez ce que vous voulez retirer, puis appuyez sur **Ctrl et la touche moins**, ou utilisez **Accueil > Supprimer**. Lorsque des cellules sont sélectionnées, la fenêtre vous demande s'il faut **décaler les cellules vers la gauche**, **décaler les cellules vers le haut**, ou supprimer la **ligne entière** ou la **colonne entière**. La suppression est une seule étape, et **Annuler** rétablit tout ce qui a été retiré, y compris les formules.

## Conséquences

Une formule qui pointe vers des cellules déplacées pointe vers elles à leur nouvel emplacement. Exemple : en B1, vous avez **=SOMME(A1:A5)**. Si vous insérez une ligne au-dessus de la ligne 3, la formule devient **=SOMME(A1:A6)**, car la plage s'agrandit pour inclure la nouvelle ligne. Si vous supprimez la ligne 3, elle devient **=SOMME(A1:A4)**. Une formule qui pointait vers une cellule supprimée affiche **#REF!**, ce qui signifie que la cellule dont elle a besoin n'existe plus. **Annuler** rétablit la cellule et la formule.

Les éléments suivants suivent aussi les cellules : les formules de toutes les feuilles, les noms (comme les zones d'impression et les titres à imprimer), les cellules fusionnées, les cellules auxquelles s'appliquent les règles de la validation des données et les mises en forme conditionnelles, les tableaux, les graphiques, les images, les notes, les liens hypertexte et les sauts de page.

## Ce qui n'est pas possible

- Des cellules ne peuvent pas être insérées ou supprimées à travers une partie d'une **formule matricielle** ou d'un tableau de données. Déplacez-les entières, ou laissez-les en place. Une cellule que un tableau de données lit comme entrée ne peut pas non plus être supprimée.
- Un **tableau croisé dynamique** ne peut pas encore être déplacé ni coupé : des lignes, des colonnes ou des cellules ne peuvent pas être insérées ou supprimées là où cela le déplacerait ou le modifierait, et des cellules ne peuvent pas être coupées depuis lui ni placées dans lui. Le reste de la feuille peut être modifié normalement. Un tableau croisé dynamique se modifie avec les commandes de l'onglet **Tableau croisé dynamique** (actualisation, modification de la source de données, champs et disposition), et non en saisissant dans ses cellules.
- Un **tableau** ne peut pas être coupé en travers : vous ne pouvez pas insérer une colonne à l'intérieur, ni supprimer sa ligne d'en-tête ou toutes ses lignes. Insérez ou supprimez des lignes à l'intérieur, ou modifiez les cellules qui l'entourent.
- Une **feuille protégée** ne peut pas recevoir de lignes, de colonnes ou de cellules insérées ou supprimées. Pour le faire, choisissez Ôter la protection de la feuille dans l'onglet **Révision**.
- Les cellules situées en bout de feuille qui contiennent quelque chose ne peuvent pas être repoussées hors de la feuille. Supprimez-les d'abord.
- Après l'enregistrement, **Annuler** revient toujours sur ce que vous avez fait auparavant, y compris les lignes et les colonnes insérées (le prochain enregistrement les retire de nouveau). Ce qu'il ne peut pas rendre, ce sont les lignes, colonnes ou cellules que vous avez **supprimées** : le fichier enregistré a perdu ce qu'elles contenaient. **Annuler** s'arrête donc à la dernière suppression enregistrée, et la ligne en bas de la fenêtre le signale lorsque vous enregistrez.
- Une étape qui déplace ou supprime tant de cellules que **Annuler** ne peut pas la conserver (plus d'environ 128 Mo) demande d'abord confirmation, comme le font les tableurs connus : « Cette opération ne peut pas être annulée. » Si vous continuez, elle est exécutée, et **Annuler** ne revient ni sur elle ni sur ce qui la précède.
