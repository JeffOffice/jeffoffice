# Ouvrir et enregistrer des classeurs
<!-- keywords: copier, ancien format, opendocument, compatibilité, nouveau, vierge, classeur1, sans titre, ouvrir, enregistrer, enregistrer sous, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, importer, exporter, récent, épingler, glisser, déposer, chargement, volumineux, gros, grand, macros, non enregistré, modifications, fermer, chaîne de calcul, modèle, xltx, xltm, impossible d'ouvrir, endommagé, chiffré, mot de passe, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir comme copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, ots, réparer, ouvrir et réparer -->

Quand JeffCrunch démarre sans fichier à ouvrir, la fenêtre contient un nouveau classeur vide, avec une seule feuille, nommé Classeur1. Chaque nouveau classeur prend le numéro suivant (Classeur2, Classeur3, et ainsi de suite), et un numéro n'est plus réutilisé une fois sa fenêtre fermée. Pour en créer un autre, appuyez sur **{key:Nouveau}**, ou cliquez sur **Fichier**, puis sur **Nouveau**, et choisissez **Classeur vierge** ; il s'ouvre dans une fenêtre qui lui est propre. Un nouveau classeur que vous n'avez pas modifié cède la place lorsque vous ouvrez un fichier dans sa fenêtre ; un classeur que vous avez modifié reste en place, et le fichier s'ouvre dans une autre fenêtre. **Enregistrer** sur un nouveau classeur demande un nom, comme **Enregistrer sous**, et l'enregistre au format .xlsx. La fermeture d'un nouveau classeur non modifié ne demande rien.

Pour ouvrir un classeur, appuyez sur **{key:Ouvrir}**, ou cliquez sur **Fichier** puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez aussi faire glisser un classeur depuis votre gestionnaire de fichiers vers la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une seconde fois : cette fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et les classeurs **.xlsm** (ceux qui contiennent des macros), ainsi que les modèles **.xltx** et **.xltm** (un modèle qui contient des macros). Il n'exécute pas les macros et les conserve intactes : le projet de macros, sa signature et les parties qui les désignent sont réécrits exactement tels qu'ils étaient. Un modèle est enregistré comme modèle, et un classeur avec macros comme classeur avec macros, car le type fait partie du fichier.

## Autres types de fichiers

JeffCrunch ouvre aussi les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et le format plat **.fods**), ainsi que les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert à partir de l'un de ces fichiers est enregistré sous la forme d'un nouveau fichier **.xlsx** (ou d'une copie **.xls** ou **.ods** : voir ci-dessous) : lorsque vous appuyez sur **{key:Enregistrer}**, JeffCrunch vous le signale et demande un nom, et le fichier que vous avez ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats conservés dans le fichier), ses formats de nombre, ses polices, ses remplissages, ses bordures, ses cellules fusionnées, ses largeurs de colonne et hauteurs de ligne, ses lignes et colonnes masquées, ses volets figés et ses noms. Les graphiques, images et autres dessins, commentaires, tableaux croisés dynamiques, macros, mises en forme conditionnelles et validations de données ne sont pas encore lus ; un message en haut vous indique lesquels le fichier contient, et une copie enregistrée depuis ici n'en contiendra pas. Une feuille de calcul protégée par un mot de passe demande ce mot de passe à l'ouverture : voir [Ouvrir des classeurs protégés par mot de passe](open-password).

Un fichier texte ne compte qu'une feuille. JeffCrunch détermine comment ses valeurs sont séparées (par des virgules, des points-virgules, des tabulations ou des barres verticales) et comment ses caractères sont codés (UTF-8 ou UTF-16, sinon la page de codes de votre langue), et lit chaque valeur comme vous la saisiriez dans votre langue : un nombre tel que 1.234,5 en allemand, un pourcentage, un montant en devise, une date avec son année (comme 15/03/2024, ou 15.03.2024 en allemand), une heure, VRAI ou FAUX (dans votre langue, comme WAHR et FALSCH en allemand, et TRUE et FALSE en anglais). Une valeur qui commence par un signe égal est une formule. Les chiffres qui commencent par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, afin qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même pour un jour et un mois sans année, dans n'importe quel ordre et dans n'importe quelle langue (comme 3/4, 12.5, 1-2, 1 mars ou 25 déc.), car l'année devrait être inventée, ainsi que pour des textes comme le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en contient davantage, JeffCrunch ouvre ce qui tient et vous le signale.

Lorsque vous ouvrez un fichier texte, une fenêtre affiche d'abord ses premières lignes et vous demande comment il doit être lu : quel caractère sépare les champs (une virgule, un point-virgule, une tabulation, une espace, une barre verticale ou un caractère que vous saisissez, avec la possibilité de considérer plusieurs séparateurs consécutifs comme un seul), quel caractère encadre un champ qui contient un séparateur, l'encodage du texte (Unicode, les pages de codes Windows, les pages ISO 8859, Mac Roman et les jeux d'Asie de l'Est) et la ligne où commencer. Au-dessus de chaque colonne, vous choisissez comment elle est lue : **Tel quel**, **Texte** (pour conserver un numéro de pièce comme 007 ou un long numéro de compte tel quel), **Date** selon l'ordre que vous indiquez, ou **Ne pas importer**. L'aperçu suit chaque modification et affiche au plus les 200 premières lignes et les 100 premières colonnes, mais le fichier entier est ouvert. **Fichier > Ouvrir > Ouvrir un fichier texte** demande ces réglages même lorsque le nom du fichier est inhabituel.

Pour ne conserver que les valeurs de la feuille que vous consultez, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv** (ou **.tsv** et **.txt**, séparés par des tabulations). Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas conserver (les autres feuilles, les formules, les images, les graphiques, les notes et les macros, par exemple), demande comment l'écrire, et propose d'**Enregistrer les valeurs uniquement**, d'**Enregistrer au format .xlsx** à la place pour tout conserver, ou d'**Annuler** :

- **Séparateur.** Une virgule, un point-virgule, une tabulation, une espace, une barre verticale ou un caractère que vous saisissez. Un **.csv** commence par un point-virgule là où votre langue écrit une virgule dans ses nombres, et par une virgule ailleurs ; un **.tsv** ou un **.txt** avec une tabulation.
- **Encodage du texte.** UTF-8 avec marque d'ordre (pour que les autres programmes lisent correctement les lettres accentuées), UTF-8 sans celle-ci, UTF-16, ou une page de codes comme Europe occidentale (1252), les pages ISO 8859 ou Mac Roman. Une lettre pour laquelle la page de codes n'a pas d'octet est écrite sous la forme d'un point d'interrogation, et la barre d'état indique combien il y en a.
- **Fins de ligne.** Windows (retour chariot et saut de ligne), Linux et Mac (saut de ligne), ou ancien Mac (retour chariot).
- **Cellules.** **Tel qu'affiché dans les cellules** écrit ce que vous voyez. **Tel qu'enregistré** écrit chaque chiffre d'un nombre (0,1 et non 10 %), et une date sous forme de texte ISO, comme 2026-03-04.

Un texte est placé entre guillemets lorsqu'il contient le séparateur, un guillemet, un saut de ligne, ou une espace en début ou en fin. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans le fichier, et votre classeur n'est pas modifié par l'écriture d'un tel fichier.

Un classeur issu d'un fichier texte est réécrit sous la forme qu'avait le fichier : le même séparateur, les mêmes fins de ligne et le même encodage. Si vous l'avez enregistré comme copie sans modifier une cellule, la copie est le même fichier, octet pour octet, et rien ne vous est demandé.

## Enregistrer une copie en .xls ou .ods

Pour faire une copie que des anciens programmes, ou des programmes qui lisent OpenDocument, peuvent ouvrir, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.xls** ou **.ods**. La copie est faite à partir de votre classeur tel qu'il est maintenant, avec les modifications que vous n'avez pas encore enregistrées, et contient :

- les valeurs, avec leurs textes, leurs nombres, leurs valeurs logiques et leurs erreurs ;
- les formules, avec les résultats qu'elles ont maintenant (une formule matricielle reste une formule matricielle) ;
- les formats de nombre, les polices, les remplissages, les bordures, l'alignement, et l'indication qu'une cellule est verrouillée ;
- les cellules fusionnées, les largeurs de colonne et hauteurs de ligne, les lignes et colonnes masquées, les plans de lignes et de colonnes, et les volets figés ;
- les feuilles, dans leur ordre, avec leurs noms et celles qui sont masquées, ainsi que les noms que vous avez définis (et les zones d'impression) ;
- les liens, et la manière dont chaque feuille s'imprime (le papier, les marges, l'orientation, l'échelle, les en-têtes et pieds de page, et les sauts de page).

Un fichier **.xls** contient aussi les tables de données, et conserve les paramètres de calcul du classeur. Une copie ne peut pas tout contenir. Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas conserver : graphiques, images et autres dessins, notes, tableaux croisés dynamiques, tableaux, mises en forme conditionnelles, validations de données, filtres, protection de feuille, contrôles de formulaire, macros et liens vers d'autres classeurs (les formules qui les utilisent conservent les valeurs qu'elles avaient). Une formule que l'ancien format ne sait pas écrire (une fonction qu'il ne connaît pas, ou une cellule au-delà de la colonne IV ou de la ligne 65 536) est écrite sous la forme de la valeur obtenue, et le message en indique le nombre. L'ancien format s'arrête aussi à 65 536 lignes, 256 colonnes, 255 feuilles et 32 767 caractères dans une cellule, et à environ 4 000 formats de cellule et 56 couleurs : ce qui ne tient pas est coupé, et le message indique de combien. Vous pouvez alors choisir d'**Enregistrer la copie**, d'**Enregistrer au format .xlsx** pour tout conserver, ou d'**Annuler**.

Votre classeur n'est pas modifié par la copie, et il n'est pas marqué comme enregistré. Le fichier que vous avez ouvert n'est jamais écrit : donnez un autre nom à la copie. Une copie d'un fichier **.xls** ou **.ods** que vous avez ouvert peut être enregistrée dans l'autre type.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie** et choisissez un nom. JeffCrunch écrit le classeur dans ce fichier, avec les modifications non enregistrées, et vous continuez à travailler dans le classeur que vous aviez. Son nom et ses modifications non enregistrées restent ce qu'ils étaient. Vous pouvez choisir n'importe quel type de fichier proposé par **Enregistrer sous**. Une copie n'est jamais écrite par-dessus le fichier propre au classeur.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, sur la page **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffCrunch copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être faite, l'application demande s'il faut enregistrer sans elle. La réponse est Non, sauf si vous répondez Oui, et le fichier reste tel qu'il était.

## Enregistrer comme modèle

Un modèle est un classeur que vous utilisez comme point de départ. Il peut contenir des formules, des styles et des feuilles. Cliquez sur **Fichier > Enregistrer comme modèle** et saisissez un nom. Le modèle (un fichier **.xltx**) est placé dans **Mes modèles**, et votre classeur n'est pas modifié. Dans **Enregistrer sous**, vous pouvez aussi choisir **Modèles (*.xltx)** ou **Modèles de feuille de calcul OpenDocument (*.ots)** pour enregistrer un fichier modèle à l'emplacement de votre choix.

Pour créer un classeur à partir de votre modèle, cliquez sur **Fichier > Nouveau**. Vos modèles sont listés sous **Mes modèles**. Cliquez sur l'un d'eux. Un nouveau classeur s'ouvre avec les cellules et les feuilles du modèle. Il n'a pas encore de fichier propre, donc **Enregistrer** demande un nom, et le modèle n'est pas modifié. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des éléments récents.

- **Ouvrir en lecture seule** ouvre le classeur uniquement pour la lecture. Vous pouvez consulter, faire défiler, rechercher et copier, mais vous ne pouvez pas modifier une cellule. La barre de titre affiche « Lecture seule ». Si vous appuyez sur **Enregistrer**, JeffCrunch demande un nouveau nom, afin que le fichier ouvert ne soit pas modifié.
- **Ouvrir une copie** ouvre le classeur sans fichier propre. **Enregistrer** demande où le placer. Le fichier que vous avez ouvert n'est jamais modifié.
- **Ouvrir dans une nouvelle fenêtre** ouvre le classeur dans une fenêtre qui lui est propre, même lorsque la fenêtre dans laquelle vous vous trouvez est vide.
- **Ouvrir et réparer** sert pour un fichier .xlsx endommagé. Voir ci-dessous.

## Ouvrir et réparer

Il arrive qu'un fichier .xlsx soit endommagé : un téléchargement interrompu à mi-parcours, ou un disque qui a commis des erreurs. Un fichier .xlsx est un ensemble de petites parties, comme les feuilles, les styles et le texte partagé. **Ouvrir et réparer** lit chaque partie qu'il peut lire. Une partie interrompue au milieu est conservée jusqu'à l'endroit où elle s'arrête, de sorte que vous pouvez n'obtenir que les premières lignes d'une feuille. Une partie qui ne peut pas être lue est omise. Si la liste des parties ou les liens entre elles ont disparu, JeffCrunch les recrée.

Le classeur réparé s'ouvre sans fichier propre, sous un nom tel que « Budget réparé ». Un message indique ce qui a été conservé jusqu'à une interruption et ce qui a été omis. Le fichier endommagé n'est pas modifié. Enregistrez le classeur réparé sous un nouveau nom, et vérifiez les chiffres avant de vous y fier.

## Grands classeurs

JeffCrunch lit un classeur en arrière-plan. Les premières lignes de la feuille qui était ouverte au moment de l'enregistrement du classeur apparaissent presque aussitôt, même lorsque le classeur est très volumineux. La barre d'état, en bas à gauche, indique ce qu'il fait : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules lues jusqu'alors, **Calcul** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles s'activent au fur et à mesure de leur lecture. Un classeur endommagé est ouvert autant qu'il peut être lu, et un message indique ce qui a été omis.

## Si un fichier ne s'ouvre pas

Lorsque JeffCrunch ne peut pas ouvrir un fichier, il l'indique en termes simples et laisse les classeurs que vous avez ouverts tels qu'ils sont. Il distingue un fichier vide d'un fichier qui n'est pas du tout un classeur (un document PDF, une image, un document texte ou une présentation, par exemple) ; un paquet zip tronqué ou endommagé, ce qui signifie en général qu'un téléchargement ou une copie a été interrompu, et qu'il faut le copier à nouveau depuis l'original ; un classeur protégé par un mot de passe, pour lequel il demande ce mot de passe (voir [Ouvrir des classeurs protégés par mot de passe](open-password)) ; et un fichier composite qui ne contient aucun classeur.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier liste les classeurs que vous avez ouverts récemment. Faites un clic droit sur l'un d'eux pour l'**épingler** en haut de la liste ou pour le retirer de la liste.

## Enregistrement

Appuyez sur **{key:Enregistrer}**, ou cliquez sur **Fichier** puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur que vous avez ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier** puis sur **Enregistrer sous**, et choisissez un nom. Un classeur avec macros est enregistré au format **.xlsm**, un modèle au format **.xltx** ou **.xltm**, et tout autre classeur au format **.xlsx**. Un nom qui se termine par un autre type de classeur n'est pas accepté, et le message indique quelle terminaison utiliser ; les noms qui se terminent par **.xls**, **.ods**, **.csv**, **.tsv** et **.txt** créent des copies, comme décrit plus haut.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier, comme les autres feuilles, les graphiques, les images, les macros et les paramètres que JeffCrunch n'affiche pas, est recopié tel quel, octet pour octet. Dans une feuille que vous avez modifiée, seules les cellules modifiées sont réécrites, ainsi que les formules qui en dépendent, qui reçoivent leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier n'est pas touché.

Si un autre programme a modifié le fichier après son ouverture, JeffCrunch n'écrase pas celui-ci sans demander, car cela ferait perdre les modifications de l'autre programme. Il demande s'il faut **Enregistrer une copie**, **Remplacer** le fichier, ou **Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien, et ne prend la place de celui-ci qu'une fois complet. Si un problème survient, un message s'affiche, et le fichier existant reste tel qu'il était.

Si les formules sont encore en cours de calcul au moment de l'enregistrement, JeffCrunch attend qu'elles soient terminées, afin que le fichier contienne leurs résultats. Si vous appuyez sur **Échap** pour arrêter le calcul entre-temps, l'enregistrement est annulé, et la barre d'état l'indique ; enregistrez de nouveau lorsque vous le souhaitez. JeffCrunch omet une chose dans le fichier : la liste de l'ordre dans lequel un programme a calculé les formules en dernier (la chaîne de calcul). Le programme qui ouvrira le classeur ensuite la recréera. Si JeffCrunch n'a pas pu calculer certaines formules, ou si vous avez arrêté le calcul, le fichier demande au programme qui l'ouvre de recalculer toutes les formules à l'ouverture du classeur.

## Fermeture avec modifications

Si vous fermez un classeur qui contient des modifications non enregistrées, JeffCrunch demande s'il faut les **enregistrer**, **ne pas les enregistrer**, ou **annuler** et poursuivre votre travail. Un astérisque (*) dans le titre de la fenêtre indique qu'il reste des modifications non enregistrées.
