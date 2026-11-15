# Sécurité : ce qu'un fichier peut atteindre

<!-- keywords: centre de gestion de la confidentialité, sécurité, macros, fichiers liés, liens, images liées, médias liés, internet, téléchargé, lecture seule, protégé, liens externes, programmes, sûr -->

Un fichier peut renvoyer vers d'autres éléments : fichiers, pages web, vidéos, voire programmes. La page **Sécurité** vous permet de décider ce que fait JeffPub à ce sujet. Cliquez sur **Fichier > Options**, puis sur l'onglet **Sécurité**. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

Ces choix s'appliquent à toutes les applications JeffOffice : si vous en modifiez un dans une application, il change dans toutes les autres. Une ligne concernant une fonction qu'une application ne possède pas indique **Ne s'applique pas** et est grisée.

## Macros

Les macros sont de petits programmes enregistrés dans un fichier. JeffPub les conserve dans le fichier, de sorte qu'elles sont toujours présentes après l'enregistrement, et il **ne les exécute jamais**. Aucun paramètre ne permet de les exécuter.

## Fichiers liés et liens de classeurs

Il s'agit ici des images **liées** à un fichier, au lieu d'être incluses dans la composition. Le choix détermine ce que fait JeffPub lorsqu'il ouvre une composition qui en contient.

- **Les mettre à jour** est le choix habituel. Lorsqu'une composition s'ouvre, JeffPub relit le fichier de chaque image liée si celui-ci se trouve dans le dossier de la composition, afin que l'image soit à jour. Il ne demande jamais confirmation.
- **Ne jamais les mettre à jour** laisse chaque image telle qu'elle a été enregistrée. Chacune affiche **Non mise à jour** jusqu'à ce que vous utilisiez **Mettre à jour le lien** sur elle. **Exporter au format PDF** et **Imprimer** vous indiquent alors dans la barre d'état que des aperçus ont été utilisés pour des images qui n'ont pas de copie complète dans la composition.

JeffPub ne lit que les fichiers situés dans le dossier de la composition (ou dans un dossier que vous avez déclaré sûr). Tout autre fichier lié attend que vous le demandiez.

## Vidéos et sons liés

Cela ne s'applique pas dans JeffPub : il ne contient aucune vidéo ni aucun son liés à lire. La ligne est grisée.

## Fichiers provenant d'Internet ou d'autres ordinateurs

Un fichier que vous avez téléchargé, ou qui se trouve sur un autre ordinateur de votre réseau, peut ne pas être sûr. Ce choix détermine la manière dont JeffPub l'ouvre.

- **Ouvrir d'abord en lecture seule** est le choix habituel. Le fichier s'ouvre pour que vous puissiez le lire, mais vous ne pouvez pas le modifier. Une barre sous le fichier l'indique. Cliquez sur **Activer la modification** dans cette barre pour modifier ce seul fichier (jusqu'à la fermeture du programme). Lorsque vous l'enregistrez ensuite à son emplacement, JeffPub retire la marque de téléchargement du fichier. Vous pouvez aussi utiliser **Enregistrer sous** pour créer une copie, ou passer à l'option suivante de ce paramètre.
- **Ouvrir normalement** ouvre tous les fichiers de la même manière.

Un fichier ouvert avec **Ouvrir une copie**, **Ouvrir et réparer** ou **Ouvrir en lecture seule** depuis l'écran **Fichier** s'ouvre de cette façon, quel que soit le réglage ci-dessus.

## Liens qui lancent un programme

Un lien qui lancerait un programme ou un script (un fichier dont l'extension est .exe, .bat, .sh, etc.) est **toujours refusé**. JeffPub vous en informe, et rien ne s'exécute. Un lien vers un fichier situé sur un autre ordinateur n'est jamais suivi sans que vous ayez été préalablement consulté. Aucun paramètre ne modifie ce comportement.

## Voir aussi

- [Modifier les paramètres de JeffPub](options)
- [Images](pictures)
