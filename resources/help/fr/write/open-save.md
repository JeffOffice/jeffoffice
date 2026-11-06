# Ouvrir, enregistrer, imprimer et exporter
<!-- keywords: page web, html, enregistrer comme page web, page web filtrée, courriel, e-mail, eml, signets, signets pdf, maj+f5, dernière modification, où vous en étiez, modèles intégrés, galerie, lettre, rapport, note de synthèse, manuscrit, bulletin, ouvrir, enregistrer, enregistrer sous, docx, doc, dot, odt, opendocument, rtf, anciens fichiers, modèle, dotx, récent, épingler, nouveau, fermer, imprimer, pdf, exporter, récupérer, récupération automatique, non enregistré, plantage, écran fichier, formats de fichier, enregistrer une copie, lecture seule, lire seulement, ouvrir une copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, ott, réparer, ouvrir et réparer, endommagé -->

Cliquez sur **Fichier**, en haut à gauche, pour afficher l'écran Fichier. Il comporte les pages Nouveau, Ouvrir, Imprimer et Exporter, ainsi que les commandes Enregistrer et Enregistrer sous. Appuyez sur **Échap** pour revenir à votre document.

## Créer un document

Appuyez sur **Ctrl+N**, ou cliquez sur **Fichier > Nouveau**, puis sur **Document vierge**. Un nouveau document s'ouvre dans sa propre fenêtre. Si la fenêtre dans laquelle vous vous trouvez est vide et n'a jamais été enregistrée, le nouveau document la remplace.

## Ouvrir un document

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**. Cliquez sur **Parcourir** pour choisir un fichier. JeffWrite ouvre les types de fichiers suivants :

- **.docx** est un document.
- **.dotx** est un modèle. Un modèle s'ouvre comme un nouveau document créé à partir de celui-ci, de sorte que le modèle reste inchangé.
- **.docm** et **.dotm** sont un document et un modèle pouvant contenir des **macros**, de petits programmes qui accompagnent un fichier. JeffWrite ne les exécute jamais. Il les conserve telles quelles lorsque vous enregistrez en .docm ou .dotm. Si vous enregistrez en .docx ou .odt, les macros sont supprimées, et JeffWrite vous en informe.
- **.txt** est du texte brut, uniquement les mots. À l'ouverture, JeffWrite le lit immédiatement s'il est en ASCII ou en Unicode simple. Sinon, il vous demande le **codage** à utiliser et affiche les premières lignes pour vérifier que les lettres s'affichent correctement. Le codage est la manière dont un fichier stocke les lettres, par exemple UTF-8 ou Windows 1252. Lors de l'enregistrement en .txt, vous choisissez le codage et les fins de ligne ; le fichier ne conserve que les mots : ni polices, ni images, ni traits de tableau.
- **.html** et **.htm** sont des pages web. JeffWrite lit les mots, les titres, le gras, l'italique, le soulignement, les couleurs, les listes et les tableaux. Les puces et les numéros des listes sont importés sous forme de marques saisies, et les images et liens sont omis. JeffWrite ne récupère jamais rien sur Internet et ignore les scripts. Une page web n'est jamais remplacée : **Enregistrer** demande un nouveau nom et enregistre un fichier .docx. Pour créer une page web, voir **Enregistrer comme page web** ci-dessous.
- **.doc** et **.dot** sont les anciens types de document et de modèle. JeffWrite les lit et enregistre vos modifications dans un nouveau fichier .docx.
- **.odt**, **.ott** et **.fodt** sont des fichiers OpenDocument. JeffWrite lit les trois. Il enregistre les fichiers .odt, et enregistre vos modifications d'un fichier .ott ou .fodt dans un nouveau fichier .odt. Voir [Ouvrir et enregistrer des fichiers OpenDocument](open-odt).
- **.rtf** est le format Rich Text Format. JeffWrite le lit et l'enregistre. Voir [Ouvrir des fichiers RTF](open-rtf).

La page **Ouvrir** affiche aussi les documents que vous avez ouverts ou enregistrés récemment. Cliquez sur l'un d'eux pour l'ouvrir. Faites un clic droit sur un document pour l'épingler en haut de la liste ou le retirer de la liste.

Si un fichier contient des éléments que JeffWrite ne peut pas encore modifier, comme la mise en page d'un tableau ou un en-tête, il les conserve. Voir [Bienvenue dans JeffWrite](start). Si une partie du fichier ne peut pas être lue du tout, JeffWrite vous indique ce qu'il a laissé de côté. Il n'enregistre alors pas par-dessus ce fichier : **Enregistrer** demande un nouveau nom, afin que l'original conserve ce que la copie a perdu.

## Enregistrer un document

Appuyez sur **Ctrl+S** pour enregistrer. La première fois, JeffWrite demande un nom. **Enregistrer sous** (F12) enregistre une copie sous un nouveau nom. Choisissez **Modèle (*.dotx)** pour enregistrer un modèle, **Document avec macros (*.docm)** pour conserver les macros, **Texte brut (*.txt)** pour ne garder que les mots, ou **Texte enrichi (*.rtf)** pour un fichier lisible par d'autres logiciels. Un fichier issu d'un ancien type de fichier est toujours enregistré sous un nouveau nom, de sorte que l'ancien fichier n'est jamais remplacé.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**. Choisissez un nom. JeffWrite écrit le document dans ce fichier, et vous continuez à travailler dans le document d'origine. Son nom et ses modifications non enregistrées restent inchangés. Cette fonction est pratique pour envoyer une version à quelqu'un tout en poursuivant la rédaction.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, sur la page **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffWrite copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, JeffWrite demande s'il faut enregistrer sans elle. La réponse est Non, sauf si vous répondez Oui, et le fichier reste tel qu'il était. Vous pouvez ainsi revenir à la dernière version enregistrée.

## Enregistrer comme modèle

Un modèle est un document que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle**, puis saisissez un nom. Le modèle est placé dans **Mes modèles**, et le document en cours n'est pas modifié. Dans **Enregistrer sous**, vous pouvez aussi choisir **Modèle (*.dotx)** ou **Modèle de texte OpenDocument (*.ott)** pour enregistrer un fichier modèle à l'emplacement de votre choix.

Pour créer un nouveau document à partir de votre modèle, cliquez sur **Fichier > Nouveau**. Vos modèles figurent sous **Mes modèles**. Cliquez sur l'un d'eux. Un nouveau document s'ouvre avec le texte et les styles du modèle. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom dans la liste des récents. Chaque méthode demande un fichier ou utilise celui sur lequel vous avez cliqué :

- **Ouvrir en lecture seule** ouvre le fichier uniquement pour la lecture. Vous pouvez consulter, faire défiler, rechercher et copier, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous appuyez sur **Enregistrer**, JeffWrite demande un nouveau nom, de sorte que le fichier ouvert n'est pas modifié.
- **Ouvrir une copie** ouvre un nouveau document contenant le texte du fichier. Il n'a pas de fichier propre, donc **Enregistrer** demande où le placer. Le fichier ouvert n'est jamais modifié.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte, même si la fenêtre active est vide.
- **Ouvrir et réparer** sert pour un fichier .docx endommagé, par exemple un fichier tronqué lors d'une copie. Voir ci-dessous.

## Ouvrir et réparer

Il arrive qu'un fichier .docx soit endommagé : une copie interrompue au milieu, ou un disque défectueux. Un fichier .docx est un ensemble de petites parties, comme le texte, les styles et les images. **Ouvrir et réparer** lit chaque partie qui peut l'être. Une partie interrompue en cours de route est conservée jusqu'à l'endroit où elle s'arrête. Une partie illisible est laissée de côté. Si la liste des parties ou les liens entre elles ont disparu, JeffWrite les recrée.

Le document réparé s'ouvre comme une copie portant un nom du type « Lettre réparée ». Un message indique ce qui a été conservé jusqu'à une coupure et ce qui a été laissé de côté. Le fichier endommagé n'est pas modifié. Enregistrez la copie réparée sous un nom qui lui est propre, puis vérifiez-la avant de vous y fier.

## Récupérer un travail non enregistré

JeffWrite enregistre une copie de secours du travail non enregistré toutes les quelques minutes, dans un fichier placé dans son propre dossier. Si le programme se ferme de façon inattendue, il vous propose ces copies au prochain démarrage. Cliquez sur **Fichier > Ouvrir > Récupérer le travail non enregistré** pour les consulter plus tard. Vous pouvez choisir celles à ouvrir et enregistrer chacune où vous le souhaitez.

## Démarrer à partir d'un modèle

**Fichier > Nouveau** affiche le **Document vierge** et un ensemble de modèles fournis avec JeffWrite, regroupés par type : une lettre, un rapport, une note de synthèse, un manuscrit et un bulletin. Chaque image représente la première page du document obtenu. Un **modèle** est un document déjà préparé, avec ses styles, ses marges et un texte d'exemple. Cliquez sur l'un d'eux, puis remplacez le texte d'exemple par le vôtre. Les modèles que vous avez enregistrés vous-même figurent sous **Mes modèles**.

## Reprendre là où vous en étiez

JeffWrite mémorise la position du curseur au moment où vous enregistrez ou fermez un fichier. À la prochaine ouverture, le curseur retrouve cette position. Appuyez sur **Maj+F5** pour aller aux endroits modifiés en dernier. Chaque pression vous mène à l'emplacement précédent : les trois derniers emplacements, puis retour au premier.

## Enregistrer en .odt

Un fichier .odt ne peut pas tout contenir comme un fichier .docx. Lorsque vous enregistrez en .odt, JeffWrite commence par afficher la liste des éléments qui seraient perdus, et vous permet d'enregistrer plutôt un fichier .docx. Voir [Informations](info).

## Imprimer

Appuyez sur **Ctrl+P**, ou cliquez sur **Fichier > Imprimer**. Choisissez une imprimante, le nombre de copies et les pages, examinez chaque page dans l'aperçu, puis cliquez sur **Imprimer**. Voir [Imprimer](print).

## Créer un PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Choisissez les pages à inclure : toutes, la page active, ou une plage telle que 1-3, 5 (p2s3 et s3 fonctionnent aussi ici). Choisissez ensuite un nom. Le PDF s'affiche de la même manière sur tous les ordinateurs, car les polices sont intégrées au fichier. Le titre, l'auteur, l'objet et les mots clés du document sont repris dans le PDF. Chaque titre devient un **signet**, une entrée de la liste que le lecteur PDF affiche à côté des pages. Cliquez sur un signet pour accéder à sa page.

## Enregistrer comme page web

Cliquez sur **Fichier > Exporter** et choisissez l'un des deux types de page web. Une **page web** est un fichier que vous pouvez ouvrir dans n'importe quel navigateur.

- **Enregistrer comme page web** crée un seul fichier .html contenant vos mots, leur mise en forme et vos images. Vous pouvez envoyer ce fichier seul. La mise en forme conservée comprend la police, la taille, la couleur, la surbrillance, le gras, l'italique, le soulignement, l'alignement et l'espace autour des paragraphes.
- **Enregistrer comme page web filtrée** crée un petit fichier .html ne contenant que la structure : titres, gras et italique, listes, tableaux, liens et images. Il n'a ni police, ni taille, ni couleur, de sorte que vous pourrez lui donner plus tard votre propre mise en forme.

Les deux conservent les titres, les listes à puces et numérotées, les tableaux avec cellules fusionnées, les liens et les images. Le texte masqué, les mots supprimés par une modification suivie, les commentaires, les en-têtes, les pieds de page et les notes sont omis. Un lien qui lancerait un programme ou exécuterait un script est enregistré sous forme de simple texte.

## Créer un PDF/A

Dans la boîte **Exporter au format PDF**, cochez **PDF/A, pour une conservation à long terme**. Un fichier PDF/A contient toutes les polices qu'il utilise et respecte une norme conçue pour conserver les fichiers pendant de nombreuses années. Il n'est pas encore balisé pour les lecteurs d'écran, et il n'a pas de mot de passe.

## Créer un courriel

Cliquez sur **Fichier > Exporter > Créer un courriel** pour créer un fichier de message (.eml). Rien n'est envoyé. Vous choisissez ce qui figure dans le message : le document, le document au format PDF, ou les pages affichées sous forme d'images dans le message (les 30 premières pages). Ouvrez le fichier dans votre messagerie, indiquez le destinataire, puis envoyez-le.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
- [Utiliser JeffWrite au clavier](keyboard)
