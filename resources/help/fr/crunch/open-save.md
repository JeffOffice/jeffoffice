# Ouvrir et enregistrer des classeurs
<!-- keywords: nouveau, vide, vierge, classeur1, sans titre, ouvrir, enregistrer, enregistrer sous, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, séparateur, importer, exporter, récent, épinglé, glisser, déposer, chargement, volumineux, gros, macros, non enregistré, modifications, fermer, copie, chaîne de calcul, modèle, xltx, xltm, impossible d'ouvrir, endommagé, chiffré, mot de passe -->

Lorsque JeffCrunch démarre sans fichier à ouvrir, la fenêtre contient un nouveau classeur vide d'une feuille, nommé Classeur1. Chaque nouveau classeur prend le numéro suivant (Classeur2, Classeur3, etc.), et un numéro n'est pas réutilisé après la fermeture de sa fenêtre. Pour en créer un autre, appuyez sur **{key:New}**, ou cliquez sur **Fichier**, puis sur **Nouveau**, et choisissez **Classeur vide** ; il s'ouvre dans sa propre fenêtre. Un nouveau classeur que vous n'avez pas modifié cède la place lorsque vous ouvrez un fichier dans sa fenêtre ; un classeur que vous avez modifié reste, et le fichier s'ouvre dans une autre fenêtre. **Enregistrer** sur un nouveau classeur demande un nom, comme **Enregistrer sous**, et l'enregistre en tant que fichier .xlsx. La fermeture d'un nouveau classeur non modifié ne demande rien.

Pour ouvrir un classeur, appuyez sur **{key:Open}**, ou cliquez sur **Fichier**, puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez aussi faire glisser un classeur depuis votre gestionnaire de fichiers sur la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une seconde fois : sa fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et **.xlsm** (ceux qui contiennent des macros), ainsi que les modèles **.xltx** et **.xltm** (modèle contenant des macros). Il n'exécute pas les macros et les laisse intactes : le projet de macros, sa signature et les parties qui y font référence sont réécrits exactement tels quels. Un modèle est enregistré en tant que modèle, et un classeur avec macros en tant que classeur avec macros, car le type fait partie du fichier.

## Autres types de fichiers

JeffCrunch ouvre aussi les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et le format plat **.fods**) et les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert à partir de l'un d'eux est enregistré dans un nouveau fichier **.xlsx** : lorsque vous appuyez sur **{key:Save}**, JeffCrunch vous l'indique et demande un nom, et le fichier ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats enregistrés dans le fichier), ses formats de nombre, ses polices, ses remplissages, ses bordures, ses cellules fusionnées, ses largeurs de colonne et hauteurs de ligne, ses lignes et colonnes masquées, ses volets figés et ses noms. Les graphiques, les images et autres dessins, les commentaires, les tableaux croisés dynamiques, les macros, les mises en forme conditionnelles et la validation des données ne sont pas encore lus ; un message en haut indique lesquels de ces éléments le fichier contient, et une copie enregistrée ici ne les contiendra pas. Une feuille de calcul protégée par mot de passe n'est pas ouverte : ouvrez-la dans le programme qui l'a créée et enregistrez-en une copie sans mot de passe.

Un fichier texte constitue une seule feuille. JeffCrunch détermine comment ses valeurs sont séparées (par des virgules, des points-virgules, des tabulations ou des barres verticales) et comment ses caractères sont encodés (UTF-8 ou UTF-16, sinon la page de codes de votre langue), et lit chaque valeur comme vous la taperiez dans votre langue : un nombre tel que 1.234,5 en allemand, un pourcentage, un montant, une date avec son année (comme 3/15/2024, ou 15.03.2024 en allemand), une heure, TRUE ou FALSE (dans votre langue, comme WAHR et FALSCH en allemand, et en anglais). Une valeur commençant par un signe égal est une formule. Les suites de chiffres commençant par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, pour qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même pour un jour et un mois sans année, dans n'importe quel ordre et n'importe quelle langue (comme 3/4, 12.5, 1-2, March 1 ou Dec 25), car l'année devrait être inventée, et pour des textes comme le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en a davantage, JeffCrunch ouvre ce qui tient et vous en informe.

Lorsque vous ouvrez un fichier texte, une fenêtre affiche d'abord ses premières lignes et demande comment le lire : ce qui sépare les champs (virgule, point-virgule, tabulation, espace, barre verticale ou caractère que vous tapez, au choix, en considérant éventuellement les séparateurs consécutifs comme un seul), ce qui encadre un champ contenant un séparateur, l'encodage du texte (Unicode, pages de codes Windows, pages ISO 8859, Mac Roman et encodages d'Asie de l'Est) et la ligne de départ. Au-dessus de chaque colonne, vous choisissez comment elle est lue : **Tel que saisi**, **Texte** (pour conserver tel quel un numéro de pièce comme 007 ou un long numéro de compte), une **Date** dans l'ordre indiqué, ou **Ignorer**. L'aperçu suit chaque modification et affiche au plus les 200 premières lignes et 100 colonnes, mais le fichier entier est ouvert. **Fichier > Ouvrir > Ouvrir un fichier texte** pose ces questions même lorsque le nom du fichier est inhabituel.

Pour ne conserver que les valeurs de la feuille affichée, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv** (ou **.tsv** et **.txt**, séparés par des tabulations). Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas contenir (les autres feuilles, les formules, les images, les graphiques, les notes et les macros, par exemple), demande comment l'écrire, et s'il faut **Enregistrer les valeurs uniquement**, plutôt **Enregistrer au format .xlsx** pour tout conserver, ou **Annuler** :

- **Séparateur.** Virgule, point-virgule, tabulation, espace, barre verticale ou caractère que vous tapez. Un fichier **.csv** commence avec le point-virgule si votre langue utilise la virgule dans les nombres, et avec la virgule sinon ; un fichier **.tsv** ou **.txt**, avec la tabulation.
- **Encodage du texte.** UTF-8 avec marque d'ordre (pour que les autres programmes lisent correctement les lettres accentuées), UTF-8 sans marque, UTF-16 ou une page de codes comme Europe occidentale (1252), les pages ISO 8859 ou Mac Roman. Un caractère sans équivalent dans la page de codes est écrit sous forme de point d'interrogation, et la barre d'état en indique le nombre.
- **Fins de ligne.** Windows (retour chariot et saut de ligne), Linux et Mac (saut de ligne) ou ancien Mac (retour chariot).
- **Cellules.** **Tel qu'affiché dans les cellules** écrit ce que vous voyez. **Tel qu'enregistré** écrit tous les chiffres d'un nombre (0.1 et non 10 %), et une date en texte ISO, comme 2026-03-04.

Le texte est placé entre guillemets lorsqu'il contient le séparateur, un guillemet, un saut de ligne ou un espace au début ou à la fin. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans le fichier, et l'écriture de ce fichier ne modifie pas votre classeur.

Un classeur issu d'un fichier texte est réécrit sous la forme du fichier d'origine : même séparateur, mêmes fins de ligne et même encodage. Si vous l'enregistrez en tant que copie sans modifier aucune cellule, la copie est identique au fichier, octet par octet, et aucune question n'est posée.

## Classeurs volumineux

JeffCrunch lit les classeurs en arrière-plan. Les premières lignes de la feuille active au moment de l'enregistrement apparaissent presque immédiatement, même pour un très gros classeur. La barre d'état, en bas à gauche, indique l'opération en cours : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules déjà lues, **Calcul en cours** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles deviennent actifs une fois celles-ci lues. Un classeur endommagé est ouvert dans la mesure où il peut être lu, et un message indique ce qui a été ignoré.

## Si un fichier ne s'ouvre pas

Lorsque JeffCrunch ne peut pas ouvrir un fichier, il en indique la raison en termes simples et laisse les classeurs ouverts tels quels. Il distingue un fichier vide d'un fichier qui n'est pas du tout un classeur (un document PDF, une image, un document texte ou une présentation, par exemple) ; un paquet zip tronqué ou endommagé, ce qui signifie généralement qu'un téléchargement ou une copie a été interrompu : copiez-le de nouveau depuis l'original ; un classeur chiffré par mot de passe, que JeffCrunch ne peut pas encore ouvrir ; et un fichier composé qui ne contient aucun classeur.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier répertorie les classeurs ouverts récemment. Cliquez avec le bouton droit sur l'un d'eux pour l'**épingler** en haut de la liste ou pour le retirer de la liste.

## Enregistrement

Appuyez sur **{key:Save}**, ou cliquez sur **Fichier**, puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez un nom. Un classeur contenant des macros est enregistré au format **.xlsm**, un modèle au format **.xltx** ou **.xltm**, et tout autre classeur au format **.xlsx**. Un nom se terminant par une autre extension n'est pas accepté, et le message indique l'extension à utiliser.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier, comme les autres feuilles, les graphiques, les images, les macros et les paramètres que JeffCrunch n'affiche pas, est copié tel quel, octet par octet. Dans une feuille modifiée, seules les cellules que vous avez modifiées sont réécrites, ainsi que les formules qui en dépendent, avec leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier n'est pas touché.

Si un autre programme a modifié le fichier après son ouverture, JeffCrunch ne l'écrase pas sans demander, car les modifications de l'autre programme seraient perdues. Il vous demande s'il faut **Enregistrer une copie**, **Remplacer** le fichier ou **Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien, et ne prend sa place qu'une fois complet. En cas de problème, un message s'affiche et le fichier existant reste tel qu'il était.

Si les formules sont encore en cours de calcul au moment de l'enregistrement, JeffCrunch attend la fin du calcul, pour que le fichier contienne leurs résultats. Si vous appuyez sur **Échap** pour arrêter le calcul entre-temps, l'enregistrement est annulé et la barre d'état l'indique ; enregistrez de nouveau quand vous le souhaitez. JeffCrunch omet un seul élément du fichier : la liste de l'ordre dans lequel un programme a calculé les formules pour la dernière fois (la chaîne de calcul). Le prochain programme qui ouvre le classeur la recrée. Si JeffCrunch n'a pas pu calculer une formule, ou si vous avez arrêté le calcul, le fichier demande à ce programme de recalculer toutes les formules à l'ouverture du classeur.

## Fermeture avec des modifications

Si vous fermez un classeur dont des modifications ne sont pas enregistrées, JeffCrunch vous demande s'il faut **Enregistrer**, **Ne pas enregistrer**, ou **Annuler** pour continuer à travailler. Un astérisque (*) dans le titre de la fenêtre signale des modifications non enregistrées.
