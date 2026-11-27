# Suivi des modifications : accepter ou refuser les modifications
<!-- keywords: comparer, comparer des documents, deux versions, ce qui a changé, suivi des modifications, modifications suivies, révisions, balisage, accepter, refuser, accepter tout, refuser tout, afficher pour la révision, balisage simple, tous les balisages, aucun balisage, original, afficher le balisage, bulles, volet révision, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

**Suivi des modifications** conserve une trace de chaque modification que vous apportez, afin que vous (ou quelqu'un d'autre) puissiez voir ce qui a changé et décider de la conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou qu'une autre personne doit relire après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique si le suivi est activé. Un clic sur ces mots inverse l'état. Ce réglage est enregistré dans le document : il reste donc activé lorsque le document vous revient après avoir été relu par quelqu'un d'autre.

Lorsque le suivi est activé :

- **Ce que vous tapez** est marqué comme une insertion.
- **Ce que vous supprimez** n'est pas effacé. Il reste dans le texte, barré, afin de pouvoir être rétabli. Le texte que vous avez tapé vous-même pendant que le suivi était activé fait exception : le supprimer l'efface simplement, car personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous ajoutez** (Entrée) est marqué comme inséré, et celui que vous supprimez (Retour arrière en début de paragraphe) est marqué comme supprimé.
- **Une modification de présentation** (gras, police, taille, couleur, alignement, retraits ou style) est enregistrée avec l'apparence qu'avait le texte auparavant.

Chaque modification est enregistrée avec son auteur et sa date. Le nom est celui qui figure dans **Fichier > Options**, sous **Nom d'utilisateur**. Modifiez-le ici s'il est erroné.

Chaque modification correspond à une étape. **Ctrl+Z** l'annule donc comme toute autre modification.

## À quoi ressemble une modification

Par défaut, les modifications de chaque personne ont une couleur qui lui est propre. La première personne dont la modification apparaît dans le document reçoit la première couleur, la suivante la deuxième couleur, et ainsi de suite ; une personne conserve la même couleur tant que le document reste ouvert. Le texte inséré est souligné dans la couleur de la personne, et le texte supprimé est barré. Le texte déplacé est en vert : doublement barré à l'endroit où il était, et doublement souligné à son nouvel emplacement. Le texte dont la mise en forme a été modifiée est en gras. Une barre dans la marge gauche signale chaque ligne contenant une modification.

Vous pouvez modifier tout cela. Cliquez sur **Révision > Suivi > Options de suivi** pour ouvrir la boîte **Options du suivi des modifications**. Sous **Balisage**, chaque type de modification dispose d'un **Aspect** et d'une **Couleur** :

- **Insertions** : soulignement, double soulignement, barré, gras, italique, couleur uniquement, ou (aucun).
- **Suppressions** : barré, double barré, masqué, accent circonflexe ou couleur uniquement. Avec **Masqué**, les mots supprimés sont retirés de la ligne. Avec **Accent circonflexe**, ils sont également retirés, et un petit ^ indique leur emplacement. Le texte déplacé est aussi retiré lorsque vous choisissez **Bulles** pour les suppressions, mais pas dans les autres cas.
- **Déplacé de** et **Déplacé vers** : l'aspect et les couleurs du texte déplacé. Ces options sont disponibles tant que **Suivre les déplacements** est coché. Si vous le décochez, le texte déplacé apparaît comme une suppression ordinaire et une insertion ordinaire.
- **Mise en forme** : gras, couleur uniquement, ou (aucun). Si vous décochez **Suivre la mise en forme**, les changements de mise en forme ne sont pas marqués dans le texte.
- **Barres de modification** : bordure gauche, bordure droite, bordure extérieure (le côté droit des pages impaires et le côté gauche des pages paires), ou (aucune), ainsi que la couleur de la barre.

Chaque zone **Couleur** propose **Par auteur** (la couleur propre à chaque personne) ou une couleur unique pour tout le monde. Ces choix vous sont personnels. Ils sont conservés sur cet ordinateur, et non dans le document ; ils s'appliquent à tous les documents et à toutes les fenêtres ouvertes, et chaque fenêtre est mise à jour dès que vous cliquez sur **OK**. Les barres sont dessinées de la même façon à l'écran, sur une impression et dans un PDF, dès que le balisage est affiché.

## Choisir ce que vous voyez : Afficher pour la révision

Cliquez sur **Révision > Afficher pour la révision**, puis choisissez :

- **Tous les balisages** affiche chaque modification dans le texte, avec les barres dans la marge.
- **Balisage simple** affiche le document tel qu'il apparaîtra une fois les modifications acceptées, avec une barre dans la marge en regard de chaque ligne contenant une modification.
- **Aucun balisage** affiche le document tel qu'il apparaîtra une fois toutes les modifications acceptées, sans rien d'autre.
- **Original** affiche le document tel qu'il était avant toute modification : le texte supprimé réapparaît, et le texte inséré disparaît.

Ces options modifient uniquement l'affichage. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document contenant des modifications s'ouvre avec **Tous les balisages**.

**Révision > Afficher le balisage** choisit les types de modifications affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. En dessous, choisissez l'affichage du texte supprimé. **Afficher toutes les révisions dans le texte** le conserve dans le texte, barré. **Afficher les révisions dans des bulles** retire le texte supprimé de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** conserve les insertions et suppressions dans le texte.

## Accepter ou refuser des modifications

Accepter une modification, c'est la conserver définitivement. La refuser, c'est l'annuler.

- **Révision > Accepter** accepte la modification où se trouve le point d'insertion (ou toutes les modifications du texte sélectionné), puis passe à la suivante. **Révision > Refuser** fait de même en annulant la modification.
- Cliquez sur la flèche sous **Accepter** ou **Refuser** pour afficher davantage d'options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications** et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher le balisage** (et, dans le volet **Révision**, le réviseur que vous avez choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, passent d'une modification à l'autre sans rien changer. Lorsque le point d'insertion se trouve tout au début du texte, d'un en-tête ou d'une note, **Suivante** va à une modification qui commence à cet endroit même.

Chacune de ces commandes correspond à une étape. **Ctrl+Z** rétablit donc le document exactement tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion efface aussi le texte. Accepter une insertion conserve le texte et retire seulement la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser une modification de présentation rétablit l'apparence qu'avait le texte. Le texte déplacé constitue une paire de modifications : l'une à l'endroit où il était, l'autre à son nouvel emplacement, reliées par le nom du déplacement. Lorsque la paire porte le même nom, comme l'enregistrent d'autres programmes, accepter ou refuser l'une des deux moitiés fait de même pour l'autre moitié : le texte est alors déplacé définitivement ou remis à sa place. Un nom utilisé plusieurs fois, ou qui n'apparaît que sur une seule moitié, ne relie rien : chaque moitié est une modification distincte.

## Le volet Révision

Cliquez sur **Révision > Volet Révisions** pour ouvrir la liste des modifications du document : type, auteur, date et texte. Un clic sur l'une d'elles l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un réviseur dans la zone située au-dessus pour ne voir que ses modifications.

## Modifications dans les en-têtes, pieds de page, notes et tableaux

Les modifications faites par d'autres personnes dans un en-tête, un pied de page, une note de bas de page ou une note de fin sont affichées et listées comme celles du texte, y compris un en-tête ou un pied de page qu'aucune page n'utilise (celui des pages paires lorsque le document ne distingue pas les pages paires et impaires) : **Suivante** le dépasse, et **Accepter** et **Refuser** traitent la modification à laquelle vous êtes arrivé en dernier à cet endroit. **Suivante**, **Précédente**, **Accepter**, **Refuser** et le volet **Révisions** vous conduisent dans l'en-tête, le pied de page ou la note qui contient la modification. L'ordre est le texte d'abord, puis les notes de bas de page, puis les en-têtes et pieds de page. **Accepter toutes les modifications** et **Refuser toutes les modifications** s'appliquent à l'ensemble de ces éléments.

Les modifications apportées aux tableaux sont également listées, dans le texte comme dans les en-têtes et pieds de page : ligne ou cellule insérée ou supprimée, cellules fusionnées, et modification de l'aspect d'un tableau, d'une ligne ou d'une cellule. Une barre dans la marge signale les lignes de la ligne ou de la cellule concernée. Accepter une ligne supprimée, ou refuser une ligne insérée, retire la ligne et son contenu. Une ligne dont toutes les cellules portent la modification constitue la modification de cette ligne. Refuser une modification d'aspect rétablit l'apparence qu'avaient le tableau, la ligne ou la cellule, et refuser une fusion sépare de nouveau les cellules. Lorsqu'annuler une modification casserait le tableau, la modification est conservée à la place, et le message indique combien de modifications ont été conservées. Cela se produit pour une cellule fusionnée avec d'autres vers le bas, qui reste et perd sa marque, et pour une modification de la grille que les lignes ne peuvent pas suivre, qui reste sous forme de modification que vous pouvez accepter. Les modifications contenues dans un tableau situé dans une note de bas de page sont conservées telles qu'elles ont été écrites : elles ne sont pas listées, et Accepter et Refuser n'agissent pas sur elles.

Un déplacement occupe une seule ligne dans la liste et compte pour une seule modification dans le total donné par Accepter toutes les modifications et Refuser toutes les modifications : sa seconde moitié et les marques de paragraphe qui l'accompagnaient sont traitées avec lui. Les modifications contenues dans une zone de texte sont listées une à une, après celles du texte et avant celles des notes de bas de page. **Suivante** et **Précédente** ouvrent la zone pour la saisie, avec la modification sélectionnée, et **Accepter** et **Refuser** règlent cette modification, chacune en une seule étape. Déplacer le point d'insertion à l'intérieur de la zone ouverte fait repartir **Suivante** et **Précédente** à partir de ce point. Une zone réglée modification par modification est enregistrée sans la copie que lisent les anciens programmes, comme lorsque vous tapez dans une zone. Les 200 premières zones contenant des modifications sont listées de cette façon. Une zone de texte faisant partie d'un groupe, ou dont le texte se poursuit dans d'autres zones, une zone située dans une autre zone, une zone dans un en-tête ou une note, ainsi que les zones au-delà des 200 premières, ne peuvent pas être ouvertes de cette façon : le volet **Révisions** indique leurs modifications sur une ligne sous la liste, et seules **Accepter toutes les modifications** et **Refuser toutes les modifications** les traitent.

Une modification de la numérotation d'un paragraphe est listée sous **Numérotation ajoutée** ou **Numérotation modifiée**. Lorsqu'une modification de l'aspect d'un paragraphe a enregistré la liste et le niveau qu'il avait, la refuser replace le paragraphe dans cette liste, à ce niveau (ou le retire de la liste dans laquelle il n'était pas). Une modification qui conserve seulement l'ancienne numérotation sous forme d'étiquette (comme le font certains programmes) ne désigne aucune liste à rétablir : l'accepter conserve la numérotation, et la refuser ne supprime la numérotation que si l'étiquette est vide ; sinon, elle laisse la modification en place et la compte comme conservée. Refuser une numérotation ajoutée rend au paragraphe ce que lui donne son style (un style de titre qui comporte une numérotation la conserve), et aucune numérotation là où le style n'en a pas. Si la liste que la modification devrait rétablir ne figure plus dans le document, rien ne change pour ce paragraphe, et la modification est comptée comme conservée.

Une modification de section (format de page, orientation, marges, colonnes, numérotation des pages ou type de saut) est listée sous **Section mise en forme**, au paragraphe qui termine la section, ou à la fin du texte pour la dernière section. Accepter la modification conserve la section telle quelle et supprime l'enregistrement ; la refuser rétablit les propriétés conservées par l'enregistrement, et les pages sont mises en page de nouveau immédiatement. Les en-têtes et pieds de page nommés par une section ne changent pas. Si les anciennes propriétés de l'enregistrement ne peuvent pas être lues, refuser laisse la section et la modification en l'état et la compte comme conservée ; rien n'est remplacé par une page par défaut. Lorsque Accepter toutes les modifications ou Refuser toutes les modifications supprime un saut de section ou en rétablit un, la modification de chaque section est d'abord réglée, afin qu'aucune ne subsiste.

Une modification qui ne peut pas être réglée sans endommager le document (un tableau qui se casserait, une liste ou une section dont l'ancien état ne peut pas être rétabli, une zone de texte trop grande pour être réécrite) est laissée telle qu'elle est, et le message affiché après **Accepter toutes les modifications** ou **Refuser toutes les modifications** indique combien ont été conservées.

## Modifications enregistrées ou non

Les modifications suivantes sont enregistrées lorsque le suivi est activé : saisie, suppression et collage de texte ; Entrée ; remplacement de mots ; l'aspect du texte et des paragraphes (polices, tailles, couleurs, alignement, espacement, bordures et styles) ; Modifier la casse ; les symboles et caractères spéciaux ; les sauts de page et de colonne ; le marque de paragraphe d'un saut de section ; et, dans les tableaux, une ligne que vous insérez (avec **Insérer au-dessus** ou **Insérer ci-dessous**, ou avec Tab dans la dernière cellule), une ligne que vous supprimez, un tableau que vous supprimez et un tableau que vous insérez. Une ligne insérée ou supprimée figure dans le volet **Révisions** comme une seule modification, avec son texte ; **Accepter** et **Refuser** la conservent ou la retirent. Supprimer une ligne contenant un tableau marque aussi les lignes de ce tableau. Une ligne insérée par quelqu'un d'autre et supprimée par vous porte les deux modifications ; **Accepter** ou **Refuser** la retire. Une ligne que vous avez insérée vous-même disparaît définitivement lorsque vous la supprimez.

Les modifications suivantes sont effectuées normalement, sans marque, et la barre d'état affiche à chaque fois « Cette modification n'a pas été enregistrée comme modification suivie. » : puces, numérotation et autres commandes de liste ; **Augmenter le retrait** et **Diminuer le retrait** ; mise en page, colonnes, marges et numéros de ligne ; en-têtes et pieds de page, ainsi que la saisie, la suppression et le remplacement de mots dans un en-tête, un pied de page, une note de bas de page ou une note de fin ; l'insertion d'une note de bas de page, d'une note de fin, d'une table des matières, d'un index, d'un champ, d'un lien ou d'un contrôle de contenu ; les images, graphiques, équations, zones de texte et formes dessinées ; les colonnes, cellules, fusions, fractionnements et l'aspect d'un tableau ; la suppression d'une ligne faisant partie d'une cellule fusionnée vers le bas à travers plusieurs lignes ; le tri des paragraphes ou des lignes d'un tableau ; et le déplacement d'un tableau ou d'une section. Lorsque vous tapez dans un en-tête ou une note, le message n'apparaît qu'une fois, et non à chaque touche. **Annuler** annule l'une de ces opérations en une seule étape.

Certaines modifications ne sont pas effectuées du tout lorsque le suivi est activé, car sans marque elles endommageraient une modification enregistrée : dans un tableau contenant une ligne insérée ou supprimée, **Supprimer les colonnes**, **Supprimer les cellules**, **Fusionner les cellules**, **Fractionner les cellules**, **Convertir en texte** et le collage sur des cellules. La barre d'état l'indique. Acceptez ou refusez d'abord les modifications de ce tableau, ou désactivez le suivi des modifications. Dans un document qui verrouille le suivi des modifications de façon à ne pas permettre d'accepter ou de refuser, ces opérations sont tout de même effectuées, et la barre d'état indique que la modification n'a pas été enregistrée.

Les modifications enregistrées par d'autres programmes dans la numérotation et les sections sont listées et peuvent être acceptées ou refusées, comme décrit ci-dessus.

## Comparer deux versions

La commande Comparer recherche les différences entre deux versions d'un document et les affiche sous forme de modifications suivies.

1. Ouvrez la version la plus ancienne. C'est l'**original**.
2. Cliquez sur **Comparer** dans le groupe **Comparer** de l'onglet **Révision**.
3. Cliquez sur **Parcourir** et choisissez la version la plus récente. C'est la version **révisée**.
4. Saisissez le nom sous lequel signer les modifications. Choisissez les options souhaitées, par exemple ignorer les majuscules. **Rechercher le texte qui a été déplacé** et **Rechercher les modifications de mise en forme** sont activées, sauf si vous les désactivez.
5. Cliquez sur **OK**.

Une nouvelle fenêtre s'ouvre, avec le texte original et les modifications qui le transforment en texte révisé. Vos deux fichiers ne sont pas modifiés. Utilisez **Accepter** et **Refuser** comme pour toute modification suivie. Si vous acceptez toutes les modifications, vous obtenez le texte révisé ; si vous les refusez toutes, vous obtenez le texte original.

La comparaison porte sur le texte du corps du document. Elle n'examine pas les tableaux, les en-têtes, les pieds de page ni les notes de bas de page.

Un paragraphe de 12 mots ou plus, coupé d'un endroit pour être placé ailleurs, est affiché comme un **déplacement** : un enregistrement à l'endroit où le texte était, et un autre à son nouvel emplacement. Accepter ou refuser l'un des deux traite les deux. Le paragraphe doit être identique, ou contenir au moins 9 mots sur 10 identiques et placés globalement dans le même ordre ; sinon, il est affiché comme une suppression et une insertion. Les mots qui diffèrent à l'intérieur d'un paragraphe déplacé sont affichés comme insertions et suppressions à l'intérieur du déplacement.

Un texte identique dans les deux versions mais présenté différemment est affiché comme une **modification de mise en forme**. La comparaison vérifie le gras, l'italique, le soulignement, la taille, la police, la couleur, la surbrillance, ainsi que le style, l'alignement, les retraits et l'espacement d'un paragraphe. Refuser la modification rétablit l'ancienne présentation.

Si les deux documents comportent trop de modifications, la comparaison le signale. Elle affiche alors certains paragraphes comme remplacés en bloc, et il se peut qu'elle ne recherche pas les déplacements ni les modifications de mise en forme dans tous les paragraphes.
