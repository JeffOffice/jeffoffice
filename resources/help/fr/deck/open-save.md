# Ouvrir et enregistrer des présentations
<!-- keywords: ouvrir, enregistrer, enregistrer sous, otp, libreoffice, ne pas afficher, fermer, nouveau, fichiers récents, épinglé, types de fichiers, pptx, potx, ppsx, ppt, odp, modèle, diaporama, renommer, Ctrl+S, sauvegarde, formats de fichiers, anciens fichiers, document OpenDocument, conservé, non affiché, polices, incorporer les polices, polices incorporées, police manquante, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir une copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, réparer, ouvrir et réparer, endommagé -->

JeffDeck enregistre votre travail dans des fichiers .pptx et dans des fichiers .odp, et ouvre plusieurs types de fichiers de présentation.

## Créer une présentation

Appuyez sur Ctrl+N. Une nouvelle fenêtre s'ouvre avec une présentation vierge. Vous pouvez aussi cliquer sur **Fichier > Nouveau**, puis sur **Présentation vierge**. Si la fenêtre dans laquelle vous vous trouvez contient une nouvelle présentation que personne n'a modifiée, la présentation vierge la remplace. Sinon, elle s'ouvre dans une fenêtre distincte.

## Enregistrer

Appuyez sur Ctrl+S, ou cliquez sur **Fichier > Enregistrer**. La première fois, JeffDeck vous demande un nom et un emplacement. Ensuite, Enregistrer remplace le fichier par votre dernière version. Un message en bas indique « Enregistré » ainsi que le nom du fichier.

Une étoile (*) après le nom dans la barre de titre signifie que vous avez des modifications non enregistrées. Enregistrez souvent. Si JeffDeck s'arrête de façon inattendue, consultez [Récupérer un travail non enregistré](recover).

Si un message indique que certains objets « ne peuvent pas être enregistrés dans ce type de fichier pour le moment », le fichier est tout de même enregistré, mais JeffDeck n'a pas pu écrire ces objets. Conservez aussi le fichier d'origine. Lorsque vous enregistrez un fichier .odp, le message indique ce que ce type de fichier ne peut pas contenir. La section « Enregistrer au format .odp » ci-dessous détaille ces limites.

## Enregistrer sous un autre nom ou un autre type

Cliquez sur **Fichier > Enregistrer sous**, ou appuyez sur F12. Saisissez un nom, choisissez le type, puis cliquez sur **Enregistrer**. Les choix sont les suivants :

- **Présentation (*.pptx)** est le choix habituel.
- **Modèle (*.potx)** enregistre les diapositives comme un modèle pour créer de nouvelles présentations.
- **Diaporama (*.ppsx)** enregistre une présentation destinée à être lue en diaporama.
- **Présentation OpenDocument (*.odp)** enregistre les diapositives au format utilisé par LibreOffice et d'autres logiciels libres.
- **Modèle de présentation OpenDocument (*.otp)** est la version modèle de ce format.

Si vous omettez l'extension, JeffDeck ajoute celle qui correspond au type choisi. Après un enregistrement, la barre de titre affiche le nouveau nom, et vous continuez à travailler dans le nouveau fichier.

Une présentation ouverte à partir d'un modèle est enregistrée comme une nouvelle présentation : **Enregistrer** demande un nom, de sorte que le modèle reste inchangé. Il en va de même pour une présentation ouverte à partir d'un ancien fichier .ppt, .pps ou .pot. Une présentation ouverte à partir d'un fichier .odp est de nouveau enregistrée en tant que fichier .odp, à la place du même fichier.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie** et choisissez un nom. JeffDeck écrit la présentation dans ce fichier, et vous continuez à travailler dans celle que vous aviez ouverte. Son nom et ses modifications non enregistrées restent inchangés.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, dans l'onglet **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffDeck copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, l'application demande si vous souhaitez enregistrer sans elle. La réponse par défaut est Non, sauf si vous répondez Oui, et le fichier reste inchangé.

## Enregistrer comme modèle

Un modèle est une présentation que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle** et saisissez un nom. Le modèle (un fichier **.potx**) est placé dans **Mes modèles**, et votre présentation n'est pas modifiée. Pour créer une nouvelle présentation à partir de ce modèle, cliquez sur **Fichier > Nouveau**, puis sur le modèle sous **Mes modèles**. La nouvelle présentation n'a pas encore de fichier propre, si bien que le modèle n'est pas modifié. Pour retirer un modèle de la liste, cliquez dessus avec le bouton droit et choisissez **Supprimer de Mes modèles**.

## Ouvrir

Appuyez sur Ctrl+O pour choisir immédiatement un fichier. Vous pouvez aussi cliquer sur **Fichier > Ouvrir**, puis sur **Parcourir**, ou sur un nom sous **Épinglés** ou **Présentations récentes**.

Le fichier s'ouvre dans cette fenêtre si elle contient une nouvelle présentation que personne n'a modifiée. Sinon, il s'ouvre dans une fenêtre distincte.

Cliquez avec le bouton droit sur un nom de la liste pour **Épingler à la liste** (il reste en haut), **Désépingler de la liste** ou **Supprimer de la liste**. Un nom en gris signifie que JeffDeck ne trouve plus ce fichier.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi cliquer avec le bouton droit sur un nom de la liste des récents.

- **Ouvrir en lecture seule** ouvre la présentation uniquement pour la consulter. Vous pouvez regarder, parcourir les diapositives et copier, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous cliquez sur **Enregistrer**, JeffDeck demande un nouveau nom, de sorte que le fichier ouvert reste inchangé.
- **Ouvrir une copie** ouvre les diapositives dans une nouvelle présentation sans fichier propre. **Enregistrer** demande où la placer.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte, même si la fenêtre dans laquelle vous vous trouvez est vide.
- **Ouvrir et réparer** sert pour un fichier .pptx endommagé, par exemple un fichier tronqué lors d'une copie. JeffDeck lit toutes les parties qu'il peut lire, conserve une partie interrompue jusqu'à l'endroit où elle s'arrête, et omet une partie qu'il ne peut pas lire. Les diapositives réparées s'ouvrent dans une copie nommée « Réparée » suivi du nom du fichier, et un message indique ce qui a été omis. Le fichier endommagé n'est pas modifié.

## Polices dans le fichier

Une police est la forme des caractères. Tous les ordinateurs ne disposent pas de toutes les polices. Si un ordinateur ne possède pas une police utilisée dans vos diapositives, il affiche les caractères dans une autre police. Les diapositives peuvent alors avoir un aspect différent.

JeffDeck peut intégrer dans le fichier les polices utilisées par vos diapositives. Ainsi, les diapositives s'affichent de la même façon sur n'importe quel ordinateur. C'est ce qu'on appelle l'incorporation. JeffDeck l'effectue à chaque enregistrement. Pour la désactiver, cliquez sur **Fichier > Options**, ouvrez l'onglet **Enregistrer**, puis décochez **Incorporer les polices dans le fichier**.

- JeffDeck n'incorpore que les polices fournies avec le logiciel, comme Fraunces, Lato et Open Sans. Les autres ordinateurs ne les possèdent pas. Il n'incorpore jamais une police que tous les ordinateurs ont, comme Calibri ou Arial. Il omet une police dont la licence interdit la copie dans un fichier.
- JeffDeck incorpore chaque style utilisé par les diapositives : normal, gras, italique et gras italique. Si les diapositives n'utilisent que le style normal, seul celui-ci est incorporé.
- JeffDeck incorpore tous les caractères de chaque police, et pas seulement ceux que vous avez utilisés. Ainsi, une personne qui ouvre le fichier peut encore modifier le texte.
- Le fichier devient plus volumineux. Chaque style d'une police ajoute environ 40 à 700 Ko (un Ko correspond à mille octets). Le fichier est compressé lors de l'enregistrement, si bien qu'il grossit d'environ la moitié de cette valeur.

Lorsque vous ouvrez un fichier contenant des polices incorporées, JeffDeck les utilise tant que la présentation est ouverte. Les diapositives s'affichent comme elles l'étaient à l'endroit où elles ont été créées. JeffDeck n'installe pas les polices sur votre ordinateur et ne les copie nulle part. Elles appartiennent uniquement à cette présentation : elles n'apparaissent pas dans vos listes de polices, et aucune autre présentation ne les utilise. Elles disparaissent à la fermeture de la présentation. Si votre ordinateur possède déjà une police, JeffDeck utilise votre propre copie.

JeffDeck lit les polices des fichiers .pptx créés par d'autres logiciels de présentation, même lorsque ces logiciels les ont compressées. Il lit aussi les polices des fichiers .odp.

Un fichier peut contenir une police que JeffDeck ne possède pas. Lors de l'enregistrement, JeffDeck réintègre cette police dans le fichier, tant que les diapositives l'utilisent. Il le fait que l'option **Incorporer les polices dans le fichier** soit activée ou non. Si une police du fichier est endommagée ou illisible, le message qui s'affiche à l'ouverture du fichier l'indique.

## Types de fichiers lus par JeffDeck

- **.pptx, .potx et .ppsx** sont un même format sous trois formes : présentation, modèle et diaporama. JeffDeck lit et écrit les trois.
- **.ppt, .pps et .pot** sont les anciens formats. JeffDeck les lit, mais ne peut pas les écrire : **Enregistrer** demande donc un nouveau nom et enregistre un fichier .pptx (un .ppsx pour un .pps). Il n'écrase jamais l'original.
- **.odp et .otp** sont les présentations au format OpenDocument et leurs modèles. JeffDeck lit et écrit les deux. Un fichier .odp ouvert ici est enregistré en .odp. Un .otp est un modèle : **Enregistrer** demande donc un nouveau nom et propose un .odp.

Si une partie d'un fichier ne peut pas être lue, JeffDeck ouvre le reste et indique ce qu'il a omis. Si le fichier entier ne peut pas être lu, il indique la raison.

## Enregistrer au format .odp

Un fichier .odp s'ouvre dans LibreOffice Impress avec vos diapositives, vos masques, vos images, vos notes et vos commentaires, ainsi que l'apparence affichée par JeffDeck. Il conserve les animations, les transitions entre diapositives, les diaporamas personnalisés et les paramètres du diaporama. La police de chaque mot est enregistrée par son nom : un ordinateur qui ne possède pas cette police affiche donc une autre police.

JeffDeck ne lit pas encore, à partir d'un fichier .odp, les animations, les commentaires, les diaporamas personnalisés ni les paramètres du diaporama. Ils sont bien présents dans le fichier, et LibreOffice les exécute, mais lorsque vous ouvrez le fichier .odp dans JeffDeck, celui-ci les signale comme omis. Conservez votre fichier .pptx si vous voulez continuer à les modifier ici.

Un fichier .odp ne peut pas contenir certains éléments. JeffDeck les énumère après l'enregistrement, dans le message en bas de la fenêtre :

- **Les graphiques, les diagrammes SmartArt et les équations** sont enregistrés sous forme d'images. Vous pouvez les voir, mais vous ne pouvez pas modifier leurs valeurs ni leurs éléments dans le fichier .odp.
- **Les sections** ne sont pas enregistrées.
- **Les biseaux et les ombres intérieures** sont omis, de même que les lueurs et les reflets sur le texte. L'ombre portée sur le texte est conservée. Un reflet sur une forme, un trait avec un dégradé, un trait formé de deux ou trois lignes et la taille d'une ombre sont conservés d'une manière que seul JeffDeck interprète. Un autre logiciel affiche la forme sans reflet, le trait dans sa première couleur, un trait simple et l'ombre à la taille de l'objet.
- **Les effets d'animation** qui démarrent lorsqu'on clique sur un objet, ainsi que les effets qui modifient une couleur, sont omis. Un clic sur une vidéo, pour la mettre en pause ou la relancer, est conservé.
- **Un fichier multimédia lié** à partir d'un autre fichier, et non intégré à la présentation, conserve son adresse et son image dans le fichier .odp. LibreOffice le lit s'il peut retrouver le fichier. Les vidéos et les sons intégrés à la présentation sont conservés dans le fichier .odp et sont lus.

Conservez aussi votre fichier .pptx si vous avez besoin de ces éléments.

## Ce qui est conservé sans être affiché

Certaines parties d'un fichier sont des éléments que JeffDeck ne peut pas encore afficher ni modifier. Il les conserve avec leurs diapositives et les réécrit lors de l'enregistrement, de sorte qu'elles ne sont pas perdues. Il s'agit notamment des animations, qui sont lues pendant le diaporama, des vidéos et des sons, qui ne sont pas encore lus, et d'un effet de transition pour lequel JeffDeck n'a pas de forme, qui est lu comme un fondu. Les graphiques et les diagrammes SmartArt s'affichent tels qu'ils ont été enregistrés. Voir [Bienvenue dans JeffDeck](start).

## Voir aussi

- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
- [Réutiliser des diapositives d'une autre présentation](reuse)
- [Impression, PDF et images des diapositives](printing)
