# Saisir du texte dans les cellules
<!-- keywords: mot en gras, partie d'une cellule, texte enrichi, mettre en forme une partie du texte, colorer un mot, saisie, saisir, taper, entrer, modifier, changer, cellule, contenu, barre de formule, F2, double-clic, Tab, Échap, abandonner, annuler, rétablir, supprimer, effacer, pourcentage, monnaie, devise, argent, date, heure, nombre, texte, apostrophe, formule, égal, remplacer, limite, caractères -->

Pour saisir quelque chose dans une cellule, sélectionnez-la et commencez à taper. Ce que vous tapez remplace le contenu de la cellule. Appuyez sur **Entrée** pour le valider, ou sur **Échap** pour l'abandonner et laisser la cellule comme elle était.

## Trois façons de commencer

- **Taper directement dans la cellule.** Sélectionnez une cellule et tapez. Le premier caractère que vous saisissez démarre la saisie, et la cellule affiche ce que vous tapez au fur et à mesure.
- **Modifier le contenu de la cellule.** Appuyez sur **F2**, ou double-cliquez sur la cellule. La cellule s'ouvre avec son contenu, et le curseur se place à la fin. Utilisez les touches fléchées pour vous déplacer dans le texte.
- **Utiliser la barre de formule.** Cliquez dans la barre de formule, à côté de la zone Nom, et tapez-y. La cellule affiche aussi ce que vous tapez.

Lorsque vous tapez directement dans une cellule, une touche fléchée valide la saisie et déplace la sélection. Lorsque vous modifiez le contenu de la cellule, les touches fléchées se déplacent dans le texte. Appuyez sur **F2** pour passer de l'un à l'autre.

## Valider la saisie

| Touches | Effet |
| --- | --- |
| Entrée | Valider la saisie, et descendre (ou selon ce que indique **Fichier > Options > Modification**) |
| Maj+Entrée | Valider la saisie, et remonter |
| Ctrl+Entrée | Mettre la saisie dans toutes les cellules sélectionnées, et rester à place |
| Tab | Valider la saisie, et aller à droite |
| Maj+Tab | Valider la saisie, et aller à gauche |
| Touches fléchées | Valider la saisie, et déplacer la sélection d'une cellule (seulement lorsque vous avez tapé directement dans la cellule) |
| Échap | Abandonner la saisie |
| Alt+Entrée | Commencer une nouvelle ligne dans la cellule |
| Ctrl+; | Insérer la date du jour (dans la saisie, lorsque vous êtes en train de taper) |
| Ctrl+Maj+; | Insérer l'heure actuelle |
| Alt+Bas | Afficher les mots déjà présents dans la colonne, et en choisir un |

**Ctrl+Entrée** avec plusieurs cellules sélectionnées met ce que vous avez tapé dans toutes. Une formule est écrite pour la cellule dans laquelle vous tapez, et dans chacune des autres elle se lit comme une copie le ferait : si vous tapez **=A1** alors que le bloc E6:F7 est sélectionné, F7 contient **=B2**. Cela compte pour une seule étape d'annulation.

**Alt+Bas** sur une cellule affiche, dans l'ordre et une seule fois chacun, les mots des cellules situées au-dessus et en dessous, sans être séparés par une cellule vide. Choisir un mot le saisit. Une cellule qui possède sa propre liste affiche cette liste à la place.

Dans **Fichier > Options > Modification**, choisissez si **Entrée** fait descendre, aller à droite, remonter, aller à gauche, ou ne déplace pas la sélection.

Si vous saisissez dans plusieurs cellules d'une ligne avec **Tab**, puis appuyez sur **Entrée**, la sélection descend à la ligne suivante, dans la colonne où vous avez commencé. Si vous sélectionnez d'abord un bloc de cellules, **Entrée** et **Tab** parcourent ce bloc.

## Mettre en forme une partie du texte

Certains mots d'une cellule peuvent être présentés différemment du reste. Par exemple, un mot peut être en gras ou en rouge.

1. Appuyez sur F2 pour modifier la cellule (ou double-cliquez dessus).
2. Sélectionnez les mots à modifier, avec Maj et les touches fléchées, ou avec la souris.
3. Utilisez **Gras**, **Italique**, **Souligné**, **Barré**, **Police**, **Taille de police** ou **Couleur de police** dans l'onglet **Accueil**, ou Ctrl+G, Ctrl+I et Ctrl+S.
4. Appuyez sur Entrée.

Seuls les mots sélectionnés changent. Le reste de la cellule conserve la présentation propre à la cellule. La zone de saisie affiche les mots dans une seule présentation pendant la modification, et la cellule affiche chaque partie dans sa propre présentation lorsque vous appuyez sur Entrée. Si vous ajoutez ou supprimez ensuite des mots, la mise en forme suit les mots. Choisir une police ou une taille sans mot sélectionné modifie toute la cellule, comme d'habitude.

Les cellules qui contiennent une mise en forme interne au texte provenant d'un autre programme l'affichent, la conservent et l'enregistrent. Une cellule que vous modifiez conserve sa mise en forme. La mise en forme interne au texte s'applique au texte, et non aux nombres ni aux formules. Une copie enregistrée au format .xls plus ancien ou au format .ods conserve les mots, et l'enregistrement vous indique que la mise en forme interne à ces mots n'est pas conservée.

## Ce que vous tapez devient

JeffCrunch interprète ce que vous tapez selon les paramètres de votre ordinateur. Le séparateur décimal, le séparateur des milliers, le symbole monétaire et l'ordre du jour et du mois dans une date viennent de ces paramètres. Un nombre saisi dans une cellule qui n'a pas de format propre reçoit le format qui l'affiche comme vous l'avez tapé.

| Vous tapez | La cellule contient | Et affiche |
| --- | --- | --- |
| 1234,5 | le nombre 1234,5 | 1234,5 |
| 5 % | le nombre 0,05 | 5 % |
| 12,50 € | le nombre 12,5 | 12,50 € |
| 1 1/2 | le nombre 1,5 | 1 1/2 |
| 4/3/2025 | la date, sous forme de nombre de jours | 04/03/2025 |
| 22:30 | l'heure, sous forme de fraction de jour | 22:30 |
| VRAI | la valeur logique vrai | VRAI |
| #N/A | l'erreur #N/A | #N/A |
| =B2*2 | une formule | le résultat de la formule |
| tout le reste | du texte | le texte |

Une cellule qui possède déjà un format propre, comme un pourcentage ou une date, le conserve. Une cellule dont le format est **Texte** conserve tout ce que vous tapez comme texte, sauf une formule.

Pour conserver comme texte quelque chose qui ressemble à un nombre, comme un code **007**, tapez d'abord une apostrophe : **'007**. L'apostrophe ne fait pas partie du texte. Lorsque vous modifiez la cellule, vous la revoyez.

## Formules

Une formule commence par un signe égal. Par exemple, **=B2*2** double le nombre contenu en B2. Tapez les noms des fonctions en majuscules ou en minuscules ; JeffCrunch les écrit en majuscules.

Les noms des fonctions sont toujours les noms anglais, dans toutes les langues : tapez **SUM**, et non le nom que votre langue lui donne. Les signes de ponctuation qui les séparent suivent les paramètres de votre ordinateur. Lorsque le séparateur décimal est la virgule, comme en français et en allemand, vous séparez les arguments d'une fonction par un point-virgule et vous écrivez une virgule dans un nombre : **=SOMME(A1;A2)*0,5**. La barre de formule et **F2** affichent les formules de la même façon. Lorsque le séparateur décimal est le point, vous séparez les arguments par une virgule : **=SUM(A1,A2)*0.5**. Le fichier conserve toujours la forme que lisent tous les programmes, avec des virgules entre les arguments et des points dans les nombres, afin qu'il s'ouvre de la même façon partout.

Si une formule comporte une erreur, comme une parenthèse qui n'est jamais fermée, JeffCrunch vous indique le problème, et la saisie reste telle que vous l'avez tapée, avec le curseur près de l'erreur, afin que vous puissiez la corriger. Rien n'est modifié tant que la formule n'est pas correcte.

Une cellule contient jusqu'à 32 767 caractères, et une formule jusqu'à 8 192. JeffCrunch vous prévient lorsqu'une saisie est plus longue.

## Effacer des cellules

Sélectionnez les cellules et appuyez sur **Suppr**. Cela efface leur contenu et conserve leur format ; c'est une étape que vous pouvez annuler. JeffCrunch efface jusqu'à 100 000 cellules à la fois.

Dans l'onglet **Accueil**, le bouton **Effacer** possède un menu. **Tout effacer** vide les cellules, leurs formats, leurs notes et leurs liens. **Effacer les formats** conserve le contenu des cellules et rétablit le format standard. **Effacer le contenu** équivaut à **Suppr**. **Effacer les commentaires et les notes** retire uniquement les notes des cellules sélectionnées. **Effacer les liens hypertextes** retire uniquement les liens, et les cellules conservent leur texte. Un choix est grisé lorsque les cellules sélectionnées n'ont rien à effacer à ce niveau.

## Annuler et rétablir

**{key:Undo}** annule la dernière modification, et **{key:Redo}** la rétablit. Ces commandes figurent aussi dans la barre d'outils rapide, en haut. Chaque cellule saisie compte pour une étape, de même que chaque effacement. Annuler remet la cellule dans son état précédent : sa valeur, sa formule, son format, et les formules qui en dépendent. JeffCrunch conserve jusqu'à 1 000 étapes, moins lorsque les modifications ont touché un très grand nombre de cellules, afin de ne jamais utiliser beaucoup de mémoire.

## Pendant que JeffCrunch calcule les formules

Lorsque vous modifiez une cellule, JeffCrunch calcule en arrière-plan les formules qui en dépendent. La cellule modifiée affiche aussitôt son nouveau contenu, et les autres cellules se mettent à jour un instant plus tard ; la barre d'état affiche **Calcul en cours** tant que ce n'est pas terminé. Vous pouvez continuer à taper entre-temps.

## Ce qui ne peut pas être modifié

- Une cellule qui fait partie d'une **formule matricielle** ou d'un tableau de données ne peut pas être modifiée isolément.
- Sur une **feuille protégée**, une cellule verrouillée ne peut pas être modifiée. Toutes les cellules sont verrouillées, sauf si le classeur indique le contraire. JeffCrunch vous le signale lorsque vous essayez. Pour modifier la cellule, choisissez Ôter la protection de la feuille dans l'onglet Révision.
- La première ligne d'un **tableau** contient les noms de ses colonnes. Tapez un nouveau nom dans l'une de ces cellules pour renommer la colonne : les formules qui utilisent l'ancien nom suivent, et le tableau du fichier reste valide. Un nom déjà utilisé par une autre colonne reçoit un numéro à la suite, un nom ne peut pas être vide, et **Suppr** n'efface pas cette ligne.
- Un classeur qui n'a pas pu être lu en entier peut être consulté, mais ne peut être ni modifié ni enregistré.
- Une fonction que JeffCrunch ne connaît pas encore laisse une nouvelle formule sans valeur dans la cellule ; le classeur demande au programme qui l'ouvre de calculer la formule.
- La mise en forme, le tri, ainsi que la recherche et le remplacement, seront disponibles dans une version ultérieure. L'insertion et la suppression, le couper-coller, le remplissage et les dimensions des lignes et des colonnes sont décrits dans [Insérer et supprimer des lignes, des colonnes et des cellules](insert-delete), [Couper, copier et coller](clipboard), [Remplir des cellules et en faire la somme](fill-sum) et [Modifier la largeur des colonnes et la hauteur des lignes](sizes).

## Nouveaux classeurs

Dans **Fichier > Options > Nouveaux classeurs**, choisissez la police, la taille de police et le nombre de feuilles avec lesquels chaque nouveau classeur démarre. Les classeurs que vous avez déjà ne sont pas modifiés.
