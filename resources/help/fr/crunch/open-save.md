# Ouvrir et enregistrer des classeurs
<!-- keywords: copie, ancien format, opendocument, compatibilité, nouveau, vide, vierge, classeur1, sans titre, ouvrir, enregistrer, enregistrer sous, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, séparateur, importer, exporter, récent, épinglé, glisser, déposer, glisser-déposer, chargement, volumineux, gros, macros, non enregistré, modifications, fermer, chaîne de calcul, modèle, xltx, xltm, impossible d'ouvrir, endommagé, chiffré, mot de passe -->

Lorsque JeffCrunch démarre sans fichier à ouvrir, la fenêtre contient un nouveau classeur vide comportant une feuille, nommé Classeur1. Chaque nouveau classeur prend le numéro suivant (Classeur2, Classeur3, etc.), et un numéro n'est pas réutilisé après la fermeture de sa fenêtre. Pour en créer un autre, appuyez sur **{key:New}**, ou cliquez sur **Fichier**, puis sur **Nouveau**, et choisissez **Classeur vide** ; il s'ouvre dans sa propre fenêtre. Un nouveau classeur que vous n'avez pas modifié cède la place au fichier que vous ouvrez dans sa fenêtre ; un classeur que vous avez modifié est conservé, et le fichier s'ouvre dans une autre fenêtre. La commande **Enregistrer** appliquée à un nouveau classeur demande un nom, comme **Enregistrer sous**, et l'enregistre au format .xlsx. La fermeture d'un nouveau classeur non modifié ne demande rien.

Pour ouvrir un classeur, appuyez sur **{key:Open}**, ou cliquez sur **Fichier**, puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez également faire glisser un classeur depuis votre gestionnaire de fichiers vers la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une seconde fois : sa fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et **.xlsm** (ceux qui contiennent des macros), ainsi que les modèles **.xltx** et **.xltm** (modèle contenant des macros). Il n'exécute pas les macros et les conserve intactes : le projet de macros, sa signature et les parties qui y font référence sont réécrits exactement tels qu'ils étaient. Un modèle est enregistré en tant que modèle, et un classeur contenant des macros en tant que classeur prenant en charge les macros, car le type fait partie du fichier.

## Autres types de fichiers

JeffCrunch ouvre aussi les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et **.fods**, version XML simple) et les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert à partir de l'un de ces fichiers est enregistré dans un nouveau fichier **.xlsx** (ou en tant que copie **.xls** ou **.ods** : voir ci-dessous) : lorsque vous appuyez sur **{key:Save}**, JeffCrunch vous en informe et demande un nom, et le fichier ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats enregistrés dans le fichier), ses formats de nombre, ses polices, ses remplissages, ses bordures, ses cellules fusionnées, ses largeurs de colonne et hauteurs de ligne, ses lignes et colonnes masquées, ses volets figés et ses noms. Les graphiques, les images et autres dessins, les commentaires, les tableaux croisés dynamiques, les macros, les mises en forme conditionnelles et la validation des données ne sont pas encore lus ; un message en haut de la fenêtre indique lesquels sont présents dans le fichier, et une copie enregistrée depuis JeffCrunch ne les contiendra pas. Une feuille de calcul protégée par un mot de passe le demande à l'ouverture : voir [Ouvrir des classeurs protégés par mot de passe](open-password).

Un fichier texte constitue une seule feuille. JeffCrunch détecte la façon dont ses valeurs sont séparées (virgules, points-virgules, tabulations ou barres verticales) et l'encodage de ses caractères (UTF-8 ou UTF-16, ou à défaut la page de codes de votre langue), et lit chaque valeur comme vous la saisiriez dans votre langue : un nombre tel que 1 234,5 en français, un pourcentage, un montant monétaire, une date avec son année (par exemple 15/03/2024), une heure, VRAI ou FAUX (dans votre langue et en anglais, TRUE et FALSE). Une valeur qui commence par un signe égal est une formule. Les chiffres commençant par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, afin qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même pour un jour et un mois sans année, dans n'importe quel ordre et dans n'importe quelle langue (comme 3/4, 12.5, 1-2, 1er mars ou 25 déc), car l'année devrait être inventée, ainsi que pour des textes comme le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en comporte davantage, JeffCrunch ouvre ce qui tient et vous en informe.

Lorsque vous ouvrez un fichier texte, une fenêtre affiche d'abord ses premières lignes et vous demande comment le lire : ce qui sépare les champs (virgule, point-virgule, tabulation, espace, barre verticale ou un caractère que vous tapez, avec la possibilité de considérer les séparateurs consécutifs comme un seul), le délimiteur de texte qui entoure un champ contenant un séparateur, l'encodage du texte (Unicode, pages de codes Windows, pages ISO 8859, Mac Roman et encodages d'Asie orientale) et la ligne à partir de laquelle commencer. Au-dessus de chaque colonne, vous choisissez comment la lire : **Tel que saisi**, **Texte** (pour conserver tel quel un numéro de pièce comme 007 ou un long numéro de compte), **Date** dans l'ordre indiqué, ou **Ignorer**. L'aperçu suit chaque modification et affiche au maximum les 200 premières lignes et les 100 premières colonnes, mais le fichier est ouvert en entier. **Fichier > Ouvrir > Ouvrir un fichier texte** pose ces questions même lorsque le nom du fichier est inhabituel.

Pour ne conserver que les valeurs de la feuille affichée, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv** (ou **.tsv** et **.txt**, séparés par des tabulations). Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas conserver (les autres feuilles, les formules, les images, les graphiques, les notes et les macros, par exemple), vous demande comment l'écrire, et s'il faut **Enregistrer les valeurs uniquement**, **Enregistrer au format .xlsx** à la place pour tout conserver, ou **Annuler** :

- **Séparateur.** Virgule, point-virgule, tabulation, espace, barre verticale ou caractère que vous tapez. Un fichier **.csv** utilise par défaut un point-virgule lorsque votre langue écrit une virgule dans les nombres, et une virgule sinon ; un fichier **.tsv** ou **.txt**, une tabulation.
- **Encodage du texte.** UTF-8 avec marque d'ordre (pour que les autres programmes lisent correctement les lettres accentuées), UTF-8 sans marque, UTF-16 ou une page de codes comme Europe occidentale (1252), les pages ISO 8859 ou Mac Roman. Un caractère pour lequel la page de codes n'a pas d'octet est écrit sous forme de point d'interrogation, et la barre d'état en indique le nombre.
- **Fins de ligne.** Windows (retour chariot et saut de ligne), Linux et Mac (saut de ligne) ou Ancien Mac (retour chariot).
- **Cellules.** **Tel qu'affiché dans les cellules** écrit ce que vous voyez. **Tel qu'enregistré** écrit tous les chiffres d'un nombre (0.1 et non 10 %) et une date sous forme de texte ISO, par exemple 2026-03-04.

Le texte est placé entre guillemets lorsqu'il contient le séparateur, un guillemet, un saut de ligne ou un espace au début ou à la fin. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans le fichier, et l'écriture de ce fichier ne modifie pas votre classeur.

Un classeur provenant d'un fichier texte est réécrit sous la forme du fichier d'origine : même séparateur, mêmes fins de ligne et même encodage. Si vous l'enregistrez en tant que copie sans avoir modifié de cellule, la copie est identique au fichier, octet par octet, et aucune question n'est posée.

## Enregistrer une copie au format .xls ou .ods

Pour créer une copie que d'anciens programmes, ou des programmes lisant le format OpenDocument, peuvent ouvrir, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.xls** ou **.ods**. La copie est créée à partir de votre classeur dans son état actuel, y compris les modifications non encore enregistrées, et contient :

- les valeurs, avec leurs textes, nombres, valeurs logiques et erreurs ;
- les formules, avec leurs résultats actuels (une formule matricielle reste une formule matricielle) ;
- les formats de nombre, les polices, les remplissages, les bordures, l'alignement et le verrouillage des cellules ;
- les cellules fusionnées, les largeurs de colonne et hauteurs de ligne, les lignes et colonnes masquées, les plans des lignes et des colonnes, et les volets figés ;
- les feuilles, dans leur ordre, avec leurs noms et leur état masqué, ainsi que les noms que vous avez définis (et les zones d'impression) ;
- les liens et la mise en page de chaque feuille (papier, marges, orientation, échelle, en-têtes et pieds de page, et sauts de page).

Un fichier **.xls** contient aussi les tables de données et conserve les paramètres de calcul du classeur. Une copie ne peut pas tout contenir. Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas conserver : graphiques, images et autres dessins, notes, tableaux croisés dynamiques, tableaux, mises en forme conditionnelles, validation des données, filtres, protection de la feuille, contrôles de formulaire, macros et liens vers d'autres classeurs (les formules qui les utilisent conservent leurs valeurs). Une formule que l'ancien format ne peut pas exprimer (une fonction qu'il ne connaît pas, ou une cellule au-delà de la colonne IV ou de la ligne 65 536) est écrite sous forme de sa valeur, et le message en indique le nombre. L'ancien format est également limité à 65 536 lignes, 256 colonnes, 255 feuilles et 32 767 caractères par cellule, ainsi qu'à environ 4 000 formats de cellule et 56 couleurs : ce qui dépasse est tronqué, et le message en indique la quantité. Vous pouvez alors choisir **Enregistrer une copie**, **Enregistrer au format .xlsx** pour tout conserver, ou **Annuler**.

La copie ne modifie pas votre classeur, qui n'est pas marqué comme enregistré. Le fichier ouvert n'est jamais écrit : donnez un autre nom à la copie. Une copie d'un fichier **.xls** ou **.ods** que vous avez ouvert peut être enregistrée dans l'autre format.

## Classeurs volumineux

JeffCrunch lit les classeurs en arrière-plan. Les premières lignes de la feuille active lors du dernier enregistrement s'affichent presque immédiatement, même pour un très gros classeur. La barre d'état, en bas à gauche, indique l'opération en cours : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules déjà lues, **Calcul** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles deviennent actifs à mesure qu'elles sont lues. Un classeur endommagé est ouvert dans la mesure du possible, et un message indique ce qui a été omis.

## Si un fichier ne s'ouvre pas

Lorsque JeffCrunch ne peut pas ouvrir un fichier, il en explique la raison en termes simples et laisse vos classeurs ouverts en l'état. Il distingue un fichier vide d'un fichier qui n'est pas du tout un classeur (un document PDF, une image, un document texte ou une présentation, par exemple) ; un paquet zip tronqué ou endommagé, ce qui signifie généralement qu'un téléchargement ou une copie a été interrompu (copiez-le à nouveau depuis l'original) ; un classeur protégé par un mot de passe, qu'il vous demande (voir [Ouvrir des classeurs protégés par mot de passe](open-password)) ; et un fichier composé qui ne contient aucun classeur.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier répertorie les classeurs que vous avez ouverts récemment. Cliquez avec le bouton droit sur l'un d'eux pour l'**épingler** en haut de la liste ou pour le supprimer de la liste.

## Enregistrer

Appuyez sur **{key:Save}**, ou cliquez sur **Fichier**, puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez un nom. Un classeur contenant des macros est enregistré au format **.xlsm**, un modèle au format **.xltx** ou **.xltm**, et tout autre classeur au format **.xlsx**. Un nom se terminant par l'extension d'un autre type de classeur n'est pas accepté, et le message indique l'extension à utiliser ; les noms se terminant par **.xls**, **.ods**, **.csv**, **.tsv** et **.txt** créent des copies, comme décrit ci-dessus.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier, comme les autres feuilles, les graphiques, les images, les macros et les paramètres que JeffCrunch n'affiche pas, est copié tel quel, octet par octet. Dans une feuille modifiée, seules les cellules que vous avez modifiées sont réécrites, ainsi que les formules qui en dépendent et qui reçoivent leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier n'est pas touché.

Si un autre programme a modifié le fichier après son ouverture, JeffCrunch ne l'écrase pas sans demander, car les modifications de l'autre programme seraient perdues. Il vous demande s'il faut **Enregistrer une copie**, **Remplacer** le fichier ou **Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien, et ne le remplace qu'une fois terminé. En cas de problème, un message s'affiche et le fichier existant reste intact.

Si les formules sont encore en cours de calcul au moment de l'enregistrement, JeffCrunch attend la fin du calcul, afin que le fichier contienne leurs résultats. Si vous appuyez sur **Échap** pour interrompre le calcul entre-temps, l'enregistrement est annulé et la barre d'état l'indique ; enregistrez de nouveau quand vous le souhaitez. JeffCrunch omet un seul élément du fichier : la liste de l'ordre dans lequel un programme a calculé les formules pour la dernière fois (la chaîne de calcul). Le prochain programme qui ouvre le classeur la recrée. Si JeffCrunch n'a pas pu calculer une formule, ou si vous avez interrompu le calcul, le fichier demande à ce programme de recalculer toutes les formules à l'ouverture du classeur.

## Fermer avec des modifications

Si vous fermez un classeur comportant des modifications non enregistrées, JeffCrunch vous demande s'il faut les **Enregistrer**, **Ne pas enregistrer**, ou **Annuler** pour continuer à travailler. Un astérisque (*) dans le titre de la fenêtre signale des modifications non enregistrées.
