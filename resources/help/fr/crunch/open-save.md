# Ouvrir et enregistrer des classeurs
<!-- keywords: récupérer le travail non enregistré, récupération automatique, plantage, copie, ancien format, opendocument, compatibilité, nouveau, vierge, classeur1, sans titre, ouvrir, enregistrer, enregistrer sous, fichier, classeur, xlsx, xlsm, xls, ods, csv, tsv, fichier texte, délimité, importer, exporter, récent, épinglé, glisser, déposer, chargement, volumineux, grand, macros, non enregistré, modifications, fermer, copie, chaîne de calcul, modèle, xltx, xltm, impossible d'ouvrir, endommagé, chiffré, mot de passe, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir comme copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, ots, réparer, ouvrir et réparer, modèles intégrés, budget, facture, feuille de temps, prêt, calendrier, carnet de notes, largeur fixe, colonnes fixes, séparateur décimal, séparateur des milliers, signe moins final, html, page web, htm, tableaux d'une page -->

Quand JeffCrunch démarre sans fichier à ouvrir, la fenêtre affiche un nouveau classeur vide comportant une seule feuille, nommé Classeur1. Chaque nouveau classeur prend le numéro suivant (Classeur2, Classeur3, et ainsi de suite), et un numéro n'est plus réutilisé après la fermeture de sa fenêtre. Pour en démarrer un autre, appuyez sur **{key:New}**, ou cliquez sur **Fichier**, puis sur **Nouveau**, et choisissez **Classeur vide** ; il s'ouvre dans une fenêtre qui lui est propre. Un nouveau classeur que vous n'avez pas modifié cède la place lorsque vous ouvrez un fichier dans sa fenêtre ; un classeur que vous avez modifié reste en place, et le fichier s'ouvre dans une autre fenêtre. **Enregistrer** sur un nouveau classeur demande un nom, comme le fait **Enregistrer sous**, et l'enregistre au format .xlsx. La fermeture d'un nouveau classeur non modifié ne demande rien.

Pour ouvrir un classeur, appuyez sur **{key:Open}**, ou cliquez sur **Fichier** puis sur **Ouvrir**, et choisissez **Parcourir**. Vous pouvez aussi faire glisser un classeur depuis votre gestionnaire de fichiers vers la fenêtre. Un classeur déjà ouvert dans une fenêtre n'est pas ouvert une seconde fois : cette fenêtre passe au premier plan.

JeffCrunch ouvre les classeurs **.xlsx** et les classeurs **.xlsm** (ceux qui contiennent des macros), ainsi que les modèles **.xltx** et **.xltm** (un modèle qui contient des macros). Il n'exécute pas les macros et les laisse intactes : le projet de macros, sa signature et les parties qui les désignent sont réécrits exactement comme ils étaient. Un modèle est enregistré en tant que modèle, et un classeur contenant des macros en tant que classeur contenant des macros, car le type fait partie du fichier.

## Récupérer le travail non enregistré

Si JeffCrunch se ferme brusquement, par exemple lors d'une coupure de courant, le prochain démarrage affiche la liste des copies qu'il a conservées de votre travail non enregistré. Choisissez celles à ouvrir et cliquez sur **Ouvrir** ; chacune s'ouvre comme un nouveau classeur sans fichier, il vous suffit alors de cliquer sur **Enregistrer** et de choisir un nom. **Supprimer** élimine les copies cochées, et **Pas maintenant** les conserve pour plus tard. Vous pouvez afficher la liste à tout moment avec **Fichier > Ouvrir > Récupérer le travail non enregistré**. Consultez [Modifier les paramètres de JeffCrunch](options) pour savoir à quelle fréquence les copies sont créées.

## Autres types de fichiers

JeffCrunch ouvre également les anciens classeurs **.xls**, les feuilles de calcul OpenDocument (**.ods** et **.fods**, au format plat) et les fichiers texte de valeurs (**.csv**, **.tsv** et **.txt**). Un classeur ouvert à partir de l'un de ces fichiers est enregistré comme nouveau fichier **.xlsx** (ou comme copie **.xls** ou **.ods** : voir ci-dessous) : lorsque vous appuyez sur **{key:Save}**, JeffCrunch vous le signale et demande un nom, et le fichier ouvert n'est jamais modifié.

Une feuille de calcul OpenDocument est lue avec ses feuilles, ses valeurs, ses formules (avec les résultats conservés dans le fichier), ses formats de nombre, ses polices, ses remplissages, ses bordures, ses cellules fusionnées, les largeurs de colonne et les hauteurs de ligne, les lignes et colonnes masquées, les volets figés et les noms. Les graphiques, images et autres dessins, commentaires, tableaux croisés dynamiques, macros, mises en forme conditionnelles et validations des données ne sont pas encore lus ; un message en haut vous indique lesquels le fichier contient, et une copie enregistrée à partir d'ici ne les contiendra pas. Une feuille de calcul protégée par un mot de passe demande ce mot de passe à l'ouverture : voir [Ouvrir des classeurs protégés par mot de passe](open-password).

Un fichier texte constitue une seule feuille. JeffCrunch détermine comment ses valeurs sont séparées (par des virgules, des points-virgules, des tabulations ou des barres verticales) et comment ses caractères sont codés (UTF-8 ou UTF-16, ou sinon la page de codes de votre langue), puis lit chaque valeur comme vous la saisiriez dans votre langue : un nombre tel que 1.234,5 en allemand, un pourcentage, un montant en devise, une date avec son année (comme 15/03/2024, ou 15.03.2024 en allemand), une heure, VRAI ou FAUX (dans votre langue, comme WAHR et FALSCH en allemand, et TRUE et FALSE en anglais). Une valeur qui commence par un signe égal est une formule. Les chiffres qui commencent par un zéro (comme 007) et les nombres de plus de 15 chiffres restent du texte, afin qu'un code postal ou un numéro de compte ne soit pas modifié ; il en va de même pour un jour et un mois sans année, dans n'importe quel ordre et dans n'importe quelle langue (comme 3/4, 12.5, 1-2, 1 mars ou 25 déc.), car l'année devrait être inventée, ainsi que pour des textes tels que le nom de gène MARCH1. Une feuille contient 1 048 576 lignes et 16 384 colonnes ; si le fichier en contient davantage, JeffCrunch ouvre ce qui peut tenir et vous en informe.

À l'ouverture d'un fichier texte, une fenêtre affiche d'abord ses premières lignes et vous demande comment le lire : ce qui sépare les champs (une virgule, un point-virgule, une tabulation, une espace, une barre verticale ou un caractère que vous saisissez, avec la possibilité de considérer plusieurs séparateurs consécutifs comme un seul), ce qui entoure un champ contenant un séparateur, l'encodage du texte (Unicode, les pages de codes Windows, les pages ISO 8859, Mac Roman et les pages d'Asie orientale) et la ligne de début. Au-dessus de chaque colonne, vous choisissez comment la lire : **Tel que saisi**, **Texte** (pour conserver telle quelle une référence comme 007 ou un long numéro de compte), **Date** dans l'ordre que vous indiquez, ou **Ignorer**. Certains fichiers alignent leurs valeurs en colonnes de même largeur, séparées par des espaces, sans aucun séparateur. Cochez **Les colonnes ont une largeur fixe** : les premières lignes du fichier s'affichent alors dans une zone. Cliquez entre deux caractères pour commencer une colonne à cet endroit, et cliquez à nouveau pour la supprimer ; vous pouvez aussi saisir les positions dans **Les colonnes commencent à**, par exemple 10, 25, 40. Chaque élément est découpé à ces positions, et les espaces qui l'entourent sont supprimés. **Séparateur décimal** et **Séparateur des milliers** indiquent à JeffCrunch la façon dont les nombres sont écrits dans ce fichier, quelle que soit la langue de votre logiciel : 1.234,50 vaut mille deux cent trente-quatre et une demi-unité si le fichier utilise la virgule comme séparateur décimal. Cochez **Un signe moins peut suivre le nombre** pour les fichiers qui écrivent -12 sous la forme 12-. L'aperçu suit chaque modification et affiche au maximum les 200 premières lignes et les 100 premières colonnes, mais la totalité du fichier est ouverte. **Fichier > Ouvrir > Ouvrir un fichier texte** affiche cette fenêtre même lorsque le nom du fichier est inhabituel.

JeffCrunch ouvre également les **pages web** (**.html** et **.htm**), ainsi qu'une page enregistrée sous le nom d'un classeur. Chaque tableau de la page devient une feuille, nommée **Tableau1**, **Tableau2**, et ainsi de suite. Les cellules sont lues comme si vous les aviez saisies : un nombre reste un nombre, et une date reste une date. Une cellule fusionnée sur plusieurs colonnes ou lignes laisse des cellules vides, afin que les autres cellules restent dans leurs colonnes. Les mots situés hors des tableaux, les images et la mise en forme de la page ne sont pas lus, et une page sans tableau ne s'ouvre pas. Rien de ce que la page désigne n'est récupéré sur Internet.

Pour ne conserver que les valeurs de la feuille affichée, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.csv** (ou **.tsv** et **.txt**, séparés par des tabulations). Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas conserver (les autres feuilles, les formules, les images, les graphiques, les notes et les macros, par exemple), puis demande comment écrire le fichier, et s'il faut **Enregistrer les valeurs uniquement**, **Enregistrer au format .xlsx** à la place pour tout conserver, ou **Annuler** :

- **Séparateur.** Une virgule, un point-virgule, une tabulation, une espace, une barre verticale, ou un caractère que vous saisissez. Un **.csv** commence par un point-virgule là où votre langue écrit une virgule dans ses nombres, et par une virgule ailleurs ; un **.tsv** ou un **.txt** utilise une tabulation.
- **Encodage du texte.** UTF-8 avec marque (afin que les autres programmes reconnaissent correctement les lettres accentuées), UTF-8 sans marque, UTF-16, ou une page de codes telle que Europe occidentale (1252), les pages ISO 8859, Mac Roman, ou les pages japonaise, chinoise, coréenne, hébreu, arabe, balte, vietnamienne et thaïe. Une lettre qui n'a pas d'octet correspondant dans la page de codes est écrite sous la forme d'un point d'interrogation, et la barre d'état indique combien il y en a.
- **Fins de ligne.** Windows (retour chariot et saut de ligne), Linux et Mac (saut de ligne), ou ancien Mac (retour chariot).
- **Cellules.** **Tel qu'affiché dans les cellules** écrit ce que vous voyez. **Tel qu'enregistré** écrit chaque chiffre d'un nombre (0,1 et non 10 %), et une date sous forme de texte ISO, comme 2026-03-04.

Un texte est placé entre guillemets lorsqu'il contient le séparateur, un guillemet, un saut de ligne, ou une espace en début ou en fin. Les autres feuilles, les formules, les formats et les paramètres ne figurent pas dans le fichier, et votre classeur n'est pas modifié par l'écriture d'un fichier.

Un classeur issu d'un fichier texte est réécrit sous la forme qu'avait ce fichier : même séparateur, mêmes fins de ligne et même encodage. Si vous l'enregistrez comme copie sans modifier une cellule, la copie est identique au fichier, octet par octet, et rien ne vous est demandé.

## Enregistrer une copie au format .xls ou .ods

Pour créer une copie que d'anciens programmes, ou des programmes qui lisent OpenDocument, peuvent ouvrir, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez le type de fichier **.xls** ou **.ods**. La copie est faite à partir de votre classeur tel qu'il est maintenant, avec les modifications que vous n'avez pas encore enregistrées, et contient :

- les valeurs, avec leurs textes, nombres, valeurs logiques et erreurs ;
- les formules, avec les résultats qu'elles ont actuellement (une formule matricielle reste une formule matricielle) ;
- les formats de nombre, polices, remplissages, bordures, alignements, et l'indication de verrouillage de chaque cellule ;
- les cellules fusionnées, les largeurs de colonne et hauteurs de ligne, les lignes et colonnes masquées, les plans de lignes et de colonnes, et les volets figés ;
- les feuilles, dans leur ordre, avec leurs noms et celles qui sont masquées, ainsi que les noms que vous avez définis (et les zones d'impression) ;
- les liens, et la manière dont chaque feuille s'imprime (le papier, les marges, l'orientation, l'échelle, les en-têtes et pieds de page, et les sauts de page).

Un fichier **.xls** contient également les tables de données, et conserve les paramètres de calcul du classeur. Une copie ne peut pas tout contenir. Avant d'écrire le fichier, JeffCrunch indique ce que votre classeur contient et que le fichier ne pourra pas conserver : graphiques, images et autres dessins, notes, tableaux croisés dynamiques, tableaux, mises en forme conditionnelles, validations des données, filtres, protection des feuilles, contrôles de formulaire, macros, et liens vers d'autres classeurs (les formules qui les utilisent conservent les valeurs qu'elles avaient). Une formule que l'ancien format ne sait pas exprimer (une fonction qu'il ne connaît pas, ou une cellule au-delà de la colonne IV ou de la ligne 65 536) est écrite sous la forme de la valeur obtenue, et le message en indique le nombre. L'ancien format s'arrête également à 65 536 lignes, 256 colonnes, 255 feuilles et 32 767 caractères par cellule, ainsi qu'à environ 4 000 formats de cellule et 56 couleurs : ce qui ne tient pas est coupé, et le message indique de combien. Vous pouvez alors choisir d'**Enregistrer la copie**, d'**Enregistrer au format .xlsx** pour tout conserver, ou d'**Annuler**.

La copie ne modifie pas votre classeur, qui n'est pas marqué comme enregistré. Le fichier que vous avez ouvert n'est jamais réécrit : donnez un autre nom à la copie. Une copie d'un fichier **.xls** ou **.ods** que vous avez ouvert peut être enregistrée dans l'autre type de fichier.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie** et choisissez un nom. JeffCrunch écrit le classeur dans ce fichier, avec les modifications non enregistrées, et vous continuez à travailler dans le classeur que vous aviez. Son nom et ses modifications non enregistrées restent inchangés. Vous pouvez choisir n'importe quel type de fichier proposé par **Enregistrer sous**. Une copie n'est jamais écrite par-dessus le fichier du classeur lui-même.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, sur la page **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffCrunch copie d'abord l'ancien fichier sous un nom commençant par « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, l'application vous demande s'il faut enregistrer sans elle. La réponse est Non, sauf si vous répondez Oui, et le fichier reste tel qu'il était.

## Enregistrer comme modèle

Un modèle est un classeur que vous utilisez comme point de départ. Il peut contenir des formules, des styles et des feuilles. Cliquez sur **Fichier > Enregistrer comme modèle** et saisissez un nom. Le modèle (un fichier **.xltx**) est placé dans **Mes modèles**, et votre classeur n'est pas modifié. Dans **Enregistrer sous**, vous pouvez aussi choisir **Modèles (*.xltx)** ou **Modèles de feuille de calcul OpenDocument (*.ots)** pour enregistrer un fichier de modèle à l'emplacement de votre choix.

Pour démarrer un classeur à partir de votre modèle, cliquez sur **Fichier > Nouveau**. Vos modèles sont listés sous **Mes modèles**. Cliquez sur l'un d'eux. Un nouveau classeur s'ouvre avec les cellules et les feuilles du modèle. Il n'a pas encore de fichier, de sorte que **Enregistrer** demande un nom, et le modèle n'est pas modifié. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des fichiers récents.

- **Ouvrir en lecture seule** ouvre le classeur uniquement pour la lecture. Vous pouvez consulter, faire défiler, rechercher et copier, mais vous ne pouvez pas modifier une cellule. La barre de titre affiche « Lecture seule ». Si vous appuyez sur **Enregistrer**, JeffCrunch demande un nouveau nom, afin que le fichier ouvert ne soit pas modifié.
- **Ouvrir comme copie** ouvre le classeur sans fichier qui lui est propre. **Enregistrer** demande où le placer. Le fichier ouvert n'est jamais modifié.
- **Ouvrir dans une nouvelle fenêtre** ouvre le classeur dans une fenêtre qui lui est propre, même lorsque la fenêtre active est vide.
- **Ouvrir et réparer** sert pour un fichier .xlsx ou .xls endommagé. Voir ci-dessous.

## Ouvrir et réparer

Il arrive qu'un fichier .xlsx soit endommagé, par exemple lorsqu'un téléchargement s'est interrompu à mi-parcours ou qu'un disque a commis des erreurs. Un fichier .xlsx est un ensemble de petites parties, comme les feuilles, les styles et le texte partagé. **Ouvrir et réparer** lit chaque partie qu'il peut lire. Une partie interrompue au milieu est conservée jusqu'à l'endroit où elle s'arrête : vous pourriez donc obtenir seulement les premières lignes d'une feuille. Une partie illisible est omise. Si la liste des parties ou les liens entre elles ont disparu, JeffCrunch les recrée.

Le classeur réparé s'ouvre sans fichier qui lui est propre, sous un nom tel que « Budget réparé ». Un message vous indique ce qui a été conservé jusqu'à une rupture et ce qui a été omis. Le fichier endommagé n'est pas modifié. Enregistrez le classeur réparé sous un nouveau nom, et vérifiez les chiffres avant de vous y fier.

### Un fichier .xls

Un fichier .xls peut lui aussi être endommagé. Si son enveloppe extérieure est tronquée, **Ouvrir et réparer** recherche dans le fichier le début du classeur et lit à partir de cet endroit. Une feuille tronquée conserve les cellules situées avant la coupure. Ce qui suit la coupure est perdu, et vous en êtes informé. Le classeur réparé s'ouvre comme copie : enregistrez-le sous un nouveau nom.

## Classeurs volumineux

JeffCrunch lit un classeur en arrière-plan. Les premières lignes de la feuille qui était ouverte au moment de l'enregistrement du classeur apparaissent presque immédiatement, même lorsque le classeur est très volumineux. La barre d'état, en bas à gauche, indique ce qu'il fait : **Ouverture**, **Lecture** d'une feuille avec le nombre de cellules lues jusque-là, **Calcul** avec un pourcentage, puis **Prêt**.

Vous pouvez faire défiler, sélectionner et copier pendant la lecture. Les onglets des autres feuilles deviennent actifs au fur et à mesure de leur lecture. Un classeur endommagé est ouvert jusqu'à l'endroit où il peut être lu, et un message indique ce qui a été omis.

## Si un fichier ne s'ouvre pas

Lorsque JeffCrunch ne peut pas ouvrir un fichier, il indique la raison en termes simples et laisse les classeurs ouverts tels quels. Il distingue un fichier vide d'un fichier qui n'est pas du tout un classeur (un document PDF, une image, un document texte ou une présentation, par exemple) ; un paquet zip tronqué ou endommagé, ce qui signifie généralement qu'un téléchargement ou une copie a été interrompu, et qu'il faut le copier de nouveau à partir de l'original ; un classeur protégé par un mot de passe, pour lequel il demande ce mot de passe (voir [Ouvrir des classeurs protégés par mot de passe](open-password)) ; et un fichier composite ne contenant aucun classeur.

## Classeurs récents

La page **Ouvrir** de l'écran Fichier liste les classeurs que vous avez ouverts récemment. Faites un clic droit sur l'un d'eux pour l'**épingler** en haut de la liste, ou pour le retirer de la liste.

## Enregistrer

Appuyez sur **{key:Save}**, ou cliquez sur **Fichier**, puis sur **Enregistrer**, pour enregistrer vos modifications dans le classeur ouvert. Pour conserver l'original et créer un nouveau fichier, cliquez sur **Fichier**, puis sur **Enregistrer sous**, et choisissez un nom. Un classeur contenant des macros est enregistré au format **.xlsm**, un modèle au format **.xltx** ou **.xltm**, et tout autre classeur au format **.xlsx**. Un nom qui se termine par un autre type de classeur n'est pas accepté, et le message indique la terminaison à utiliser ; les noms qui se terminent par **.xls**, **.ods**, **.csv**, **.tsv** et **.txt** créent des copies, comme décrit ci-dessus.

JeffCrunch n'enregistre que ce que vous avez modifié. Tout le reste du fichier, comme les autres feuilles, les graphiques, les images, les macros et les paramètres que JeffCrunch n'affiche pas, est recopié tel quel, octet par octet. Dans une feuille que vous avez modifiée, seules les cellules changées sont réécrites, ainsi que les formules qui en dépendent, qui reçoivent leurs nouvelles valeurs. Si vous n'avez modifié aucune cellule, le fichier n'est pas touché.

Si un autre programme a modifié le fichier après son ouverture, JeffCrunch ne l'écrase pas sans demander confirmation, car cela ferait perdre les modifications de cet autre programme. Il demande s'il faut **Enregistrer une copie**, **Remplacer** le fichier, ou **Annuler**.

Le nouveau fichier est d'abord écrit à côté de l'ancien, et ne prend la place de celui-ci que lorsqu'il est complet. Si quelque chose ne va pas, un message s'affiche, et le fichier existant reste tel qu'il était.

Si les formules sont encore en cours de calcul au moment de l'enregistrement, JeffCrunch attend qu'elles soient terminées, afin que le fichier contienne leurs résultats. Si vous appuyez sur **Échap** pour arrêter le calcul entre-temps, l'enregistrement est annulé, et la barre d'état l'indique ; enregistrez de nouveau lorsque vous le souhaitez. JeffCrunch omet un élément du fichier : la liste de l'ordre dans lequel un programme a calculé les formules pour la dernière fois (la chaîne de calcul). Un programme qui ouvrira le classeur ensuite la recréera. Si JeffCrunch n'a pas pu calculer certaines formules, ou si vous avez arrêté le calcul, le fichier demande au programme qui l'ouvrira de recalculer toutes les formules.

## Fermer en ayant des modifications

Si vous fermez un classeur qui contient des modifications non enregistrées, JeffCrunch demande s'il faut **Enregistrer** ces modifications, **Ne pas enregistrer**, ou **Annuler** pour poursuivre votre travail. Un astérisque (*) dans le titre de la fenêtre indique la présence de modifications non enregistrées.

## Modèles intégrés

Sous **Fichier > Nouveau**, sous le classeur vierge, se trouvent des classeurs prêts à l'emploi, regroupés par catégories : **Finances** (un budget mensuel, un tableau d'amortissement de prêt et un registre de chéquier), **Entreprise** (une facture, une note de frais, une feuille de temps, une liste d'inventaire, un journal kilométrique et un rapport de ventes), **Planification** (un calendrier et un plan de projet) et **Éducation** (un carnet de notes). Chacun affiche une petite image de son en-tête. Cliquez sur l'un d'eux pour démarrer un nouveau classeur à partir de celui-ci. Saisissez vos données dans les cases jaunes. Les autres nombres sont calculés automatiquement. Le classeur n'a pas de fichier qui lui est propre tant que vous ne l'avez pas enregistré.
