# Sécurité : ce à quoi un fichier peut accéder

<!-- keywords: centre de gestion de la confidentialité, sécurité, macros, fichiers liés, liens, images liées, médias liés, internet, téléchargé, lecture seule, protégé, liens externes, programmes, sûr -->

Un fichier peut renvoyer vers d'autres éléments : fichiers, pages web, vidéos, voire programmes. La page **Sécurité** vous permet de décider ce que JeffPub fait dans ces cas-là. Cliquez sur **Fichier > Options**, puis sur l'onglet **Sécurité**. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

Ces choix valent pour toutes les applications JeffOffice : si vous en modifiez un dans une application, il change dans toutes les autres. Une ligne correspondant à une fonction absente de l'application indique **Non applicable** et est grisée.

## Macros

Les macros sont de petits programmes intégrés à un fichier. JeffPub **ne les exécute jamais**, et aucun paramètre ne permet de les exécuter. JeffPub ne conserve pas non plus les macros d'une composition : lorsque vous enregistrez un fichier `.pub` que vous avez ouvert, la copie n'en contient aucune. Conservez donc l'original si vous en avez encore besoin.

## Fichiers liés et liens vers des classeurs

Il s'agit des images **liées** à un fichier, et non incorporées dans la composition. Ce choix détermine ce que JeffPub fait lorsqu'il ouvre une composition qui en contient.

- **Les mettre à jour** est le choix habituel. Lorsqu'une composition s'ouvre, JeffPub relit le fichier de chaque image liée si ce fichier se trouve dans le dossier de la composition, afin que l'image soit à jour. Il ne demande jamais confirmation.
- **Ne jamais les mettre à jour** laisse chaque image telle qu'elle a été enregistrée. Chacune affiche **Non mise à jour** jusqu'à ce que vous utilisiez **Mettre à jour le lien** sur elle. **Exporter au format PDF** et **Imprimer** indiquent alors dans la barre d'état que des aperçus ont été utilisés pour les images qui n'ont pas de copie complète dans la composition.

JeffPub ne lit que les fichiers situés dans le dossier de la composition (ou dans un dossier que vous avez déclaré sûr). Tout autre fichier lié attend que vous le demandiez.

## Vidéos et sons liés

Cette option ne s'applique pas dans JeffPub : il n'a ni vidéo ni son liés à lire. La ligne est grisée.

## Fichiers provenant d'Internet ou d'autres ordinateurs

Un fichier que vous avez téléchargé, ou qui se trouve sur un autre ordinateur de votre réseau, peut ne pas être sûr. Ce choix détermine la manière dont JeffPub l'ouvre.

- **Ouvrir d'abord en lecture seule** est le choix habituel. Le fichier s'ouvre pour que vous puissiez le lire, mais vous ne pouvez pas le modifier. Une barre sous le fichier l'indique. Cliquez sur **Activer la modification** dans cette barre pour modifier ce seul fichier (jusqu'à la fermeture du programme). Lorsque vous l'enregistrez ensuite à son emplacement, JeffPub retire la marque de téléchargement du fichier. Vous pouvez aussi utiliser **Enregistrer sous** pour en faire une copie, ou changer ce paramètre pour le choix suivant.
- **Ouvrir normalement** ouvre tous les fichiers de la même manière.

Un fichier ouvert avec **Ouvrir une copie**, **Ouvrir et réparer** ou **Ouvrir en lecture seule** depuis l'écran **Fichier** s'ouvre de cette façon, quel que soit ce paramètre.

## Liens qui lancent un programme

Un lien qui lancerait un programme ou un script (un fichier dont l'extension est .exe, .bat, .sh, etc.) est **toujours refusé**. JeffPub vous en informe, et rien n'est exécuté. Un lien vers un fichier situé sur un autre ordinateur n'est jamais suivi sans que l'on vous demande d'abord votre accord. Aucun paramètre ne modifie ce comportement.

## Voir aussi

- [Modifier les paramètres de JeffPub](options)
- [Images](pictures)
