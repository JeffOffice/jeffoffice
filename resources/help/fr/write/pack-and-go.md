# Pack and Go : envoyer un document avec ce dont il a besoin
<!-- keywords: pack and go, paquet, empaqueter, envoyer, partager, autre ordinateur, zip, dossier, polices, fichiers liés, images liées, images manquantes, copier, emporter, clé USB, licence, INCLUDEPICTURE, INCLUDETEXT, champ -->

Lorsque vous envoyez un document à quelqu'un, les images et les polices qu'il utilise peuvent se perdre en chemin. **Pack and Go** enregistre le document avec les éléments dont il a besoin, dans un seul fichier ZIP ou un seul dossier, afin qu'il s'affiche de la même façon sur un autre ordinateur.

## Utilisation

1. Cliquez sur **Fichier**, puis sur **Exporter**.
2. Cliquez sur **Pack and Go : enregistrer pour un autre ordinateur** pour créer un seul fichier ZIP, ou sur **Pack and Go : enregistrer dans un dossier** pour créer un nouveau dossier.
3. Choisissez l'emplacement d'enregistrement. Pour un fichier ZIP, saisissez un nom. Pour un dossier, choisissez le dossier dans lequel créer le nouveau.
4. Lisez le message qui s'affiche. Il indique où se trouve le paquet, combien de fichiers liés et de fichiers de polices y ont été ajoutés, ainsi que les éléments qui n'ont pas pu être empaquetés.

Un **fichier ZIP** est un fichier unique qui contient d'autres fichiers ; il est donc facile à envoyer. La personne qui le reçoit l'ouvre pour en extraire les fichiers. Un dossier est préférable lorsque vous voulez copier l'ensemble sur une clé USB.

## Contenu du paquet

- **Le document lui-même**, avec le nom qu'il porte actuellement. Il s'agit d'une copie : votre fichier d'origine ne change pas.
- **Images liées.** Une image liée affiche une image provenant d'un fichier, au lieu de contenir l'image elle-même. Le paquet contient une copie de chacune d'elles.
- **Fichiers désignés par des champs.** Un champ est un code qui affiche son propre résultat. Un champ **INCLUDEPICTURE** affiche une image provenant d'un fichier, et un champ **INCLUDETEXT** affiche un texte provenant d'un autre document. Le paquet contient une copie de chaque fichier, et le champ désigne désormais cette copie.
- **Polices.** Une police correspond à l'apparence des caractères. Le paquet contient les fichiers de polices fournis avec le programme et utilisés par le document, dans un dossier nommé **fonts**. Pack and Go n'empaquette jamais une police dont la licence interdit la diffusion. Le message indique chaque police laissée de côté. Les polices que vous avez installées vous-même sur votre ordinateur ne sont pas empaquetées : l'autre personne pourrait donc avoir besoin de les installer.

## Modification des liens

Dans la copie, chaque lien empaqueté pointe vers le fichier situé dans le paquet, désigné par un nom court, par exemple **linked/photo.png**. Les liens n'utilisent plus le chemin de votre ordinateur, ils fonctionnent donc quel que soit l'endroit où le paquet est ouvert. Si deux fichiers portent le même nom, un numéro est ajouté au second.

Un lien que Pack and Go ne peut pas suivre reste tel quel, et le message le signale. Pack and Go n'empaquette pas :

- Un fichier introuvable.
- Une adresse web. Elles restent telles quelles.
- Un fichier situé sur un autre ordinateur de votre réseau.
- Un lien qui remonte hors du dossier du document, par exemple **../../notes.png**.
- Un document, un classeur ou un autre objet qui ne se trouve pas dans le dossier du document (ou dans un dossier qu'il contient), même lorsque le lien indique son chemin complet. Seules les images, vidéos et sons sont empaquetés, quel que soit leur emplacement.
- Un lien qui sort du dossier du document par un raccourci vers un autre dossier (lien symbolique).
- Un lien vers le document lui-même.
- Un fichier qui n'est ni une image, ni une vidéo, ni un son, ni un document. Un programme n'est jamais empaqueté.

Pack and Go ne recherche pas d'autres liens à l'intérieur des fichiers liés. Un fichier qui renvoie vers ce document ne peut donc pas créer de boucle.

## Limites

Pour éviter qu'un paquet ne devienne trop volumineux, Pack and Go vous demande confirmation dès qu'il y a plus de 200 fichiers liés, ou plus de 256 Mo de fichiers liés. Si vous répondez **Non**, rien n'est enregistré. Pack and Go n'empaquette jamais plus de 2 000 fichiers ni plus de 1 Go, quelle que soit votre réponse. Les fichiers au-delà de cette limite sont laissés de côté et listés dans le message.

Si vous protégez le document par un mot de passe, la copie contenue dans le paquet conserve le même mot de passe.
