# Ouvrir et enregistrer des classeurs
<!-- keywords: ouvrir, enregistrer, enregistrer sous, sauvegarder, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, séparateur, importer, exporter, récent, épinglé, glisser, déposer, glisser-déposer, chargement, volumineux, gros, macros, non enregistré, modifications, fermer, copie, chaîne de calcul -->

Pour ouvrir un classeur, appuyez sur **{key:Open}**, ou cliquez sur **Fichier** puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez aussi faire glisser un classeur depuis votre gestionnaire de fichiers vers la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une deuxième fois : cette fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et les classeurs **.xlsm** (ceux qui contiennent des macros). Il n'exécute pas les macros et les conserve intactes.

## Autres types de fichiers

JeffCrunch ouvre aussi les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et le format à plat **.fods**) et les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert à partir de l'un de ces fichiers est enregistré comme nouveau fichier **.xlsx** : lorsque vous appuyez sur **{key:Save}**, JeffCrunch vous en informe et vous demande un nom, et le fichier que vous avez ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats conservés dans le fichier), ses formats de nombre, ses polices, ses remplissages, ses bordures, ses cellules fusionnées, ses largeurs de colonne et hauteurs de ligne, ses lignes et colonnes masquées, ses volets figés et ses noms. Les graphiques, les images et autres dessins, les commentaires, les tableaux croisés dynamiques, les macros, les mises en forme conditionnelles et la validation des données ne sont pas encore lus ; un message en haut indique lesquels de ces éléments le fichier contient, et une copie enregistrée depuis JeffCrunch ne les contiendra pas. Une feuille de calcul protégée par un mot de passe n'est pas ouverte : ouvrez-la dans le programme qui l'a créée et enregistrez-en une copie sans mot de passe.

Un fichier texte forme une seule feuille. JeffCrunch détecte la façon dont ses valeurs sont séparées (par des virgules, des points-virgules, des tabulations ou des barres verticales) et l'encodage de ses caractères (UTF-8 ou UTF-16, ou sinon la page de codes de votre langue), et lit chaque valeur comme vous la saisiriez dans votre langue : un nombre comme 1 234,5 en français, un pourcentage, un montant, une date avec son année (comme 15/03/2024), une heure, VRAI ou FAUX (dans votre langue, ainsi qu'en anglais, TRUE et FALSE). Une valeur qui commence par un signe égal est une formule. Les chiffres qui commencent par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, afin qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même d'un jour et d'un mois sans année, dans n'importe quel ordre et dans n'importe quelle langue (comme 3/4, 12.5, 1-2, 1er mars ou 25 déc.), car l'année devrait être inventée, ainsi que de textes comme le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en contient davantage, JeffCrunch ouvre ce qui tient et vous en informe.

Lorsque vous ouvrez un fichier texte, une fenêtre affiche d'abord ses premières lignes et vous demande comment le lire : ce qui sépare les champs (virgule, point-virgule, tabulation, espace, barre verticale ou un caractère que vous tapez, en combinant plusieurs si besoin, avec la possibilité de considérer les séparateurs consécutifs comme un seul), le délimiteur qui entoure un champ contenant un séparateur, l'encodage du texte et la ligne à partir de laquelle commencer. Au-dessus de chaque colonne, vous choisissez comment la lire : **Tel que saisi**, **Texte** (pour conserver tel quel un numéro de pièce comme 007 ou un long numéro de compte), **Date** dans l'ordre que vous indiquez, ou **Ignorer**. L'aperçu suit chaque modification et affiche au plus les 200 premières lignes et les 100 premières colonnes, mais le fichier est ouvert en entier. **Fichier > Ouvrir > Ouvrir un fichier texte** pose la question même lorsque le nom du fichier est inhabituel.

Pour ne conserver que les valeurs de la feuille affichée, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv**. JeffCrunch écrit les valeurs telles que les cellules les affichent, en UTF-8, en les séparant par un point-virgule si votre langue utilise la virgule dans les nombres (comme le français) et par une virgule sinon, et met le texte entre guillemets lorsque c'est nécessaire. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans un fichier .csv, et l'écriture de ce fichier ne modifie pas votre classeur.

## Classeurs volumineux

JeffCrunch lit un classeur en arrière-plan. Les premières lignes de la feuille qui était ouverte lors de l'enregistrement du classeur s'affichent presque immédiatement, même lorsque le classeur est très volumineux. La barre d'état, en bas à gauche, indique l'opération en cours : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules lues jusqu'ici, **Calcul en cours** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles s'activent à mesure qu'elles sont lues. Un classeur endommagé est ouvert dans la mesure où il peut être lu, et un message indique ce qui a été ignoré.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier répertorie les classeurs que vous avez ouverts récemment. Cliquez avec le bouton droit sur l'un d'eux pour l'**épingler** en haut de la liste ou pour le retirer de la liste.

## Enregistrement

Appuyez sur **{key:Save}**, ou cliquez sur **Fichier** puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur que vous avez ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier** puis sur **Enregistrer sous**, et choisissez un nom. Un classeur contenant des macros est enregistré en tant que fichier **.xlsm**, et un classeur sans macros en tant que fichier **.xlsx**.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier, comme les autres feuilles, les graphiques, les images, les macros et les paramètres que JeffCrunch n'affiche pas, est recopié tel quel, octet par octet. Dans une feuille que vous avez modifiée, seules les cellules modifiées sont réécrites, ainsi que les formules qui en dépendent, qui reçoivent leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier reste inchangé.

Si un autre programme a modifié le fichier après que vous l'avez ouvert, JeffCrunch ne l'écrase pas sans vous demander, car les modifications de l'autre programme seraient perdues. Il vous propose d'**Enregistrer une copie**, de **Remplacer** le fichier ou d'**Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien et ne le remplace que lorsqu'il est complet. En cas de problème, un message s'affiche et le fichier existant reste tel qu'il était.

Si les formules sont encore en cours de calcul lorsque vous enregistrez, JeffCrunch attend la fin du calcul afin que le fichier contienne leurs résultats. Si vous appuyez sur **Échap** pour arrêter le calcul entre-temps, l'enregistrement est annulé et la barre d'état l'indique ; enregistrez de nouveau quand vous le souhaitez. JeffCrunch omet un seul élément du fichier : la liste de l'ordre dans lequel un programme a calculé les formules la dernière fois (la chaîne de calcul). Le prochain programme qui ouvre le classeur la reconstitue. Si JeffCrunch n'a pas pu calculer certaines formules, ou si vous avez arrêté le calcul, le fichier demande à ce programme de recalculer toutes les formules à l'ouverture du classeur.

## Fermeture avec des modifications

Si vous fermez un classeur dont des modifications ne sont pas enregistrées, JeffCrunch vous demande s'il faut les **Enregistrer**, **Ne pas enregistrer** ou **Annuler** pour continuer à travailler. Un astérisque (*) dans le titre de la fenêtre indique que des modifications ne sont pas enregistrées.
