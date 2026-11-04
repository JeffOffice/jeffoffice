# Saisir dans les cellules
<!-- keywords: saisie, saisir, taper, entrer, modifier, changer, cellule, contenu, barre de formule, F2, double-clic, Tab, Échap, annuler, abandonner, rétablir, supprimer, effacer, pourcentage, monétaire, argent, date, heure, nombre, texte, apostrophe, formule, égal, remplacer, limite, caractères -->

Pour mettre quelque chose dans une cellule, sélectionnez la cellule et commencez à taper. Ce que vous tapez remplace le contenu de la cellule. Appuyez sur **Entrée** pour le conserver, ou sur **Échap** pour l'abandonner et laisser la cellule telle qu'elle était.

## Trois façons de commencer

- **Saisir directement dans la cellule.** Sélectionnez une cellule et tapez. Le premier caractère que vous tapez démarre la saisie, et la cellule affiche ce que vous tapez au fur et à mesure.
- **Modifier le contenu de la cellule.** Appuyez sur **F2** ou double-cliquez sur la cellule. La cellule s'ouvre avec son contenu, et le curseur se place à la fin. Utilisez les touches fléchées pour vous déplacer dans le texte.
- **Utiliser la barre de formule.** Cliquez dans la barre de formule, à côté de la zone Nom, et tapez-y. La cellule affiche aussi ce que vous tapez.

Lorsque vous saisissez directement dans une cellule, une touche fléchée termine la saisie et déplace la sélection. Lorsque vous modifiez le contenu de la cellule, les touches fléchées se déplacent dans le texte. Appuyez sur **F2** pour passer de l'un à l'autre.

## Terminer la saisie

| Touches | Ce qu'elles font |
| --- | --- |
| Entrée | Conserve la saisie et descend (ou va où **Fichier > Options > Modification** l'indique) |
| Maj+Entrée | Conserve la saisie et remonte |
| Ctrl+Entrée | Met la saisie dans toutes les cellules sélectionnées, et reste à votre position |
| Tab | Conserve la saisie et va à droite |
| Maj+Tab | Conserve la saisie et va à gauche |
| Touches fléchées | Conservent la saisie et déplacent d'une cellule (seulement si vous avez saisi directement dans la cellule) |
| Échap | Abandonne la saisie |
| Alt+Entrée | Démarre une nouvelle ligne dans la cellule |
| Ctrl+; | Insère la date du jour (dans la saisie, lorsque vous en tapez une) |
| Ctrl+Maj+; | Insère l'heure actuelle |
| Alt+Flèche bas | Affiche les mots déjà présents dans la colonne, pour en choisir un |

**Ctrl+Entrée** avec plusieurs cellules sélectionnées met ce que vous avez tapé dans toutes. Une formule est écrite pour la cellule où vous l'avez saisie, et dans chacune des autres elle se comporte comme une copie de cette formule : si vous tapez **=A1** alors que le bloc E6:F7 est sélectionné, F7 contient **=B2**. Cette opération compte comme une seule étape d'annulation.

**Alt+Flèche bas** sur une cellule affiche, dans l'ordre et une seule fois chacun, les mots présents dans les cellules situées au-dessus et en dessous, à condition qu'elles ne soient pas séparées par une cellule vide. Choisir un mot le saisit. Si la cellule possède sa propre liste, elle affiche cette liste à la place.

Dans **Fichier > Options > Modification**, choisissez si **Entrée** descend, va à droite, monte, va à gauche, ou ne déplace pas la sélection.

Si vous saisissez dans plusieurs cellules d'une ligne avec **Tab**, puis appuyez sur **Entrée**, la sélection descend à la ligne suivante, dans la colonne où vous avez commencé. Si vous sélectionnez d'abord un bloc de cellules, **Entrée** et **Tab** parcourent le bloc.

## Ce que devient votre saisie

JeffCrunch interprète ce que vous tapez selon les paramètres de votre ordinateur. Le séparateur décimal, le séparateur des milliers, le symbole monétaire et l'ordre du jour et du mois dans une date proviennent de ces paramètres. Un nombre saisi dans une cellule qui n'a pas de format propre reçoit le format qui l'affiche comme vous l'avez tapé.

| Vous tapez | La cellule contient | Et affiche |
| --- | --- | --- |
| 1234.5 | le nombre 1234,5 | 1234,5 |
| 5% | le nombre 0,05 | 5 % |
| $12.50 | le nombre 12,5 | 12,50 $ |
| 1 1/2 | le nombre 1,5 | 1 1/2 |
| 3/4/2025 | la date, sous forme de nombre de jours | 04/03/2025 |
| 22:30 | l'heure, sous forme de fraction de jour | 22:30 |
| VRAI | la valeur logique vrai | VRAI |
| #N/A | l'erreur #N/A | #N/A |
| =B2*2 | une formule | le résultat de la formule |
| tout le reste | du texte | le texte |

Une cellule qui a déjà un format propre, comme un pourcentage ou une date, le conserve. Une cellule dont le format est **Texte** conserve tout ce que vous tapez comme texte, sauf une formule.

Pour conserver comme texte quelque chose qui ressemble à un nombre, comme un code **007**, tapez d'abord une apostrophe : **'007**. L'apostrophe ne fait pas partie du texte. Lorsque vous modifiez la cellule, l'apostrophe réapparaît.

## Formules

Une formule commence par un signe égal. Par exemple, **=B2*2** double le nombre contenu dans B2. Vous pouvez taper les noms des fonctions en majuscules ou en minuscules ; JeffCrunch les écrit en majuscules.

Les noms des fonctions sont toujours les noms anglais, dans toutes les langues : tapez **SOMME**, et non le nom de votre langue. Les signes de ponctuation qui les séparent suivent les paramètres de votre ordinateur. Lorsque le séparateur décimal est la virgule, comme en allemand et en français, vous séparez les arguments d'une fonction par un point-virgule, et vous écrivez une virgule dans un nombre : **=SOMME(A1;A2)*0,5**. La barre de formule et **F2** affichent les formules de la même façon. Lorsque le séparateur décimal est le point, vous séparez les arguments par une virgule : **=SOMME(A1,A2)*0.5**. Le fichier conserve toujours la forme que tous les logiciels savent lire, avec des virgules entre les arguments et des points dans les nombres, afin qu'il s'ouvre de la même façon partout.

Si une formule comporte une erreur, par exemple une parenthèse qui n'est jamais fermée, JeffCrunch vous indique le problème, et la saisie reste telle que vous l'avez tapée, avec le curseur près de l'erreur, pour que vous puissiez la corriger. Rien n'est modifié tant que la formule n'est pas correcte.

Une cellule contient jusqu'à 32 767 caractères, et une formule jusqu'à 8 192. JeffCrunch vous prévient lorsqu'une saisie est plus longue.

## Effacer des cellules

Sélectionnez les cellules et appuyez sur **Suppr**. Cela efface ce que contiennent les cellules et conserve leurs formats ; c'est une étape que vous pouvez annuler. JeffCrunch efface jusqu'à 100 000 cellules à la fois.

Dans l'onglet **Accueil**, le bouton **Effacer** possède un menu. **Tout effacer** vide les cellules, leurs formats, leurs notes et leurs liens. **Effacer les formats** conserve ce que contiennent les cellules et remet le format normal. **Effacer le contenu** équivaut à **Suppr**. **Effacer les commentaires et les notes** retire seulement les notes des cellules sélectionnées. **Effacer les liens hypertextes** retire seulement les liens, et les cellules gardent leur texte. Un choix est grisé lorsque les cellules sélectionnées n'ont rien à effacer à ce niveau.

## Annuler et rétablir

**{key:Undo}** annule la dernière modification, et **{key:Redo}** la rétablit. Ces commandes se trouvent aussi dans la barre d'outils rapide, en haut. Chaque saisie dans une cellule constitue une étape, et chaque effacement aussi. Annuler remet la cellule dans son état précédent : sa valeur, sa formule, son format, et les formules qui en dépendent. JeffCrunch conserve jusqu'à 1 000 étapes, moins lorsque les modifications ont touché un très grand nombre de cellules, afin de ne jamais utiliser beaucoup de mémoire.

## Pendant que JeffCrunch calcule les formules

Lorsque vous modifiez une cellule, JeffCrunch calcule en arrière-plan les formules qui en dépendent. La cellule modifiée affiche aussitôt son nouveau contenu, et les autres cellules se mettent à jour un instant plus tard ; la barre d'état affiche **Calcul en cours…** jusqu'à la fin du calcul. Vous pouvez continuer à taper pendant ce temps.

## Ce qui ne peut pas être modifié

- Une cellule qui fait partie d'une **formule matricielle** ou d'une table de données ne peut pas être modifiée seule.
- Sur une **feuille protégée**, une cellule verrouillée ne peut pas être modifiée. Toutes les cellules sont verrouillées, sauf si le classeur indique le contraire. JeffCrunch vous en avertit lorsque vous essayez. Pour modifier la cellule, choisissez **Ôter la protection de la feuille** dans l'onglet **Révision**.
- La première ligne d'un **tableau** contient les noms de ses colonnes. Tapez un nouveau nom dans l'une de ces cellules pour renommer la colonne : les formules qui utilisent l'ancien nom suivent, et le tableau du fichier reste valide. Un nom déjà utilisé par une autre colonne reçoit un numéro à la suite, un nom ne peut pas être vide, et **Suppr** n'efface pas cette ligne.
- Un classeur qui n'a pas pu être lu en entier peut être consulté, mais ni modifié ni enregistré.
- Une fonction que JeffCrunch ne possède pas encore laisse une nouvelle formule sans valeur dans la cellule ; le classeur demande au programme qui l'ouvre de calculer la formule.
- La mise en forme, le tri, ainsi que la recherche et le remplacement, viendront dans une version ultérieure. L'insertion et la suppression, le couper-coller, la recopie et les dimensions des lignes et des colonnes sont traités dans [Insérer et supprimer des lignes, des colonnes et des cellules](insert-delete), [Couper, copier et coller](clipboard), [Remplir les cellules et en faire la somme](fill-sum) et [Modifier la largeur des colonnes et la hauteur des lignes](sizes).
