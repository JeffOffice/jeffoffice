# Ouvrir, enregistrer, imprimer et exporter
<!-- keywords: polices dans le fichier, polices incorporées, police manquante, livre électronique, e-book, epub, liseuse, chapitres, sommaire, page web, html, enregistrer comme page web, page web filtrée, courriel, courrier électronique, eml, signets, signets pdf, maj+f5, dernière modification, là où vous vous êtes arrêté, modèles intégrés, galerie, lettre, lettre de motivation, lettre commerciale, curriculum vitae, CV, facture, ordre du jour, compte rendu de réunion, prospectus, rapport, note de synthèse, manuscrit, bulletin, ouvrir, enregistrer, enregistrer sous, docx, doc, dot, odt, opendocument, rtf, anciens fichiers, modèle, dotx, récent, épingler, nouveau, fermer, imprimer, pdf, exporter, récupérer, récupération automatique, non enregistré, plantage, écran Fichier, formats de fichier, enregistrer une copie, lecture seule, ouvrir en lecture seule, ouvrir une copie, nouvelle fenêtre, copie de sauvegarde, mes modèles, enregistrer comme modèle, ott, réparer, ouvrir et réparer, endommagé, mot de passe, chiffrer, protéger par un mot de passe -->

Cliquez sur **Fichier** en haut à gauche pour afficher l'écran Fichier. Il comporte les pages Nouveau, Ouvrir, Imprimer et Exporter, ainsi que les commandes Enregistrer et Enregistrer sous. Appuyez sur **Échap** pour revenir à votre document.

## Créer un document

Appuyez sur **Ctrl+N**, ou cliquez sur **Fichier > Nouveau**, puis sur **Document vierge**. Un nouveau document s'ouvre dans sa propre fenêtre. Si la fenêtre dans laquelle vous vous trouvez est vide et n'a jamais été enregistrée, le nouveau document prend sa place.

## Ouvrir un document

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**. Cliquez sur **Parcourir** pour choisir un fichier. JeffWrite ouvre les types de fichiers suivants :

- **.docx** est un document.
- **.dotx** est un modèle. Un modèle s'ouvre comme un nouveau document créé à partir de ce modèle, de sorte que le modèle reste inchangé.
- **.docm** et **.dotm** sont un document et un modèle pouvant contenir des **macros**, de petits programmes qui accompagnent un fichier. JeffWrite ne les exécute jamais. Il les conserve telles quelles lorsque vous enregistrez au format .docm ou .dotm. Si vous enregistrez au format .docx ou .odt, les macros sont omises, et JeffWrite vous en avertit.
- **.txt** est un texte brut, constitué uniquement de mots. À l'ouverture, JeffWrite le lit immédiatement s'il s'agit d'ASCII ou d'Unicode. Sinon, JeffWrite demande dans quel **codage** le lire et affiche les premières lignes, afin que vous puissiez vérifier que les lettres s'affichent correctement. Un **codage** est la manière dont un fichier stocke les lettres, comme UTF-8 ou Windows 1252. Lorsque vous enregistrez au format .txt, vous choisissez le codage et les fins de ligne, et le fichier ne conserve que les mots : ni polices, ni images, ni filets de tableau.
- **.html** et **.htm** sont des pages web. JeffWrite lit les mots, les titres, le gras, l'italique, le souligné, les couleurs, les listes et les tableaux. Les puces et les numéros des listes sont importés sous forme de marques saisies, et les images et les liens sont omis. Il ne récupère jamais rien sur internet et ignore les scripts. Une page web n'est jamais écrasée : **Enregistrer** demande un nouveau nom et enregistre un fichier .docx. Pour créer une page web, reportez-vous à **Enregistrer comme page web** ci-dessous.
- **.doc** et **.dot** sont les anciens types de documents et de modèles. JeffWrite les lit, et enregistre vos modifications dans un nouveau fichier .docx.
- **.odt**, **.ott** et **.fodt** sont des fichiers OpenDocument. JeffWrite les lit tous les trois. Il enregistre les fichiers .odt, et il enregistre ce que vous faites d'un fichier .ott ou .fodt dans un nouveau fichier .odt. Voir [Ouvrir et enregistrer des fichiers OpenDocument](open-odt).
- **.rtf** est au format Rich Text Format. JeffWrite le lit et l'enregistre. Voir [Ouvrir des fichiers RTF](open-rtf).

La page **Ouvrir** affiche aussi les documents que vous avez ouverts ou enregistrés récemment. Cliquez sur l'un d'eux pour l'ouvrir. Faites un clic droit sur l'un d'eux pour l'épingler en haut de la liste ou le retirer de la liste.

Si un fichier comporte des éléments que JeffWrite ne peut pas encore modifier, comme la disposition d'un tableau ou un en-tête, il les conserve. Voir [Bienvenue dans JeffWrite](start). Si une partie du fichier ne peut pas du tout être lue, JeffWrite vous indique ce qu'il a omis. Il n'écrase alors pas ce fichier : **Enregistrer** demande un nouveau nom, afin que l'original conserve ce que la copie a perdu.

## Enregistrer un document

Appuyez sur **Ctrl+S** pour enregistrer. La première fois, JeffWrite demande un nom. **Enregistrer sous** (F12) enregistre une copie sous un nouveau nom. Choisissez **Modèle (*.dotx)** pour enregistrer un modèle, **Document avec macros (*.docm)** pour conserver les macros, **Texte brut (*.txt)** pour ne conserver que les mots, ou **Texte enrichi (*.rtf)** pour un fichier que d'autres programmes peuvent lire. Un fichier issu d'un ancien type de fichier est toujours enregistré sous un nouveau nom, afin que l'ancien fichier ne soit jamais écrasé.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**. Choisissez un nom. JeffWrite écrit le document dans ce fichier, et vous continuez à travailler dans le document d'origine. Son nom et ses modifications non enregistrées restent inchangés. C'est pratique lorsque vous voulez envoyer une version à quelqu'un tout en continuant à écrire.

## Conserver une copie de sauvegarde

Cliquez sur **Fichier > Options**, et, sur la page **Général**, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffWrite copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi de son nom, dans le même dossier. Si la copie ne peut pas être créée, JeffWrite demande s'il faut enregistrer sans elle. La réponse est Non, sauf si vous répondez Oui, et le fichier reste inchangé. Vous pouvez ainsi revenir à la dernière version enregistrée.

## Enregistrer comme modèle

Un modèle est un document que vous utilisez comme point de départ. Cliquez sur **Fichier > Enregistrer comme modèle**, puis saisissez un nom. Le modèle est placé dans **Mes modèles**, et le document en cours n'est pas modifié. Dans **Enregistrer sous**, vous pouvez aussi choisir **Modèle (*.dotx)** ou **Modèle de texte OpenDocument (*.ott)** pour enregistrer un fichier modèle à l'emplacement de votre choix.

Pour commencer un nouveau document à partir de votre modèle, cliquez sur **Fichier > Nouveau**. Vos modèles figurent sous **Mes modèles**. Cliquez sur l'un d'eux. Un nouveau document s'ouvre avec le texte et les styles du modèle. Pour retirer un modèle de la liste, faites un clic droit dessus et choisissez **Supprimer de Mes modèles**.

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des récents. Chaque méthode demande un fichier, ou utilise celui sur lequel vous avez cliqué :

- **Ouvrir en lecture seule** ouvre le fichier uniquement pour la lecture. Vous pouvez le consulter, le faire défiler, y rechercher et y copier du texte, mais vous ne pouvez rien modifier. La barre de titre affiche « Lecture seule ». Si vous appuyez sur **Enregistrer**, JeffWrite demande un nouveau nom, de sorte que le fichier ouvert n'est pas modifié.
- **Ouvrir une copie** ouvre un nouveau document contenant le texte du fichier. Il n'a pas de fichier propre, donc **Enregistrer** demande où le placer. Le fichier ouvert n'est jamais modifié.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans sa propre fenêtre, même si la fenêtre dans laquelle vous vous trouvez est vide.
- **Ouvrir et réparer** concerne un fichier .docx ou .doc endommagé, par exemple un fichier interrompu lors d'une copie. Voir ci-dessous.

## Ouvrir et réparer

Il arrive qu'un fichier .docx ou .doc soit endommagé : une copie interrompue en cours de route, ou un disque qui a commis des erreurs. Un fichier .docx est un ensemble de petites parties, comme le texte, les styles et les images. **Ouvrir et réparer** lit chaque partie qu'il peut lire. Une partie interrompue en cours de route est conservée jusqu'à l'endroit où elle s'arrête. Une partie qui ne peut pas être lue est omise. Si la liste des parties ou les liens entre elles ont disparu, JeffWrite les recrée.

Un ancien fichier .doc est une grande enveloppe contenant quelques flux de données. Un flux contient le texte, et un autre contient le tableau qui indique où se trouve chaque morceau de texte et à quoi il ressemble. Si l'enveloppe est interrompue ou si ses listes sont endommagées, **Ouvrir et réparer** retrouve ces flux d'après leur contenu et récupère autant que possible. Lorsque les listes qui indiquent comment les morceaux s'enchaînent sont endommagées, le programme prend chaque morceau à la suite du précédent, et affiche ce qu'il y trouve. Ce texte peut provenir d'une ancienne copie du document ou d'une autre partie du fichier ; le message indique donc le paragraphe où il commence et vous invite à le comparer avec une sauvegarde si vous en avez une. Si la supposition précède le texte, le message indique que le document entier a été reconstitué ; si elle le suit, il indique que la mise en forme l'a été. Si les images n'ont pas pu être entièrement récupérées, le message le signale. Si le tableau est perdu, le texte est conservé sous forme de paragraphes simples, sans styles, listes, tableaux ni images. Un fichier .doc protégé par un mot de passe ne peut pas être réparé.

Le document réparé s'ouvre comme une copie portant un nom tel que « Lettre réparée ». Un message indique ce qui a été conservé jusqu'à une interruption et ce qui a été omis. Le fichier endommagé n'est pas modifié. Enregistrez la copie réparée sous un nom qui lui est propre, puis vérifiez-la avant de vous y fier.

## Récupérer un travail non enregistré

JeffWrite enregistre une copie de secours du travail non enregistré toutes les quelques minutes, dans un fichier placé dans son propre dossier. Si le programme se ferme de façon inattendue, il vous propose ces copies au prochain démarrage. Cliquez sur **Fichier > Ouvrir > Récupérer le travail non enregistré** pour les consulter plus tard. Vous choisissez celles à ouvrir, et vous enregistrez chacune à l'endroit de votre choix.

## Démarrer à partir d'un modèle

**Fichier > Nouveau** affiche le **Document vierge** et un ensemble de modèles fournis avec JeffWrite, regroupés par type : lettres (une lettre personnelle, une lettre de motivation et une lettre commerciale), rapports (un rapport, un rapport avec page de garde et table des matières, et une note de synthèse), un manuscrit, des publications (un bulletin et un prospectus), des CV (un CV classique et un CV avec colonne latérale) et des documents professionnels (une facture, un ordre du jour de réunion et un compte rendu de réunion). Pointez une image pour voir à quoi sert le modèle. Chaque image correspond à la première page du document que vous obtenez. Un **modèle** est un document déjà mis en forme, avec ses styles, ses marges et des textes d'exemple. Cliquez sur l'un d'eux, puis remplacez les textes d'exemple par les vôtres. Les modèles que vous avez enregistrés vous-même figurent sous **Mes modèles**.

## Reprendre là où vous vous êtes arrêté

JeffWrite se souvient de l'emplacement du curseur lorsque vous enregistrez ou fermez un fichier. À la prochaine ouverture du fichier, le curseur se trouve à cet endroit. Appuyez sur **Maj+F5** pour revenir aux endroits modifiés en dernier. Chaque pression vous mène à l'endroit précédent : les trois derniers emplacements, puis de nouveau le premier.

## Enregistrer au format .odt

Un fichier .odt ne peut pas contenir tout ce que peut contenir un fichier .docx. Lorsque vous enregistrez au format .odt, JeffWrite commence par afficher la liste de ce qui serait perdu, et vous permet d'enregistrer plutôt un fichier .docx. Voir [Informations](info).

## Imprimer

Appuyez sur **Ctrl+P**, ou cliquez sur **Fichier > Imprimer**. Choisissez une imprimante, le nombre de copies et les pages, examinez chaque page dans l'aperçu, puis cliquez sur **Imprimer**. Voir [Imprimer](print).

## Créer un PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Le PDF reprend le titre, l'auteur, l'objet et les mots clés du document issus de **Fichier > Informations** (avec **Inclure les propriétés du document** activé), la date de création et JeffWrite comme programme créateur ; l'auteur et un titre dérivé du nom du fichier sont omis lorsque les informations personnelles sont supprimées. Chacun de ces éléments est conservé en entier jusqu'à 4 000 caractères. Cochez **Ouvrir le PDF après l'enregistrement** pour que le fichier s'ouvre dans le programme PDF de votre ordinateur lors de l'enregistrement. Cette option est désactivée tant que vous ne la cochez pas. Si votre ordinateur ne peut pas ouvrir le fichier, la barre d'état l'indique (le fichier est tout de même enregistré). Choisissez les pages à inclure : toutes, la page active ou une plage telle que 1-3, 5 (p2s3 et s3 sont aussi acceptés). Puis choisissez le mode de création :

- **Optimiser pour** propose **Standard**, qui conserve vos images telles quelles, et **Taille minimale**, qui limite chaque image à 150 pixels par pouce, afin que le fichier soit plus petit. Utilisez Taille minimale pour un PDF que vous publierez sur une page web ou que vous enverrez par courriel.
- **Inclure les propriétés du document** place le titre, l'auteur, l'objet et les mots clés du document dans le PDF.
- **Créer des signets à partir des titres** transforme chaque titre en **signet**, c'est-à-dire en entrée de la liste affichée à côté des pages par le lecteur PDF. Cliquez sur un signet pour aller à sa page.
- **Balises de structure du document pour l'accessibilité** balise les titres, les paragraphes, les listes, les tableaux (avec leurs lignes d'en-tête) et les images (avec leur texte de remplacement), et indique la langue, afin qu'un lecteur d'écran lise le PDF dans l'ordre.

Choisissez ensuite un nom. Le PDF s'affiche de la même manière sur tous les ordinateurs, car les polices sont incluses dans le fichier. Le PDF que **Imprimer** crée lorsque vous choisissez un fichier PDF comporte également les signets et les balises. Lorsqu'il place plusieurs pages sur une même feuille, chaque signet et chaque lien mène à la feuille qui contient sa page, et les pages d'une feuille sont lues dans l'ordre où elles y figurent : de gauche à droite, puis de haut en bas.

## Enregistrer comme page web

Cliquez sur **Fichier > Exporter**, puis choisissez l'un des deux types de page web. Une **page web** est un fichier que vous pouvez ouvrir dans n'importe quel navigateur.

- **Enregistrer comme page web** crée un seul fichier .html contenant vos mots, leur mise en forme et vos images. Vous pouvez envoyer ce seul fichier. La mise en forme conservée comprend la police, la taille, la couleur, la surbrillance, le gras, l'italique, le souligné, l'alignement et l'espace autour des paragraphes.
- **Enregistrer comme page web filtrée** crée un petit fichier .html ne contenant que la structure : titres, gras et italique, listes, tableaux, liens et images. Il ne comporte ni polices, ni tailles, ni couleurs, de sorte que vous pouvez lui donner ensuite votre propre mise en forme.

Les deux conservent les titres, les listes à puces et numérotées, les tableaux avec cellules fusionnées, les liens et les images. Le texte masqué, les mots supprimés dans une modification suivie, les commentaires, les en-têtes, les pieds de page et les notes sont omis. Un lien qui lancerait un programme ou exécuterait un script est enregistré sous forme de simple texte.

## Enregistrer au format livre électronique

Un **livre électronique** est un livre que vous lisez sur une liseuse, un téléphone ou une tablette. Le fichier est au format **EPUB**. Le texte s'adapte à tous les écrans, et le lecteur peut agrandir ou réduire les caractères.

1. Cliquez sur **Fichier > Exporter > Enregistrer au format e-book (EPUB)**.
2. Vérifiez le **titre** et l'**auteur**. Ils reprennent le titre et l'auteur des propriétés de votre document (**Fichier > Informations**). Vous pouvez les modifier pour ce livre uniquement.
3. Activez **Utiliser la première image comme couverture** si vous voulez que la première image de votre document serve de couverture. Sans cette option, le livre n'a pas de couverture.
4. Cliquez sur **OK**, puis choisissez l'emplacement d'enregistrement du fichier.

Fonctionnement de la création du livre :

- Chaque **Titre 1** commence un nouveau chapitre. Le texte situé avant le premier Titre 1 forme un chapitre à part.
- Le **sommaire** affiche chaque Titre 1, Titre 2 et Titre 3, et le lecteur peut aller directement à chacun. Les titres plus petits figurent dans le livre, mais pas dans le sommaire.
- Les images et les tableaux restent à l'endroit où vous les avez placés. Le texte de remplacement d'une image l'accompagne, afin qu'un lecteur d'écran puisse décrire ce qu'elle montre.
- Les notes de bas de page et les notes de fin sont placées à la fin du chapitre où elles se trouvent. Chaque note comporte un lien vers l'endroit du texte où elle se rattache, et chaque numéro du texte est un lien vers sa note.

Un livre est soumis à des limites, afin qu'un fichier endommagé ne produise pas un livre énorme : jusqu'à 500 chapitres, 4 000 lignes dans le sommaire, 1 000 images (aucune de plus de 32 Mo) et 20 000 notes. Ce qui dépasse une limite reste dans le dernier chapitre, ou devient du texte simple, et vos mots ne sont jamais perdus : une image au-delà de la limite est remplacée par son texte de remplacement, et une note au-delà de la limite n'a pas de mise en forme. Une image que le lecteur ne peut pas afficher (un métafichier, une image bitmap ou un TIFF) est convertie en PNG. Exporter à nouveau le même document enregistré produit le même livre, de sorte qu'un lecteur retrouve sa place.

## Protéger un PDF par un mot de passe

Dans la zone **Exporter au format PDF**, cliquez sur **Chiffrer avec un mot de passe…**. Saisissez le mot de passe deux fois. Toute personne qui ouvre le PDF doit alors le saisir. Le mot de passe protège réellement le fichier : les pages sont brouillées par un chiffrement robuste (AES-256), et personne ne peut les lire sans lui.

Vous pouvez aussi cocher **Restreindre l'impression, la copie et la modification**, saisir un second mot de passe et choisir ce qui est autorisé. Ces restrictions ne sont qu'une demande. La plupart des lecteurs PDF les respectent, mais elles n'empêchent pas une personne déterminée de les contourner ; ne comptez donc pas sur elles pour garder un fichier privé.

Un mot de passe oublié ne peut pas être récupéré : conservez-le en lieu sûr. Le mot de passe ne sert qu'à ce PDF et n'est jamais enregistré. Tant qu'un mot de passe est défini, la zone désactive **PDF/A**, car un fichier PDF/A ne peut pas en comporter.

## Créer un PDF/A

Dans la zone **Exporter au format PDF**, cochez **PDF/A, pour une conservation à long terme**. Un fichier PDF/A (PDF/A-1b) contient toutes les polices qu'il utilise, nomme ses couleurs et respecte une norme conçue pour conserver les fichiers pendant de nombreuses années. Il peut être balisé pour les lecteurs d'écran, mais il ne peut pas être protégé par un mot de passe.

## Créer un courriel

Cliquez sur **Fichier > Exporter > Créer un courriel** pour créer un fichier de message (.eml). Rien n'est envoyé. Vous choisissez ce qui figure dans le message : le document, le document au format PDF, ou les pages affichées sous forme d'images dans le message (les 30 premières pages). Ouvrez le fichier dans votre logiciel de messagerie, indiquez le destinataire, puis envoyez-le. Pour un PDF, vous pouvez cliquer sur **Chiffrer avec un mot de passe…** dans la zone ; communiquez le mot de passe à la personne par un autre moyen que le message.

## Polices dans le fichier

Un fichier .docx peut contenir les polices utilisées par son texte, afin que le document s'affiche comme son auteur l'a conçu, même sur un ordinateur qui ne possède pas ces polices. JeffWrite les lit à l'ouverture du fichier. Elles servent uniquement à ce document, pendant qu'il est ouvert. Elles ne sont jamais installées sur votre ordinateur, aucun autre document ni programme ne les voit, et elles disparaissent à la fermeture du document. Une police contenue dans le fichier ne compte pas comme une police manquante.

JeffWrite n'utilise pas une police dont la licence de l'éditeur interdit l'incorporation, une police endommagée, ni une police au-delà des 64 premières d'un même fichier. Dans ce cas, le message affiché à l'ouverture du fichier le signale, et le texte est affiché dans une police semblable. Le fichier conserve ses polices telles qu'elles étaient lors de l'enregistrement.

Pour inclure dans les fichiers que vous enregistrez les polices utilisées par votre texte, cliquez sur **Fichier > Options**, ouvrez la page **Enregistrement** et activez **Enregistrer les polices dans le fichier**. Cette option est désactivée au départ. JeffWrite inclut alors les polices fournies avec lui et utilisées par le texte, dans chaque style employé par le document, lors de l'enregistrement d'un fichier .docx. Il n'inclut jamais une police dont la licence ne l'autorise pas, et lorsqu'il en omet une, le message affiché après l'enregistrement la nomme. Le fichier devient plus volumineux, et au plus 64 polices et 256 Mo peuvent être inclus dans un même fichier.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
- [Utiliser JeffWrite au clavier](keyboard)
