# Ouvrir et enregistrer des classeurs
<!-- keywords: nouveau, vierge, vide, classeur1, book1, sans titre, ouvrir, enregistrer, enregistrer sous, sauvegarder, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, séparateur, importer, exporter, récent, épinglé, glisser, déposer, chargement, volumineux, gros, macros, non enregistré, modifications, fermer, copie, chaîne de calcul -->

Lorsque JeffCrunch démarre sans fichier à ouvrir, la fenêtre contient un nouveau classeur vide d'une feuille, nommé Book1. Chaque nouveau classeur prend le numéro suivant (Book2, Book3, etc.), et un numéro n'est pas réutilisé après la fermeture de sa fenêtre. Pour en créer un autre, appuyez sur **{key:New}**, ou cliquez sur **Fichier**, puis sur **Nouveau**, et choisissez **Classeur vide** ; il s'ouvre dans sa propre fenêtre. Un nouveau classeur que vous n'avez pas modifié laisse la place au fichier que vous ouvrez dans sa fenêtre ; s'il a été modifié, il reste, et le fichier s'ouvre dans une autre fenêtre. **Enregistrer** sur un nouveau classeur demande un nom, comme **Enregistrer sous**, et l'enregistre en tant que fichier .xlsx. La fermeture d'un nouveau classeur non modifié ne pose aucune question.

Pour ouvrir un classeur, appuyez sur **{key:Open}**, ou cliquez sur **Fichier**, puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez aussi faire glisser un classeur depuis votre gestionnaire de fichiers sur la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une seconde fois : sa fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et les classeurs **.xlsm** (ceux qui contiennent des macros). Il n'exécute pas les macros et les conserve intactes.

## Autres types de fichiers

JeffCrunch ouvre aussi les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et le format plat **.fods**) et les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert depuis l'un de ces fichiers est enregistré dans un nouveau fichier **.xlsx** : lorsque vous appuyez sur **{key:Save}**, JeffCrunch vous l'indique et demande un nom, et le fichier ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats conservés dans le fichier), ses formats de nombre, ses polices, ses remplissages, ses bordures, ses cellules fusionnées, ses largeurs de colonne et hauteurs de ligne, ses lignes et colonnes masquées, ses volets figés et ses noms. Les graphiques, les images et autres dessins, les commentaires, les tableaux croisés dynamiques, les macros, les mises en forme conditionnelles et la validation des données ne sont pas encore lus ; un message en haut de la fenêtre indique lesquels le fichier contient, et une copie enregistrée depuis JeffCrunch ne les contiendra pas. Une feuille de calcul protégée par un mot de passe n'est pas ouverte : ouvrez-la dans le programme qui l'a créée et enregistrez-en une copie sans mot de passe.

Un fichier texte forme une seule feuille. JeffCrunch détermine comment ses valeurs sont séparées (par des virgules, des points-virgules, des tabulations ou des barres verticales) et comment ses caractères sont codés (UTF-8 ou UTF-16, sinon la page de codes de votre langue), et lit chaque valeur comme vous la taperiez dans votre langue : un nombre comme 1 234,5, un pourcentage, un montant, une date avec son année (comme 15/03/2024, ou 3/15/2024 en anglais), une heure, VRAI ou FAUX (dans votre langue, et en anglais TRUE et FALSE). Une valeur qui commence par un signe égal est une formule. Les chiffres qui commencent par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, pour qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même d'un jour et d'un mois sans année, dans n'importe quel ordre et dans n'importe quelle langue (comme 3/4, 12.5, 1-2, 1er mars ou Dec 25), car l'année devrait être inventée, et de textes comme le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en a davantage, JeffCrunch ouvre ce qui tient et vous en informe.

Lorsque vous ouvrez un fichier texte, une fenêtre affiche d'abord ses premières lignes et vous demande comment le lire : ce qui sépare les champs (virgule, point-virgule, tabulation, espace, barre verticale ou un caractère que vous tapez, en considérant si vous le souhaitez les séparateurs consécutifs comme un seul), ce qui délimite un champ contenant un séparateur, l'encodage du texte et la ligne de départ. Au-dessus de chaque colonne, vous choisissez comment la lire : **Tel que saisi**, **Texte** (pour conserver tel quel un numéro de pièce comme 007 ou un long numéro de compte), **Date** dans l'ordre indiqué, ou **Ignorer**. L'aperçu suit chaque modification et affiche au plus les 200 premières lignes et 100 premières colonnes, mais le fichier entier est ouvert. **Fichier > Ouvrir > Ouvrir un fichier texte** pose ces questions même lorsque le nom du fichier est inhabituel.

Pour ne conserver que les valeurs de la feuille affichée, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv**. JeffCrunch écrit les valeurs telles que les cellules les affichent, en UTF-8, séparées par un point-virgule si votre langue utilise la virgule dans les nombres et par une virgule sinon, avec du texte entre guillemets lorsque c'est nécessaire. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans un fichier .csv, et votre classeur n'est pas modifié par son écriture.

## Classeurs volumineux

JeffCrunch lit un classeur en arrière-plan. Les premières lignes de la feuille qui était ouverte lors de l'enregistrement du classeur apparaissent presque immédiatement, même pour un classeur très volumineux. La barre d'état, en bas à gauche, indique ce qu'il fait : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules lues jusque-là, **Calcul** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles s'activent une fois celles-ci lues. Un classeur endommagé est ouvert dans la mesure où il peut être lu, et un message indique ce qui a été omis.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier répertorie les classeurs ouverts récemment. Cliquez avec le bouton droit sur l'un d'eux pour l'**épingler** en haut de la liste ou pour le retirer de la liste.

## Enregistrement

Appuyez sur **{key:Save}**, ou cliquez sur **Fichier**, puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez un nom. Un classeur contenant des macros est enregistré au format **.xlsm**, et un classeur sans macros au format **.xlsx**.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier, comme les autres feuilles, les graphiques, les images, les macros et les paramètres que JeffCrunch n'affiche pas, est recopié tel quel, octet par octet. Dans une feuille modifiée, seules les cellules que vous avez modifiées sont réécrites, ainsi que les formules qui en dépendent, avec leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier n'est pas touché.

Si un autre programme a modifié le fichier après son ouverture, JeffCrunch ne l'écrase pas sans vous le demander, car les modifications de l'autre programme seraient perdues. Il vous propose d'**Enregistrer une copie**, de **Remplacer** le fichier ou d'**Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien et ne le remplace qu'une fois complet. En cas de problème, un message s'affiche et le fichier existant reste intact.

Si les formules sont encore en cours de calcul lors de l'enregistrement, JeffCrunch attend la fin du calcul, afin que le fichier contienne leurs résultats. Si vous appuyez sur **Échap** pour interrompre le calcul entre-temps, l'enregistrement est annulé et la barre d'état l'indique ; enregistrez de nouveau quand vous le souhaitez. JeffCrunch omet un seul élément dans le fichier : la liste de l'ordre dans lequel un programme a calculé les formules la dernière fois (la chaîne de calcul). Le prochain programme qui ouvre le classeur la recrée. Si JeffCrunch n'a pas pu calculer une formule, ou si vous avez interrompu le calcul, le fichier demande à ce programme de recalculer toutes les formules à l'ouverture du classeur.

## Fermeture avec des modifications

Si vous fermez un classeur contenant des modifications non enregistrées, JeffCrunch vous demande s'il faut les enregistrer (**Enregistrer**), ne pas les enregistrer (**Ne pas enregistrer**) ou annuler la fermeture pour continuer à travailler (**Annuler**). Un astérisque (*) dans le titre de la fenêtre signale des modifications non enregistrées.
