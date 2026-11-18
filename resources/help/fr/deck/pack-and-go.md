# Pack and Go : emporter une présentation avec soi
<!-- keywords: pack and go, package, envoyer, partager, autre ordinateur, zip, dossier, polices, fichiers liés, images liées, images manquantes, copier, emporter, clé USB, licence, film, son, vidéo, audio, diaporama -->

Une présentation peut lire un film ou un son depuis un fichier qui ne se trouve pas à l'intérieur de celle-ci. Sur un autre ordinateur, ce fichier est absent, et le diaporama présente alors un trou. **Pack and Go** enregistre la présentation avec ses fichiers liés et ses polices, dans un seul fichier ZIP ou un seul dossier.

## Utilisation

1. Cliquez sur **Fichier**, puis sur **Exporter**.
2. Cliquez sur **Pack and Go : enregistrer pour un autre ordinateur** pour créer un fichier ZIP, ou sur **Pack and Go : enregistrer dans un dossier** pour créer un nouveau dossier.
3. Choisissez l'emplacement d'enregistrement. Pour un fichier ZIP, saisissez un nom. Pour un dossier, choisissez le dossier dans lequel créer le nouveau.
4. Lisez le message qui s'affiche. Il indique où se trouve le paquet, combien de fichiers liés et de fichiers de polices y ont été ajoutés, ainsi que les éléments qui n'ont pas pu être empaquetés.

Un **fichier ZIP** est un fichier unique qui contient d'autres fichiers ; il est donc facile à envoyer. Le destinataire l'ouvre pour en extraire les fichiers. Un dossier est préférable lorsque vous voulez copier l'ensemble sur une clé USB.

## Contenu du paquet

- **La présentation elle-même**, avec le même nom que actuellement. Il s'agit d'une copie : votre fichier d'origine ne change pas.
- **Films et sons liés.** Un film ou un son peut être lié, ce qui signifie que la présentation le lit depuis son fichier au lieu de le contenir. Le paquet contient une copie de chacun d'eux, afin que le diaporama se déroule de la même façon sur l'autre ordinateur. Si vous n'avez pas inséré le fichier dans la présentation, c'est ainsi que vous l'emportez avec elle.
- **Images liées.** Le paquet contient une copie de chaque image liée à son fichier.
- **Polices.** Une police correspond à l'apparence des caractères. Le paquet contient les fichiers de polices fournis avec le programme et utilisés par la présentation, dans un dossier nommé **fonts**. Pack and Go n'empaquette jamais une police dont la licence interdit la diffusion. Le message indique chaque police laissée de côté. Les polices que vous avez installées vous-même sur votre ordinateur ne sont pas empaquetées : l'autre personne pourra donc en avoir besoin.

## Modification des liens

Dans la copie, chaque lien empaqueté pointe vers le fichier situé dans le paquet, par un nom court, par exemple **linked/photo.png**. Les liens n'utilisent plus le chemin de votre ordinateur : ils fonctionnent donc quel que soit l'emplacement où le paquet est ouvert. Si deux fichiers portent le même nom, un numéro est ajouté au second.

Un lien que Pack and Go ne peut pas suivre reste inchangé, et le message le signale. Pack and Go n'empaquette pas :

- un fichier introuvable ;
- une adresse web, qui reste telle quelle ;
- un fichier situé sur un autre ordinateur du réseau ;
- un lien qui remonte à l'extérieur du dossier de la présentation, par exemple **../../notes.png** ;
- un document, un classeur ou tout autre objet qui ne se trouve pas dans le dossier de la présentation (ou dans un dossier inclus dans celui-ci), même si le lien donne son chemin complet. Seuls les images, films et sons sont empaquetés, quel que soit leur emplacement ;
- un lien qui sort du dossier de la présentation par un raccourci vers un autre dossier (lien symbolique) ;
- un lien vers la présentation elle-même ;
- un fichier qui n'est ni une image, ni un film, ni un son, ni un document. Un programme n'est jamais empaqueté.

Pack and Go ne cherche pas de liens supplémentaires à l'intérieur des fichiers liés : un fichier qui renvoie vers celui-ci ne peut donc pas provoquer de boucle.

## Limites

Pour éviter qu'un paquet ne devienne trop volumineux, Pack and Go vous demande confirmation lorsqu'il y a plus de 200 fichiers liés, ou plus de 256 Mo de fichiers liés. Si vous choisissez **Non**, rien n'est enregistré. Pack and Go n'empaquette jamais plus de 2 000 fichiers, ni plus de 1 Go, quelle que soit votre réponse. Les fichiers au-delà de cette limite sont laissés de côté et mentionnés dans le message.

Si vous protégez la présentation par un mot de passe, la copie contenue dans le paquet a le même mot de passe.
