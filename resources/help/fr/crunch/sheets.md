# Gérer les feuilles de calcul
<!-- keywords: feuille, feuilles, feuille de calcul, onglet, onglets, ajouter, insérer, nouvelle feuille, supprimer, effacer, renommer, nom, déplacer, copier, dupliquer, réorganiser, ordre, faire glisser, masquer, afficher, démasquer, couleur d'onglet, couleur, sélectionner plusieurs feuilles, Maj+F11, #REF!, référence 3D, Déplacer ou copier -->

Un classeur est composé de **feuilles**, et les **onglets** situés en bas de la fenêtre portent leurs noms. Cliquez sur un onglet pour afficher la feuille correspondante. Chacune des modifications ci-dessous constitue une étape que vous pouvez annuler, et elle est enregistrée dans le classeur.

## Ajouter une feuille

- Cliquez sur le bouton **+** situé après le dernier onglet pour ajouter une feuille à la fin.
- Appuyez sur **Maj+F11**, ou utilisez **Accueil > Insertion > Insérer une feuille**, pour ajouter une feuille avant celle que vous consultez.

La nouvelle feuille est vide, reçoit le prochain nom disponible (Feuil4, Feuil5, etc.) et s'ouvre immédiatement. Un classeur peut comporter jusqu'à 1 000 feuilles ; au-delà, JeffCrunch vous en avertit et n'en ajoute aucune, car il ne pourrait pas ouvrir un fichier qui en contient davantage.

## Renommer une feuille

**Double-cliquez** sur l'onglet, tapez le nouveau nom, puis appuyez sur **Entrée** (**Échap** annule). Vous pouvez aussi utiliser **Accueil > Format > Renommer la feuille**.

Un nom peut comporter jusqu'à 31 caractères. Il ne peut pas être vide, ne peut contenir aucun des caractères **: \ / ? * [ ]**, ne peut pas commencer ni se terminer par une apostrophe, et ne peut pas être identique au nom d'une autre feuille (les majuscules ne comptent pas comme une différence). Le nom « History » est réservé au programme. Si le nom est refusé, JeffCrunch en indique la raison et la feuille conserve son nom.

Toutes les formules qui nomment la feuille suivent le nouveau nom. Exemple : **=Budget!B2** devient **='Budget T1'!B2** lorsque vous renommez Budget en Budget T1. Les noms qui font référence à la feuille, les règles des mises en forme conditionnelles et de la validation des données, ainsi que les graphiques, suivent également le changement.

## Supprimer une feuille

Cliquez avec le bouton droit sur l'onglet et choisissez **Supprimer la feuille**, ou utilisez **Accueil > Supprimer > Supprimer la feuille**. Si la feuille contient quelque chose, JeffCrunch vous demande d'abord confirmation. Les formules des autres feuilles qui lisaient la feuille supprimée affichent **#REF!**, ce qui signifie que ce qu'elles lisaient n'existe plus. **Annuler** rétablit la feuille et ces formules.

Un classeur conserve toujours au moins une feuille affichée.

## Déplacer ou copier une feuille

- **Faites glisser** un onglet vers un nouvel emplacement pour déplacer la feuille.
- Ou utilisez **Accueil > Format > Déplacer ou copier la feuille**. Choisissez la feuille devant laquelle placer la feuille choisie (ou **(après la dernière feuille)**), et cochez **Les copier, et laisser les originaux à leur place** pour la copier au lieu de la déplacer. Une copie reçoit un nom du type **Budget (2)** et s'ouvre.

Les formules continuent de lire les feuilles qu'elles lisaient, où que se trouvent ces feuilles. Une copie comporte les mêmes cellules, formules, formats et tailles que la feuille d'origine. Une référence sans nom de feuille (comme **A1**) lit la cellule de la copie elle-même ; une référence qui nomme une feuille (comme **Feuil1!A1**) désigne toujours cette feuille, même s'il s'agit de la feuille d'origine de la copie.

Une copie reprend aussi la protection, le filtre, les liens, la mise en page et les sauts de page de l'original, les groupes de lignes et de colonnes, ainsi que l'apparence de ses barres de données ; les liens suivent les lignes que vous avez insérées ou supprimées, dans l'original avant la copie, puis dans la copie après sa création. Elle ne reprend pas les images, les graphiques, les notes, les tableaux croisés dynamiques, les graphiques sparkline ni les noms propres à l'original, car JeffCrunch ne les copie pas encore ; il vous indique lesquels lorsque cela se produit. Une feuille qui contient des tableaux est copiée avec des copies de ceux-ci (chacune avec son propre nom, son propre identifiant et sa propre partie, et les formules de la copie nomment ses propres tableaux). La suppression d'une feuille entraîne celle de ses tableaux ; une formule d'une autre feuille qui en nommait un affiche alors #REF!, et Annuler la rétablit.

## Masquer et afficher une feuille

Cliquez avec le bouton droit sur l'onglet et choisissez **Masquer la feuille** (ou utilisez **Accueil > Format > Masquer la feuille**). La feuille n'est plus affichée, mais elle reste dans le classeur et les formules continuent de la lire. Pour l'afficher de nouveau, cliquez avec le bouton droit sur n'importe quel onglet, choisissez **Afficher la feuille**, puis cliquez sur son nom dans la liste. La liste est vide lorsqu'aucune feuille n'est masquée.

## Colorer un onglet

Cliquez avec le bouton droit sur l'onglet et choisissez **Couleur d'onglet**, puis choisissez une couleur, ou choisissez **Aucune couleur**. La couleur s'affiche sous la forme d'un trait sous le nom. Une couleur choisie dans la palette du thème est enregistrée comme la couleur simple qu'elle est actuellement.

## Plusieurs feuilles à la fois

**Ctrl+clic** sur des onglets permet d'en sélectionner plusieurs ; vous pouvez aussi cliquer sur un onglet puis faire **Maj+clic** sur un autre pour sélectionner tous les onglets compris entre les deux. Les onglets sélectionnés sont ombrés. **Supprimer**, **Masquer**, **Couleur d'onglet** et **Déplacer ou copier** s'appliquent alors à tous en une seule étape. Cliquez sur un onglet non sélectionné pour ne sélectionner que celui-ci.

Ce que vous tapez ou mettez en forme lorsque plusieurs feuilles sont sélectionnées ne modifie que la feuille que vous consultez.

## Limites

- Dans un classeur qui contient une formule portant sur une plage de feuilles, comme **=SOMME(Feuil1:Feuil3!A1)**, il n'est pas encore possible d'ajouter, de supprimer, de renommer ni de déplacer des feuilles, car cela modifierait les feuilles couvertes par la formule.
- Après un enregistrement, **Annuler** ne peut pas revenir avant une feuille ajoutée, copiée, supprimée, renommée ou déplacée avant l'enregistrement. Le fichier enregistré les contient dans leur état actuel. Le masquage, l'affichage et la couleur d'onglet peuvent toujours être annulés.
