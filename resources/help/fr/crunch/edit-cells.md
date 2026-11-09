# Saisir du texte dans les cellules
<!-- keywords: correction automatique, guillemets typographiques, tiret, fraction, lien, adresse, mot en gras, partie d'une cellule, texte enrichi, mise en forme d'une partie du texte, colorer un mot, saisie, saisir, taper, entrer, modifier, changer, cellule, contenu, barre de formule, F2, double-clic, Tab, Échap, abandonner, annuler, rétablir, supprimer, effacer, pourcentage, monétaire, argent, date, heure, nombre, texte, apostrophe, formule, égal, remplacer, limite, caractères -->

Pour saisir quelque chose dans une cellule, sélectionnez la cellule et commencez à taper. Ce que vous tapez remplace le contenu de la cellule. Appuyez sur **Entrée** pour le valider, ou sur **Échap** pour abandonner la saisie et laisser la cellule telle qu'elle était.

## Trois façons de commencer

- **Taper directement dans la cellule.** Sélectionnez une cellule et tapez. Le premier caractère que vous tapez commence la saisie, et la cellule affiche ce que vous tapez au fur et à mesure.
- **Modifier le contenu de la cellule.** Appuyez sur **F2**, ou double-cliquez sur la cellule. La cellule s'ouvre avec son contenu, et le curseur se place à la fin. Utilisez les touches de direction pour vous déplacer dans le texte.
- **Utiliser la barre de formule.** Cliquez dans la barre de formule, à côté de la zone Nom, et tapez-y. La cellule affiche aussi ce que vous tapez.

Lorsque vous tapez directement dans une cellule, une touche de direction valide la saisie et déplace la sélection. Lorsque vous modifiez le contenu de la cellule, les touches de direction se déplacent dans le texte. Appuyez sur **F2** pour passer de l'un à l'autre.

## Valider la saisie

| Touches | Ce qu'elles font |
| --- | --- |
| Entrée | Valider la saisie, et descendre (ou aller là où **Fichier > Options > Modification** l'indique) |
| Maj+Entrée | Valider la saisie, et remonter |
| Ctrl+Entrée | Mettre la saisie dans toutes les cellules sélectionnées, et rester à place |
| Tab | Valider la saisie, et aller à droite |
| Maj+Tab | Valider la saisie, et aller à gauche |
| Touches de direction | Valider la saisie, et aller d'une cellule (seulement lorsque vous avez tapé directement dans la cellule) |
| Échap | Abandonner la saisie |
| Alt+Entrée | Commencer une nouvelle ligne dans la cellule |
| Ctrl+; | Saisir la date du jour (dans la saisie, lorsque vous êtes en train d'en taper une) |
| Ctrl+Maj+; | Saisir l'heure actuelle |
| Alt+Bas | Afficher les mots déjà présents dans la colonne, et en choisir un |

**Ctrl+Entrée** avec plusieurs cellules sélectionnées met ce que vous avez tapé dans toutes. Une formule est écrite pour la cellule où vous tapez, et dans chacune des autres elle se lit comme une copie de celle-ci : si vous tapez **=A1** avec le bloc E6:F7 sélectionné, F7 reçoit **=B2**. C'est une seule étape d'annulation.

**Alt+Bas** sur une cellule affiche, dans l'ordre et une seule fois chacun, les mots des cellules situées au-dessus et en dessous, sans cellule vide entre elles. Choisir un mot le saisit. Une cellule qui a sa propre liste affiche cette liste à la place.

Dans **Fichier > Options > Modification**, choisissez si **Entrée** descend, va à droite, monte, va à gauche, ou ne déplace pas la sélection.

Si vous saisissez dans plusieurs cellules d'une ligne avec **Tab**, puis appuyez sur **Entrée**, la sélection descend à la ligne suivante, dans la colonne où vous avez commencé. Si vous sélectionnez d'abord un bloc de cellules, **Entrée** et **Tab** se déplacent dans ce bloc.

## Correction automatique pendant la saisie

Lorsque vous tapez une espace ou un signe de ponctuation, comme un point, après un mot, JeffCrunch peut corriger ce mot dès que vous l'achevez. Il corrige les fautes de frappe courantes (comme « le » pour « the »), deux majuscules au début d'un mot, une petite lettre qui commence une phrase ou un jour de la semaine, et un guillemet droit (il devient un guillemet typographique). Il remplace aussi deux traits d'union entre des mots par un tiret cadratin, et 1/2, 1/4 et 3/4 par ½, ¼ et ¾ lorsque la saisie contient des mots. Un nombre comme 1/2 tapé seul reste une date ou une fraction, comme auparavant.

Chaque correction est une étape à part. Appuyez sur **{key:Undo}** juste après une correction pour l'annuler seule et conserver ce que vous avez tapé. Rien n'est modifié dans une formule : une saisie qui commence par un signe égal, ou par un signe plus ou moins suivi d'une formule, reste telle que vous l'avez tapée. Une adresse Internet (comme https://example.org) ou une adresse e-mail qui constitue toute la saisie devient un lien dans sa cellule lorsque vous validez, si **Convertir les adresses web en liens au fur et à mesure de la saisie** est activé.

Activez ou désactivez ces options dans **Fichier > Options > Vérification**. Un mot mal orthographié est signalé pendant la saisie : voir [Orthographe](notes-links).

## Mettre en forme une partie du texte

Certains mots d'une cellule peuvent avoir un aspect différent du reste. Par exemple, un mot peut être en gras ou en rouge.

1. Appuyez sur F2 pour modifier la cellule (ou double-cliquez dessus).
2. Sélectionnez les mots à modifier, avec Maj et les touches de direction, ou avec la souris.
3. Utilisez **Gras**, **Italique**, **Souligné**, **Barré**, **Police**, **Taille de police** ou **Couleur de police** dans l'onglet **Accueil**, ou Ctrl+G, Ctrl+I et Ctrl+S.
4. Appuyez sur Entrée.

Seuls les mots sélectionnés changent. Le reste de la cellule conserve l'aspect propre à la cellule. La zone de saisie affiche les mots avec un seul aspect pendant la modification, et la cellule affiche chaque partie avec son propre aspect lorsque vous appuyez sur Entrée. Si vous tapez ou supprimez des mots plus tard, la mise en forme suit les mots. Choisir une police ou une taille sans mot sélectionné modifie toute la cellule, comme auparavant.

Les cellules qui contiennent une mise en forme à l'intérieur du texte, issue d'un autre programme, l'affichent, la conservent et l'enregistrent. Une cellule que vous modifiez conserve sa mise en forme. La mise en forme à l'intérieur d'un texte s'applique au texte, et non aux nombres ni aux formules. Une copie enregistrée au format .xls ancien ou au format .ods conserve les mots, et l'enregistrement vous indique que la mise en forme à l'intérieur de ces mots n'est pas conservée.

## Ce que devient ce que vous tapez

JeffCrunch lit ce que vous tapez selon les paramètres de votre ordinateur. Le séparateur décimal, le séparateur des milliers, le symbole monétaire et l'ordre du jour et du mois dans une date proviennent de ces paramètres. Un nombre tapé dans une cellule qui n'a pas de format propre reçoit le format qui l'affiche comme vous l'avez tapé.

| Vous tapez | La cellule contient | Et affiche |
| --- | --- | --- |
| 1234.5 | le nombre 1234.5 | 1234.5 |
| 5% | le nombre 0.05 | 5 % |
| 12,50 € | le nombre 12.5 | 12,50 € |
| 1 1/2 | le nombre 1.5 | 1 1/2 |
| 4/3/2025 | la date, sous forme de nombre de jours | 04/03/2025 |
| 22:30 | l'heure, sous forme de fraction de jour | 22:30 |
| VRAI | la valeur logique vraie | VRAI |
| #N/A | l'erreur #N/A | #N/A |
| =B2*2 | une formule | le résultat de la formule |
| tout autre texte | du texte | le texte |

Une cellule qui a déjà un format propre, comme un pourcentage ou une date, le conserve. Une cellule dont le format est **Texte** conserve tout ce que vous tapez sous forme de texte, sauf une formule.

Pour garder sous forme de texte quelque chose qui ressemble à un nombre, comme un code **007**, tapez d'abord une apostrophe : **'007**. L'apostrophe ne fait pas partie du texte. Lorsque vous modifiez la cellule, vous la revoyez.

## Formules

Une formule commence par un signe égal. Par exemple, **=B2*2** double le nombre de B2. Tapez les noms des fonctions en majuscules ou en minuscules ; JeffCrunch les écrit en majuscules.

Les noms des fonctions sont toujours les noms anglais, quelle que soit la langue : tapez **SUM**, et non le nom que votre langue lui donne. Les signes entre les éléments suivent les paramètres de votre ordinateur. Lorsque le séparateur décimal est la virgule, comme en français, vous séparez les arguments d'une fonction par un point-virgule et vous écrivez une virgule dans un nombre : **=SOMME(A1;A2)*0,5**. La barre de formule et **F2** affichent les formules de la même façon. Lorsque le séparateur décimal est le point, vous séparez les arguments par une virgule : **=SUM(A1,A2)*0.5**. Le fichier conserve toujours la forme que tous les programmes lisent, avec des virgules entre les arguments et des points dans les nombres, afin qu'il s'ouvre de la même façon partout.

Si une formule comporte une erreur, comme une parenthèse qui n'est jamais fermée, JeffCrunch vous indique le problème, et la saisie reste telle que vous l'avez tapée, avec le curseur près de l'erreur, pour que vous puissiez la corriger. Rien n'est modifié tant que la formule n'est pas correcte.

Une cellule contient jusqu'à 32 767 caractères, et une formule jusqu'à 8 192. JeffCrunch vous signale quand une saisie est plus longue.

## Effacer des cellules

Sélectionnez les cellules et appuyez sur **Suppr**. Cela efface le contenu des cellules et conserve leur mise en forme, et c'est une étape que vous pouvez annuler. JeffCrunch efface jusqu'à 100 000 cellules à la fois.

Dans l'onglet **Accueil**, le bouton **Effacer** possède un menu. **Tout effacer** vide les cellules, leur mise en forme, leurs notes, leurs commentaires (les fils avec des réponses) et leurs liens. Un fil de plus de 500 commentaires est conservé, et JeffCrunch vous le signale, car une annulation ne pourrait pas restaurer les commentaires qu'il n'a pas lus. **Effacer les formats** conserve le contenu des cellules et rétablit le format standard. **Effacer le contenu** est identique à **Suppr**. **Effacer les commentaires et les notes** retire seulement les notes et les commentaires des cellules sélectionnées. **Effacer les liens hypertextes** retire seulement les liens, et les cellules conservent leur texte. Un choix est grisé lorsque les cellules sélectionnées n'ont rien à effacer à cet égard.

## Annuler et rétablir

**{key:Undo}** annule la dernière modification, et **{key:Redo}** la refait. Ces commandes figurent aussi dans la barre d'outils Accès rapide, en haut. Chaque saisie dans une cellule est une étape, et chaque effacement aussi. L'annulation remet la cellule dans son état précédent : sa valeur, sa formule, son format, et les formules qui en dépendent. JeffCrunch mémorise jusqu'à 1 000 étapes, moins lorsqu'elles ont modifié un très grand nombre de cellules, afin de ne jamais occuper beaucoup de mémoire.

## Pendant que JeffCrunch calcule les formules

Lorsque vous modifiez une cellule, JeffCrunch calcule en arrière-plan les formules qui en dépendent. La cellule modifiée affiche aussitôt son nouveau contenu, et les autres cellules se mettent à jour un instant plus tard ; la barre d'état affiche **Calcul en cours** jusqu'à la fin. Vous pouvez continuer à taper pendant ce temps.

## Ce qui ne peut pas être modifié

- Une cellule qui fait partie d'une **formule matricielle** ou d'une table de données ne peut pas être modifiée seule.
- Sur une **feuille protégée**, une cellule verrouillée ne peut pas être modifiée. Toutes les cellules sont verrouillées, sauf si le classeur indique le contraire. JeffCrunch vous le signale lorsque vous essayez. Pour modifier la cellule, choisissez Ôter la protection de la feuille dans l'onglet Révision.
- La première ligne d'un **tableau** contient les noms de ses colonnes. Tapez un nouveau nom dans l'une de ces cellules pour renommer la colonne : les formules qui utilisent l'ancien nom suivent, et le tableau du fichier reste valide. Un nom déjà utilisé par une autre colonne reçoit un numéro à la suite, un nom ne peut pas être vide, et **Suppr** n'efface pas cette ligne.
- Un classeur qui n'a pas pu être lu entièrement peut être consulté, mais ni modifié ni enregistré.
- Une fonction que JeffCrunch ne connaît pas encore laisse une nouvelle formule sans valeur dans la cellule ; le classeur demande au programme qui l'ouvre de calculer la formule.
- La mise en forme, le tri, ainsi que rechercher et remplacer, seront disponibles dans une version ultérieure. L'insertion et la suppression, le couper-coller, le remplissage et les dimensions des lignes et des colonnes sont décrits dans [Insérer et supprimer des lignes, des colonnes et des cellules](insert-delete), [Couper, copier et coller](clipboard), [Remplir des cellules et en faire la somme](fill-sum) et [Modifier la largeur des colonnes et la hauteur des lignes](sizes).

## Nouveaux classeurs

Dans **Fichier > Options > Nouveaux classeurs**, choisissez la police, la taille de police et le nombre de feuilles avec lesquels chaque nouveau classeur démarre. Les classeurs que vous avez déjà ne sont pas modifiés.
