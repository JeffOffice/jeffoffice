# Ouvrir, enregistrer, imprimer et exporter
<!-- keywords: page web, html, enregistrer comme page web, page web filtrée, e-mail, courriel, eml, signets, signets pdf, maj+f5, dernière modification, où vous en étiez, modèles intégrés, galerie, lettre, rapport, note de synthèse, manuscrit, bulletin, ouvrir, enregistrer, enregistrer sous, docx, doc, dot, odt, opendocument, rtf, anciens fichiers, modèle, dotx, récent, épingler, nouveau, fermer, imprimer, pdf, exporter, récupérer, récupération automatique, non enregistré, plantage, écran fichier, formats de fichier, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir une copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, ott, réparer, ouvrir et réparer, endommagé, mot de passe, chiffrer, protéger par un mot de passe -->

Cliquez sur **Fichier** en haut à gauche pour afficher l'écran Fichier. Il comporte les pages Nouveau, Ouvrir, Imprimer et Exporter, ainsi que les commandes Enregistrer et Enregistrer sous. Appuyez sur **Échap** pour revenir à votre document.

## Créer un document

Appuyez sur **Ctrl+N**, ou cliquez sur **Fichier > Nouveau**, puis sur **Document vierge**. Un nouveau document s'ouvre dans sa propre fenêtre. Si la fenêtre dans laquelle vous vous trouvez est vide et n'a jamais été enregistrée, le nouveau document prend sa place.

## Ouvrir un document

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**. Cliquez sur **Parcourir** pour choisir un fichier. JeffWrite ouvre ces types de fichiers :

- **.docx** est un document.
- **.dotx** est un modèle. Un modèle s'ouvre comme un nouveau document créé à partir de celui-ci, si bien que le modèle reste inchangé.
- **.docm** et **.dotm** sont un document et un modèle pouvant contenir des **macros**, de petits programmes qui accompagnent un fichier. JeffWrite ne les exécute jamais, et les conserve telles quelles lorsque vous enregistrez au format .docm ou .dotm. Si vous enregistrez au format .docx ou .odt, les macros sont retirées, et JeffWrite vous en avertit.
- **.txt** est un texte brut, qui ne contient que les mots. Lorsque vous l'ouvrez, JeffWrite le lit immédiatement s'il s'agit d'un texte ASCII ou Unicode. Sinon, JeffWrite vous demande avec quel codage le lire et affiche les premières lignes, pour que vous vérifiiez que les lettres s'affichent correctement. Un **codage** est la manière dont un fichier enregistre les lettres, par exemple UTF-8 ou Windows 1252. Lorsque vous enregistrez au format .txt, vous choisissez le codage et les fins de ligne, et le fichier ne conserve que les mots : ni polices, ni images, ni filets de tableau.
- **.html** et **.htm** sont des pages web. JeffWrite lit les mots, les titres, le gras, l'italique, le souligné, les couleurs, les listes et les tableaux. Les puces et les numéros des listes sont importés sous forme de marques saisies, et les images et les liens sont omis. JeffWrite ne télécharge jamais rien depuis Internet et ignore les scripts. Une page web n'est jamais écrasée : **Enregistrer** demande un nouveau nom et enregistre un fichier .docx. Pour créer une page web, voir **Enregistrer en tant que page web** plus bas.
- **.doc** et **.dot** sont les anciens types de documents et de modèles. JeffWrite les lit, et enregistre vos modifications dans un nouveau fichier .docx.
- **.odt**, **.ott** et **.fodt** sont des fichiers OpenDocument. JeffWrite les lit tous les trois. Il enregistre les fichiers .odt, et enregistre vos modifications d'un fichier .ott ou .fodt dans un nouveau fichier .odt. Voir [Ouvrir et enregistrer des fichiers OpenDocument](open-odt).
- **.rtf** est le format Rich Text Format. JeffWrite le lit et l'enregistre. Voir [Ouvrir des fichiers RTF](open-rtf).

La page **Ouvrir** affiche aussi les documents que vous avez ouverts ou enregistrés récemment. Cliquez sur l'un d'eux pour l'ouvrir. Faites un clic droit sur un document pour l'épingler en haut de la liste, ou pour le retirer de la liste.

Si un fichier comporte des éléments que JeffWrite ne peut pas encore modifier, comme la mise en page d'un tableau ou un en-tête, il les conserve. Voir [Bienvenue dans JeffWrite](start). Si une partie du fichier ne peut pas être lue du tout, JeffWrite vous indique ce qu'il a laissé de côté. Il n'enregistre alors pas par-dessus ce fichier : **Enregistrer** demande un nouveau nom, afin que l'original conserve ce que la copie a perdu.

## Enregistrer un document

Appuyez sur **Ctrl+S** pour enregistrer. La première fois, JeffWrite demande un nom. **Enregistrer sous** (F12) enregistre une copie sous un nouveau nom. Choisissez **Modèle (*.dotx)** pour enregistrer un modèle, **Document prenant en charge les macros (*.docm)** pour conserver les macros, **Texte brut (*.txt)** pour ne conserver que les mots, ou **Rich Text Format (*.rtf)** pour un fichier que d'autres logiciels peuvent lire. Un fichier issu d'un ancien format est toujours enregistré sous un nouveau nom, afin que l'ancien fichier ne soit jamais écrasé.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**. Choisissez un nom. JeffWrite écrit le document dans ce fichier, et vous continuez à travailler dans le document que vous aviez ouvert. Son nom et ses modifications non enregistrées restent inchangés. C'est pratique lorsque vous voulez envoyer une version à quelqu'un tout en continuant à écrire.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, puis, dans la page **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffWrite copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, JeffWrite demande s'il faut enregistrer sans elle. La réponse est Non, sauf si vous répondez Oui, et le fichier reste tel qu'il était. Vous pouvez alors revenir à la dernière version enregistrée.

## Enregistrer comme modèle

Un modèle est un document que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle**, puis saisissez un nom. Le modèle est placé dans **Mes modèles**, et le document en cours de rédaction n'est pas modifié. Dans **Enregistrer sous**, vous pouvez aussi choisir **Modèle (*.dotx)** ou **Modèle de texte OpenDocument (*.ott)** pour enregistrer un fichier modèle à l'emplacement de votre choix.

Pour créer un nouveau document à partir de votre modèle, cliquez sur **Fichier > Nouveau**. Vos modèles figurent sous **Mes modèles**. Cliquez sur l'un d'eux. Un nouveau document s'ouvre avec le texte et les styles du modèle. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des récents. Chacune de ces méthodes demande un fichier, ou utilise celui sur lequel vous avez cliqué :

- **Ouvrir en lecture seule** ouvre le fichier uniquement pour la lecture. Vous pouvez consulter, faire défiler, rechercher et copier, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous appuyez sur **Enregistrer**, JeffWrite demande un nouveau nom, si bien que le fichier ouvert n'est pas modifié.
- **Ouvrir une copie** ouvre un nouveau document contenant le texte du fichier. Il n'a pas de fichier propre, si bien qu'**Enregistrer** demande où le placer. Le fichier ouvert n'est jamais modifié.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte, même si la fenêtre dans laquelle vous vous trouvez est vide.
- **Ouvrir et réparer** sert pour un fichier .docx endommagé, par exemple un fichier tronqué lors d'une copie. Voir ci-dessous.

## Ouvrir et réparer

Il arrive qu'un fichier .docx soit endommagé. Une copie interrompue en cours de route, ou un disque défectueux, en sont des causes possibles. Un fichier .docx est un ensemble de petites parties, comme le texte, les styles et les images. **Ouvrir et réparer** lit chaque partie qu'il peut lire. Une partie interrompue en cours de route est conservée jusqu'à l'endroit où elle s'arrête. Une partie qui ne peut pas être lue est omise. Si la liste des parties ou les liens entre elles ont disparu, JeffWrite les recrée.

Le document réparé s'ouvre comme une copie portant un nom du type « Lettre réparée ». Un message vous indique ce qui a été conservé jusqu'à une coupure et ce qui a été omis. Le fichier endommagé n'est pas modifié. Enregistrez la copie réparée sous un nom qui lui est propre, puis vérifiez-la avant de vous y fier.

## Récupérer un travail non enregistré

JeffWrite enregistre une copie de secours du travail non enregistré toutes les quelques minutes, dans un fichier placé dans son propre dossier. Si le programme se ferme de façon inattendue, il vous propose ces copies au prochain démarrage. Cliquez sur **Fichier > Ouvrir > Récupérer le travail non enregistré** pour les consulter plus tard. Vous pouvez choisir celles à ouvrir, et enregistrer chacune à l'emplacement de votre choix.

## Démarrer à partir d'un modèle

**Fichier > Nouveau** affiche le **Document vierge** ainsi qu'un ensemble de modèles fournis avec JeffWrite, regroupés par type : une lettre, un rapport, une note de synthèse, un manuscrit et un bulletin. Chaque image correspond à la première page du document obtenu. Un **modèle** est un document déjà préparé, avec ses styles, ses marges et des textes d'exemple. Cliquez sur l'un d'eux, puis remplacez les textes d'exemple par les vôtres. Les modèles que vous avez enregistrés vous-même figurent sous **Mes modèles**.

## Reprendre là où vous vous étiez arrêté

JeffWrite retient la position du curseur au moment où vous enregistrez ou fermez un fichier. À la prochaine ouverture du fichier, le curseur se trouve à cet endroit. Appuyez sur **Maj+F5** pour revenir aux emplacements modifiés en dernier. Chaque pression vous mène à l'emplacement précédent : les trois derniers emplacements, puis retour au premier.

## Enregistrer en .odt

Un fichier .odt ne peut pas contenir tout ce qu'un fichier .docx peut contenir. Lorsque vous enregistrez au format .odt, JeffWrite commence par afficher la liste des éléments qui seraient perdus, et vous permet d'enregistrer plutôt un fichier .docx. Voir [Informations](info).

## Imprimer

Appuyez sur **Ctrl+P**, ou cliquez sur **Fichier > Imprimer**. Choisissez une imprimante, le nombre de copies et les pages, examinez chaque page dans l'aperçu, puis cliquez sur **Imprimer**. Voir [Imprimer](print).

## Créer un PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Choisissez les pages à inclure : toutes, la page active, ou une plage telle que 1-3, 5 (p2s3 et s3 fonctionnent aussi ici). Choisissez ensuite un nom. Le PDF s'affiche de la même façon sur tous les ordinateurs, car les polices sont intégrées au fichier. Le titre, l'auteur, l'objet et les mots clés du document sont inscrits dans le PDF. Chaque titre devient un **signet**, une entrée de la liste qu'un lecteur PDF affiche à côté des pages. Cliquez sur un signet pour accéder à sa page.

## Enregistrer comme page web

Cliquez sur **Fichier > Exporter**, puis choisissez l'un des deux types de page web. Une **page web** est un fichier que vous pouvez ouvrir dans n'importe quel navigateur.

- **Enregistrer en tant que page web** crée un seul fichier .html qui contient vos mots, leur mise en forme et vos images. Vous pouvez envoyer ce seul fichier. La mise en forme conservée comprend la police, la taille, la couleur, la surbrillance, le gras, l'italique, le souligné, l'alignement et l'espace autour des paragraphes.
- **Enregistrer comme page web filtrée** crée un petit fichier .html qui ne contient que la structure : titres, gras et italique, listes, tableaux, liens et images. Il ne comporte ni polices, ni tailles, ni couleurs, si bien que vous pourrez lui donner plus tard votre propre mise en forme.

Les deux conservent les titres, les listes à puces et numérotées, les tableaux avec cellules fusionnées, les liens et les images. Le texte masqué, les mots supprimés par une modification suivie, les commentaires, les en-têtes, les pieds de page et les notes sont omis. Un lien qui lancerait un programme ou exécuterait un script est enregistré sous forme de simple texte.

## Protéger un PDF par un mot de passe

Dans la boîte **Exporter au format PDF**, cliquez sur **Chiffrer avec un mot de passe…**. Saisissez le mot de passe deux fois. Toute personne qui ouvre le PDF doit alors le saisir. Le mot de passe protège réellement le fichier : les pages sont brouillées par un chiffrement fort (AES-256), et personne ne peut les lire sans ce mot de passe.

Vous pouvez aussi cocher **Restreindre l'impression, la copie et la modification**, saisir un second mot de passe et choisir ce qui est autorisé. Ces restrictions ne sont qu'une demande. La plupart des lecteurs PDF les respectent, mais elles n'empêchent pas une personne déterminée de les contourner ; ne comptez donc pas sur elles pour garder un fichier confidentiel.

Un mot de passe oublié ne peut pas être récupéré : conservez-le en lieu sûr. Le mot de passe ne sert que pour ce PDF et n'est jamais enregistré. La case **PDF/A** est désactivée lorsqu'un mot de passe est défini, car un fichier PDF/A ne peut pas en comporter.

## Créer un PDF/A

Dans la boîte **Exporter au format PDF**, cochez **PDF/A, pour une conservation à long terme**. Un fichier PDF/A contient toutes les polices qu'il utilise et respecte une norme conçue pour conserver les fichiers pendant de nombreuses années. Il n'est pas encore balisé pour les lecteurs d'écran, et il ne peut pas comporter de mot de passe.

## Créer un e-mail

Cliquez sur **Fichier > Exporter > Créer un e-mail** pour créer un fichier de message (.eml). Rien n'est envoyé. Vous choisissez ce que contient le message : le document, le document au format PDF, ou les pages affichées sous forme d'images dans le message (les 30 premières pages). Ouvrez le fichier dans votre logiciel de messagerie, indiquez le destinataire, puis envoyez-le. Pour un PDF, vous pouvez cliquer sur **Chiffrer avec un mot de passe…** dans la boîte ; communiquez le mot de passe à la personne par un autre moyen que le message.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
- [Utiliser JeffWrite au clavier](keyboard)
