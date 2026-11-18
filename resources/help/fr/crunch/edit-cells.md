# Saisir dans les cellules
<!-- keywords: symbole, police de symboles, caractères spéciaux, raccourci clavier pour un symbole, correction automatique, guillemets typographiques, tiret, fraction, lien, adresse, mot en gras, partie d'une cellule, texte enrichi, mettre en forme une partie du texte, colorer un mot, saisie, saisir, taper, entrer, modifier, changer, cellule, contenu, barre de formule, F2, double-clic, Tab, Échap, abandonner, annuler, rétablir, supprimer, effacer, pourcentage, monnaie, argent, date, heure, nombre, texte, apostrophe, formule, égal, remplacer, limite, caractères -->

Pour saisir quelque chose dans une cellule, sélectionnez la cellule et commencez à taper. Ce que vous tapez remplace le contenu de la cellule. Appuyez sur **Entrée** pour le valider, ou sur **Échap** pour l'abandonner et laisser la cellule telle qu'elle était.

## Trois façons de commencer

- **Taper directement dans la cellule.** Sélectionnez une cellule et tapez. Le premier caractère que vous saisissez démarre la saisie, et la cellule affiche ce que vous tapez au fur et à mesure.
- **Modifier le contenu de la cellule.** Appuyez sur **F2**, ou double-cliquez sur la cellule. La cellule s'ouvre avec son contenu, et le curseur se place à la fin. Utilisez les touches fléchées pour vous déplacer dans le texte.
- **Utiliser la barre de formule.** Cliquez dans la barre de formule, à côté de la zone Nom, et tapez-y. La cellule affiche aussi ce que vous tapez.

Lorsque vous tapez directement dans une cellule, une touche fléchée termine la saisie et déplace la sélection. Lorsque vous modifiez le contenu de la cellule, les touches fléchées se déplacent dans le texte. Appuyez sur **F2** pour passer de l'un à l'autre.

## Terminer la saisie

| Touches | Effet |
| --- | --- |
| Entrée | Valide la saisie, et descend (ou va où indique **Fichier > Options > Modification**) |
| Maj+Entrée | Valide la saisie, et remonte |
| Ctrl+Entrée | Place la saisie dans toutes les cellules sélectionnées, et reste à sa place |
| Tab | Valide la saisie, et va à droite |
| Maj+Tab | Valide la saisie, et va à gauche |
| Touches fléchées | Valide la saisie, et passe à la cellule voisine (seulement lorsque vous avez tapé directement dans la cellule) |
| Échap | Abandonne la saisie |
| Alt+Entrée | Insère un saut de ligne dans la cellule |
| Ctrl+; | Insère la date du jour (dans la saisie, lorsque vous en tapez une) |
| Ctrl+Maj+; | Insère l'heure actuelle |
| Alt+Bas | Affiche les mots déjà présents dans la colonne, et permet d'en choisir un |

**Ctrl+Entrée** avec plusieurs cellules sélectionnées place ce que vous avez tapé dans toutes. Une formule est écrite pour la cellule où vous la saisissez, et dans les autres elle se comporte comme une copie de celle-ci : si vous tapez **=A1** alors que le bloc E6:F7 est sélectionné, F7 reçoit **=B2**. Cette opération constitue une seule étape d'annulation.

**Alt+Bas** sur une cellule affiche, dans l'ordre et une seule fois chacun, les mots des cellules situées au-dessus et en dessous, sans qu'une cellule vide les sépare. Choisir un mot le tape. Une cellule qui possède sa propre liste affiche cette liste à la place.

Dans **Fichier > Options > Modification**, choisissez si **Entrée** descend, va à droite, remonte, va à gauche, ou ne déplace pas la sélection.

Si vous saisissez plusieurs cellules d'une ligne avec **Tab**, puis appuyez sur **Entrée**, la sélection descend à la ligne suivante, dans la colonne où vous avez commencé. Si vous sélectionnez d'abord un bloc de cellules, **Entrée** et **Tab** se déplacent à l'intérieur de ce bloc.

## Correction automatique pendant la saisie

Lorsque vous tapez une espace ou un signe de ponctuation, comme un point, après un mot, JeffCrunch peut corriger le mot dès que vous l'avez terminé. Il corrige les fautes de frappe courantes (par exemple « teh » pour « the »), les deux majuscules au début d'un mot, une petite lettre qui commence une phrase ou un jour de la semaine, et les guillemets droits, qui deviennent typographiques. Il remplace aussi deux traits d'union entre des mots par un tiret long, et 1/2, 1/4 et 3/4 par ½, ¼ et ¾ lorsque la saisie contient des mots. Un nombre comme 1/2 tapé seul reste une date ou une fraction, comme auparavant.

Chaque correction constitue une étape à part. Appuyez sur **Ctrl+Z** juste après une correction pour annuler seulement cette correction et conserver ce que vous avez tapé. Rien n'est modifié dans une formule : une saisie qui commence par un signe égal, ou par un plus ou un moins suivi d'une formule, reste telle que vous l'avez tapée. Une adresse web (comme https://example.org) ou une adresse e-mail qui constitue à elle seule la saisie devient un lien dans sa cellule lorsque vous validez, si **Convertir les adresses web en liens au fur et à mesure de la saisie** est activé.

Activez ou désactivez ces options dans **Fichier > Options > Vérification**. Un mot mal orthographié est signalé pendant la saisie : voir [Orthographe](notes-links).

## Mettre en forme une partie du texte

Certains mots d'une cellule peuvent avoir un aspect différent du reste. Par exemple, un mot peut être en gras ou en rouge.

1. Appuyez sur F2 pour modifier la cellule (ou double-cliquez dessus).
2. Sélectionnez les mots à modifier, avec Maj et les touches fléchées, ou avec la souris.
3. Utilisez **Gras**, **Italique**, **Souligné**, **Barré**, **Police**, **Taille de police** ou **Couleur de police** dans l'onglet **Accueil**, ou Ctrl+G, Ctrl+I et Ctrl+S.
4. Appuyez sur Entrée.

Seuls les mots sélectionnés changent. Le reste de la cellule conserve son aspect propre. La zone de saisie affiche les mots dans un seul aspect pendant la modification, et la cellule affiche chaque partie avec son aspect lorsque vous appuyez sur Entrée. Si vous tapez ou supprimez des mots ensuite, la mise en forme suit les mots. Choisir une police ou une taille sans mot sélectionné modifie toute la cellule, comme auparavant.

## La mini barre d'outils

Une petite barre des mises en forme que vous utilisez le plus apparaît là où vous travaillez, afin de ne pas devoir remonter au ruban. Elle ne prend jamais le clavier à la cellule : ce que vous étiez en train de taper reste à sa place.

- **Texte sélectionné dans une cellule :** sélectionnez des mots à la souris pendant la modification d'une cellule, et la barre apparaît au-dessus d'eux. Elle comporte les zones **police** et **taille**, **Gras**, **Italique**, **Souligné**, **Augmenter la taille de police**, **Réduire la taille de police** et **Couleur de police**. Elles ne modifient que les mots sélectionnés, comme sur le ruban.
- **Cellules sélectionnées :** faites un clic droit sur les cellules, et la barre se trouve en haut du menu contextuel. Elle comporte aussi **Couleur de remplissage**, **Bordures**, **Monétaire**, **Style de pourcentage**, **Séparateur de milliers**, **Ajouter une décimale**, **Réduire les décimales**, **Fusionner et centrer** et **Reproduire la mise en forme**. Ce sont les commandes du ruban elles-mêmes, et chacune constitue une seule étape que vous pouvez annuler.

La barre au-dessus du texte sélectionné s'estompe lorsque vous éloignez le pointeur, et disparaît lorsque le pointeur est loin. Elle disparaît aussi lorsque vous tapez, appuyez sur Échap ou faites défiler la page. Pour la désactiver, décochez **Afficher la mini barre d'outils à la sélection** dans la page **Général** de **Fichier > Options** ; les autres applications JeffOffice suivent ce choix également. Le bouton Analyse rapide qui apparaît sous une plage sélectionnée n'est pas disponible dans cette version.

Les cellules qui contiennent une mise en forme à l'intérieur du texte, provenant d'un autre programme, l'affichent, la conservent et l'enregistrent. Une cellule que vous modifiez conserve sa mise en forme. La mise en forme à l'intérieur d'un texte s'applique au texte, et non aux nombres ni aux formules. Une copie enregistrée au format .xls ancien ou au format .ods conserve les mots, et l'enregistrement vous indique que la mise en forme à l'intérieur de ces mots n'est pas conservée.

## Ce que vous tapez devient

JeffCrunch interprète ce que vous tapez selon les paramètres de votre ordinateur. Le séparateur décimal, le séparateur des milliers, le symbole monétaire et l'ordre du jour et du mois dans une date proviennent de ces paramètres. Un nombre saisi dans une cellule qui n'a pas de format propre reçoit le format qui l'affiche comme vous l'avez tapé.

| Vous tapez | La cellule contient | Et affiche |
| --- | --- | --- |
| 1234.5 | le nombre 1234,5 | 1234,5 |
| 5 % | le nombre 0,05 | 5 % |
| 12,50 € | le nombre 12,5 | 12,50 € |
| 1 1/2 | le nombre 1,5 | 1 1/2 |
| 4/3/2025 | la date, sous forme de nombre de jours | 04/03/2025 |
| 22:30 | l'heure, sous forme de fraction de jour | 22:30 |
| VRAI | la valeur logique vrai | VRAI |
| #N/A | l'erreur #N/A | #N/A |
| =B2*2 | une formule | le résultat de la formule |
| tout le reste | du texte | le texte |

Une cellule qui possède déjà un format propre, comme un pourcentage ou une date, le conserve. Une cellule dont le format est **Texte** conserve tout ce que vous tapez comme du texte, sauf une formule.

Pour conserver comme texte quelque chose qui ressemble à un nombre, comme un code **007**, tapez d'abord une apostrophe : **'007**. L'apostrophe ne fait pas partie du texte. Lorsque vous modifiez la cellule, elle réapparaît.

## Formules

Une formule commence par un signe égal. Par exemple, **=B2*2** double le nombre contenu dans B2. Les noms des fonctions sont toujours les noms anglais, quelle que soit la langue : saisissez **SUM**, et non le nom que donne votre langue. Vous pouvez les taper en majuscules ou en minuscules ; JeffCrunch les écrit en majuscules.

Les séparateurs entre les éléments d'une formule suivent les paramètres de votre ordinateur. Lorsque le séparateur décimal est une virgule, comme en français et en allemand, vous séparez les arguments d'une fonction par un point-virgule, et vous écrivez une virgule dans un nombre : **=SUM(A1;A2)*0,5**. La barre de formule et **F2** affichent les formules de la même façon. Lorsque le séparateur décimal est un point, vous séparez les arguments par une virgule : **=SUM(A1,A2)*0.5**. Le fichier conserve toujours la forme que tous les programmes lisent, avec des virgules entre les arguments et des points dans les nombres, afin qu'il s'ouvre de la même façon partout.

Si une formule comporte une erreur, comme une parenthèse jamais fermée, JeffCrunch vous indique le problème. La saisie reste telle que vous l'avez tapée, avec le curseur près de l'erreur, afin que vous puissiez la corriger. Rien n'est modifié tant que la formule n'est pas juste.

Une cellule contient jusqu'à 32 767 caractères, et une formule jusqu'à 8 192. JeffCrunch vous prévient lorsqu'une saisie est plus longue.

## Effacer des cellules

Sélectionnez les cellules et appuyez sur **Suppr**. Cela efface leur contenu et conserve leur format ; c'est une étape que vous pouvez annuler. JeffCrunch efface jusqu'à 100 000 cellules à la fois.

Dans l'onglet **Accueil**, le bouton **Effacer** possède un menu. **Tout effacer** vide les cellules, ainsi que leurs formats, leurs notes, leurs commentaires (les fils avec réponses) et leurs liens. Un fil de plus de 500 commentaires est conservé, et JeffCrunch le signale, car une annulation ne pourrait pas rétablir les commentaires qu'il n'a pas lus. **Effacer les formats** conserve le contenu des cellules et rétablit le format standard. **Effacer le contenu** équivaut à **Suppr**. **Effacer les commentaires et les notes** supprime seulement les notes et les commentaires des cellules sélectionnées. **Effacer les liens hypertextes** supprime seulement les liens, et les cellules conservent leur texte. Un choix est grisé lorsque les cellules sélectionnées n'ont rien à effacer selon ce choix.

## Annuler et rétablir

**Ctrl+Z** annule la dernière modification, et **Ctrl+Y** la rétablit. Ces commandes figurent aussi dans la barre d'outils Accès rapide, en haut. Chaque cellule saisie constitue une étape, et chaque effacement aussi. L'annulation rétablit la cellule telle qu'elle était : sa valeur, sa formule, son format, ainsi que les formules qui en dépendent. JeffCrunch conserve jusqu'à 1 000 étapes, moins lorsque les modifications portent sur un très grand nombre de cellules, afin de ne jamais occuper beaucoup de mémoire.

## Pendant que JeffCrunch calcule les formules

Lorsque vous modifiez une cellule, JeffCrunch calcule en arrière-plan les formules qui en dépendent. La cellule modifiée affiche aussitôt son nouveau contenu, et les autres cellules se mettent à jour un instant plus tard ; la barre d'état affiche **Calcul en cours** jusqu'à la fin. Vous pouvez continuer à taper pendant ce temps.

## Ce qui ne peut pas être modifié

- Une cellule qui fait partie d'une formule matricielle ou d'un tableau de données ne peut pas être modifiée seule.
- Sur une feuille protégée, une cellule verrouillée ne peut pas être modifiée. Toutes les cellules sont verrouillées, sauf si le classeur indique le contraire. JeffCrunch vous le signale lorsque vous essayez. Pour modifier la cellule, choisissez **Ôter la protection de la feuille** dans l'onglet **Révision**.
- La première ligne d'un **tableau** contient le nom de ses colonnes. Tapez un nouveau nom dans l'une de ces cellules pour renommer la colonne : les formules qui utilisent l'ancien nom suivent, et le tableau du fichier reste valide. Un nom déjà utilisé par une autre colonne reçoit un numéro à la suite, un nom ne peut pas être vide, et **Suppr** n'efface pas cette ligne.
- Un classeur qui n'a pas pu être lu en entier peut être consulté, mais ni modifié ni enregistré.
- Une fonction que JeffCrunch ne connaît pas encore laisse une nouvelle formule sans valeur dans la cellule ; le classeur demande au programme qui l'ouvre de calculer la formule.
- La mise en forme, le tri, ainsi que la recherche et le remplacement arriveront dans une version ultérieure. L'insertion et la suppression, le couper-coller, le remplissage et les dimensions des lignes et des colonnes sont décrits dans [Insérer et supprimer des lignes, des colonnes et des cellules](insert-delete), [Couper, copier et coller](clipboard), [Recopier des cellules et les additionner](fill-sum) et [Modifier la largeur des colonnes et la hauteur des lignes](sizes).

## Insérer un symbole

Cliquez sur **Insertion > Symboles > Symbole** pour ouvrir la boîte Symbole. Choisissez un caractère et cliquez sur **Insérer**, ou double-cliquez dessus. Il s'insère au curseur de la saisie en cours. Si vous ne saisissez rien, le contenu de la cellule s'ouvre avec le symbole ajouté à la fin, et **Entrée** le place dans la cellule. Exemple : avec une cellule contenant le mot *Total*, le choix du signe euro donne la saisie *Total€*. Les symboles utilisés en dernier sont listés en premier.

**Prendre le caractère dans une police.** Choisissez une police dans la zone **Police**, puis choisissez le caractère. Le caractère s'insère dans une portion qui lui est propre, rédigée dans cette police, et le reste du texte de la cellule conserve la police de la cellule. Exemple : dans une cellule contenant *Total*, une coche choisie dans une police de symboles donne *Total✓*, avec seulement la coche dans cette police. Avec **(texte normal)** choisi, c'est la police de la cellule qui dessine le symbole, de sorte qu'un symbole absent de la police apparaît sous forme de carré. Dans une formule, seul le caractère est inséré.

**Caractères spéciaux.** L'onglet **Caractères spéciaux** liste l'espace insécable et le trait d'union insécable, le trait d'union conditionnel, le tiret demi-cadratin et le tiret cadratin, l'espace demi-cadratin et l'espace cadratin, les guillemets, ainsi que la coupure facultative de largeur nulle, chacun avec son nom. Double-cliquez sur l'un d'eux pour l'insérer.

**Donner un raccourci clavier à un symbole.** Choisissez un symbole ou un caractère spécial, cliquez sur **Raccourci clavier**, appuyez sur les touches souhaitées, par exemple **Ctrl+Alt+Maj+J**, puis cliquez sur **OK**. Ces touches insèrent alors ce symbole, de la même façon que la boîte. Laissez le champ vide et cliquez sur **OK** pour les supprimer. JeffCrunch vous signale lorsque d'autres touches sont déjà attribuées à une commande. Le symbole devient une commande, il figure donc aussi dans **Fichier > Options > Personnaliser > Clavier**, où vous pouvez modifier ses touches (voir [Raccourcis clavier](keyboard)).

## Nouveaux classeurs

Dans **Fichier > Options > Nouveaux classeurs**, choisissez la police, la taille de police et le nombre de feuilles avec lesquels chaque nouveau classeur démarre. Les classeurs que vous avez déjà ne sont pas modifiés.
