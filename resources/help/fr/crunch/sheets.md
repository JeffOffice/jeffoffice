# Gérer les feuilles
<!-- keywords: feuille, feuilles, onglet, onglets, ajouter, insérer, nouvelle feuille, supprimer, effacer, renommer, nom, déplacer, copier, dupliquer, réorganiser, glisser, masquer, afficher, démasquer, couleur d'onglet, couleur, sélectionner plusieurs feuilles, Maj+F11, #REF!, référence 3D, Déplacer ou copier -->

Un classeur est composé de **feuilles**, et les **onglets** situés en bas de la fenêtre portent leur nom. Cliquez sur un onglet pour afficher cette feuille. Chaque modification décrite ci-dessous correspond à une étape que vous pouvez annuler, et elle est enregistrée dans le classeur.

## Ajouter une feuille

- Cliquez sur le bouton **+** situé après le dernier onglet pour ajouter une feuille à la fin.
- Appuyez sur **Maj+F11**, ou utilisez **Accueil > Insérer > Insérer une feuille**, pour ajouter une feuille devant celle que vous affichez.

La nouvelle feuille est vide, prend le prochain nom libre (Feuil4, Feuil5, et ainsi de suite) et s'ouvre immédiatement. Un classeur peut comporter jusqu'à 1 000 feuilles ; au-delà, JeffCrunch le signale et n'en ajoute aucune, car un fichier qui en contient davantage n'est pas un fichier qu'il peut ouvrir.

## Renommer une feuille

**Double-cliquez** sur l'onglet, saisissez le nouveau nom, puis appuyez sur **Entrée** (**Échap** annule la modification). Vous pouvez aussi utiliser **Accueil > Format > Renommer la feuille**.

Un nom peut comporter jusqu'à 31 caractères. Il ne peut pas être vide, ni contenir l'un des caractères **: \ / ? * [ ]**, ni commencer ou se terminer par une apostrophe, ni être identique au nom d'une autre feuille (les majuscules ne constituent pas une différence). Le nom « Historique » est réservé au programme. Si le nom est refusé, JeffCrunch indique la raison, et la feuille conserve son nom.

Toute formule qui désigne la feuille suit le nouveau nom. Exemple : **=Budget!B2** devient **='T1 Budget'!B2** lorsque vous renommez Budget en T1 Budget. Les noms qui pointent vers la feuille, les règles des mises en forme conditionnelles et de la validation des données, ainsi que les graphiques, suivent également ce changement.

## Supprimer une feuille

Faites un clic droit sur l'onglet et choisissez **Supprimer la feuille**, ou utilisez **Accueil > Supprimer > Supprimer la feuille**. Si la feuille contient des données, JeffCrunch vous demande d'abord confirmation. Les formules situées sur d'autres feuilles et qui lisent la feuille supprimée affichent **#REF!**, ce qui signifie que les données qu'elles lisaient n'existent plus. **Annuler** restaure la feuille et ces formules.

Un classeur conserve toujours au moins une feuille affichée.

## Déplacer ou copier une feuille

- **Glissez** un onglet à un nouvel emplacement pour déplacer la feuille.
- Ou utilisez **Accueil > Format > Déplacer ou copier la feuille**. Choisissez la feuille devant laquelle placer la feuille sélectionnée (ou **(après la dernière feuille)**), puis cochez **Copier, et laisser les originaux à leur emplacement** pour copier la feuille au lieu de la déplacer. Une copie porte un nom du type **Budget (2)** et s'ouvre.

Les formules continuent de lire les feuilles qu'elles lisaient, quel que soit l'emplacement de ces feuilles. Une copie contient les mêmes cellules, formules, formats et dimensions que la feuille d'origine. Une référence sans nom de feuille (comme **A1**) lit la cellule de la copie elle-même ; une référence qui nomme une feuille (comme **Feuil1!A1**) désigne toujours cette feuille, même s'il s'agit de la feuille d'origine de la copie.

Une copie reprend également la protection, le filtre, les liens, la mise en page et les sauts de page de l'original, les groupes de lignes et de colonnes, ainsi que l'aspect de ses barres de données ; les liens suivent les lignes insérées ou supprimées, dans l'original avant la copie et dans la copie après celle-ci. Elle ne reprend pas les images, graphiques, notes, tableaux croisés dynamiques, sparklines ni les noms propres à l'original, car JeffCrunch ne sait pas encore les copier ; il vous indique lesquels dans ce cas. Une feuille qui contient des tableaux est copiée avec des copies de ceux-ci (chacune avec un nom, un identifiant et une partie propre), et les formules de la copie désignent ses propres tableaux. Supprimer une feuille supprime aussi ses tableaux ; une formule située sur une autre feuille et qui en désignait un affiche alors #REF!, et **Annuler** la restaure.

Pour copier une feuille dans **un autre classeur** ouvert, choisissez-le dans **Vers le classeur** dans la même boîte de dialogue. La liste indique ce que la copie ne contiendra pas (pour l'instant, les images, graphiques, mises en forme conditionnelles, tableaux, notes et la mise en page d'impression de la feuille) et le signale avant toute copie. Les formules qui lisent d'autres feuilles du premier classeur deviennent des liens vers son fichier ; le premier classeur doit donc avoir été enregistré au moins une fois au format .xlsx. La copie affiche les valeurs actuelles du premier classeur, qui changent lorsque les liens sont mis à jour (**Actualiser les valeurs** dans **Données > Modifier les liens**). Si le premier classeur contient des modifications non enregistrées, les liens lisent son fichier enregistré lors de la mise à jour ; enregistrez-le donc pour conserver ces valeurs. Les formules qui lisent la feuille copiée elle-même restent sur la copie. Les cellules qui utilisent des noms, des tableaux, une matrice ou un déversement, ou qui lisent un troisième classeur, sont copiées sous forme de valeurs affichées, et la boîte de dialogue indique leur nombre. L'affichage de la feuille (lignes et colonnes figées, zoom, quadrillage, sens de lecture), la hauteur de ligne et la largeur de colonne par défaut sont également copiés. La copie reprend le même aspect : les formats, polices et couleurs sont ajoutés à l'autre classeur (une couleur issue du thème du premier classeur est conservée sous la couleur qu'elle affiche lorsque les deux thèmes diffèrent). Les deux classeurs doivent compter les dates à partir de la même année (1900 ou 1904). **Annuler** dans l'autre classeur retire la copie ; le premier classeur n'est pas modifié. Le déplacement d'une feuille vers un autre classeur n'est pas encore disponible.

## Masquer et afficher une feuille

Faites un clic droit sur l'onglet et choisissez **Masquer la feuille** (ou utilisez **Accueil > Format > Masquer la feuille**). La feuille n'est plus affichée, mais elle reste dans le classeur et les formules continuent de la lire. Pour l'afficher de nouveau, faites un clic droit sur n'importe quel onglet, choisissez **Afficher la feuille**, puis cliquez sur son nom dans la liste. La liste est vide lorsqu'aucune feuille n'est masquée.

## Colorer un onglet

Faites un clic droit sur l'onglet et choisissez **Couleur d'onglet**, puis sélectionnez une couleur, ou choisissez **Aucune couleur**. La couleur apparaît sous forme de trait sous le nom. Une couleur choisie dans la palette du thème est enregistrée sous la couleur simple qu'elle représente actuellement.

## Sélectionner plusieurs feuilles à la fois

**Ctrl+clic** sur des onglets pour en sélectionner plusieurs, ou cliquez sur un onglet puis **Maj+clic** sur un autre pour sélectionner tous les onglets situés entre les deux. Les onglets sélectionnés sont grisés. **Supprimer**, **Masquer**, **Couleur d'onglet** et **Déplacer ou copier** s'appliquent alors à l'ensemble, en une seule étape. Cliquez sur un onglet non sélectionné pour ne sélectionner que celui-ci.

Ce que vous saisissez ou mettez en forme pendant que plusieurs feuilles sont sélectionnées ne modifie que la feuille que vous affichez.

## Opérations impossibles

- Un classeur qui contient une formule portant sur une plage de feuilles, comme **=SOMME(Feuil1:Feuil3!A1)**, ne peut pas encore recevoir de feuilles ajoutées, supprimées, renommées ou déplacées, car cela modifierait les feuilles couvertes par la formule.
- Après l'enregistrement, **Annuler** ne peut pas revenir en arrière au-delà d'une feuille ajoutée, copiée, supprimée, renommée ou déplacée avant l'enregistrement. Le fichier enregistré conserve ces modifications telles quelles. Le masquage, l'affichage et la couleur d'onglet peuvent toujours être annulés.
