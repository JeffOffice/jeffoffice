# Travailler avec des fichiers .pub

<!-- keywords: pub, .pub, fichier pub, ouvrir pub, enregistrer pub, convertir, compatibilité, importer, anciennes compositions, autres programmes, jpub, format, polices manquantes, sauvegarder l'original, format de fichier -->

Un fichier .pub est une composition créée avec d'autres logiciels de PAO. JeffPub ouvre les fichiers .pub créés depuis 1998. Il peut aussi enregistrer votre travail au format .pub. Le format propre à JeffPub est .jpub.

## Ouvrir un fichier .pub

1. Appuyez sur Ctrl+O, ou cliquez sur **Fichier > Ouvrir**.
2. Cliquez sur **Parcourir**, choisissez le fichier .pub, puis cliquez sur **Ouvrir**.

Vous pouvez aussi faire glisser le fichier sur la page. Le texte, les images, les formes et les tableaux sont importés, ainsi que les zones de texte liées. La plupart des pages ressemblent beaucoup à l'original, mais tous les détails ne correspondent pas encore. Certaines zones de texte peuvent faire passer un ou deux mots à la ligne suivante, ou les laisser à la ligne précédente, par rapport à l'original.

Si une police du fichier n'est pas installée sur votre ordinateur, JeffPub utilise à la place une police libre et semblable. Les polices incorporées dans le fichier .pub ne sont pas importées. Si JeffPub a un avertissement à vous communiquer après l'ouverture, une note apparaît dans la barre d'état, en bas de la fenêtre.

Si le fichier ne s'ouvre pas, JeffPub indique qu'il est peut-être endommagé ou qu'il utilise des fonctionnalités qu'il ne sait pas encore lire. Il en va de même pour un fichier qui contient du texte ou des formes, mais aucun élément que JeffPub peut placer sur une page : il n'est pas ouvert sous forme de pages vierges, afin que vous ne puissiez pas l'écraser par erreur, et le fichier reste inchangé.

## Vérifier le résultat

Une ouverture est une conversion : vérifiez donc chaque page. Contrôlez les sauts de ligne, les polices et les images. Utilisez le [Vérificateur de conception](design-checker) pour repérer le texte qui ne tient pas dans son cadre.

## Enregistrer au format .pub

- Cliquez sur **Fichier > Exporter**, puis sur **Enregistrer en tant que fichier .pub**.
- Ou cliquez sur **Fichier > Enregistrer sous**, puis choisissez **Fichiers de composition .pub (*.pub)** dans **Type de fichier**.

Lorsque JeffPub enregistre un fichier .pub, il peut écrire les pages, les zones de texte et les zones de texte liées, la mise en forme du texte, les formes, les traits, les images, les tableaux, les remplissages, les ombres, le Texte artistique, les styles, les livrets, les cartes pliées, les enveloppes, les numéros de page et les liens. Si un élément n'a pas d'équivalent au format .pub, JeffPub vous indique combien d'objets il a omis. Ces objets restent dans votre composition et sont conservés lorsque vous enregistrez au format .jpub. Les commentaires n'ont pas non plus d'équivalent au format .pub : JeffPub signale qu'il les a omis, et ils ne sont conservés que dans les fichiers .jpub. La rubrique Commentaires de cette aide donne plus de détails.

## Protéger votre original

Lorsque vous ouvrez un fichier .pub et appuyez sur Ctrl+S, JeffPub écrit les modifications dans ce même fichier .pub. Pour conserver l'original en lieu sûr :

- Activez **Toujours créer une copie de sauvegarde** dans **Fichier > Options**, sous l'onglet **Enregistrement**. JeffPub conserve alors l'ancien fichier sous le nom « Sauvegarde de » suivi de son nom d'origine.
- Ou commencez par **Fichier > Enregistrer sous** et enregistrez un fichier .jpub. Un fichier .jpub conserve tout.

Gardez vos fichiers d'origine jusqu'à ce que vous soyez certain que les nouveaux sont corrects.

## Voir aussi

- [Ouvrir et enregistrer des compositions](open-save)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
- [Modifier les paramètres de JeffPub](options)
