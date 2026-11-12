# Travailler avec les fichiers .pub

<!-- keywords: pub, .pub, fichier pub, ouvrir pub, enregistrer pub, convertir, compatibilité, importer, anciennes compositions, autres logiciels, jpub, format, polices manquantes, sauvegarde de l'original, format de fichier -->

Un fichier .pub est une composition créée avec d'autres logiciels de PAO. JeffPub ouvre les fichiers .pub créés depuis 1998. Il peut aussi enregistrer votre travail au format .pub. Le format propre à JeffPub est .jpub.

## Ouvrir un fichier .pub

1. Appuyez sur Ctrl+O, ou cliquez sur **Fichier > Ouvrir**.
2. Cliquez sur **Parcourir**, choisissez le fichier .pub, puis cliquez sur **Ouvrir**.

Vous pouvez aussi glisser le fichier sur la page. Le texte, les images, les formes et les tableaux sont importés, ainsi que les zones de texte liées. La plupart des pages ressemblent beaucoup à l'original, mais certains détails ne correspondent pas encore. Quelques zones de texte peuvent couper un mot un peu plus tôt ou plus tard qu'auparavant.

Si une police du fichier n'est pas installée sur votre ordinateur, JeffPub utilise à la place une police libre et semblable. Les polices incorporées dans le fichier .pub ne sont pas importées. Si JeffPub doit vous signaler quelque chose après l'ouverture, une note apparaît dans la barre d'état, en bas de la fenêtre.

Si le fichier ne s'ouvre pas, JeffPub indique qu'il est peut-être endommagé ou qu'il utilise des fonctions qu'il ne peut pas encore lire.

## Vérifier le résultat

L'ouverture est une conversion : contrôlez donc chaque page. Vérifiez les retours à la ligne, les polices et les images. Utilisez le [Vérificateur de conception](design-checker) pour repérer les textes qui ne tiennent pas dans leur cadre.

## Enregistrer au format .pub

- Cliquez sur **Fichier > Exporter**, puis sur **Enregistrer en tant que fichier .pub**.
- Ou cliquez sur **Fichier > Enregistrer sous**, puis choisissez **Fichiers de composition .pub (*.pub)** pour **Type**.

Lorsque JeffPub enregistre un fichier .pub, il peut conserver les pages, les zones de texte et les zones de texte liées, la mise en forme du texte, les formes, les traits, les images, les tableaux, les remplissages, les ombres, le texte artistique, les styles, les livrets, les cartes pliées, les enveloppes, les numéros de page et les liens. Si un élément n'a pas d'équivalent au format .pub, JeffPub vous indique combien d'objets il a omis. Ces objets restent dans votre composition et sont conservés lorsque vous l'enregistrez au format .jpub. Les commentaires non plus n'ont pas d'équivalent au format .pub : JeffPub vous signale qu'il les a omis, et ils ne sont conservés que dans les fichiers .jpub. La rubrique Commentaires de cette Aide donne plus de détails.

## Protéger l'original

Lorsque vous ouvrez un fichier .pub et appuyez sur Ctrl+S, JeffPub enregistre les modifications dans ce même fichier .pub. Pour conserver l'original en lieu sûr :

- Activez **Toujours créer une copie de sauvegarde** dans **Fichier > Options**, sous l'onglet **Enregistrer**. JeffPub conserve alors l'ancien fichier sous le nom « Sauvegarde de » suivi de son nom d'origine.
- Ou commencez par **Fichier > Enregistrer sous** et enregistrez un fichier .jpub. Un fichier .jpub conserve tout.

Gardez vos fichiers d'origine jusqu'à ce que vous soyez certain que les nouveaux sont corrects.

## Voir aussi

- [Ouvrir et enregistrer des compositions](open-save)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
- [Modifier les paramètres de JeffPub](options)
