# Suivi des modifications, et acceptation ou refus des modifications
<!-- keywords: comparer, comparer des documents, deux versions, ce qui a changé, suivi des modifications, modifications suivies, révisions, balisage, marques, accepter, refuser, tout accepter, tout refuser, afficher pour la révision, balisage simple, tous les balisages, aucun balisage, original, afficher les marques, bulles, volet Révisions, barre de modification, insertion, suppression, ctrl+maj+e, auteur, réviseur, nom d'utilisateur, modifications d'une autre personne -->

**Suivi des modifications** conserve une trace de chaque modification que vous faites, afin que vous (ou une autre personne) puissiez voir ce qui a changé et décider de le conserver ou non. Utilisez-le lorsque vous modifiez un document qui appartient à quelqu'un d'autre, ou qu'une autre personne relira après vous.

## Activer et désactiver le suivi des modifications

Cliquez sur **Révision > Suivi des modifications**, ou appuyez sur **Ctrl+Maj+E**. La barre d'état, en bas de la fenêtre, indique si le suivi est activé. Un clic sur ces mots inverse le réglage. Ce réglage est enregistré dans le document : il reste donc activé lorsque le document vous revient après avoir été relu par quelqu'un d'autre.

Lorsque le suivi est activé :

- **Ce que vous saisissez** est marqué comme inséré.
- **Ce que vous supprimez** n'est pas effacé. Il reste dans le texte, barré, afin de pouvoir être rétabli. Le texte que vous avez saisi vous-même pendant que le suivi était activé fait exception : le supprimer le retire simplement, puisque personne d'autre ne l'a jamais vu.
- **Un saut de paragraphe que vous insérez** (Entrée) est marqué comme inséré, et celui que vous supprimez (Retour arrière en début de paragraphe) est marqué comme supprimé.
- **Un changement de mise en forme** (gras, police, taille, couleur, alignement, retraits ou style) est enregistré avec l'aspect que le texte avait auparavant.

Chaque modification est enregistrée avec son auteur et sa date. Le nom est celui qui figure dans **Fichier > Options**, sous **Nom d'utilisateur**. Modifiez-le là si nécessaire.

Chaque modification constitue une étape, donc **Ctrl+Z** l'annule comme toute autre modification.

## À quoi ressemble une modification

Par défaut, les modifications de chaque personne ont une couleur qui lui est propre. La première personne dont la modification apparaît dans le document reçoit la première couleur, la suivante la deuxième couleur, et ainsi de suite ; une personne conserve la même couleur tant que le document reste ouvert. Le texte inséré est souligné dans la couleur de la personne, et le texte supprimé est barré. Le texte déplacé est en vert : doublement barré à l'endroit d'où il est parti, et doublement souligné à l'endroit où il est arrivé. Le texte dont la mise en forme a été modifiée est en gras. Une barre dans la marge de gauche signale chaque ligne contenant une modification.

Vous pouvez modifier tout cela. Cliquez sur **Révision > Suivi > Options de suivi** pour ouvrir la boîte de dialogue **Options du suivi des modifications**. Sous **Balisage**, chaque type de modification possède un **Aspect** et une **Couleur** :

- **Insertions** : soulignement, double soulignement, barré, gras, italique, couleur uniquement ou (aucun).
- **Suppressions** : barré, double barré, masqué, accent circonflexe ou couleur uniquement. Avec **Masquée**, les mots supprimés ne figurent pas dans la ligne. Avec **Accent circonflexe**, ils sont également omis, et un petit ^ indique leur emplacement. Le texte déplacé est lui aussi omis lorsque vous choisissez **Bulles** pour les suppressions, mais pas dans les autres cas.
- **Déplacé de** et **Déplacé vers** : les aspects et couleurs du texte déplacé. Ces options sont disponibles tant que **Suivre les déplacements** est coché. Si vous le décochez, le texte déplacé apparaît comme une suppression ordinaire et une insertion ordinaire.
- **Mise en forme** : gras, couleur uniquement ou (aucun). Si vous décochez **Suivre la mise en forme**, les changements de mise en forme ne sont pas marqués dans le texte.
- **Barres de modification** : bordure gauche, bordure droite, bordure extérieure (le côté droit des pages impaires et le côté gauche des pages paires) ou (aucune), ainsi que la couleur de la barre.

Chaque zone **Couleur** propose **Par auteur** (la couleur propre à chaque personne) ou une seule couleur pour tout le monde. Ces choix vous appartiennent. Ils sont conservés sur cet ordinateur, et non dans le document ; ils s'appliquent à tous les documents et à toutes les fenêtres ouvertes, et chaque fenêtre se met à jour dès que vous cliquez sur **OK**. Les barres sont dessinées de la même façon à l'écran, sur une impression et dans un PDF, chaque fois que le balisage est affiché.

## Choisir ce que vous voyez : Afficher pour la révision

Cliquez sur **Révision > Afficher pour la révision**, puis choisissez :

- **Tous les balisages** affiche chaque modification dans le texte, avec les barres dans la marge.
- **Balisage simple** affiche le document tel qu'il apparaîtra une fois les modifications acceptées, avec une barre dans la marge en regard de chaque ligne contenant une modification.
- **Aucun balisage** affiche le document tel qu'il apparaîtra une fois toutes les modifications acceptées, sans rien d'autre.
- **Original** affiche le document tel qu'il était avant toute modification : le texte supprimé réapparaît et le texte inséré disparaît.

Ces options modifient seulement ce que vous voyez. Elles ne changent jamais le document et ne sont pas enregistrées avec lui. Un document contenant des modifications s'ouvre avec **Tous les balisages**.

**Révision > Afficher les marques** choisit les types de modifications affichés : **Insertions et suppressions**, **Mise en forme** et **Commentaires**. En dessous, choisissez comment afficher le texte supprimé. **Afficher toutes les révisions dans le texte** le conserve dans le texte, barré. **Afficher les révisions dans des bulles** le retire de la ligne et le place dans une bulle dans la marge. **Afficher uniquement les commentaires et la mise en forme dans des bulles** conserve les insertions et suppressions dans le texte.

## Accepter ou refuser des modifications

Accepter une modification, c'est la conserver définitivement. La refuser, c'est l'annuler.

- **Révision > Accepter** accepte la modification où se trouve le point d'insertion (ou chaque modification du texte sélectionné), puis passe à la suivante. **Révision > Refuser** fait la même chose en annulant la modification.
- Cliquez sur la flèche sous **Accepter** ou **Refuser** pour plus d'options : **Accepter toutes les modifications affichées**, **Accepter toutes les modifications** et **Accepter toutes les modifications et arrêter le suivi** ; ainsi que **Refuser toutes les modifications affichées** et **Refuser toutes les modifications**. « Affichées » désigne les modifications des types activés dans **Afficher les marques** (et, dans le volet **Révisions**, le réviseur que vous avez choisi).
- **Précédente** et **Suivante**, dans le groupe **Modifications**, passent d'une modification à la suivante sans rien changer.

Chacune de ces actions constitue une étape, donc **Ctrl+Z** rétablit le document exactement tel qu'il était.

Accepter une suppression efface définitivement le texte. Refuser une insertion supprime aussi le texte. Accepter une insertion conserve le texte et retire la marque indiquant qu'il est nouveau. Refuser une suppression conserve le texte et retire le barré. Refuser un changement de mise en forme rétablit l'aspect que le texte avait. Le texte déplacé forme un couple de modifications, l'une à l'endroit où il est parti et l'autre à l'endroit où il est arrivé. Lorsque le couple est marqué par le même nom, comme le font d'autres logiciels à l'enregistrement, accepter ou refuser l'une des moitiés produit le même effet sur l'autre : le texte est déplacé définitivement, ou remis à sa place.

## Le volet Révisions

Cliquez sur **Révision > Volet Révisions** pour ouvrir une liste des modifications du document : leur type, leur auteur, leur date et les mots concernés. Un clic sur l'une d'elles l'affiche dans le texte. Utilisez **Accepter**, **Refuser**, **Précédente** et **Suivante** sous la liste, ou choisissez un seul réviseur dans la zone située au-dessus pour ne voir que ses modifications.

## Ce qui n'est pas encore suivi

Les modifications apportées aux tableaux (lignes, cellules, fusions), à la numérotation et aux listes, aux sections et à la mise en page, ainsi qu'au texte des en-têtes, pieds de page et notes, sont effectuées normalement, sans être enregistrées. Les modifications enregistrées par d'autres logiciels à ces emplacements sont conservées et enregistrées telles quelles.

## Comparer deux versions

La commande Comparer recherche les différences entre deux versions d'un document et les affiche sous forme de modifications suivies.

1. Ouvrez la version la plus ancienne. C'est l'**original**.
2. Cliquez sur **Comparer** dans le groupe **Comparer** de l'onglet **Révision**.
3. Cliquez sur **Parcourir** et choisissez la version la plus récente. C'est la version **révisée**.
4. Saisissez le nom sous lequel signer les modifications. Choisissez les options souhaitées, par exemple ignorer les différences de majuscules. **Rechercher le texte déplacé** et **Rechercher les modifications de mise en forme** sont activées par défaut, sauf si vous les désactivez.
5. Cliquez sur **OK**.

Une nouvelle fenêtre s'ouvre, avec le texte d'origine et les modifications qui le transforment en texte révisé. Vos deux fichiers ne sont pas modifiés. Utilisez **Accepter** et **Refuser** comme pour toute modification suivie. Si vous acceptez toutes les modifications, vous obtenez le texte révisé ; si vous les refusez toutes, vous obtenez le texte d'origine.

La comparaison porte sur le texte du corps du document. Elle n'examine pas les tableaux, en-têtes, pieds de page ni notes de bas de page.

Un paragraphe de 12 mots ou plus, coupé à un endroit et placé à un autre, est affiché comme un **déplacement** : un enregistrement à l'endroit d'où le texte est parti, et un autre à l'endroit où il est arrivé. Accepter ou refuser l'un de ces deux enregistrements touche les deux. Le paragraphe doit être identique, ou au moins identique à 9 mots sur 10 et dans l'essentiel du même ordre ; sinon il est affiché comme une suppression et une insertion. Les mots qui diffèrent à l'intérieur d'un paragraphe déplacé sont affichés comme des insertions et suppressions à l'intérieur du déplacement.

Un texte identique dans les deux versions mais d'aspect différent est affiché comme une **modification de mise en forme**. La comparaison vérifie le gras, l'italique, le soulignement, la taille, la police, la couleur, la surbrillance, ainsi que le style, l'alignement, les retraits et l'espacement d'un paragraphe. Refuser la modification rétablit l'ancien aspect.

Si les deux documents comportent trop de modifications, la comparaison le signale. Elle affiche alors certains paragraphes comme remplacés en bloc, et peut ne pas rechercher les déplacements ni les modifications de mise en forme dans tous les paragraphes.
