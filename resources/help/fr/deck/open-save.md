# Ouvrir et enregistrer des présentations
<!-- keywords: macros, macro-enabled, pptm, potm, ppsm, ouvrir, enregistrer, enregistrer sous, otp, libreoffice, omis, fermer, nouveau, fichiers récents, épinglé, types de fichiers, pptx, potx, ppsx, ppt, odp, modèle, diaporama, renommer, Ctrl+S, sauvegarde, formats de fichiers, anciens fichiers, document OpenDocument, conservé, non affiché, polices, incorporer les polices, polices incorporées, police manquante, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir une copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, réparer, ouvrir et réparer, endommagé -->

JeffDeck enregistre votre travail dans des fichiers .pptx et des fichiers .odp, et ouvre plusieurs types de fichiers de présentation.

## Créer une présentation

Appuyez sur Ctrl+N. Une nouvelle fenêtre s'ouvre avec une présentation vierge. Vous pouvez aussi cliquer sur **Fichier > Nouveau**, puis sur **Présentation vierge**. Si la fenêtre dans laquelle vous vous trouvez contient une nouvelle présentation que personne n'a modifiée, la présentation vierge la remplace. Sinon, elle s'ouvre dans une fenêtre distincte.

## Enregistrer

Appuyez sur Ctrl+S, ou cliquez sur **Fichier > Enregistrer**. La première fois, JeffDeck vous demande un nom et un emplacement. Ensuite, Enregistrer remplace le fichier par votre dernière version. Un message en bas indique « Enregistré » et le nom du fichier.

Une étoile (*) après le nom dans la barre de titre signale des modifications non enregistrées. Enregistrez souvent. Si JeffDeck s'arrête brusquement, consultez [Récupérer un travail non enregistré](recover).

Si un message indique que certains objets « ne peuvent pas être enregistrés dans ce type de fichier pour le moment », le fichier est enregistré, mais JeffDeck n'a pas pu écrire ces objets. Conservez aussi le fichier d'origine. Lorsque vous enregistrez un fichier .odp, le message énumère ce que ce type de fichier ne peut pas contenir. La section « Enregistrer au format .odp » ci-dessous indique ce qu'il ne peut pas contenir.

## Enregistrer sous un nouveau nom ou un autre type

Cliquez sur **Fichier > Enregistrer sous**, ou appuyez sur F12. Saisissez un nom, choisissez le type, puis cliquez sur **Enregistrer**. Les choix sont les suivants :

- **Présentation (*.pptx)** est le choix habituel.
- **Modèle (*.potx)** enregistre les diapositives comme modèle pour créer de nouvelles présentations.
- **Diaporama (*.ppsx)** enregistre une présentation destinée à être lue.
- **Présentation avec macros (*.pptm)**, **Modèle avec macros (*.potm)** et **Diaporama avec macros (*.ppsm)** correspondent aux trois mêmes types, pour une présentation contenant des **macros**. Une macro est un petit programme livré avec un fichier. JeffDeck n'exécute jamais les macros et ne les lit ni ne les affiche jamais. Si vous ouvrez l'un de ces fichiers et l'enregistrez dans le même type, les macros sont conservées telles quelles. Si vous l'enregistrez en .pptx, .potx ou .ppsx, les macros sont omises, et JeffDeck vous le signale. Une présentation sans macros peut aussi être enregistrée dans ces types, mais elle n'a alors aucune macro à conserver. Si les macros d'un fichier sont trop volumineuses pour être conservées (plus de 32 Mo dans une partie, ou 64 Mo au total), JeffDeck les omet et vous en informe à l'ouverture.
- **Présentation OpenDocument (*.odp)** enregistre les diapositives dans le format de fichier qu'utilisent LibreOffice et d'autres logiciels libres.
- **Modèle de présentation OpenDocument (*.otp)** est la forme modèle de ce type.

Si vous omettez l'extension, JeffDeck ajoute celle qui correspond au type choisi. Après un enregistrement, la barre de titre affiche le nouveau nom, et vous continuez à travailler dans le nouveau fichier.

Une présentation ouverte à partir d'un modèle est enregistrée comme nouvelle présentation : **Enregistrer** demande un nom, afin que le modèle reste inchangé. Il en va de même pour une présentation ouverte à partir d'un ancien fichier .ppt, .pps ou .pot. Une présentation ouverte à partir d'un fichier .odp est de nouveau enregistrée en fichier .odp, par-dessus le même fichier.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**, puis choisissez un nom. JeffDeck écrit la présentation dans ce fichier, et vous continuez à travailler dans celle que vous aviez. Son nom et ses modifications non enregistrées restent inchangés.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, dans l'onglet **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffDeck copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, l'application demande s'il faut enregistrer sans elle. La réponse par défaut est Non, sauf si vous répondez Oui, et le fichier reste inchangé.

## Enregistrer comme modèle

Un modèle est une présentation que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle**, puis saisissez un nom. Le modèle (un fichier **.potx**) est placé dans **Mes modèles**, et votre présentation n'est pas modifiée. Pour créer une nouvelle présentation à partir de ce modèle, cliquez sur **Fichier > Nouveau**, puis sur le modèle sous **Mes modèles**. La nouvelle présentation n'a pas encore de fichier propre, donc le modèle reste inchangé. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Ouvrir

Appuyez sur Ctrl+O pour choisir immédiatement un fichier. Vous pouvez aussi cliquer sur **Fichier > Ouvrir**, puis sur **Parcourir**, ou sur un nom sous **Épinglé** ou **Présentations récentes**.

Le fichier s'ouvre dans cette fenêtre si elle contient une nouvelle présentation que personne n'a modifiée. Sinon, il s'ouvre dans une fenêtre distincte.

Faites un clic droit sur un nom de la liste pour **Épingler à la liste** (il reste en haut), **Détacher de la liste** ou **Supprimer de la liste**. Un nom affiché en gris signifie que JeffDeck ne trouve plus ce fichier.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des fichiers récents.

- **Ouvrir en lecture seule** ouvre la présentation uniquement pour la consulter. Vous pouvez regarder, parcourir les diapositives et copier, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous cliquez sur **Enregistrer**, JeffDeck demande un nouveau nom, afin que le fichier ouvert ne soit pas modifié.
- **Ouvrir une copie** ouvre les diapositives comme une nouvelle présentation sans fichier propre. **Enregistrer** demande où la placer.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte, même si la fenêtre active est vide.
- **Ouvrir et réparer** sert pour un fichier .pptx ou .ppt endommagé, par exemple un fichier tronqué lors d'une copie. JeffDeck lit toutes les parties qu'il peut lire, conserve une partie interrompue jusqu'à l'endroit où elle s'arrête, et omet une partie qu'il ne peut pas lire. Pour un ancien fichier .ppt, il retrouve les diapositives d'après leur contenu lorsque les listes indiquant leur emplacement sont endommagées ou perdues. Si la liste qui donne l'ordre des diapositives est perdue, les diapositives sont présentées dans l'ordre où elles ont été enregistrées, ce qui peut ne pas être celui d'origine. Un fichier protégé par un mot de passe ne peut pas être réparé. Les diapositives réparées s'ouvrent comme une copie nommée « Réparé » suivi du nom du fichier, et un message indique ce qui a été omis. Le fichier endommagé n'est pas modifié.

## Fichiers protégés par un mot de passe

Un fichier **.pptx** ou **.odp** qui demande un mot de passe s'ouvre normalement : JeffDeck demande le mot de passe, et vous pouvez réessayer si celui-ci est incorrect, ou cliquer sur **Annuler**. Le fichier reste protégé lorsque vous l'enregistrez. Pour modifier ou supprimer le mot de passe, utilisez **Fichier > Informations > Chiffrer avec un mot de passe** (voir L'écran Fichier). Une copie enregistrée en fichier .odp conserve aussi le mot de passe.

## Polices dans le fichier

Une police est la forme des caractères. Tous les ordinateurs n'ont pas toutes les polices. Si un ordinateur ne possède pas une police utilisée par vos diapositives, il affiche les caractères dans une autre police. Les diapositives peuvent alors apparaître différemment.

JeffDeck peut placer dans le fichier les polices utilisées par vos diapositives. Les diapositives s'affichent alors de la même façon sur tous les ordinateurs. C'est ce qu'on appelle l'incorporation. JeffDeck le fait à chaque enregistrement. Pour le désactiver, cliquez sur **Fichier > Options**, ouvrez l'onglet **Enregistrer**, puis décochez **Incorporer les polices dans le fichier**.

- JeffDeck n'incorpore que les polices fournies avec lui, comme Fraunces, Lato et Open Sans. Les autres ordinateurs ne les possèdent pas. Il n'incorpore jamais une police que tous les ordinateurs possèdent, comme Calibri ou Arial. Il omet une police dont la licence interdit la copie dans un fichier.
- JeffDeck incorpore chaque style utilisé par les diapositives : normal, gras, italique et gras italique. Si les diapositives n'utilisent que le style normal, seul celui-ci est incorporé.
- JeffDeck incorpore tous les caractères de chaque police, et pas seulement ceux que vous avez utilisés. Ainsi, une personne qui ouvre le fichier peut encore modifier les textes.
- Le fichier grossit. Chaque style d'une police ajoute environ 40 à 700 Ko (un Ko correspond à mille octets). Le fichier est compressé à l'enregistrement, donc il grossit d'environ la moitié de cette valeur.

Lorsque vous ouvrez un fichier qui contient des polices incorporées, JeffDeck les utilise pendant que la présentation est ouverte. Les diapositives s'affichent comme elles ont été créées. JeffDeck n'installe pas les polices sur votre ordinateur et ne les copie nulle part. Elles n'appartiennent qu'à cette présentation : elles n'apparaissent pas dans vos listes de polices, et aucune autre présentation ne les utilise. Elles disparaissent à la fermeture de la présentation. Si votre ordinateur possède déjà une police, JeffDeck utilise votre propre copie.

JeffDeck lit les polices des fichiers .pptx créés par d'autres logiciels de présentation, même lorsque ces polices sont compressées. Il lit aussi les polices des fichiers .odp.

Un fichier peut contenir une police que JeffDeck ne possède pas. À l'enregistrement, JeffDeck remet cette police dans le fichier, tant que les diapositives l'utilisent. Il le fait que l'option **Incorporer les polices dans le fichier** soit activée ou non. Si une police du fichier est endommagée ou ne peut pas être lue, le message affiché à l'ouverture du fichier l'indique.

## Types de fichiers que JeffDeck lit

- **.pptx, .potx et .ppsx** sont un même format sous trois formes : présentation, modèle et diaporama. JeffDeck lit et écrit les trois.
- **.ppt, .pps et .pot** sont l'ancien format. JeffDeck les lit, mais ne peut pas les écrire : **Enregistrer** demande donc un nouveau nom et enregistre un .pptx (un .ppsx pour un .pps). Il n'écrase jamais l'original.
- **.odp et .otp** sont les présentations au format OpenDocument et leurs modèles. JeffDeck lit et écrit les deux. Un .odp ouvert ici est enregistré en .odp. Un .otp est un modèle : **Enregistrer** demande donc un nouveau nom et propose un .odp.

Si une partie d'un fichier ne peut pas être lue, JeffDeck ouvre le reste et énumère ce qu'il a omis. Si le fichier entier ne peut pas être lu, il indique pourquoi.

## Enregistrer au format .odp

Un fichier .odp s'ouvre dans LibreOffice Impress avec vos diapositives, vos masques, vos images, vos notes et vos commentaires, ainsi que l'aspect affiché par JeffDeck. Il contient les animations, les transitions entre diapositives, les diaporamas personnalisés et les paramètres du diaporama. La police de chaque mot est enregistrée par son nom : un ordinateur qui ne possède pas cette police affiche donc une autre police.

JeffDeck ne lit pas encore les animations, les commentaires, les diaporamas personnalisés ni les paramètres du diaporama à partir d'un fichier .odp. Ils sont bien dans le fichier, et LibreOffice les lit, mais lorsque vous ouvrez le fichier .odp dans JeffDeck, celui-ci les signale comme omis. Conservez votre fichier .pptx si vous souhaitez continuer à les modifier ici.

Un fichier .odp ne peut pas contenir certains éléments. JeffDeck les énumère après l'enregistrement, dans le message en bas de la fenêtre :

- **Les graphiques, diagrammes et équations** sont enregistrés sous forme d'images. Vous pouvez les voir, mais vous ne pouvez pas modifier leurs valeurs ni leurs éléments dans le fichier .odp.
- **Les sections** ne sont pas enregistrées.
- **Les biseaux et les ombres intérieures** sont omis, de même que les lueurs et les reflets sur le texte. L'ombre sur le texte est conservée. Un reflet sur une forme, un trait à dégradé, un trait composé de deux ou trois lignes, et la taille d'une ombre sont conservés d'une manière que seul JeffDeck lit. Un autre logiciel affiche la forme sans reflet, le trait dans sa première couleur, un trait simple, et l'ombre à la taille de l'objet.
- **Les effets d'animation** qui démarrent au clic sur un objet, et les effets qui modifient une couleur, sont omis. Un clic sur une vidéo, pour la mettre en pause ou la relancer, est conservé.
- **Une vidéo ou un son lié** à partir d'un autre fichier, et non intégré à la présentation, conserve son adresse et son image dans le fichier .odp. LibreOffice le lit s'il peut trouver le fichier. Les vidéos et les sons intégrés à la présentation sont conservés dans le fichier .odp et se lisent.

Conservez aussi votre fichier .pptx si vous avez besoin de ces éléments.

## Ce qui est conservé mais non affiché

Certaines parties d'un fichier sont des éléments que JeffDeck ne peut pas encore afficher ni modifier. Il les conserve avec leurs diapositives et les réécrit à l'enregistrement, afin qu'elles ne soient pas perdues. Il s'agit notamment des animations, qui se jouent pendant le diaporama, des vidéos et des sons, qui ne se lisent pas encore, et d'un effet de transition pour lequel JeffDeck n'a pas de forme, lequel s'affiche comme un fondu. Les graphiques et les diagrammes s'affichent tels qu'ils ont été enregistrés. Voir [Bienvenue dans JeffDeck](start).

## Voir aussi

- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
- [Réutiliser des diapositives d'une autre présentation](reuse)
- [Impression, PDF et images des diapositives](printing)
