# Sécurité : ce vers quoi un fichier peut renvoyer
<!-- keywords: centre de gestion de la confidentialité, sécurité, macros, fichiers liés, liens, images liées, médias liés, internet, téléchargé, lecture seule, protégé, liens externes, programmes, sûr, confiance -->

Un fichier peut pointer vers d'autres éléments : des fichiers, des pages web, des vidéos, voire des programmes. La page **Sécurité** vous permet de décider ce que JeffCrunch fait dans ces cas-là. Cliquez sur **Fichier > Options**, puis sur l'onglet **Sécurité**. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

Ces choix concernent toutes les applications JeffOffice : si vous en modifiez un dans une application, il change dans toutes les autres. Une ligne correspondant à une fonction que l'application ne possède pas indique **Ne s'applique pas** et apparaît grisée.

## Macros

Les macros sont de petits programmes enregistrés dans un fichier. JeffCrunch les conserve dans le fichier, de sorte qu'elles restent présentes à l'enregistrement, et il **ne les exécute jamais**. Aucun paramètre ne permet de les exécuter.

## Fichiers liés et liaisons de classeurs

Il s'agit des autres classeurs dont un classeur tire des nombres, appelés **liaisons**. Le choix détermine ce que JeffCrunch fait lorsqu'il ouvre un classeur qui en contient.

- **Me demander d'abord** est le choix habituel. JeffCrunch vous demande si vous voulez mettre à jour les nombres provenant des autres classeurs.
- **Les mettre à jour** les met à jour sans demander confirmation.
- **Ne jamais les mettre à jour** conserve les nombres enregistrés dans le fichier.

Un choix effectué pour un classeur dans **Données > Modifier les liens** (**Invite de démarrage**) l'emporte pour ce classeur. Un classeur pour lequel l'option « Ne jamais les mettre à jour » est choisie n'est jamais modifié. Un classeur situé sur un autre ordinateur n'est jamais lu à l'ouverture d'un fichier, quel que soit votre choix. Vous pouvez toujours utiliser **Mettre à jour les valeurs** dans **Modifier les liens** à tout moment.

## Vidéos et sons liés

Cette option ne s'applique pas dans JeffCrunch : ce programme ne contient aucune vidéo ni aucun son liés à lire. La ligne est grisée.

## Fichiers provenant d'Internet ou d'autres ordinateurs

Un fichier que vous avez téléchargé, ou qui se trouve sur un autre ordinateur de votre réseau, peut ne pas être sûr. Ce choix détermine la manière dont JeffCrunch l'ouvre.

- **Ouvrir d'abord en lecture seule** est le choix habituel. Le fichier s'ouvre pour que vous puissiez le lire, mais vous ne pouvez pas le modifier. Une barre sous le fichier l'indique. Cliquez sur **Activer la modification** dans cette barre pour modifier ce seul fichier (jusqu'à la fermeture du programme). Lorsque vous l'enregistrez ensuite sur place, la marque de téléchargement est retirée du fichier. Vous pouvez aussi utiliser **Enregistrer sous** pour créer une copie, ou modifier ce paramètre pour choisir l'option suivante.
- **Ouvrir normalement** ouvre tous les fichiers de la même manière.

Un fichier ouvert avec **Ouvrir une copie**, **Ouvrir et réparer** ou **Ouvrir en lecture seule** depuis l'écran **Fichier** s'ouvre de cette façon, quel que soit ce paramètre.

## Liens qui lancent un programme

Un lien qui lancerait un programme ou un script (un fichier dont l'extension est .exe, .bat, .sh, etc.) est **toujours refusé**. JeffCrunch vous en informe, et rien n'est exécuté. Un lien vers un fichier situé sur un autre ordinateur n'est jamais suivi sans que vous soyez d'abord consulté. Aucun paramètre ne modifie ce comportement.

## Voir aussi

- [Modifier les paramètres de JeffCrunch](options)
- [Notes et liens](notes-links)
