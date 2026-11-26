# Annuler, rétablir et répéter
<!-- keywords: annuler, rétablir, répéter, f4, ctrl+y, historique, revenir en arrière, refaire, dernière action, étape -->

**{key:Undo}** annule votre dernière modification, et **{key:Redo}** la rétablit. Les deux boutons se trouvent dans la barre d'outils rapide, en haut. Chaque commande que vous lancez compte pour une étape, et une suite de mots saisis compte pour une seule étape.

## Revenir en arrière de plusieurs étapes à la fois

Chacun des boutons **Annuler** et **Rétablir** est accompagné d'une flèche. Cliquez sur la flèche pour afficher les étapes, de la plus récente à la plus ancienne, avec jusqu'à 100 étapes listées. Une étape porte le nom de la commande qui l'a produite, par exemple « Gras » ou « Saisie », et « Modification » lorsque la modification vient d'ailleurs. Déplacez le pointeur vers le bas de la liste pour indiquer le nombre d'étapes voulues, puis cliquez pour les effectuer. Si vous choisissez la troisième étape, les trois premières sont annulées. Les étapes annulées restent dans la liste **Rétablir** jusqu'à ce que vous fassiez une nouvelle modification.

## Répéter la dernière commande

**F4** relance votre dernière commande sur la sélection actuelle. Par exemple, mettez un mot en gras, sélectionnez un autre mot, puis appuyez sur **F4**. Le second mot passe en gras, et un seul **Annuler** le ramène à son état précédent. Une répétition constitue toujours une étape à part entière, même juste après une saisie.

Les commandes qui activent ou désactivent un effet, comme le gras, l'italique, le soulignement, les puces et la numérotation, sont répétées sous la forme du résultat obtenu, et non comme une bascule. Si vous avez mis un mot en gras, **F4** met en gras la sélection suivante, même si une partie de celle-ci est déjà en gras, et une sélection déjà entièrement en gras reste inchangée. Si vous avez retiré le gras d'un mot, **F4** retire le gras de la sélection suivante.

La répétition fonctionne pour :

- La mise en forme : gras, italique, soulignement, barré, exposant, indice, police, taille de police, couleur de police, surlignage, trame de texte, modification de la casse et effacement de la mise en forme.
- Les paramètres de paragraphe : alignement, retraits, espacements, puces, numérotation, bordures et trame.
- Les styles, depuis la galerie, ainsi que Normal et les styles de titre.
- Les commandes de tableau qui insèrent, suppriment, fusionnent, alignent ou définissent des bordures.
- Les sauts de page et les pages vierges, ainsi que les caractères spéciaux insérés à l'aide d'une commande.
- La saisie : après avoir tapé quelques mots, **F4** saisit de nouveau les mêmes mots à l'emplacement du point d'insertion. Seuls les mots d'une même suite sont conservés, jusqu'à 1 000 lettres, et une suite se termine dès que vous déplacez le point d'insertion. La suite correspond au texte tel qu'il a été obtenu après les corrections, par exemple celles de la correction automatique, de sorte qu'une répétition insère ces mots tels quels. La touche Retour arrière raccourcit la suite des lettres qu'elle supprime.

La répétition ne fonctionne pas pour l'ouverture et l'enregistrement, le changement d'affichage, la recherche et le remplacement, le copier-coller, ni pour les commandes qui ouvrent une fenêtre pour vous poser une question. Ces commandes ne sont jamais considérées comme la « dernière commande » pour la répétition : **F4** répète donc toujours la mise en forme effectuée avant elles.

L'info-bulle du bouton de la barre d'outils rapide indique ce que la commande ferait, par exemple « Répéter Gras (F4) ». La répétition suit les mêmes règles que la commande qu'elle répète : lorsque le suivi des modifications est activé, la répétition est enregistrée comme une modification suivie. Dans un document ouvert en lecture seule, ou protégé, elle ne fait rien et la barre d'état indique la raison. Chaque document a sa propre dernière commande : la répétition ne transporte donc jamais une commande d'un document à un autre.

La répétition fonctionne là où vous êtes en train de modifier. Dans une zone de texte ou une forme, elle tape dans la zone et exécute les commandes que la zone propose ; une commande que la zone ne propose pas n'est pas répétée, et la barre d'état l'indique. Dans un en-tête, un pied de page, une note ou une cellule de tableau, elle agit à cet endroit.

**Ctrl+Y** correspond à **Rétablir** lorsqu'il reste des étapes à rétablir. Lorsqu'il n'y en a plus, **Ctrl+Y** répète comme **F4**.
