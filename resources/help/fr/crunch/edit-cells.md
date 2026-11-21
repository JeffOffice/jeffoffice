# Saisir dans les cellules
<!-- keywords: symbole, police de symboles, caractères spéciaux, touche de raccourci pour un symbole, correction automatique, guillemets typographiques, tiret, fraction, lien, adresse, mot en gras, partie d'une cellule, texte enrichi, mettre en forme une partie du texte, colorer un mot, saisie, saisir, taper, entrer, modifier, changer, cellule, contenu, barre de formule, F2, double-clic, Tab, Échap, abandonner, annuler, rétablir, supprimer, effacer, pourcentage, monétaire, argent, date, heure, nombre, texte, apostrophe, formule, égal, remplacer, limite, caractères -->

Pour saisir quelque chose dans une cellule, sélectionnez la cellule et commencez à taper. Ce que vous tapez remplace le contenu de la cellule. Appuyez sur **Entrée** pour le valider, ou sur **Échap** pour l'abandonner et laisser la cellule telle qu'elle était.

## Trois façons de commencer

- **Taper directement dans la cellule.** Sélectionnez une cellule et tapez. Le premier caractère lance la saisie, et la cellule affiche ce que vous tapez au fur et à mesure.
- **Modifier le contenu de la cellule.** Appuyez sur **F2**, ou double-cliquez sur la cellule. La cellule s'ouvre avec son contenu, et le curseur se place à la fin. Utilisez les touches fléchées pour vous déplacer dans le texte.
- **Utiliser la barre de formule.** Cliquez dans la barre de formule, à côté de la zone Nom, et tapez-y. La cellule affiche aussi ce que vous tapez.

Lorsque vous tapez directement dans une cellule, une touche fléchée valide la saisie et déplace la sélection. Lorsque vous modifiez le contenu de la cellule, les touches fléchées se déplacent dans le texte. Appuyez sur **F2** pour passer de l'un à l'autre.

## Valider la saisie

| Touches | Ce qu'elles font |
| --- | --- |
| Entrée | Valider la saisie, et descendre (ou aller où indique **Fichier > Options > Modification**) |
| Maj+Entrée | Valider la saisie, et remonter |
| Ctrl+Entrée | Mettre la saisie dans toutes les cellules sélectionnées, et rester sur place |
| Tab | Valider la saisie, et aller à droite |
| Maj+Tab | Valider la saisie, et aller à gauche |
| Touches fléchées | Valider la saisie, et aller d'une cellule (seulement lorsque vous avez tapé directement dans la cellule) |
| Échap | Abandonner la saisie |
| Alt+Entrée | Commencer une nouvelle ligne dans la cellule |
| Ctrl+; | Insérer la date du jour (dans la saisie, lorsque vous tapez une date) |
| Ctrl+Maj+; | Insérer l'heure actuelle |
| Alt+Flèche bas | Afficher les mots déjà présents dans la colonne, et en choisir un |

**Ctrl+Entrée** avec plusieurs cellules sélectionnées met ce que vous avez tapé dans toutes. Une formule est écrite pour la cellule où vous tapez, et dans chacune des autres elle se lit comme une copie le ferait : si vous tapez **=A1** alors que le bloc E6:F7 est sélectionné, F7 reçoit **=B2**. Cette action compte comme une seule étape d'annulation.

**Alt+Flèche bas** sur une cellule affiche, dans l'ordre et une seule fois chacun, les mots des cellules situées au-dessus et au-dessous, sans cellule vide entre elles. Le choix d'un mot le tape. Une cellule qui possède sa propre liste affiche cette liste à la place.

Dans **Fichier > Options > Modification**, choisissez si **Entrée** descend, va à droite, remonte, va à gauche ou ne déplace pas la sélection.

Si vous saisissez dans plusieurs cellules d'une ligne avec **Tab**, puis appuyez sur **Entrée**, la sélection descend à la ligne suivante, dans la colonne où vous avez commencé. Si vous sélectionnez d'abord un bloc de cellules, **Entrée** et **Tab** parcourent le bloc.

## Correction automatique pendant la saisie

Lorsque vous tapez une espace ou un signe de ponctuation, comme un point, après un mot, JeffCrunch peut corriger le mot dès que vous le terminez. Il corrige les fautes de frappe courantes (par exemple « teh » devient « the »), deux majuscules au début d'un mot, une petite lettre qui commence une phrase ou un jour de la semaine, et un guillemet droit (il devient typographique). Il remplace aussi deux traits d'union entre des mots par un tiret cadratin, et 1/2, 1/4 et 3/4 par ½, ¼ et ¾ lorsque la saisie contient des mots. Un nombre comme 1/2 tapé seul reste une date ou une fraction, comme auparavant.

Chaque correction est une étape à part. Appuyez sur **Annuler** juste après une correction pour annuler seulement celle-ci et garder ce que vous avez tapé. Une formule n'est jamais modifiée : une saisie qui commence par un signe égal, ou par un signe plus ou moins suivi d'une formule, reste telle que vous l'avez tapée. Une adresse web (comme https://example.org) ou une adresse e-mail qui constitue toute la saisie devient un lien dans sa cellule lorsque vous validez la saisie, si l'option **Transformer les adresses web en liens au fur et à mesure de la saisie** est activée.

Activez ou désactivez ces options dans **Fichier > Options > Vérification**. Un mot mal orthographié est signalé pendant la saisie : voir [Orthographe](notes-links).

## Mettre en forme une partie du texte

Certains mots d'une cellule peuvent avoir un aspect différent du reste. Par exemple, un mot peut être en gras ou en rouge.

1. Appuyez sur F2 pour modifier la cellule (ou double-cliquez dessus).
2. Sélectionnez les mots à modifier, avec Maj et les touches fléchées, ou avec la souris.
3. Utilisez **Gras**, **Italique**, **Souligné**, **Barré**, **Police**, **Taille de police** ou **Couleur de police** dans l'onglet **Accueil**, ou Ctrl+G, Ctrl+I et Ctrl+S.
4. Appuyez sur Entrée.

Seuls les mots sélectionnés changent. Le reste de la cellule conserve son aspect propre. La zone de saisie affiche les mots avec un seul aspect pendant la modification, et la cellule affiche chaque partie avec son aspect propre lorsque vous appuyez sur Entrée. Si vous ajoutez ou supprimez des mots plus tard, la mise en forme suit les mots. Le choix d'une police ou d'une taille sans mot sélectionné modifie toute la cellule, comme d'habitude.

## La mini barre d'outils

Une petite barre des mises en forme les plus utilisées apparaît à proximité de votre travail, pour éviter de remonter au ruban. Elle ne retire jamais le clavier à la cellule : ce que vous étiez en train de taper reste à sa place.

- **Texte sélectionné dans une cellule :** sélectionnez des mots à la souris pendant la modification d'une cellule, et la barre apparaît au-dessus d'eux. Elle comprend les zones **police** et **taille**, **Gras**, **Italique**, **Souligné**, **Augmenter la taille de police**, **Réduire la taille de police** et **Couleur de police**. Elles ne modifient que les mots sélectionnés, comme sur le ruban.
- **Cellules sélectionnées :** faites un clic droit sur les cellules, et la barre se trouve en haut du menu contextuel. Elle comprend aussi **Couleur de remplissage**, **Bordures**, **Monétaire**, **Style pourcentage**, **Style milliers**, **Ajouter une décimale**, **Réduire les décimales**, **Fusionner et centrer** et **Reproduire la mise en forme**. Ce sont les commandes du ruban elles-mêmes, et chacune est une étape unique que vous pouvez annuler.

La barre au-dessus du texte sélectionné s'estompe lorsque vous éloignez le pointeur, et disparaît lorsqu'il est loin. Elle disparaît aussi lorsque vous tapez, appuyez sur Échap ou faites défiler. Pour la désactiver, décochez **Afficher la mini barre d'outils à la sélection** dans la page **Général** de **Fichier > Options** ; les autres applications JeffOffice suivent ce choix. Le bouton Analyse rapide qui apparaît sous une plage sélectionnée ne figure pas dans cette version.

Les cellules qui contiennent une mise en forme à l'intérieur du texte, provenant d'un autre programme, l'affichent, la conservent et l'enregistrent. Une cellule que vous modifiez conserve sa mise en forme. La mise en forme à l'intérieur d'un texte s'applique au texte, et non aux nombres ni aux formules. Une copie enregistrée au format .xls ancien ou au format .ods conserve les mots, et l'enregistrement vous indique que la mise en forme à l'intérieur de ceux-ci n'est pas conservée.

## Ce que vous tapez devient

JeffCrunch interprète ce que vous tapez selon les paramètres de votre ordinateur. Le séparateur décimal, le séparateur des milliers, le symbole monétaire et l'ordre du jour et du mois dans une date proviennent de ces paramètres. Un nombre tapé dans une cellule qui n'a pas de format propre reçoit le format qui l'affiche comme vous l'avez tapé.

| Vous tapez | La cellule contient | Et affiche |
| --- | --- | --- |
| 1234.5 | le nombre 1234.5 | 1234.5 |
| 5% | le nombre 0,05 | 5 % |
| 12,50 € | le nombre 12,5 | 12,50 € |
| 1 1/2 | le nombre 1,5 | 1 1/2 |
| 4/3/2025 | la date, sous forme de nombre de jours | 04/03/2025 |
| 22:30 | l'heure, sous forme de fraction de jour | 22:30 |
| VRAI | la valeur logique vrai | VRAI |
| #N/A | l'erreur #N/A | #N/A |
| =B2*2 | une formule | le résultat de la formule |
| tout le reste | du texte | le texte |

Une cellule qui possède déjà un format propre, comme un pourcentage ou une date, le conserve. Une cellule dont le format est **Texte** conserve tout ce que vous tapez comme du texte, sauf une formule.

Pour garder comme texte quelque chose qui ressemble à un nombre, comme un code **007**, tapez d'abord une apostrophe : **'007**. L'apostrophe ne fait pas partie du texte. Lorsque vous modifiez la cellule, vous la revoyez.

## Formules

Une formule commence par un signe égal. Par exemple, **=B2*2** double le nombre de B2. Vous pouvez taper les noms des fonctions en majuscules ou en minuscules ; JeffCrunch les écrit en majuscules.

Les noms des fonctions sont toujours les noms anglais, quelle que soit la langue : tapez **SUM**, et non le nom que donne votre langue. Les signes entre eux suivent les paramètres de votre ordinateur. Lorsque le séparateur décimal est la virgule, comme en allemand et en français, vous séparez les arguments d'une fonction par un point-virgule, et vous écrivez une virgule dans un nombre : **=SOMME(A1;A2)*0,5**. La barre de formule et **F2** affichent les formules de la même manière. Lorsque le séparateur décimal est le point, vous séparez les arguments par une virgule : **=SUM(A1,A2)*0.5**. Le fichier conserve toujours la forme que tous les programmes lisent, avec des virgules entre les arguments et des points dans les nombres, afin qu'il s'ouvre de la même manière partout.

Si une formule comporte une erreur, comme une parenthèse jamais fermée, JeffCrunch vous indique le problème, et la saisie reste telle que vous l'avez tapée, avec le curseur près de l'erreur, pour que vous puissiez la corriger. Rien n'est modifié tant que la formule n'est pas juste.

Une cellule contient jusqu'à 32 767 caractères, et une formule jusqu'à 8 192. JeffCrunch vous prévient lorsqu'une saisie est plus longue.

## Effacer des cellules

Sélectionnez les cellules et appuyez sur **Suppr**. Cela efface leur contenu et conserve leur mise en forme, et c'est une étape que vous pouvez annuler. JeffCrunch efface jusqu'à 100 000 cellules à la fois.

Dans l'onglet **Accueil**, le bouton **Effacer** possède un menu. **Tout effacer** vide les cellules, ainsi que leur mise en forme, leurs notes, leurs commentaires (les fils de discussion avec des réponses) et leurs liens. Un fil de plus de 500 commentaires est conservé, et JeffCrunch le signale, car une annulation ne pourrait pas rétablir des commentaires qu'il n'a pas lus. **Effacer les formats** conserve le contenu des cellules et rétablit le format standard. **Effacer le contenu** équivaut à **Suppr**. **Effacer les commentaires et les notes** retire seulement les notes et les commentaires des cellules sélectionnées. **Effacer les liens hypertextes** retire seulement les liens, et les cellules conservent leur texte. Un choix est grisé lorsque les cellules sélectionnées ne contiennent rien à effacer.

## Annuler et rétablir

**Annuler** annule la dernière modification, et **Rétablir** la refait. Ces commandes figurent aussi dans la barre d'outils Accès rapide, en haut. Chaque saisie dans une cellule est une étape, et chaque effacement aussi. Annuler remet la cellule dans son état précédent : sa valeur, sa formule, son format, ainsi que les formules qui en dépendent. JeffCrunch conserve jusqu'à 1 000 étapes, moins lorsqu'elles ont modifié un très grand nombre de cellules, afin de ne jamais occuper beaucoup de mémoire.

**Revenir en arrière de plusieurs étapes d'un coup.** Dans la barre d'outils Accès rapide, les boutons **Annuler** et **Rétablir** ont chacun une flèche à côté. Cliquez sur la flèche pour voir les étapes, de la plus récente à la plus ancienne (jusqu'à 100 sont listées). Déplacez le pointeur vers le bas de la liste pour indiquer le nombre d'étapes voulu (la ligne sous la liste indique, par exemple, « Annuler 3 actions »), puis cliquez pour les effectuer. Choisir la troisième étape annule les trois premières. Les étapes restent dans l'ordre, si bien que Rétablir les restaure de la même manière.

## Répéter

**F4** refait votre dernière commande sur les cellules actuellement sélectionnées. Exemple : mettez **A1** en gras, sélectionnez **C4:C9**, puis appuyez sur **F4** : les cellules de C4:C9 deviennent grasses, en une seule étape que vous pouvez annuler. Répéter fonctionne pour la mise en forme, les styles de cellule, l'effacement, ainsi que l'insertion et la suppression de lignes et de colonnes. L'info-bulle du bouton, ou la commande **Répéter**, indique ce qu'elle ferait (« Répéter Gras »). **Ctrl+Y** répète aussi, mais seulement lorsqu'il n'y a aucune étape à rétablir (tant qu'une telle étape existe, Ctrl+Y rétablit). Pendant la saisie d'une formule, **F4** continue de faire passer une référence de relative à fixe, comme auparavant. Les commandes qui ouvrent une boîte de dialogue, ainsi que la saisie dans une cellule, ne sont pas répétées.

## Pendant que JeffCrunch calcule les formules

Lorsque vous modifiez une cellule, JeffCrunch calcule en arrière-plan les formules qui en dépendent. La cellule modifiée affiche aussitôt son nouveau contenu, et les autres cellules se mettent à jour un instant plus tard ; la barre d'état affiche **Calcul en cours** jusqu'à la fin. Vous pouvez continuer à taper pendant ce temps.

## Ce qui ne peut pas être modifié

- Une cellule qui fait partie d'une **formule matricielle** ou d'un tableau de données ne peut pas être modifiée seule. Pour créer une formule matricielle, sélectionnez les cellules qu'elle doit remplir, tapez la formule, puis appuyez sur **Ctrl+Maj+Entrée**. Une seule formule remplit alors toutes les cellules, et la barre de formule l'affiche entre accolades, par exemple **{=A1:A3*2}**. Pour la modifier, sélectionnez une cellule quelconque de la matrice, appuyez sur **F2**, changez la formule, puis appuyez de nouveau sur **Ctrl+Maj+Entrée**. Pour la supprimer, sélectionnez toutes ses cellules et appuyez sur **Suppr**. Ces opérations portent sur la matrice entière, en une seule étape d'annulation. F2 sur une cellule quelconque d'une matrice ouvre la formule de la matrice, et une feuille protégée ou une plage protégée par mot de passe refuse ou demande confirmation, comme pour toute cellule. Copier une matrice, ou une partie de celle-ci, et recopier à partir d'une de ses cellules, donnent les valeurs affichées, et non une formule.
- Sur une **feuille protégée**, une cellule verrouillée ne peut pas être modifiée. Toutes les cellules sont verrouillées, sauf si le classeur indique le contraire. JeffCrunch vous le signale lorsque vous essayez. Pour modifier la cellule, choisissez Ôter la protection de la feuille dans l'onglet Révision.
- La première ligne d'un **tableau** contient les noms de ses colonnes. Tapez un nouveau nom dans une de ces cellules pour renommer la colonne : les formules qui utilisent l'ancien nom suivent, et le tableau du fichier reste valide. Un nom déjà utilisé par une autre colonne reçoit un numéro à la suite, un nom ne peut pas être vide, et **Suppr** n'efface pas cette ligne.
- Un classeur qui n'a pas pu être lu en entier peut être consulté, mais ni modifié ni enregistré.
- Une fonction que JeffCrunch ne connaît pas encore laisse une nouvelle formule sans valeur dans la cellule ; le classeur demande au programme qui l'ouvre de calculer la formule.
- La mise en forme, le tri, ainsi que la recherche et le remplacement seront disponibles dans une version ultérieure. L'insertion et la suppression, le couper-coller, le recopiage et les dimensions des lignes et des colonnes sont décrits dans [Insérer et supprimer des lignes, des colonnes et des cellules](insert-delete), [Couper, copier et coller](clipboard), [Recopier des cellules et les additionner](fill-sum) et [Modifier la largeur des colonnes et la hauteur des lignes](sizes).

## Insérer un symbole

Cliquez sur **Insertion > Symboles > Symbole** pour ouvrir la boîte Symbole. Choisissez un caractère et cliquez sur **Insérer**, ou double-cliquez dessus. Il est inséré au curseur de la saisie en cours. Si vous ne saisissez rien, la saisie de la cellule s'ouvre avec le symbole ajouté à la fin, et **Entrée** le place dans la cellule. Exemple : si une cellule contient le mot *Total*, le choix du signe euro donne la saisie *Total€*. Les symboles utilisés en dernier figurent en premier dans la liste.

**Prendre le caractère dans une police.** Choisissez une police dans la zone **Police**, puis choisissez le caractère. Le caractère est inséré dans une portion à part, écrite dans cette police, et le reste du texte de la cellule conserve la police de la cellule. Exemple : dans une cellule contenant *Total*, une coche prise dans une police de symboles donne *Total✓*, avec seule la coche dans cette police. Avec **(texte normal)** choisi, c'est la police de la cellule qui dessine le symbole, et un symbole que la police ne possède pas s'affiche comme une case. Dans une formule, seul le caractère est inséré.

**Caractères spéciaux.** L'onglet **Caractères spéciaux** liste l'espace insécable et le trait d'union insécable, le trait d'union conditionnel, le tiret demi-cadratin et le tiret cadratin, l'espace demi-cadratin et l'espace cadratin, les guillemets, ainsi que la coupure facultative de largeur nulle, chacun avec son nom. Double-cliquez sur l'un d'eux pour l'insérer.

**Attribuer une touche de raccourci à un symbole.** Choisissez un symbole ou un caractère spécial, cliquez sur **Touche de raccourci**, appuyez sur les touches voulues, par exemple **Ctrl+Alt+Maj+J**, puis cliquez sur **OK**. Ces touches insèrent alors ce symbole, de la même manière que la boîte. Laissez les touches vides et cliquez sur **OK** pour les retirer. JeffCrunch vous signale si une autre commande utilise déjà ces touches. Le symbole devient une commande, si bien qu'il figure aussi dans **Fichier > Options > Personnaliser > Clavier**, où vous pouvez modifier ses touches (voir [Raccourcis clavier](keyboard)).

## Nouveaux classeurs

Dans **Fichier > Options > Nouveaux classeurs**, choisissez la police, la taille de police et le nombre de feuilles avec lesquels chaque nouveau classeur démarre. Les classeurs existants ne sont pas modifiés.
