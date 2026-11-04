# Suivi des modifications : accepter ou refuser les modifications
<!-- keywords: comparer, comparer des documents, deux versions, ce qui a changé, suivi des modifications, modifications suivies, révisions, marques, accepter, refuser, tout accepter, tout refuser, afficher pour la révision, marques simples, toutes les marques, aucune marque, original, afficher les marques, bulles, volet Révisions, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

Le **suivi des modifications** conserve une trace de chaque modification que vous faites, afin que vous (ou une autre personne) puissiez voir ce qui a changé et décider de la conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou qu'une autre personne doit relire après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique si le suivi est activé. Un clic sur ces mots inverse le réglage. Le réglage est enregistré dans le document : il reste activé lorsque le document revient d'une autre personne.

Lorsque le suivi est activé :

- **Ce que vous saisissez** est marqué comme une insertion.
- **Ce que vous supprimez** n'est pas effacé. Il reste dans le texte, barré, afin de pouvoir être rétabli. Le texte que vous avez saisi vous-même pendant que le suivi était activé constitue la seule exception : le supprimer l'efface simplement, puisque personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous ajoutez** (Entrée) est marqué comme inséré, et celui que vous supprimez (Retour arrière en début de paragraphe) est marqué comme supprimé.
- **Une modification de mise en forme** (gras, police, taille, couleur, alignement, retraits ou style) est enregistrée avec l'aspect que le texte avait auparavant.

Chaque modification est enregistrée avec son auteur et sa date. Le nom est celui indiqué dans **Fichier > Options**, sous **Nom d'utilisateur**. Modifiez-le ici s'il est incorrect.

Chaque modification correspond à une étape. **Ctrl+Z** l'annule donc comme toute autre modification.

## À quoi ressemble une modification

Les modifications de chaque personne ont une couleur qui leur est propre. Le texte inséré est souligné dans la couleur de la personne, et le texte supprimé est barré. Une barre dans la marge de gauche signale chaque ligne qui comporte une modification.

## Choisir ce que vous voyez : Afficher pour la révision

Cliquez sur **Révision > Afficher pour la révision**, puis choisissez :

- **Toutes les marques** affiche toutes les modifications dans le texte, avec les barres dans la marge.
- **Marques simples** affiche le document tel qu'il apparaîtra une fois les modifications acceptées, ainsi qu'une barre dans la marge en regard de chaque ligne qui comporte une modification.
- **Aucune marque** affiche le document tel qu'il apparaîtra une fois toutes les modifications acceptées, sans rien d'autre.
- **Original** affiche le document tel qu'il était avant toute modification : le texte supprimé réapparaît, et le texte inséré disparaît.

Ces options ne modifient que ce que vous voyez. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document comportant des modifications s'ouvre avec **Toutes les marques**.

**Révision > Afficher les marques** choisit les types de modifications affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. En dessous, choisissez l'affichage du texte supprimé. **Afficher toutes les révisions dans le texte** conserve le texte supprimé dans le texte, barré. **Afficher les révisions dans des bulles** retire le texte supprimé de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** conserve les insertions et suppressions dans le texte.

## Accepter ou refuser les modifications

Accepter une modification, c'est la conserver définitivement. La refuser, c'est l'annuler.

- **Révision > Accepter** accepte la modification où se trouve le point d'insertion (ou toutes les modifications du texte sélectionné), puis passe à la suivante. **Révision > Refuser** fait de même, en annulant la modification.
- Cliquez sur la flèche située sous **Accepter** ou **Refuser** pour plus d'options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications** et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher les marques** (et, dans le volet **Révisions**, du réviseur choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, permettent de passer d'une modification à l'autre sans rien changer.

Chacune de ces actions correspond à une étape. **Ctrl+Z** rétablit donc le document exactement tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion efface également le texte. Accepter une insertion conserve le texte et supprime la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser une modification de mise en forme rétablit l'aspect que le texte avait.

## Le volet Révisions

Cliquez sur **Révision > Volet Révisions** pour afficher la liste des modifications du document : leur type, leur auteur, leur date et les mots concernés. Un clic sur une modification l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un seul réviseur dans la zone située au-dessus pour ne voir que ses modifications.

## Ce qui n'est pas encore suivi

Les modifications apportées aux tableaux (lignes, cellules, fusions), à la numérotation et aux listes, aux sections et à la mise en page, ainsi qu'au texte des en-têtes, des pieds de page et des notes, sont effectuées normalement, sans être enregistrées. Les modifications que d'autres programmes ont enregistrées à ces emplacements sont conservées et enregistrées telles quelles.

## Comparer deux versions

La commande Comparer recherche les différences entre deux versions d'un document et les affiche sous forme de modifications suivies.

1. Ouvrez la version la plus ancienne. Elle est l'**original**.
2. Cliquez sur **Comparer** dans le groupe **Comparer** de l'onglet **Révision**.
3. Cliquez sur **Parcourir** et choisissez la version la plus récente. Elle est la version **révisée**.
4. Saisissez le nom sous lequel signer les modifications. Choisissez les options souhaitées, par exemple ignorer les différences de majuscules.
5. Cliquez sur **OK**.

Une nouvelle fenêtre s'ouvre, contenant le texte original ainsi que les modifications qui le transforment en texte révisé. Vos deux fichiers ne sont pas modifiés. Utilisez **Accepter** et **Refuser** comme pour toute modification suivie. Si vous acceptez toutes les modifications, vous obtenez le texte révisé. Si vous les refusez toutes, vous obtenez le texte original.

La comparaison porte sur le texte du corps du document. Elle ne prend pas en compte les tableaux, les en-têtes, les pieds de page, les notes, ni l'aspect du texte. Un changement de mise en forme seul n'est pas détecté.
