# Ouvrir et enregistrer des présentations
<!-- keywords: ouvrir, enregistrer, enregistrer sous, otp, libreoffice, omis, ignoré, fermer, nouveau, nouvelle présentation, fichiers récents, épinglé, types de fichiers, formats de fichiers, pptx, potx, ppsx, ppt, odp, modèle, diaporama, renommer, Ctrl+S, sauvegarde, copie de sauvegarde, anciens fichiers, OpenDocument, conservé, non affiché, polices, incorporer les polices, polices incorporées, police manquante -->

JeffDeck enregistre votre travail dans des fichiers .pptx et .odp, et ouvre plusieurs types de fichiers de présentation.

## Démarrer une nouvelle présentation

Appuyez sur Ctrl+N. Une nouvelle fenêtre s'ouvre avec une présentation vierge. Vous pouvez aussi cliquer sur **Fichier > Nouveau**, puis sur **Présentation vierge**. Si la fenêtre active contient une nouvelle présentation que personne n'a modifiée, la présentation vierge la remplace. Sinon, elle s'ouvre dans sa propre fenêtre.

## Enregistrer

Appuyez sur Ctrl+S ou cliquez sur **Fichier > Enregistrer**. La première fois, JeffDeck vous demande un nom et un emplacement. Ensuite, Enregistrer remplace le fichier par votre dernière version. Un message en bas de la fenêtre indique « Enregistré » et le nom du fichier.

Un astérisque (*) après le nom dans la barre de titre signale des modifications non enregistrées. Enregistrez souvent. Si JeffDeck s'arrête de manière inattendue, consultez [Récupérer le travail non enregistré](recover).

Si un message indique que certains objets « ne peuvent pas encore être enregistrés dans ce type de fichier », le fichier est enregistré, mais JeffDeck n'a pas pu écrire ces objets. Conservez aussi le fichier d'origine. Lorsque vous enregistrez un fichier .odp, le message liste ce que ce type de fichier ne peut pas contenir. La section « Enregistrer au format .odp » ci-dessous le détaille.

## Enregistrer sous un nouveau nom ou un autre type

Cliquez sur **Fichier > Enregistrer sous** ou appuyez sur F12. Tapez un nom, choisissez le type, puis cliquez sur **Enregistrer**. Les choix sont les suivants :

- **Présentation (*.pptx)** est le choix habituel.
- **Modèle (*.potx)** enregistre les diapositives comme base pour créer de nouvelles présentations.
- **Diaporama (*.ppsx)** enregistre une présentation destinée à être projetée.
- **Présentation OpenDocument (*.odp)** enregistre les diapositives dans le format utilisé par LibreOffice et d'autres logiciels libres.
- **Modèle de présentation OpenDocument (*.otp)** est la version modèle de ce format.

Si vous omettez l'extension, JeffDeck ajoute celle qui correspond au type choisi. Après l'enregistrement, la barre de titre affiche le nouveau nom et vous continuez à travailler dans le nouveau fichier.

Une présentation ouverte à partir d'un modèle est enregistrée comme nouvelle présentation : **Enregistrer** vous demande un nom, de sorte que le modèle reste intact. Il en va de même pour une présentation ouverte à partir d'un ancien fichier .ppt, .pps ou .pot. Une présentation ouverte à partir d'un fichier .odp est de nouveau enregistrée au format .odp, dans le même fichier.

## Ouvrir

Appuyez sur Ctrl+O pour choisir directement un fichier. Vous pouvez aussi cliquer sur **Fichier > Ouvrir**, puis sur **Parcourir** ou sur un nom sous **Épinglé** ou **Présentations récentes**.

Le fichier s'ouvre dans la fenêtre active si elle contient une nouvelle présentation que personne n'a modifiée. Sinon, il s'ouvre dans sa propre fenêtre.

Cliquez avec le bouton droit sur un nom de la liste pour choisir **Épingler à la liste** (il reste en haut), **Détacher de la liste** ou **Retirer de la liste**. Un nom grisé signifie que JeffDeck ne trouve plus ce fichier.

## Polices dans le fichier

Une police définit la forme des lettres. Tous les ordinateurs ne disposent pas de toutes les polices. Si une police utilisée par vos diapositives manque sur un ordinateur, celui-ci affiche les lettres dans une autre police, et les diapositives peuvent alors paraître différentes.

JeffDeck peut placer dans le fichier les polices utilisées par vos diapositives. Les diapositives s'affichent alors de la même façon sur n'importe quel ordinateur. C'est ce qu'on appelle l'incorporation. JeffDeck l'effectue à chaque enregistrement. Pour la désactiver, cliquez sur **Fichier > Options**, ouvrez l'onglet **Enregistrement** et désactivez **Incorporer les polices dans le fichier**.

- JeffDeck n'incorpore que les polices fournies avec lui, comme Fraunces, Lato et Open Sans, que les autres ordinateurs n'ont pas. Il n'incorpore jamais une police présente sur tous les ordinateurs, comme Calibri ou Arial. Il ignore une police dont la licence interdit la copie dans un fichier.
- JeffDeck incorpore chaque style utilisé par les diapositives : normal, gras, italique et gras italique. Si les diapositives n'utilisent que le style normal, seul celui-ci est incorporé.
- JeffDeck incorpore tous les caractères de chaque police, et pas seulement ceux que vous avez utilisés. Une personne qui ouvre le fichier peut ainsi modifier le texte.
- Le fichier devient plus volumineux. Chaque style d'une police ajoute environ 40 à 700 Ko (un Ko correspond à mille octets). Le fichier étant compressé lors de l'enregistrement, il grossit d'environ la moitié de cette valeur.

Lorsque vous ouvrez un fichier qui contient des polices, JeffDeck les utilise tant que la présentation est ouverte. Les diapositives s'affichent telles qu'elles ont été créées. JeffDeck n'installe pas ces polices sur votre ordinateur et ne les copie nulle part. Elles n'appartiennent qu'à cette présentation : elles n'apparaissent pas dans vos listes de polices et aucune autre présentation ne les utilise. Elles disparaissent à la fermeture de la présentation. Si votre ordinateur dispose déjà d'une police, JeffDeck utilise votre propre copie.

JeffDeck lit les polices des fichiers .pptx créés par d'autres logiciels de présentation, même lorsque ces logiciels les ont compressées. Il lit également les polices des fichiers .odp.

Un fichier peut contenir une police dont JeffDeck ne dispose pas. Lors de l'enregistrement, JeffDeck la replace dans le fichier, tant que les diapositives l'utilisent encore, que l'option **Incorporer les polices dans le fichier** soit activée ou non. Si une police du fichier est endommagée ou illisible, le message affiché à l'ouverture du fichier le signale.

## Types de fichiers lus par JeffDeck

- **.pptx, .potx et .ppsx** sont un même format sous trois formes : présentation, modèle et diaporama. JeffDeck les lit et les écrit tous les trois.
- **.ppt, .pps et .pot** sont l'ancien format. JeffDeck les lit mais ne peut pas les écrire : **Enregistrer** vous demande donc un nouveau nom et enregistre un fichier .pptx (un .ppsx pour un .pps). Il n'écrase jamais l'original.
- **.odp et .otp** sont les présentations OpenDocument et leurs modèles. JeffDeck lit et écrit les deux. Un fichier .odp ouvert dans JeffDeck est enregistré au format .odp. Un fichier .otp étant un modèle, **Enregistrer** vous demande un nouveau nom et propose le format .odp.

Si une partie d'un fichier est illisible, JeffDeck ouvre le reste et liste ce qu'il a ignoré. Si le fichier entier est illisible, il en indique la raison.

## Enregistrer au format .odp

Un fichier .odp s'ouvre dans LibreOffice Impress avec vos diapositives, masques, images, commentaires du présentateur et commentaires, et avec l'apparence affichée par JeffDeck. Il contient aussi les animations, les transitions, les diaporamas personnalisés et les paramètres du diaporama. La police de chaque mot est enregistrée par son nom : un ordinateur qui ne dispose pas de la police en affiche une autre.

JeffDeck ne relit pas encore les animations, commentaires, diaporamas personnalisés ni paramètres du diaporama d'un fichier .odp. Ils figurent dans le fichier et LibreOffice les exécute, mais lorsque vous ouvrez le fichier .odp dans JeffDeck, ceux-ci sont listés comme ignorés. Conservez votre fichier .pptx si vous souhaitez continuer à les modifier ici.

Un fichier .odp ne peut pas contenir certains éléments. JeffDeck les liste après l'enregistrement, dans le message en bas de la fenêtre :

- **Les graphiques, diagrammes SmartArt et équations** sont enregistrés en tant qu'images. Vous pouvez les voir, mais pas modifier leurs valeurs ni leurs éléments dans le fichier .odp.
- **Les sections** ne sont pas enregistrées.
- **Les biseaux et les ombres intérieures** sont omis, tout comme la lumière et la réflexion appliquées au texte. L'ombre du texte est conservée. La réflexion d'une forme, un trait dégradé, un trait composé de deux ou trois traits et la taille d'une ombre sont conservés sous une forme que seul JeffDeck lit. Un autre logiciel affiche la forme sans réflexion, le trait dans sa première couleur, un trait simple et l'ombre à la taille de l'objet.
- **Les effets d'animation** déclenchés par un clic sur un objet et les effets qui modifient une couleur sont omis. Un clic sur une vidéo pour la mettre en pause ou la relancer est conservé.
- **Une vidéo ou un son lié** à partir d'un autre fichier, et non stocké dans la présentation, conserve son adresse et son image dans le fichier .odp. LibreOffice le lit s'il trouve le fichier. Les vidéos et sons stockés dans la présentation sont conservés dans le fichier .odp et sont lus.

Conservez aussi votre fichier .pptx si vous avez besoin de récupérer ces éléments.

## Éléments conservés mais non affichés

Certaines parties d'un fichier ne peuvent pas encore être affichées ni modifiées par JeffDeck. Il les conserve avec leurs diapositives et les réécrit à l'enregistrement, afin qu'elles ne soient pas perdues. Il s'agit notamment des animations, qui sont lues pendant le diaporama, des vidéos et des sons, qui ne sont pas encore lus, et des effets de transition que JeffDeck ne prend pas en charge, qui sont lus sous forme de fondu. Les graphiques et diagrammes SmartArt s'affichent tels qu'ils ont été enregistrés. Consultez [Bienvenue dans JeffDeck](start).

## Voir aussi

- [Récupérer le travail non enregistré](recover)
- [L'écran Fichier](file-screen)
- [Réutiliser des diapositives d'une autre présentation](reuse)
- [Impression, PDF et images de diapositives](printing)
