# Ouvrir et enregistrer des présentations
<!-- keywords: ouvrir, enregistrer, enregistrer sous, otp, libreoffice, exclure, fermer, nouveau, fichiers récents, épinglé, types de fichiers, pptx, potx, ppsx, ppt, odp, modèle, diaporama, renommer, Ctrl+S, sauvegarde, formats de fichiers, anciens fichiers, document OpenDocument, conservé, non affiché, polices, incorporer les polices, polices incorporées, police manquante, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir comme copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, réparer, ouvrir et réparer, endommagé -->

JeffDeck enregistre votre travail dans des fichiers .pptx et dans des fichiers .odp, et ouvre plusieurs types de fichiers de présentation.

## Créer une présentation

Appuyez sur Ctrl+N. Une nouvelle fenêtre s'ouvre avec une présentation vierge. Vous pouvez aussi cliquer sur **Fichier > Nouveau**, puis sur **Présentation vierge**. Si la fenêtre active contient une nouvelle présentation que personne n'a modifiée, la présentation vierge la remplace. Sinon, elle s'ouvre dans une fenêtre distincte.

## Enregistrer

Appuyez sur Ctrl+S, ou cliquez sur **Fichier > Enregistrer**. La première fois, JeffDeck demande un nom et un emplacement. Ensuite, Enregistrer remplace le fichier par votre dernière version. Un message en bas indique « Enregistré » ainsi que le nom du fichier.

Une étoile (*) après le nom dans la barre de titre signale des modifications non enregistrées. Enregistrez souvent. Si JeffDeck s'arrête brutalement, consultez [Récupérer un travail non enregistré](recover).

Si un message indique que certains objets « ne peuvent pas encore être enregistrés dans ce type de fichier », le fichier est tout de même enregistré, mais JeffDeck n'a pas pu écrire ces objets. Conservez aussi le fichier d'origine. Lorsque vous enregistrez un fichier .odp, le message énumère ce que ce type de fichier ne peut pas contenir. La section « Enregistrer au format .odp » ci-dessous précise ces éléments.

## Enregistrer sous un autre nom ou un autre type

Cliquez sur **Fichier > Enregistrer sous**, ou appuyez sur F12. Saisissez un nom, choisissez le type, puis cliquez sur **Enregistrer**. Les choix sont les suivants :

- **Présentation (*.pptx)** est le choix habituel.
- **Modèle (*.potx)** enregistre les diapositives comme modèle pour créer de nouvelles présentations.
- **Diaporama (*.ppsx)** enregistre une présentation destinée à être lue.
- **Présentation OpenDocument (*.odp)** enregistre les diapositives dans le format utilisé par LibreOffice et d'autres logiciels libres.
- **Modèle de présentation OpenDocument (*.otp)** est la forme modèle de ce format.

Si vous omettez l'extension, JeffDeck ajoute celle qui correspond au type choisi. Après un enregistrement, la barre de titre affiche le nouveau nom, et vous continuez à travailler dans le nouveau fichier.

Une présentation ouverte à partir d'un modèle est enregistrée comme une nouvelle présentation : **Enregistrer** demande un nom, de sorte que le modèle reste inchangé. Il en va de même pour une présentation ouverte à partir d'un fichier .ppt, .pps ou .pot plus ancien. Une présentation ouverte à partir d'un fichier .odp est de nouveau enregistrée au format .odp, dans le même fichier.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**, puis choisissez un nom. JeffDeck écrit la présentation dans ce fichier, et vous continuez à travailler dans celle que vous aviez ouverte. Son nom et ses modifications non enregistrées restent inchangés.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, dans l'onglet **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffDeck copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, l'application demande s'il faut enregistrer sans elle. La réponse par défaut est Non, sauf si vous répondez Oui, et le fichier reste inchangé.

## Enregistrer comme modèle

Un modèle est une présentation que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle**, puis saisissez un nom. Le modèle (un fichier **.potx**) est placé dans **Mes modèles**, et votre présentation n'est pas modifiée. Pour créer une présentation à partir de ce modèle, cliquez sur **Fichier > Nouveau**, puis sur le modèle sous **Mes modèles**. La nouvelle présentation n'a pas encore de fichier, et le modèle n'est donc pas modifié. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Ouvrir

Appuyez sur Ctrl+O pour choisir directement un fichier. Vous pouvez aussi cliquer sur **Fichier > Ouvrir**, puis sur **Parcourir**, ou sur un nom sous **Épinglé** ou **Présentations récentes**.

Le fichier s'ouvre dans cette fenêtre si elle contient une nouvelle présentation que personne n'a modifiée. Sinon, il s'ouvre dans une fenêtre distincte.

Faites un clic droit sur un nom de la liste pour **Épingler à la liste** (il reste en haut), **Détacher de la liste** ou **Supprimer de la liste**. Un nom en gris signifie que JeffDeck ne trouve pas ce fichier pour le moment.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des fichiers récents.

- **Ouvrir en lecture seule** ouvre la présentation pour la consulter uniquement. Vous pouvez la parcourir, passer d'une diapositive à l'autre et copier son contenu, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous cliquez sur **Enregistrer**, JeffDeck demande un nouveau nom, de sorte que le fichier ouvert reste inchangé.
- **Ouvrir comme copie** ouvre les diapositives comme une nouvelle présentation sans fichier propre. **Enregistrer** demande où la placer.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte, même si la fenêtre active est vide.
- **Ouvrir et réparer** sert pour un fichier .pptx endommagé, par exemple un fichier tronqué lors d'une copie. JeffDeck lit toutes les parties qu'il peut lire, conserve une partie interrompue jusqu'à l'endroit où elle s'arrête, et exclut une partie qu'il ne peut pas lire. Les diapositives réparées s'ouvrent comme une copie nommée « Réparée » suivi du nom, et un message indique ce qui a été exclu. Le fichier endommagé n'est pas modifié.

## Fichiers protégés par un mot de passe

Un fichier **.pptx** qui demande un mot de passe s'ouvre normalement : JeffDeck demande le mot de passe, et vous pouvez réessayer s'il est incorrect, ou cliquer sur **Annuler**. Le fichier reste protégé lorsque vous l'enregistrez. Pour modifier ou supprimer le mot de passe, utilisez **Fichier > Informations > Chiffrer avec un mot de passe** (voir L'écran Fichier). Un fichier **.odp** ne peut pas encore contenir de mot de passe.

## Polices dans le fichier

Une police est la forme des caractères. Tous les ordinateurs ne possèdent pas toutes les polices. Si un ordinateur ne possède pas une police utilisée par vos diapositives, il dessine les caractères avec une autre police. Les diapositives peuvent alors avoir un aspect différent.

JeffDeck peut inclure dans le fichier les polices utilisées par vos diapositives. Les diapositives s'affichent alors de la même façon sur n'importe quel ordinateur. C'est ce qu'on appelle l'incorporation. JeffDeck la réalise à chaque enregistrement. Pour la désactiver, cliquez sur **Fichier > Options**, ouvrez l'onglet **Enregistrer**, puis décochez **Incorporer les polices dans le fichier**.

- JeffDeck n'incorpore que les polices fournies avec lui, comme Fraunces, Lato et Open Sans, que les autres ordinateurs ne possèdent pas. Il n'incorpore jamais une police que tous les ordinateurs possèdent, comme Calibri ou Arial. Il laisse de côté une police dont la licence interdit la copie dans un fichier.
- JeffDeck incorpore chaque style utilisé par les diapositives : normal, gras, italique et gras italique. Si les diapositives n'utilisent que le style normal, seul celui-ci est incorporé.
- JeffDeck incorpore tous les caractères de chaque police, et pas seulement ceux que vous avez utilisés. Ainsi, une personne qui ouvre le fichier peut encore modifier les textes.
- Le fichier devient plus volumineux. Chaque style d'une police ajoute environ 40 à 700 Ko (un Ko correspond à mille octets). Le fichier est compressé à l'enregistrement, de sorte qu'il grossit d'environ la moitié de cette taille.

Lorsque vous ouvrez un fichier contenant des polices incorporées, JeffDeck les utilise pendant que la présentation est ouverte. Les diapositives s'affichent comme elles l'ont été à leur création. JeffDeck n'installe pas les polices sur votre ordinateur et ne les copie nulle part. Elles appartiennent à cette présentation uniquement : elles n'apparaissent pas dans vos listes de polices, et aucune autre présentation ne les utilise. Elles disparaissent à la fermeture de la présentation. Si votre ordinateur possède déjà une police, JeffDeck utilise votre propre copie.

JeffDeck lit les polices des fichiers .pptx créés par d'autres logiciels de présentation, même lorsqu'elles ont été compressées. Il lit aussi les polices des fichiers .odp.

Un fichier peut contenir une police que JeffDeck ne possède pas. À l'enregistrement, JeffDeck réintègre cette police dans le fichier, tant que les diapositives l'utilisent. Il le fait que l'option **Incorporer les polices dans le fichier** soit activée ou non. Si une police contenue dans un fichier est endommagée ou ne peut pas être lue, le message affiché à l'ouverture du fichier l'indique.

## Types de fichiers que JeffDeck lit

- **.pptx, .potx et .ppsx** sont un même format sous trois formes : présentation, modèle et diaporama. JeffDeck lit et écrit les trois.
- **.ppt, .pps et .pot** sont les anciens formats. JeffDeck les lit, mais ne peut pas les écrire : **Enregistrer** demande donc un nouveau nom et enregistre un .pptx (un .ppsx pour un .pps). Il n'écrase jamais le fichier d'origine.
- **.odp et .otp** sont les présentations au format OpenDocument et leurs modèles. JeffDeck lit et écrit les deux. Un .odp ouvert ici est enregistré en .odp. Un .otp est un modèle : **Enregistrer** demande donc un nouveau nom et propose un .odp.

Si une partie d'un fichier ne peut pas être lue, JeffDeck ouvre le reste et indique ce qu'il a laissé de côté. Si le fichier entier ne peut pas être lu, il en indique la raison.

## Enregistrer au format .odp

Un fichier .odp s'ouvre dans LibreOffice Impress avec vos diapositives, vos masques, vos images, vos notes et vos commentaires, ainsi qu'avec l'apparence affichée par JeffDeck. Il contient aussi les animations, les transitions entre diapositives, les diaporamas personnalisés et les paramètres du diaporama. La police de chaque mot est enregistrée par son nom, de sorte qu'un ordinateur ne possédant pas cette police affiche une autre police.

JeffDeck ne relit pas encore les animations, les commentaires, les diaporamas personnalisés ni les paramètres du diaporama à partir d'un fichier .odp. Ils sont bien présents dans le fichier, et LibreOffice les lit, mais lorsque vous ouvrez le fichier .odp dans JeffDeck, celui-ci les signale comme exclus. Conservez votre fichier .pptx si vous voulez continuer à les modifier ici.

Un fichier .odp ne prévoit pas de place pour certains éléments. JeffDeck les énumère après l'enregistrement, dans le message en bas de la fenêtre :

- **Les graphiques, les diagrammes et les équations** sont enregistrés comme des images. Vous pouvez les voir, mais vous ne pouvez pas modifier leurs valeurs ni leurs éléments dans le fichier .odp.
- **Les sections** ne sont pas enregistrées.
- **Les biseaux et les ombres intérieures** sont exclus, de même que les lueurs et les reflets sur le texte. Une ombre sur le texte est conservée. Un reflet sur une forme, un trait avec un dégradé, un trait composé de deux ou trois lignes et la taille d'une ombre sont conservés d'une manière que seul JeffDeck lit. Un autre logiciel affiche la forme sans reflet, le trait dans sa première couleur, un trait simple et l'ombre à la taille de l'objet.
- **Les effets d'animation** qui démarrent lorsque vous cliquez sur un objet, ainsi que les effets qui modifient une couleur, sont exclus. Un clic sur une vidéo, pour la mettre en pause ou la relancer, est conservé.
- **Un son ou une vidéo lié** à partir d'un autre fichier, et non intégré à la présentation, conserve son adresse et son image dans le fichier .odp. LibreOffice le lit s'il peut trouver le fichier. Les vidéos et les sons intégrés à la présentation sont conservés dans le fichier .odp et se lisent.

Conservez aussi votre fichier .pptx si vous avez besoin de ces éléments.

## Ce qui est conservé mais non affiché

Certaines parties d'un fichier sont des éléments que JeffDeck ne peut pas encore afficher ni modifier. Il les conserve avec leurs diapositives et les réécrit à l'enregistrement, de sorte qu'elles ne sont pas perdues. Il s'agit des animations, qui se jouent pendant le diaporama, des vidéos et des sons, qui ne se lisent pas encore, et d'un effet de transition pour lequel JeffDeck n'a pas d'équivalent, qui se joue comme un fondu. Les graphiques et les diagrammes s'affichent tels qu'ils ont été enregistrés. Voir [Bienvenue dans JeffDeck](start).

## Voir aussi

- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
- [Réutiliser des diapositives d'une autre présentation](reuse)
- [Impression, PDF et images des diapositives](printing)
