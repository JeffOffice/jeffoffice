# Saisir dans les cellules
<!-- keywords: saisir, saisie, taper, frappe, entrer, valider, modifier, changer, cellule, entrée, barre de formule, F2, double-clic, Tab, Échap, abandonner, annuler, rétablir, supprimer, effacer, pourcentage, monétaire, devise, argent, date, heure, nombre, texte, apostrophe, formule, signe égal, remplacer, limite, caractères -->

Pour placer quelque chose dans une cellule, sélectionnez-la et commencez à taper. Ce que vous tapez remplace le contenu de la cellule. Appuyez sur **Entrée** pour le conserver, ou sur **Échap** pour l'abandonner et laisser la cellule telle qu'elle était.

## Trois façons de commencer

- **Remplacer le contenu de la cellule.** Sélectionnez une cellule et tapez. Le premier caractère tapé commence la saisie, et la cellule affiche votre saisie au fur et à mesure.
- **Modifier le contenu de la cellule.** Appuyez sur **F2**, ou double-cliquez sur la cellule. La cellule s'ouvre avec son contenu, et le curseur se trouve à la fin. Utilisez les touches de direction pour vous déplacer dans le texte.
- **Utiliser la barre de formule.** Cliquez dans la barre de formule, à côté de la zone Nom, et tapez-y. La cellule affiche aussi votre saisie.

Lorsque vous remplacez le contenu d'une cellule, une touche de direction termine la saisie et déplace la sélection. Lorsque vous modifiez le contenu de la cellule, les touches de direction se déplacent dans le texte. Appuyez sur **F2** pour passer de l'un à l'autre.

## Terminer la saisie

| Touches | Action |
| --- | --- |
| Entrée | Conserver la saisie et descendre (ou aller dans le sens défini dans **Fichier > Options > Modification**) |
| Maj+Entrée | Conserver la saisie et aller dans l'autre sens |
| Ctrl+Entrée | Placer la saisie dans toutes les cellules sélectionnées, sans se déplacer |
| Tab | Conserver la saisie et aller à droite |
| Maj+Tab | Conserver la saisie et aller à gauche |
| Touches de direction | Conserver la saisie et se déplacer d'une cellule (uniquement si vous avez remplacé le contenu de la cellule) |
| Échap | Abandonner la saisie |
| Alt+Entrée | Commencer une nouvelle ligne dans la cellule |
| Ctrl+; | Insérer la date du jour (dans la saisie, si vous êtes en train de taper) |
| Ctrl+Maj+; | Insérer l'heure actuelle |
| Alt+Bas | Afficher la liste des mots déjà présents dans la colonne et en choisir un |

**Ctrl+Entrée**, lorsque plusieurs cellules sont sélectionnées, place votre saisie dans chacune d'elles. Une formule est écrite pour la cellule dans laquelle vous avez tapé, et dans chacune des autres, elle se lit comme le ferait une copie : taper **=A1** avec la plage E6:F7 sélectionnée donne **=B2** en F7. L'opération ne compte que pour une étape d'annulation.

**Alt+Bas** sur une cellule affiche, triés et sans doublons, les mots des cellules situées au-dessus et en dessous qui ne sont pas séparées d'elle par une cellule vide. En choisir un le saisit. Une cellule dotée de sa propre liste affiche cette liste à la place.

Dans **Fichier > Options > Modification**, choisissez si **Entrée** déplace la sélection vers le bas, la droite, le haut ou la gauche, ou ne la déplace pas.

Si vous saisissez dans plusieurs cellules d'une ligne avec **Tab**, puis appuyez sur **Entrée**, la sélection descend à la ligne suivante, dans la colonne où vous avez commencé. Si vous sélectionnez d'abord une plage de cellules, **Entrée** et **Tab** se déplacent à l'intérieur de la plage.

## Ce que devient votre saisie

JeffCrunch interprète votre saisie selon les paramètres de votre ordinateur. Le séparateur décimal, le séparateur de milliers, le symbole monétaire et l'ordre du jour et du mois dans une date proviennent tous de ces paramètres. Un nombre tapé dans une cellule sans format propre reçoit le format qui l'affiche tel que vous l'avez tapé.

| Vous tapez | La cellule contient | Et affiche |
| --- | --- | --- |
| 1234,5 | le nombre 1234,5 | 1234,5 |
| 5% | le nombre 0,05 | 5% |
| 12,50 € | le nombre 12,5 | 12,50 € |
| 1 1/2 | le nombre 1,5 | 1 1/2 |
| 03/04/2025 | la date, sous forme de nombre de jours | 03/04/2025 |
| 22:30 | l'heure, sous forme de fraction de jour | 22:30 |
| VRAI | la valeur logique vrai | VRAI |
| #N/A | l'erreur #N/A | #N/A |
| =B2*2 | une formule | le résultat de la formule |
| tout le reste | du texte | le texte |

Une cellule qui possède déjà son propre format, comme un pourcentage ou une date, le conserve. Une cellule au format **Texte** conserve tout ce que vous tapez sous forme de texte, sauf une formule.

Pour conserver sous forme de texte une saisie qui ressemble à un nombre, comme un code **007**, tapez d'abord une apostrophe : **'007**. L'apostrophe ne fait pas partie du texte. Lorsque vous modifiez la cellule, elle réapparaît.

## Formules

Une formule commence par un signe égal. Par exemple, **=B2*2** double le nombre de la cellule B2. Tapez les noms des fonctions en majuscules ou en minuscules ; JeffCrunch les écrit en majuscules.

Les noms des fonctions sont toujours les noms anglais, quelle que soit la langue : tapez **SUM**, et non le nom que lui donne votre langue. Les séparateurs suivent les paramètres de votre ordinateur. Là où le séparateur décimal est la virgule, comme en français et en allemand, séparez les arguments d'une fonction par un point-virgule et écrivez une virgule dans les nombres : **=SUM(A1;A2)*0,5**. La barre de formule et **F2** affichent les formules de la même façon. Là où le séparateur décimal est le point, séparez les arguments par une virgule : **=SUM(A1,A2)*0.5**. Le fichier conserve toujours la forme que tous les programmes lisent, avec des virgules entre les arguments et des points dans les nombres ; il s'ouvre ainsi de la même façon partout.

Si une formule comporte une erreur, comme une parenthèse jamais fermée, JeffCrunch vous indique le problème, et la saisie reste telle que vous l'avez tapée, avec le curseur près de l'erreur, pour que vous puissiez la corriger. Rien n'est modifié tant que la formule n'est pas correcte.

Une cellule contient jusqu'à 32 767 caractères, et une formule jusqu'à 8 192. JeffCrunch vous prévient lorsqu'une saisie est plus longue.

## Effacer des cellules

Sélectionnez les cellules et appuyez sur **Suppr**. Le contenu des cellules est effacé et leurs formats sont conservés ; l'opération ne compte que pour une étape d'annulation. JeffCrunch efface jusqu'à 100 000 cellules à la fois.

## Annuler et Rétablir

**{key:Undo}** annule la dernière modification, et **{key:Redo}** la rétablit. Ces commandes figurent aussi dans la barre d'outils Accès rapide, en haut. Chaque cellule saisie compte pour une étape, de même que chaque effacement. L'annulation rétablit la cellule telle qu'elle était : sa valeur, sa formule, son format et les formules qui en dépendent. JeffCrunch mémorise jusqu'à 1 000 étapes, moins lorsqu'elles ont modifié un très grand nombre de cellules, afin de ne jamais occuper beaucoup de mémoire.

## Pendant le calcul des formules

Lorsque vous modifiez une cellule, JeffCrunch calcule en arrière-plan les formules qui en dépendent. La cellule modifiée affiche immédiatement son nouveau contenu, et les autres cellules se mettent à jour un instant plus tard ; la barre d'état indique **Calcul en cours…** jusqu'à la fin. Vous pouvez continuer à saisir pendant ce temps.

## Ce qui ne peut pas être modifié

- Une cellule qui fait partie d'une **formule matricielle** ou d'une table de données ne peut pas être modifiée seule.
- Sur une **feuille protégée**, une cellule verrouillée ne peut pas être modifiée. Toutes les cellules sont verrouillées, sauf indication contraire dans le classeur. JeffCrunch vous prévient si vous essayez. Pour modifier la cellule, choisissez Ôter la protection de la feuille dans l'onglet Révision.
- La première ligne d'un **tableau** contient les noms de ses colonnes. Tapez un nouveau nom dans l'une de ces cellules pour renommer la colonne : les formules qui utilisent l'ancien nom suivent, et le tableau reste valide dans le fichier. Un nom déjà utilisé par une autre colonne est suivi d'un numéro, un nom ne peut pas être vide, et **Suppr** n'efface pas cette ligne.
- Un classeur qui n'a pas pu être lu entièrement peut être consulté, mais pas modifié ni enregistré.
- Une fonction que JeffCrunch ne prend pas encore en charge laisse une nouvelle formule sans valeur dans la cellule ; le classeur demande au prochain programme qui l'ouvrira de calculer la formule.
- La mise en forme, le tri, ainsi que la recherche et le remplacement arriveront dans une version ultérieure. L'insertion et la suppression, le couper-coller, la recopie et la taille des lignes et des colonnes sont décrits dans [Insérer et supprimer des lignes, des colonnes et des cellules](insert-delete), [Couper, copier et coller](clipboard), [Recopier des cellules et les additionner](fill-sum) et [Modifier la largeur des colonnes et la hauteur des lignes](sizes).
