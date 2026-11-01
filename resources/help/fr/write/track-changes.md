# Suivre les modifications, puis les accepter ou les refuser
<!-- keywords: suivi des modifications, modifications suivies, révisions, marques, accepter, refuser, tout accepter, tout refuser, afficher pour la révision, marques simples, toutes les marques, aucune marque, original, afficher les marques, bulles, volet révisions, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

Le **Suivi des modifications** conserve une trace de chaque modification que vous apportez, afin que vous (ou une autre personne) puissiez voir ce qui a changé et décider de le conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou qu'une autre personne relira après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique s'il est activé. Un clic sur cette mention l'active ou le désactive. Ce paramètre est enregistré dans le document : il est donc toujours activé lorsque le document vous revient d'une autre personne.

Lorsqu'il est activé :

- **Ce que vous tapez** est marqué comme une insertion.
- **Ce que vous supprimez** n'est pas effacé. Le texte reste en place, barré, afin de pouvoir être rétabli. Seule exception : le texte que vous avez tapé vous-même pendant le suivi. Le supprimer l'efface simplement, car personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous créez** (Entrée) est marqué comme inséré, et un saut que vous supprimez (Retour arrière au début d'un paragraphe) est marqué comme supprimé.
- **Une modification d'apparence** (gras, police, taille, couleur, alignement, retraits ou style) est enregistrée avec l'apparence précédente du texte.

Chaque modification est enregistrée avec son auteur et sa date. Le nom utilisé est celui indiqué dans **Fichier > Options**, sous **Nom d'utilisateur**. Corrigez-le à cet endroit s'il est erroné.

Chaque modification constitue une étape : **Ctrl+Z** l'annule donc comme n'importe quelle autre modification.

## Apparence d'une modification

Les modifications de chaque personne ont leur propre couleur. Le texte inséré est souligné dans la couleur de la personne, et le texte supprimé est barré. Une barre dans la marge de gauche signale chaque ligne qui contient une modification.

## Choisir ce que vous voyez : Afficher pour la révision

Cliquez sur **Révision > Afficher pour la révision** et choisissez une option :

- **Toutes les marques** affiche toutes les modifications dans le texte, avec les barres dans la marge.
- **Marques simples** affiche le document tel qu'il sera une fois les modifications acceptées, avec une barre dans la marge en regard de chaque ligne modifiée.
- **Aucune marque** affiche le document tel qu'il sera une fois toutes les modifications acceptées, sans rien d'autre.
- **Original** affiche le document tel qu'il était avant toute modification : le texte supprimé réapparaît et le texte inséré disparaît.

Ces options ne modifient que l'affichage. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document qui contient des modifications s'ouvre avec **Toutes les marques**.

**Révision > Afficher les marques** permet de choisir les types de modifications affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. En dessous, choisissez comment afficher le texte supprimé. **Afficher toutes les révisions dans le texte** le laisse dans le texte, barré. **Afficher les révisions dans des bulles** retire le texte supprimé de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** laisse les insertions et les suppressions dans le texte.

## Accepter ou refuser des modifications

Accepter une modification revient à la conserver définitivement. La refuser revient à l'annuler.

- **Révision > Accepter** accepte la modification dans laquelle se trouve le curseur (ou toutes les modifications du texte sélectionné), puis passe à la suivante. **Révision > Refuser** fait de même, en annulant la modification.
- Cliquez sur la flèche sous **Accepter** ou **Refuser** pour d'autres options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications** et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher les marques** (et, dans le volet **Révisions**, celles du réviseur choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, permettent de passer d'une modification à l'autre sans rien changer.

Chacune de ces actions constitue une étape : **Ctrl+Z** rétablit donc le document exactement tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion efface également le texte. Accepter une insertion conserve le texte et retire la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser une modification d'apparence rétablit l'apparence d'origine du texte.

## Le volet Révisions

Cliquez sur **Révision > Volet Révisions** pour ouvrir la liste des modifications du document : leur type, leur auteur, leur date et le texte concerné. Un clic sur une modification l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un réviseur dans la zone située au-dessus pour n'afficher que ses modifications.

## Ce qui n'est pas encore suivi

Les modifications apportées aux tableaux (lignes, cellules, fusion), à la numérotation et aux listes, aux sections et à la mise en page, ainsi qu'au texte des en-têtes, des pieds de page et des notes, s'effectuent normalement, sans être enregistrées. Les modifications enregistrées par d'autres programmes à ces emplacements sont conservées et enregistrées telles quelles.
