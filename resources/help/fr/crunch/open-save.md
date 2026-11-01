# Ouvrir et enregistrer des classeurs
<!-- keywords: copie, ancien format, opendocument, compatibilité, nouveau, vierge, vide, classeur1, sans titre, ouvrir, enregistrer, enregistrer sous, sauvegarder, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, séparateur, importer, exporter, récent, épinglé, glisser, déposer, chargement, volumineux, gros, macros, non enregistré, modifications, fermer, chaîne de calcul, modèle, xltx, xltm, impossible d'ouvrir, endommagé, chiffré, mot de passe -->

Lorsque JeffCrunch démarre sans fichier à ouvrir, la fenêtre contient un nouveau classeur vide comportant une seule feuille, nommé Classeur1. Chaque nouveau classeur prend le numéro suivant (Classeur2, Classeur3, etc.), et un numéro n'est pas réutilisé après la fermeture de sa fenêtre. Pour en créer un autre, appuyez sur **{key:New}**, ou cliquez sur **Fichier**, puis sur **Nouveau**, et choisissez **Classeur vide** ; il s'ouvre dans sa propre fenêtre. Un nouveau classeur que vous n'avez pas modifié cède la place au fichier que vous ouvrez dans sa fenêtre ; s'il a été modifié, il reste, et le fichier s'ouvre dans une autre fenêtre. **Enregistrer** sur un nouveau classeur demande un nom, comme **Enregistrer sous**, et l'enregistre au format .xlsx. La fermeture d'un nouveau classeur non modifié ne demande rien.

Pour ouvrir un classeur, appuyez sur **{key:Open}**, ou cliquez sur **Fichier**, puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez aussi faire glisser un classeur depuis votre gestionnaire de fichiers vers la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une seconde fois : sa fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et **.xlsm** (ceux qui contiennent des macros), ainsi que les modèles **.xltx** et **.xltm** (modèle contenant des macros). Il n'exécute pas les macros et les conserve intactes : le projet de macros, sa signature et les parties qui y font référence sont réécrits exactement tels qu'ils étaient. Un modèle est enregistré en tant que modèle, et un classeur avec macros en tant que classeur avec macros, car le type fait partie du fichier.

## Autres types de fichiers

JeffCrunch ouvre aussi les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et le format à plat **.fods**) et les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert à partir de l'un d'eux est enregistré dans un nouveau fichier **.xlsx** (ou dans une copie **.xls** ou **.ods** : voir plus loin) : lorsque vous appuyez sur **{key:Save}**, JeffCrunch vous le signale et demande un nom ; le fichier ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats conservés dans le fichier), ses formats de nombre, polices, remplissages, bordures, cellules fusionnées, largeurs de colonne et hauteurs de ligne, lignes et colonnes masquées, volets figés et noms. Les graphiques, images et autres dessins, commentaires, tableaux croisés dynamiques, macros, mises en forme conditionnelles et la validation des données ne sont pas encore lus ; un message en haut indique ceux que contient le fichier, et une copie enregistrée depuis JeffCrunch ne les contiendra pas. Une feuille de calcul protégée par mot de passe n'est pas ouverte : ouvrez-la dans le programme qui l'a créée et enregistrez-en une copie sans mot de passe.

Un fichier texte forme une seule feuille. JeffCrunch détecte la façon dont ses valeurs sont séparées (virgules, points-virgules, tabulations ou barres verticales) et le codage de ses caractères (UTF-8 ou UTF-16, sinon la page de codes de votre langue), et lit chaque valeur comme vous la taperiez dans votre langue : un nombre tel que 1 234,5, un pourcentage, un montant, une date avec son année (comme 15/03/2024, ou 3/15/2024 en anglais), une heure, VRAI ou FAUX (dans votre langue, par exemple WAHR et FALSCH en allemand, ainsi qu'en anglais). Une valeur commençant par un signe égal est une formule. Les chiffres commençant par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, pour qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même pour un jour et un mois sans année, dans n'importe quel ordre et n'importe quelle langue (comme 3/4, 12.5, 1-2, 1er mars ou Dec 25), car l'année devrait être inventée, et pour des textes tels que le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en comporte davantage, JeffCrunch ouvre ce qui tient et vous en informe.

Lorsque vous ouvrez un fichier texte, une fenêtre affiche d'abord ses premières lignes et vous demande comment le lire : le séparateur des champs (virgule, point-virgule, tabulation, espace, barre verticale ou un caractère que vous tapez, avec éventuellement plusieurs séparateurs consécutifs considérés comme un seul), le délimiteur de texte qui entoure un champ contenant un séparateur, l'encodage du texte (Unicode, pages de codes Windows, pages ISO 8859, Mac Roman et encodages d'Asie orientale) et la ligne à partir de laquelle commencer. Au-dessus de chaque colonne, vous choisissez sa lecture : **Tel que saisi**, **Texte** (pour conserver une référence comme 007 ou un long numéro de compte tel quel), **Date** dans l'ordre indiqué, ou **Ignorer**. L'aperçu suit chaque modification et affiche au plus les 200 premières lignes et 100 premières colonnes, mais le fichier est ouvert en entier. **Fichier > Ouvrir > Ouvrir un fichier texte** pose la question même si le nom du fichier est inhabituel.

Pour ne conserver que les valeurs de la feuille affichée, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv** (ou **.tsv** et **.txt**, séparés par des tabulations). Avant d'écrire le fichier, JeffCrunch indique ce que contient votre classeur et que le fichier ne pourra pas contenir (les autres feuilles, les formules, les images, les graphiques, les notes et les macros, par exemple), vous demande comment l'écrire et s'il faut **Enregistrer les valeurs uniquement**, **Enregistrer au format .xlsx** à la place pour tout conserver, ou **Annuler** :

- **Séparateur.** Virgule, point-virgule, tabulation, espace, barre verticale ou caractère que vous tapez. Un fichier **.csv** utilise d'abord le point-virgule lorsque votre langue écrit les nombres avec une virgule décimale, et la virgule sinon ; un fichier **.tsv** ou **.txt**, la tabulation.
- **Encodage du texte.** UTF-8 avec marque d'ordre (pour que les autres programmes lisent correctement les lettres accentuées), UTF-8 sans marque, UTF-16, ou une page de codes comme Europe occidentale (1252), les pages ISO 8859 ou Mac Roman. Un caractère absent de la page de codes est écrit sous la forme d'un point d'interrogation, et la barre d'état indique leur nombre.
- **Fins de ligne.** Windows (retour chariot et saut de ligne), Linux et Mac (saut de ligne) ou Ancien Mac (retour chariot).
- **Cellules.** **Tel qu'affiché dans les cellules** écrit ce que vous voyez. **Tel qu'enregistré** écrit tous les chiffres d'un nombre (0,1 et non 10 %) et une date en texte ISO, par exemple 2026-03-04.

Un texte est placé entre guillemets lorsqu'il contient le séparateur, un guillemet, un saut de ligne ou un espace au début ou à la fin. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans le fichier, et l'écriture du fichier ne modifie pas votre classeur.

Un classeur issu d'un fichier texte est réécrit sous la forme d'origine du fichier : même séparateur, mêmes fins de ligne et même encodage. Si vous l'enregistrez en tant que copie sans avoir modifié de cellule, la copie est identique au fichier, octet pour octet, et aucune question n'est posée.

## Enregistrer une copie au format .xls ou .ods

Pour créer une copie lisible par d'anciens programmes ou par des programmes qui lisent l'OpenDocument, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.xls** ou **.ods**. La copie est faite à partir de votre classeur dans son état actuel, y compris les modifications non encore enregistrées, et contient :

- les valeurs, avec leurs textes, nombres, valeurs logiques et erreurs ;
- les formules, avec leurs résultats actuels (une formule matricielle reste une formule matricielle) ;
- les formats de nombre, polices, remplissages, bordures, alignements et l'état verrouillé des cellules ;
- les cellules fusionnées, largeurs de colonne et hauteurs de ligne, lignes et colonnes masquées, plans des lignes et des colonnes, et volets figés ;
- les feuilles, dans leur ordre, avec leurs noms et leur état masqué, ainsi que les noms que vous avez définis (et les zones d'impression) ;
- les liens et la mise en page de chaque feuille (papier, marges, orientation, échelle, en-têtes et pieds de page, sauts de page).

Un fichier **.xls** contient aussi les tables de données et conserve les paramètres de calcul du classeur. Une copie ne peut pas tout contenir. Avant d'écrire le fichier, JeffCrunch indique ce que contient votre classeur et que le fichier ne pourra pas contenir : graphiques, images et autres dessins, notes, tableaux croisés dynamiques, tableaux, mises en forme conditionnelles, validation des données, filtres, protection de la feuille, contrôles de formulaire, macros et liens vers d'autres classeurs (les formules qui les utilisent conservent leurs valeurs). Une formule que l'ancien format ne peut pas exprimer (fonction inexistante, ou cellule au-delà de la colonne IV ou de la ligne 65 536) est écrite sous forme de sa valeur, et le message en indique le nombre. L'ancien format est aussi limité à 65 536 lignes, 256 colonnes, 255 feuilles et 32 767 caractères par cellule, ainsi qu'à environ 4 000 formats de cellule et 56 couleurs : ce qui dépasse est tronqué, et le message indique dans quelle mesure. Vous pouvez alors choisir **Enregistrer une copie**, **Enregistrer au format .xlsx** pour tout conserver, ou **Annuler**.

La copie ne modifie pas votre classeur, qui n'est pas marqué comme enregistré. Le fichier ouvert n'est jamais écrit : donnez un autre nom à la copie. Une copie d'un fichier **.xls** ou **.ods** ouvert peut être enregistrée dans l'autre format.

## Classeurs volumineux

JeffCrunch lit les classeurs en arrière-plan. Les premières lignes de la feuille qui était affichée lors de l'enregistrement apparaissent presque immédiatement, même pour un très gros classeur. La barre d'état, en bas à gauche, indique l'opération en cours : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules déjà lues, **Calcul en cours…** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles s'activent une fois celles-ci lues. Un classeur endommagé est ouvert dans la mesure du possible, et un message indique ce qui a été omis.

## Si un fichier ne s'ouvre pas

Lorsque JeffCrunch ne peut pas ouvrir un fichier, il en explique la raison en termes simples et laisse les classeurs ouverts tels quels. Il distingue un fichier vide d'un fichier qui n'est pas du tout un classeur (document PDF, image, document texte ou présentation, par exemple) ; une archive zip tronquée ou endommagée, ce qui signifie généralement qu'un téléchargement ou une copie a été interrompu : copiez-la de nouveau depuis l'original ; un classeur chiffré par mot de passe, que JeffCrunch ne peut pas encore ouvrir ; et un fichier composé ne contenant aucun classeur.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier répertorie les classeurs ouverts récemment. Cliquez avec le bouton droit sur l'un d'eux pour l'**épingler** en haut de la liste ou le retirer de la liste.

## Enregistrer

Appuyez sur **{key:Save}**, ou cliquez sur **Fichier**, puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez un nom. Un classeur avec macros est enregistré au format **.xlsm**, un modèle au format **.xltx** ou **.xltm**, et tout autre classeur au format **.xlsx**. Un nom se terminant par l'extension d'un autre type de classeur n'est pas accepté, et le message indique l'extension à utiliser ; les noms se terminant par **.xls**, **.ods**, **.csv**, **.tsv** et **.txt** créent des copies, comme décrit plus haut.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier (autres feuilles, graphiques, images, macros, paramètres que JeffCrunch n'affiche pas) est recopié tel quel, octet pour octet. Dans une feuille modifiée, seules les cellules modifiées sont réécrites, ainsi que les formules qui en dépendent, avec leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier reste inchangé.

Si un autre programme a modifié le fichier depuis que vous l'avez ouvert, JeffCrunch ne l'écrase pas sans vous le demander, car les modifications de l'autre programme seraient perdues. Il vous demande s'il faut **Enregistrer une copie…**, **Remplacer** le fichier ou **Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien, et ne le remplace qu'une fois complet. En cas de problème, un message s'affiche et le fichier existant reste intact.

Si les formules sont encore en cours de calcul lors de l'enregistrement, JeffCrunch attend la fin du calcul pour que le fichier contienne leurs résultats. Si vous appuyez entre-temps sur **Échap** pour interrompre le calcul, l'enregistrement est annulé et la barre d'état l'indique ; enregistrez de nouveau quand vous le souhaitez. JeffCrunch omet un seul élément du fichier : la liste de l'ordre dans lequel un programme a calculé les formules la dernière fois (la chaîne de calcul). Le programme qui ouvrira ensuite le classeur la reconstituera. Si JeffCrunch n'a pas pu calculer une formule, ou si vous avez interrompu le calcul, le fichier demande à ce programme de recalculer toutes les formules à l'ouverture du classeur.

## Fermer un classeur modifié

Si vous fermez un classeur dont des modifications ne sont pas enregistrées, JeffCrunch vous demande s'il faut **Enregistrer** les modifications, **Ne pas enregistrer**, ou **Annuler** pour continuer à travailler. Un astérisque (*) dans le titre de la fenêtre signale des modifications non enregistrées.
