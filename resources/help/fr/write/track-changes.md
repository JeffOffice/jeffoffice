# Suivi des modifications, et acceptation ou refus des modifications
<!-- keywords: comparer, comparer des documents, deux versions, ce qui a changé, suivi des modifications, modifications suivies, révisions, marques, accepter, refuser, accepter tout, refuser tout, afficher pour la révision, marquage simple, tous les marquages, aucun marquage, original, afficher les marques, bulles, volet Révision, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

**Suivi des modifications** conserve une trace de chaque modification que vous effectuez, afin que vous ou une autre personne puissiez voir ce qui a changé et décider de la conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou que quelqu'un relira après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique si le suivi est activé. Un clic sur ces mots inverse l'état. Ce réglage est enregistré dans le document : il reste donc activé lorsque le document vous revient après avoir été relu par quelqu'un d'autre.

Lorsque le suivi est activé :

- **Ce que vous tapez** est marqué comme une insertion.
- **Ce que vous supprimez** n'est pas effacé. Il reste dans le texte, barré, afin de pouvoir être rétabli. Le texte que vous avez vous-même tapé pendant le suivi constitue la seule exception : le supprimer le retire simplement, car personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous ajoutez** (touche Entrée) est marqué comme inséré, et celui que vous supprimez (touche Retour arrière en début de paragraphe) est marqué comme supprimé.
- **Un changement de mise en forme** (gras, police, taille, couleur, alignement, retraits ou style) est enregistré avec l'aspect que le texte avait auparavant.

Chaque modification est enregistrée avec le nom de son auteur et la date. Ce nom est celui indiqué dans **Fichier > Options**, sous **Nom d'utilisateur**. Modifiez-le à cet endroit s'il est incorrect.

Chaque modification correspond à une seule étape : **Ctrl+Z** l'annule comme toute autre modification.

## À quoi ressemble une modification

Par défaut, les modifications de chaque personne ont une couleur qui lui est propre. La première personne dont la modification apparaît dans le document reçoit la première couleur, la suivante la deuxième couleur, et ainsi de suite. Une personne conserve la même couleur tant que le document reste ouvert. Le texte inséré est souligné dans la couleur de la personne, et le texte supprimé est barré. Le texte déplacé est vert : doublement barré à l'endroit où il a été retiré, et doublement souligné à l'endroit où il a été placé. Le texte dont la mise en forme a été modifiée est en gras. Une barre dans la marge de gauche signale chaque ligne contenant une modification.

Vous pouvez modifier tout cela. Cliquez sur **Révision > Suivi > Options de suivi** pour ouvrir la boîte de dialogue **Options du suivi des modifications**. Sous **Marquage**, chaque type de modification possède un **Aspect** et une **Couleur** :

- **Insertions** : souligné, double soulignement, barré, gras, italique, couleur uniquement, ou (aucun).
- **Suppressions** : barré, double barré, masqué, accent circonflexe, ou couleur uniquement. Avec **Masquée**, les mots supprimés ne sont pas affichés dans la ligne. Avec **Accent circonflexe**, ils ne le sont pas non plus, mais un petit ^ indique leur emplacement. Le texte déplacé vers un autre endroit est également masqué lorsque vous choisissez **Bulles** pour les suppressions, mais pas dans les autres cas.
- **Déplacé de** et **Déplacé vers** : les aspects et les couleurs du texte déplacé. Ces options sont disponibles tant que **Suivre les déplacements** est coché. Si vous le décochez, le texte déplacé apparaît comme une suppression ordinaire et une insertion ordinaire.
- **Mise en forme** : gras, couleur uniquement, ou (aucun). Si vous décochez **Suivre la mise en forme**, les changements de mise en forme ne sont pas signalés dans le texte.
- **Barres de modification** : bordure gauche, bordure droite, bordure extérieure (côté droit des pages impaires et côté gauche des pages paires), ou (aucune), ainsi que la couleur de la barre.

Chaque zone **Couleur** propose **Par auteur** (la couleur propre à chaque personne) ou une couleur unique pour tous. Ces choix vous sont personnels. Ils sont conservés sur cet ordinateur, et non dans le document ; ils s'appliquent à tous les documents et à toutes les fenêtres ouvertes, et chaque fenêtre est mise à jour dès que vous cliquez sur **OK**. Les barres sont dessinées de la même manière à l'écran, à l'impression et dans un PDF, chaque fois que les marques sont affichées.

## Choisir ce que vous voyez : Afficher pour la révision

Cliquez sur **Révision > Afficher pour la révision**, puis choisissez l'une des options :

- **Tous les marquages** affiche toutes les modifications dans le texte, avec les barres dans la marge.
- **Marquage simple** affiche le document tel qu'il apparaîtra une fois les modifications acceptées, avec une barre dans la marge en regard de chaque ligne contenant une modification.
- **Aucun marquage** affiche le document tel qu'il apparaîtra une fois toutes les modifications acceptées, sans autre indication.
- **Original** affiche le document tel qu'il était avant toute modification : le texte supprimé réapparaît, et le texte inséré disparaît.

Ces options modifient seulement ce que vous voyez. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document contenant des modifications s'ouvre avec **Tous les marquages**.

**Révision > Afficher les marques** détermine les types de modifications affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. Plus bas, choisissez la manière d'afficher le texte supprimé. **Afficher toutes les révisions dans le texte** le conserve dans le texte, barré. **Afficher les révisions dans des bulles** retire le texte supprimé de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** conserve les insertions et les suppressions dans le texte.

## Accepter ou refuser des modifications

Accepter une modification, c'est la conserver définitivement. La refuser, c'est l'annuler.

- **Révision > Accepter** accepte la modification où se trouve le point d'insertion (ou toutes les modifications du texte sélectionné), puis passe à la suivante. **Révision > Refuser** agit de la même façon, en annulant la modification.
- Cliquez sur la flèche sous **Accepter** ou **Refuser** pour afficher d'autres options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications**, et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher les marques** (et, dans le volet **Révision**, le réviseur que vous avez choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, passent d'une modification à l'autre sans rien changer.

Chacune de ces actions correspond à une seule étape : **Ctrl+Z** rétablit exactement le document tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion efface également le texte. Accepter une insertion conserve le texte et retire simplement la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser un changement de mise en forme rétablit l'aspect que le texte avait. Le texte déplacé constitue une paire de modifications : l'une à l'endroit où il a été retiré, l'autre à l'endroit où il a été placé, reliées par le nom du déplacement. Lorsque la paire est marquée par le même nom, comme le font d'autres programmes lors de l'enregistrement, accepter ou refuser l'une des deux moitiés a le même effet sur l'autre : le texte est définitivement déplacé ou remis à sa place. Un nom utilisé plusieurs fois, ou qui n'apparaît qu'une seule fois, ne relie rien : chaque moitié constitue alors une modification distincte.

## Le volet Révision

Cliquez sur **Révision > Volet Révisions** pour ouvrir la liste des modifications du document : type, auteur, date et texte concerné. Un clic sur l'une d'elles l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un seul réviseur dans la zone située au-dessus pour n'afficher que ses modifications.

## Modifications dans les en-têtes, les pieds de page, les notes et les tableaux

Les modifications apportées par d'autres personnes dans un en-tête, un pied de page, une note de bas de page ou une note de fin sont affichées et listées comme celles du texte, y compris un en-tête ou un pied de page qu'aucune page n'utilise (celui des pages paires lorsque le document ne distingue pas les pages paires et impaires). **Suivante** passe par-dessus, et **Accepter** et **Refuser** traitent la modification à laquelle vous vous êtes arrêté en dernier. **Suivante**, **Précédente**, **Accepter**, **Refuser** et le volet **Révision** vous conduisent dans l'en-tête, le pied de page ou la note qui contient la modification. L'ordre est le suivant : d'abord le texte, puis les notes de bas de page, puis les en-têtes et pieds de page. **Accepter toutes les modifications** et **Refuser toutes les modifications** s'appliquent à l'ensemble de ces éléments.

Les modifications apportées aux tableaux sont également listées, dans le texte comme dans les en-têtes et pieds de page : une ligne ou une cellule insérée ou supprimée, des cellules fusionnées, et un changement d'aspect d'un tableau, d'une ligne ou d'une cellule. Une barre dans la marge signale les lignes de la ligne ou de la cellule concernée. Accepter une ligne supprimée, ou refuser une ligne insérée, retire la ligne et son contenu. Une ligne dont toutes les cellules portent la modification constitue la modification de cette ligne. Refuser un changement d'aspect rétablit l'aspect qu'avaient le tableau, la ligne ou la cellule, et refuser une fusion sépare de nouveau les cellules. Lorsque retirer une modification désorganiserait le tableau, la modification est conservée à la place, et le message indique combien ont été conservées. C'est le cas d'une cellule fusionnée avec d'autres en dessous, qui reste en place et perd sa marque, et d'une modification de la grille que les lignes ne peuvent pas suivre, qui reste sous forme de modification que vous pouvez accepter. Les modifications effectuées dans un tableau situé dans une note de bas de page sont conservées telles qu'elles ont été écrites : elles ne sont pas listées, et Accepter et Refuser n'agissent pas sur elles.

Un déplacement occupe une seule ligne dans la liste et compte pour une seule modification dans le total qu'affichent Accepter toutes les modifications et Refuser toutes les modifications : sa seconde moitié et les marques de paragraphe qui l'accompagnaient sont traitées avec lui. Les modifications contenues dans des zones de texte ne sont pas listées une à une. Le volet **Révision** indique leur nombre (par exemple, « 3 autres modifications se trouvent dans des zones de texte ») ; **Accepter toutes les modifications** et **Refuser toutes les modifications** les traitent avec le reste.

## Ce qui n'est pas encore suivi

Les modifications que vous effectuez vous-même sur les tableaux (lignes, cellules, fusion), sur la numérotation et les listes, sur les sections et la mise en page, ainsi que sur le texte des en-têtes, des pieds de page et des notes, sont réalisées normalement, sans être enregistrées. Les modifications de numérotation et de sections enregistrées par d'autres programmes sont conservées et enregistrées telles quelles, mais elles ne sont pas listées, et Accepter et Refuser n'agissent pas sur elles.

## Comparer deux versions

Comparer recherche les différences entre deux versions d'un document et les affiche sous forme de modifications suivies.

1. Ouvrez la version la plus ancienne. C'est l'**original**.
2. Cliquez sur **Comparer** dans le groupe **Comparer** de l'onglet **Révision**.
3. Cliquez sur **Parcourir** et choisissez la version la plus récente. C'est la version **révisée**.
4. Saisissez le nom sous lequel signer les modifications. Choisissez les options souhaitées, par exemple ignorer les différences de majuscules. **Rechercher le texte déplacé** et **Rechercher les modifications de mise en forme** sont activés, sauf si vous les décochez.
5. Cliquez sur **OK**.

Une nouvelle fenêtre s'ouvre, présentant le texte d'origine et les modifications qui le transforment en texte révisé. Vos deux fichiers ne sont pas modifiés. Utilisez **Accepter** et **Refuser** comme pour toute modification suivie. Si vous acceptez toutes les modifications, vous obtenez le texte révisé. Si vous les refusez toutes, vous obtenez le texte d'origine.

La comparaison porte sur le texte du corps du document. Elle n'examine ni les tableaux, ni les en-têtes, ni les pieds de page, ni les notes de bas de page.

Un paragraphe de 12 mots ou plus, coupé à un endroit et placé à un autre, est présenté comme un **déplacement** : un enregistrement à l'endroit où le texte a été retiré, et un autre à l'endroit où il a été placé. Accepter ou refuser l'un de ces enregistrements agit sur les deux. Le paragraphe doit être identique, ou comporter au moins 9 mots sur 10 identiques, en grande partie dans le même ordre ; sinon, il est présenté comme une suppression et une insertion. Les mots qui diffèrent à l'intérieur d'un paragraphe déplacé sont présentés comme des insertions et des suppressions à l'intérieur du déplacement.

Un texte identique dans les deux versions, mais d'aspect différent, est présenté comme un **changement de mise en forme**. La comparaison vérifie le gras, l'italique, le soulignement, la taille, la police, la couleur, la surbrillance, ainsi que le style, l'alignement, les retraits et l'espacement d'un paragraphe. Refuser la modification rétablit l'ancien aspect.

Si les deux documents comportent trop de modifications, la comparaison l'indique. Elle présente alors certains paragraphes comme entièrement remplacés, et peut ne pas rechercher les déplacements ni les changements de mise en forme dans tous les paragraphes.
