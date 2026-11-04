# Ouvrir, enregistrer, imprimer et exporter
<!-- keywords: ouvrir, enregistrer, enregistrer sous, docx, doc, dot, odt, opendocument, rtf, anciens fichiers, modèle, dotx, récent, épingler, nouveau, fermer, imprimer, pdf, exporter, récupérer, récupération automatique, non enregistré, plantage, écran fichier, formats de fichier, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir comme copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, ott, réparer, ouvrir et réparer, endommagé -->

Cliquez sur **Fichier** en haut à gauche pour afficher l'écran Fichier. Il comporte les pages Nouveau, Ouvrir, Imprimer et Exporter, ainsi que les commandes Enregistrer et Enregistrer sous. Appuyez sur **Échap** pour revenir à votre document.

## Créer un document

Appuyez sur **Ctrl+N**, ou cliquez sur **Fichier > Nouveau**, puis sur **Document vierge**. Un nouveau document s'ouvre dans sa propre fenêtre. Si la fenêtre dans laquelle vous vous trouvez est vide et n'a jamais été enregistrée, le nouveau document prend sa place.

## Ouvrir un document

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**. Cliquez sur **Parcourir** pour choisir un fichier. JeffWrite ouvre les types de fichiers suivants :

- **.docx** est un document.
- **.dotx** est un modèle. Un modèle s'ouvre comme un nouveau document créé à partir de celui-ci, si bien que le modèle reste inchangé.
- **.docm** et **.dotm** s'ouvrent également. JeffWrite ne conserve pas leurs macros. Il vous en avertit la première fois que vous enregistrez un tel document, et il l'enregistre au format .docx.
- **.doc** et **.dot** sont les anciens types de document et de modèle. JeffWrite les lit et enregistre vos modifications dans un nouveau fichier .docx.
- **.odt**, **.ott** et **.fodt** sont des fichiers OpenDocument. JeffWrite lit ces trois types. Il enregistre les fichiers .odt, et il enregistre vos modifications d'un fichier .ott ou .fodt dans un nouveau fichier .odt. Voir [Ouvrir et enregistrer des fichiers OpenDocument](open-odt).
- **.rtf** est le format Texte enrichi. JeffWrite le lit et enregistre vos modifications dans un nouveau fichier .docx. Voir [Ouvrir des fichiers RTF](open-rtf).

La page **Ouvrir** affiche aussi les documents que vous avez ouverts ou enregistrés récemment. Cliquez sur l'un d'eux pour l'ouvrir. Faites un clic droit sur un document pour l'épingler en haut de la liste, ou pour le retirer de la liste.

Si un fichier contient des éléments que JeffWrite ne peut pas encore modifier, comme la disposition d'un tableau ou un en-tête, il les conserve. Voir [Bienvenue dans JeffWrite](start). Si une partie du fichier ne peut pas être lue du tout, JeffWrite vous indique ce qu'il a omis. Il n'écrase alors pas ce fichier : Enregistrer demande un nouveau nom, afin que l'original conserve ce que la copie a perdu.

## Enregistrer un document

Appuyez sur **Ctrl+S** pour enregistrer. La première fois, JeffWrite demande un nom. **Enregistrer sous** (F12) enregistre une copie sous un nouveau nom. Choisissez **Modèle (*.dotx)** pour enregistrer un modèle. Un fichier provenant d'un ancien type de fichier est toujours enregistré sous un nouveau nom, afin que l'ancien fichier ne soit jamais écrasé.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**. Choisissez un nom. JeffWrite écrit le document dans ce fichier, et vous continuez à travailler dans le document que vous aviez ouvert. Son nom et ses modifications non enregistrées restent inchangés. C'est pratique lorsque vous voulez envoyer une version à quelqu'un tout en continuant à écrire.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, dans la page **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffWrite copie d'abord l'ancien fichier dans un fichier nommé « Backup of » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, JeffWrite vous demande s'il faut enregistrer sans elle. La réponse est Non, sauf si vous répondez Oui, et le fichier reste tel qu'il était. Vous pouvez alors revenir à la dernière version enregistrée.

## Enregistrer comme modèle

Un modèle est un document que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle**, puis saisissez un nom. Le modèle est placé dans **Mes modèles**, et le document en cours n'est pas modifié. Dans **Enregistrer sous**, vous pouvez aussi choisir **Modèle (*.dotx)** ou **Modèle de texte OpenDocument (*.ott)** pour enregistrer un fichier modèle à l'emplacement de votre choix.

Pour démarrer un nouveau document à partir de votre modèle, cliquez sur **Fichier > Nouveau**. Vos modèles sont listés sous **Mes modèles**. Cliquez sur l'un d'eux. Un nouveau document s'ouvre avec le texte et les styles du modèle. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des récents. Chaque méthode demande un fichier, ou utilise celui sur lequel vous avez cliqué :

- **Ouvrir en lecture seule** ouvre le fichier uniquement pour la lecture. Vous pouvez consulter, faire défiler, rechercher et copier, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous appuyez sur **Enregistrer**, JeffWrite demande un nouveau nom, si bien que le fichier ouvert n'est pas modifié.
- **Ouvrir comme copie** ouvre un nouveau document contenant le texte du fichier. Il n'a pas de fichier propre, si bien que **Enregistrer** demande où le placer. Le fichier ouvert n'est jamais modifié.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte, même lorsque la fenêtre dans laquelle vous vous trouvez est vide.
- **Ouvrir et réparer** sert pour un fichier .docx endommagé, par exemple un fichier tronqué lors d'une copie. Voir ci-dessous.

## Ouvrir et réparer

Il arrive qu'un fichier .docx soit endommagé : une copie interrompue à mi-parcours, ou un disque défaillant. Un fichier .docx est un ensemble de petites parties, comme le texte, les styles et les images. **Ouvrir et réparer** lit chaque partie qu'il peut lire. Une partie interrompue au milieu est conservée jusqu'à l'endroit où elle s'arrête. Une partie qui ne peut pas être lue est omise. Si la liste des parties ou les liens entre elles ont disparu, JeffWrite les recrée.

Le document réparé s'ouvre comme une copie portant un nom du type « Lettre réparée ». Un message vous indique ce qui a été conservé jusqu'à une interruption et ce qui a été omis. Le fichier endommagé n'est pas modifié. Enregistrez la copie réparée sous un nom qui lui est propre, puis vérifiez-la avant de vous y fier.

## Récupérer un travail non enregistré

JeffWrite enregistre une copie de secours du travail non enregistré toutes les quelques minutes, dans un fichier placé dans son propre dossier. Si le programme se ferme de façon inattendue, il vous propose ces copies au prochain démarrage. Cliquez sur **Fichier > Ouvrir > Récupérer le travail non enregistré** pour les consulter plus tard. Vous pouvez choisir celles à ouvrir, et enregistrer chacune à l'emplacement de votre choix.

## Imprimer

Appuyez sur **Ctrl+P**, ou cliquez sur **Fichier > Imprimer**. Choisissez une imprimante, le nombre de copies et les pages, vérifiez chaque page dans l'aperçu, puis cliquez sur **Imprimer**. Voir [Imprimer](print).

## Créer un PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Choisissez un nom. Le PDF s'affiche de la même façon sur tous les ordinateurs, car les polices sont incorporées au fichier.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
- [Utiliser JeffWrite au clavier](keyboard)
