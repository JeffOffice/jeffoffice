# Sécurité : ce qu'un fichier peut atteindre

<!-- keywords: centre de gestion de la confidentialité, sécurité, macros, fichiers liés, liens, images liées, médias liés, internet, téléchargé, lecture seule, protégé, liens externes, programmes, sécurisé, approuvé, confiance -->

Un fichier peut renvoyer vers d'autres éléments : fichiers, pages web, vidéos, voire programmes. La page **Sécurité** permet de décider de la manière dont JeffDeck réagit. Cliquez sur **Fichier > Options**, puis sur l'onglet **Sécurité**. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

Ces choix concernent toutes les applications JeffOffice : si vous en modifiez un dans une application, il change dans toutes les autres. Une ligne portant sur une fonction qu'une application ne possède pas indique **Non applicable** et apparaît grisée.

## Macros

Les macros sont de petits programmes enregistrés dans un fichier. JeffDeck les conserve dans le fichier, de sorte qu'elles restent présentes à l'enregistrement, et il **ne les exécute jamais**. Aucun paramètre ne permet de les exécuter.

## Fichiers liés et liaisons de classeurs

Cela ne s'applique pas dans JeffDeck : ce logiciel ne dispose d'aucun fichier lié à mettre à jour. La ligne est grisée.

## Vidéos et sons liés

Il s'agit d'une vidéo ou d'un son qu'une diapositive **lie** au lieu de le contenir dans la présentation.

- **Ne jamais le lire** est le choix habituel. Une vidéo ou un son lié indique qu'il est lié et ne se lance pas.
- **Me demander d'abord** demande une fois, pour chaque fichier lié, la première fois que vous le lisez.
- **Le lire** le lit sans demander.

Même avec **Le lire**, seul un fichier vidéo ou audio dont le nom figure sur cet ordinateur est lu. Une adresse web, un fichier situé sur un autre ordinateur, un programme, un lien ou un fichier spécial, ainsi qu'un fichier qui n'est pas réellement une vidéo ou un son, ne sont jamais lus. Un fichier lié peut atteindre 256 Mo. Le fichier est lu en arrière-plan pendant 20 secondes au maximum, de sorte qu'un disque lent ne peut pas bloquer JeffDeck. Un lecteur réseau qui ressemble à un lecteur local (lecteur réseau mappé, ou dossier situé sous /Volumes ou /mnt) est lu comme un fichier local, ce qui peut être lent. Avec **Me demander d'abord**, un diaporama demande une seule fois, au démarrage, l'autorisation pour tous les fichiers liés qu'il contient.

## Fichiers provenant d'Internet ou d'autres ordinateurs

Un fichier téléchargé, ou situé sur un autre ordinateur de votre réseau, peut ne pas être sûr. Ce choix détermine la manière dont JeffDeck l'ouvre.

- **Ouvrir d'abord en lecture seule** est le choix habituel. Le fichier s'ouvre pour que vous puissiez le lire, mais vous ne pouvez pas le modifier. Une barre sous le fichier l'indique. Cliquez sur **Activer la modification** dans cette barre pour modifier ce seul fichier (jusqu'à la fermeture du programme). Lorsque vous l'enregistrez ensuite sur place, la marque de téléchargement est retirée du fichier. Vous pouvez aussi utiliser **Enregistrer sous** pour créer une copie, ou modifier ce paramètre pour choisir l'option suivante.
- **Ouvrir normalement** ouvre tous les fichiers de la même manière.

Un fichier ouvert avec **Ouvrir une copie**, **Ouvrir et réparer** ou **Ouvrir en lecture seule** depuis l'écran **Fichier** s'ouvre de cette façon, quel que soit ce paramètre.

## Liens qui lancent un programme

Un lien qui lancerait un programme ou un script (un fichier se terminant par .exe, .bat, .sh, etc.) est **toujours refusé**. JeffDeck vous en informe, et rien ne s'exécute. Un lien vers un fichier situé sur un autre ordinateur n'est jamais suivi sans que l'on vous demande d'abord votre accord. Aucun paramètre ne modifie ce comportement.

## Voir aussi

- [Modifier les paramètres de JeffDeck](options)
- [Lancer le diaporama](show)
