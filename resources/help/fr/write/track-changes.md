# Suivi des modifications, et acceptation ou refus des modifications
<!-- keywords: comparer, comparer des documents, deux versions, ce qui a changé, suivi des modifications, modifications suivies, révisions, balisage, accepter, refuser, tout accepter, tout refuser, afficher pour la révision, balisage simple, tous les balisages, aucun balisage, original, afficher le balisage, bulles, volet révisions, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

Le **suivi des modifications** conserve une trace de chaque modification que vous faites, afin que vous (ou quelqu'un d'autre) puissiez voir ce qui a changé et décider de la conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou que quelqu'un relira après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique si le suivi est activé. Un clic sur ces mots inverse l'état. Ce réglage est enregistré dans le document : il reste activé lorsque le document revient de quelqu'un d'autre.

Lorsque le suivi est activé :

- **Ce que vous saisissez** est marqué comme une insertion.
- **Ce que vous supprimez** n'est pas effacé. Il reste dans le texte, barré, afin de pouvoir être rétabli. Le texte que vous avez saisi vous-même pendant que le suivi était activé fait exception : le supprimer l'efface simplement, car personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous ajoutez** (Entrée) est marqué comme inséré, et celui que vous supprimez (Retour arrière en début de paragraphe) est marqué comme supprimé.
- **Un changement de présentation** (gras, police, taille, couleur, alignement, retraits ou style) est enregistré avec l'aspect que le texte avait auparavant.

Chaque modification est enregistrée avec son auteur et sa date. Le nom est celui qui figure dans **Fichier > Options**, sous **Nom d'utilisateur**. Modifiez-le là si nécessaire.

Chaque modification correspond à une seule étape : **Ctrl+Z** l'annule comme toute autre modification.

## À quoi ressemble une modification

Par défaut, les modifications de chaque personne ont une couleur propre. La première personne dont le document affiche une modification reçoit la première couleur, la suivante la deuxième couleur, et ainsi de suite ; une personne garde la même couleur tant que le document reste ouvert. Le texte inséré est souligné dans la couleur de la personne, et le texte supprimé est barré. Le texte déplacé est en vert : doublement barré à l'endroit où il était, et doublement souligné à l'endroit où il est arrivé. Le texte dont la mise en forme a été modifiée est en gras. Une barre dans la marge gauche signale chaque ligne qui contient une modification.

Vous pouvez modifier tout cela. Cliquez sur **Révision > Suivi > Options de suivi** pour ouvrir la boîte **Options de suivi des modifications**. Sous **Balisage**, chaque type de modification possède un **Aspect** et une **Couleur** :

- **Insertions** : soulignement, double soulignement, barré, gras, italique, couleur uniquement, ou (aucun).
- **Suppressions** : barré, double barré, masquée, accent circonflexe ou couleur uniquement. Avec **Masquée**, les mots supprimés ne figurent pas dans la ligne. Avec **Accent circonflexe**, ils n'y figurent pas non plus, et un petit ^ indique où ils se trouvaient. Le texte déplacé est également masqué lorsque vous choisissez **Bulles** pour les suppressions, mais pas dans le cas contraire.
- **Déplacé de** et **Déplacé vers** : les aspects et couleurs du texte déplacé. Ces options sont disponibles lorsque **Suivre les déplacements** est coché. Si vous le décochez, le texte déplacé apparaît comme une suppression ordinaire et une insertion ordinaire.
- **Mise en forme** : gras, couleur uniquement, ou (aucun). Si vous décochez **Suivre la mise en forme**, les changements de mise en forme ne sont pas signalés dans le texte.
- **Barres de modification** : bordure gauche, bordure droite, bordure extérieure (le côté droit des pages impaires et le côté gauche des pages paires), ou (aucun), ainsi que la couleur de la barre.

Chaque zone **Couleur** propose **Par auteur** (la couleur propre à chaque personne) ou une couleur unique pour tout le monde. Ces choix vous sont personnels. Ils sont conservés sur cet ordinateur, et non dans le document ; ils s'appliquent à tous les documents et à toutes les fenêtres ouvertes, et chaque fenêtre change dès que vous cliquez sur **OK**. Les barres sont dessinées de la même façon à l'écran, sur une impression et dans un PDF, chaque fois que le balisage est affiché.

## Choisir ce que vous voyez : Affichage pour la révision

Cliquez sur **Révision > Affichage pour la révision**, puis choisissez :

- **Tous les balisages** affiche toutes les modifications dans le texte, avec les barres dans la marge.
- **Balisage simple** affiche le document tel qu'il sera une fois les modifications acceptées, avec une barre dans la marge en regard de chaque ligne qui contient une modification.
- **Aucun balisage** affiche le document tel qu'il sera une fois toutes les modifications acceptées, sans rien d'autre.
- **Original** affiche le document tel qu'il était avant toutes les modifications : le texte supprimé réapparaît, et le texte inséré disparaît.

Ces options modifient uniquement ce que vous voyez. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document contenant des modifications s'ouvre avec **Tous les balisages**.

**Révision > Afficher le balisage** choisit les types de modifications affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. En dessous, choisissez l'affichage du texte supprimé. **Afficher toutes les révisions dans le texte** conserve le texte supprimé dans le texte, barré. **Afficher les révisions dans des bulles** retire le texte supprimé de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** conserve les insertions et suppressions dans le texte.

## Accepter ou refuser les modifications

Accepter une modification, c'est la conserver définitivement. La refuser, c'est l'annuler.

- **Révision > Accepter** accepte la modification où se trouve le point d'insertion (ou chaque modification du texte sélectionné), puis passe à la suivante. **Révision > Refuser** fait de même en annulant la modification.
- Cliquez sur la flèche située sous **Accepter** ou **Refuser** pour plus d'options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications** et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher le balisage** (et, dans le volet **Révisions**, du réviseur choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, passent d'une modification à la suivante sans rien changer.

Chacune de ces actions correspond à une seule étape : **Ctrl+Z** rétablit le document exactement tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion efface aussi le texte. Accepter une insertion conserve le texte et retire la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser un changement de présentation rétablit la présentation d'origine du texte. Le texte déplacé forme un couple de modifications : l'une à l'endroit où il était, l'autre à l'endroit où il est arrivé. Lorsque le couple est marqué par le même nom, comme le font d'autres logiciels à l'enregistrement, accepter ou refuser l'une des moitiés agit de la même façon sur l'autre : le texte est déplacé définitivement, ou remis à sa place.

## Le volet Révisions

Cliquez sur **Révision > Volet Révisions** pour ouvrir la liste des modifications du document : leur type, leur auteur, leur date et les mots concernés. Un clic sur une modification l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un seul réviseur dans la zone située au-dessus pour ne voir que ses modifications.

## Ce qui n'est pas encore suivi

Les modifications apportées aux tableaux (lignes, cellules, fusions), à la numérotation et aux listes, aux sections et à la mise en page, ainsi qu'au texte des en-têtes, pieds de page et notes, sont effectuées normalement, sans être enregistrées. Les modifications enregistrées par d'autres logiciels à ces endroits sont conservées et enregistrées telles quelles.

## Comparer deux versions

Comparer recherche les différences entre deux versions d'un document et les affiche sous forme de modifications suivies.

1. Ouvrez la version la plus ancienne. Elle est l'**original**.
2. Cliquez sur **Comparer** dans le groupe **Comparer** de l'onglet **Révision**.
3. Cliquez sur **Parcourir** et choisissez la version la plus récente. Elle est la version **révisée**.
4. Saisissez le nom sous lequel signer les modifications. Choisissez les options souhaitées, comme ignorer les différences de majuscules.
5. Cliquez sur **OK**.

Une nouvelle fenêtre s'ouvre, avec le texte original et les modifications qui le transforment en texte révisé. Vos deux fichiers ne sont pas modifiés. Utilisez **Accepter** et **Refuser** comme pour toute modification suivie. Si vous acceptez toutes les modifications, vous obtenez le texte révisé. Si vous les refusez toutes, vous obtenez le texte original.

Comparer examine le texte du corps du document. Il n'examine ni les tableaux, ni les en-têtes, ni les pieds de page, ni les notes, ni l'aspect du texte. Un changement de présentation seul n'est pas détecté.
