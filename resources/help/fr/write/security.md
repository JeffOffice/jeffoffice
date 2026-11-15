# Sécurité : ce qu'un fichier peut atteindre
<!-- keywords: centre de gestion de la confidentialité, sécurité, macros, fichiers liés, liens, images liées, médias liés, internet, téléchargé, lecture seule, protégé, liens externes, programmes, sûr -->

Un fichier peut renvoyer vers d'autres éléments : fichiers, pages web, vidéos, voire programmes. La page **Sécurité** vous permet de décider ce que JeffWrite fait dans ces cas-là. Cliquez sur **Fichier > Options**, puis sur l'onglet **Sécurité**. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

Ces choix valent pour toutes les applications JeffOffice : si vous en modifiez un dans une application, il change dans toutes les autres. Une ligne correspondant à une fonction absente de l'application indique **Ne s'applique pas** et apparaît en grisé.

## Macros

Les macros sont de petits programmes intégrés à un fichier. JeffWrite les conserve dans le fichier, de sorte qu'elles sont toujours présentes après l'enregistrement, et il **ne les exécute jamais**. Aucun paramètre ne permet de les exécuter.

## Fichiers liés et liaisons de classeurs

Cela ne s'applique pas dans JeffWrite : il n'a pas de fichiers liés à mettre à jour. La ligne est grisée.

## Films et sons liés

Cela ne s'applique pas dans JeffWrite : il n'a pas de films ni de sons liés à lire. La ligne est grisée.

## Fichiers provenant d'Internet ou d'autres ordinateurs

Un fichier que vous avez téléchargé, ou qui se trouve sur un autre ordinateur de votre réseau, peut ne pas être sûr. Ce choix détermine la façon dont JeffWrite l'ouvre.

- **Ouvrir d'abord en lecture seule** est le choix habituel. Le fichier s'ouvre pour que vous puissiez le lire, mais vous ne pouvez pas le modifier. Une barre sous le fichier l'indique. Cliquez sur **Activer la modification** dans cette barre pour modifier ce seul fichier (jusqu'à la fermeture du programme). Lorsque vous l'enregistrez ensuite à le même emplacement, la marque de téléchargement est retirée du fichier. Vous pouvez aussi utiliser **Enregistrer sous** pour créer une copie, ou modifier ce paramètre et choisir l'option suivante.
- **Ouvrir normalement** ouvre tous les fichiers de la même manière.

Un fichier ouvert avec **Ouvrir une copie**, **Ouvrir et réparer** ou **Ouvrir en lecture seule** depuis l'écran **Fichier** s'ouvre de cette façon, quel que soit le réglage ci-dessus.

## Liens qui lancent un programme

Un lien qui lancerait un programme ou un script (un fichier dont l'extension est .exe, .bat, .sh, etc.) est **toujours refusé**. JeffWrite vous en informe, et rien ne s'exécute. Un lien vers un fichier situé sur un autre ordinateur n'est jamais suivi sans que vous soyez d'abord consulté. Aucun paramètre ne modifie ce comportement.

## Voir aussi

- [Modifier les paramètres de JeffWrite](options)
- [Liens](links)
