# Suivi des modifications, et acceptation ou refus des modifications
<!-- keywords: comparer, comparer des documents, deux versions, ce qui a changé, suivi des modifications, modifications suivies, révisions, marques, accepter, refuser, accepter tout, refuser tout, afficher pour la révision, marquage simple, tous les marquages, aucun marquage, original, afficher les marques, bulles, volet Révisions, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

**Suivi des modifications** conserve la trace de chaque modification que vous faites, afin que vous ou quelqu'un d'autre puissiez voir ce qui a changé et décider de la conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou qu'une autre personne doit relire après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique si le suivi est activé. Un clic sur ces mots inverse le réglage. Ce réglage est enregistré dans le document : il reste donc activé lorsque le document vous revient de quelqu'un d'autre.

Lorsqu'il est activé :

- **Ce que vous tapez** est marqué comme une insertion.
- **Ce que vous supprimez** n'est pas effacé. Il reste dans le texte, barré, afin de pouvoir être rétabli. Le texte que vous avez tapé vous-même pendant que le suivi était activé fait exception : le supprimer l'enlève simplement, puisque personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous ajoutez** (Entrée) est marqué comme inséré, et un saut que vous supprimez (Retour arrière en début de paragraphe) est marqué comme supprimé.
- **Un changement de mise en forme** (gras, police, taille, couleur, alignement, retraits ou style) est enregistré avec l'apparence que le texte avait auparavant.

Chaque modification est enregistrée avec le nom de son auteur et la date. Le nom est celui indiqué dans **Fichier > Options**, sous **Nom d'utilisateur**. Modifiez-le ici s'il est incorrect.

Chaque modification constitue une seule étape : **Ctrl+Z** l'annule comme toute autre modification.

## À quoi ressemble une modification

Par défaut, les modifications de chaque personne ont une couleur propre. La première personne dont la modification apparaît dans le document reçoit la première couleur, la suivante la deuxième couleur, et ainsi de suite ; une personne garde la même couleur tant que le document reste ouvert. Le texte inséré est souligné dans la couleur de son auteur, et le texte supprimé est barré. Le texte déplacé est vert : doublement barré à l'endroit où il a été retiré, et doublement souligné à l'endroit où il a été placé. Le texte dont la mise en forme a été modifiée est en gras. Une barre dans la marge de gauche signale chaque ligne contenant une modification.

Vous pouvez modifier tout cela. Cliquez sur **Révision > Suivi > Options de suivi** pour ouvrir la boîte de dialogue **Options du suivi des modifications**. Sous **Marquage**, chaque type de modification possède un **Aspect** et une **Couleur** :

- **Insertions** : souligné, double soulignement, barré, gras, italique, couleur uniquement, ou (aucun).
- **Suppressions** : barré, double barré, masqué, accent circonflexe, ou couleur uniquement. Avec **Masqué**, les mots supprimés sont retirés de la ligne. Avec **Accent circonflexe**, ils sont également retirés, et un petit ^ indique leur emplacement. Le texte déplacé est aussi retiré lorsque vous choisissez **Bulles** pour les suppressions, mais pas autrement.
- **Déplacé de** et **Déplacé vers** : l'aspect et la couleur du texte déplacé. Ces options sont disponibles tant que **Suivre les déplacements** est coché. Si vous le décochez, le texte déplacé apparaît comme une suppression ordinaire et une insertion ordinaire.
- **Mise en forme** : gras, couleur uniquement, ou (aucun). Si vous décochez **Suivre la mise en forme**, les changements de mise en forme ne sont pas marqués dans le texte.
- **Barres de modification** : bordure gauche, bordure droite, bordure extérieure (le côté droit des pages impaires et le côté gauche des pages paires), ou (aucune), ainsi que la couleur de la barre.

Chaque zone **Couleur** propose **Par auteur** (la couleur propre à chaque personne) ou une couleur unique pour tous. Ces choix vous sont propres. Ils sont conservés sur cet ordinateur, et non dans le document ; ils s'appliquent à tous les documents et à toutes les fenêtres ouvertes, et chaque fenêtre change dès que vous cliquez sur **OK**. Les barres sont dessinées de la même manière à l'écran, à l'impression et dans un PDF, dès que les marques sont affichées.

## Choisir ce que vous voyez : Afficher pour la révision

Cliquez sur **Révision > Afficher pour la révision**, puis choisissez :

- **Tous les marquages** affiche toutes les modifications dans le texte, avec les barres dans la marge.
- **Marquage simple** affiche le document tel qu'il apparaîtra une fois les modifications acceptées, avec une barre dans la marge en regard de chaque ligne contenant une modification.
- **Aucun marquage** affiche le document tel qu'il apparaîtra une fois toutes les modifications acceptées, sans rien d'autre.
- **Original** affiche le document tel qu'il était avant toutes les modifications : le texte supprimé réapparaît, et le texte inséré disparaît.

Ces options ne modifient que ce que vous voyez. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document contenant des modifications s'ouvre avec **Tous les marquages**.

**Révision > Afficher les marques** choisit les types de modification affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. En dessous, choisissez la manière d'afficher le texte supprimé. **Afficher toutes les révisions dans le texte** le conserve dans le texte, barré. **Afficher les révisions dans des bulles** retire le texte supprimé de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** conserve les insertions et suppressions dans le texte.

## Accepter ou refuser des modifications

Accepter une modification, c'est la conserver définitivement. La refuser, c'est l'annuler.

- **Révision > Accepter** accepte la modification où se trouve le point d'insertion (ou chaque modification du texte sélectionné), puis passe à la suivante. **Révision > Refuser** fait de même, en annulant la modification.
- Cliquez sur la flèche sous **Accepter** ou **Refuser** pour plus d'options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications** et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher les marques** (et, dans le volet **Révisions**, le réviseur que vous avez choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, passent d'une modification à l'autre sans rien changer. Lorsque le point d'insertion se trouve tout au début du texte, d'un en-tête ou d'une note, **Suivante** va à une modification qui commence à cet endroit.

Chacune de ces actions constitue une seule étape : **Ctrl+Z** rétablit le document exactement tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion supprime également le texte. Accepter une insertion conserve le texte et supprime la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser un changement de mise en forme rétablit l'apparence qu'avait le texte. Le texte déplacé constitue une paire de modifications : une à l'endroit où il a été retiré, une à l'endroit où il a été placé, reliées par le nom du déplacement. Lorsque la paire porte le même nom, comme l'enregistrent d'autres logiciels, accepter ou refuser l'une des deux moitiés fait de même pour l'autre, de sorte que le texte est déplacé définitivement ou remis à sa place. Un nom utilisé plusieurs fois, ou n'ayant qu'une seule moitié, ne relie rien : chaque moitié est alors une modification distincte.

## Le volet Révisions

Cliquez sur **Révision > Volet Révisions** pour ouvrir la liste des modifications du document : leur type, leur auteur, leur date et les mots concernés. Un clic sur l'une d'elles l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un seul réviseur dans la liste placée au-dessus pour ne voir que ses modifications.

## Modifications dans les en-têtes, les pieds de page, les notes et les tableaux

Les modifications apportées par d'autres personnes dans un en-tête, un pied de page, une note de bas de page ou une note de fin sont affichées et listées comme celles du texte, y compris un en-tête ou un pied de page qu'aucune page n'utilise (celui des pages paires lorsque le document n'applique pas des pages paires et impaires différentes) : **Suivante** passe au-delà, et **Accepter** et **Refuser** traitent la modification à laquelle vous êtes parvenu en dernier à cet endroit. **Suivante**, **Précédente**, **Accepter**, **Refuser** et le volet **Révisions** vous conduisent dans l'en-tête, le pied de page ou la note qui contient la modification ; l'ordre est d'abord le texte, puis les notes de bas de page, puis les en-têtes et pieds de page. **Accepter tout** et **Refuser tout** s'appliquent à tous ces éléments.

Les modifications apportées aux tableaux sont également listées, dans le texte, les en-têtes et les pieds de page : une ligne ou une cellule insérée ou supprimée, des cellules fusionnées, et un changement d'aspect d'un tableau, d'une ligne ou d'une cellule. Une barre dans la marge signale les lignes de la ligne ou de la cellule concernée. Accepter une ligne supprimée, ou refuser une ligne insérée, retire la ligne et son contenu. Une ligne dont chaque cellule porte la modification constitue la modification de cette ligne. Refuser un changement d'aspect rétablit l'aspect qu'avaient le tableau, la ligne ou la cellule, et refuser une fusion sépare de nouveau les cellules. Lorsque retirer une modification détruirait le tableau, la modification est conservée et le message indique combien ont été conservées. C'est le cas d'une cellule reliée à d'autres vers le bas, qui reste et perd sa marque, et d'un changement de la grille que les lignes ne peuvent pas suivre, qui reste sous forme de modification que vous pouvez accepter. Les modifications d'un tableau situé dans une note de bas de page sont conservées telles qu'elles ont été écrites : elles ne sont pas listées, et Accepter et Refuser n'agissent pas sur elles.

Un déplacement occupe une seule ligne de la liste, et une seule modification dans le décompte que donnent Accepter tout et Refuser tout : sa seconde moitié et les marques de paragraphe qui l'accompagnaient sont traitées avec lui. Les modifications contenues dans une zone de texte sont listées une à une, après les modifications du texte et avant celles des notes de bas de page. **Suivante** et **Précédente** ouvrent la zone pour la saisie, avec la modification sélectionnée, et **Accepter** et **Refuser** règlent cette modification, chacune comme une seule étape. Déplacer le point d'insertion dans la zone ouverte fait repartir **Suivante** et **Précédente** à partir de ce point. Une zone réglée modification par modification est enregistrée sans la copie que lisent les anciens logiciels, comme lorsque vous tapez dans une zone. Les 200 premières zones contenant des modifications sont listées ainsi. Une zone de texte faisant partie d'un groupe, ou dont le texte se poursuit dans d'autres zones, une zone placée dans une autre zone, une zone située dans un en-tête ou une note, ainsi que les zones au-delà des 200 premières ne peuvent pas être ouvertes de cette manière : le volet **Révisions** indique leur nombre de modifications sur une ligne sous la liste, et seuls **Accepter tout** et **Refuser tout** les traitent.

Une modification de la numérotation d'un paragraphe est listée sous le nom **Numérotée** ou **Numérotation modifiée**. Lorsqu'un changement d'aspect d'un paragraphe a enregistré la liste et le niveau qu'il avait, le refuser replace le paragraphe dans cette liste, à ce niveau (ou le retire de la liste dans laquelle il ne se trouvait pas). Une modification qui conserve seulement l'ancienne numérotation sous forme d'étiquette (comme le font certains logiciels) ne désigne aucune liste à rétablir : l'accepter conserve la numérotation, et la refuser ne supprime la numérotation que si l'étiquette est vide ; sinon, elle laisse la modification en place et la compte comme conservée. Refuser une numérotation ajoutée rend au paragraphe ce que lui donne son style (un style de titre qui comporte une numérotation la conserve), et aucune numérotation là où le style n'en a pas. Si la liste que la modification devrait rétablir n'existe plus dans le document, rien ne change pour ce paragraphe, et la modification est comptée comme conservée.

Une modification d'une section (format de page, orientation, marges, colonnes, numérotation des pages ou type de saut) est listée sous le nom **Section mise en forme**, au paragraphe qui termine la section, ou à la fin du texte pour la dernière section. L'accepter conserve la section telle quelle et supprime l'enregistrement de la modification ; la refuser rétablit les propriétés conservées dans cet enregistrement, et les pages sont mises en forme à nouveau immédiatement. Les en-têtes et pieds de page qu'une section désigne ne changent pas. Si les anciennes propriétés de l'enregistrement ne peuvent pas être lues, refuser laisse la section et la modification en l'état, et la compte comme conservée ; rien n'est remplacé par une page par défaut. Lorsque Accepter tout ou Refuser tout supprime ou rétablit aussi un saut de section, la modification de chaque section est d'abord réglée, afin qu'aucune ne subsiste.

Une modification qui ne peut pas être réglée sans endommager le document (un tableau qui serait détruit, une liste ou une section dont l'ancien état ne peut pas être rétabli, une zone de texte trop grande pour être réécrite) est laissée telle qu'elle est écrite, et le message affiché après **Accepter tout** ou **Refuser tout** indique combien ont été conservées.

## Ce qui n'est pas encore suivi

Les modifications que vous apportez vous-même aux tableaux (lignes, cellules, fusion), à la numérotation et aux listes, aux sections et à la mise en page, ainsi qu'au texte des en-têtes, pieds de page et notes, sont effectuées normalement, sans enregistrement. Les modifications de numérotation et de sections enregistrées par d'autres logiciels sont listées et peuvent être acceptées ou refusées, comme décrit ci-dessus.

## Comparer deux versions

Comparer recherche les différences entre deux versions d'un document et les affiche sous forme de modifications suivies.

1. Ouvrez la version la plus ancienne. C'est l'**original**.
2. Cliquez sur **Comparer** dans le groupe **Comparer** de l'onglet **Révision**.
3. Cliquez sur **Parcourir** et choisissez la version la plus récente. C'est la version **révisée**.
4. Saisissez le nom sous lequel signer les modifications. Choisissez les options souhaitées, comme ignorer les majuscules. **Rechercher le texte déplacé** et **Rechercher les modifications de mise en forme** sont activées par défaut, sauf si vous les désactivez.
5. Cliquez sur **OK**.

Une nouvelle fenêtre s'ouvre avec le texte original et les modifications qui le transforment en texte révisé. Vos deux fichiers ne sont pas modifiés. Utilisez **Accepter** et **Refuser** comme pour toute modification suivie. Si vous acceptez toutes les modifications, vous obtenez le texte révisé. Si vous les refusez toutes, vous obtenez le texte original.

La comparaison porte sur le texte du corps du document. Elle ne examine pas les tableaux, les en-têtes, les pieds de page ni les notes de bas de page.

Un paragraphe de 12 mots ou plus, retiré d'un endroit et placé dans un autre, est affiché comme un **déplacement** : un enregistrement à l'endroit où le texte a été retiré, et un autre à l'endroit où il a été placé. Accepter ou refuser l'un de ces enregistrements règle les deux. Le paragraphe doit être identique, ou comporter au moins 9 mots sur 10 identiques et, pour la plupart, dans le même ordre ; sinon il est affiché comme une suppression et une insertion. Les mots qui diffèrent à l'intérieur d'un paragraphe déplacé sont affichés comme des insertions et des suppressions à l'intérieur du déplacement.

Un texte identique dans les deux versions mais d'aspect différent est affiché comme un **changement de mise en forme**. La comparaison vérifie le gras, l'italique, le soulignement, la taille, la police, la couleur, la surbrillance, ainsi que le style, l'alignement, les retraits et l'espacement d'un paragraphe. Refuser la modification rétablit l'ancien aspect.

Si les deux documents comportent trop de modifications, la comparaison l'indique. Elle affiche alors certains paragraphes comme remplacés en bloc, et peut ne pas rechercher les déplacements ni les changements de mise en forme dans tous les paragraphes.
