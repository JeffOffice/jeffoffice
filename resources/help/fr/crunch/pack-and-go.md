# Pack and Go : envoyer un classeur avec les fichiers dont il a besoin
<!-- keywords: pack and go, empaqueter, paquet, envoyer, partager, autre ordinateur, zip, dossier, polices, fichiers liés, images liées, images manquantes, copier, emporter, clé USB, licence, lien externe, classeur lié, cellules, graphiques, pack and go, archiver -->

Lorsque vous envoyez un classeur à quelqu'un, les images et les autres classeurs auxquels il est lié peuvent se perdre en chemin. **Pack and Go** enregistre le classeur avec ces fichiers et ses polices, dans un seul fichier ZIP ou un seul dossier, afin qu'il s'affiche de la même façon sur un autre ordinateur.

## Utilisation

1. Cliquez sur **Fichier**, puis sur **Exporter**.
2. Cliquez sur **Pack and Go: Enregistrer pour un autre ordinateur** pour créer un seul fichier ZIP, ou sur **Pack and Go: Enregistrer dans un dossier** pour créer un nouveau dossier.
3. Choisissez où enregistrer le paquet. Pour un fichier ZIP, tapez un nom. Pour un dossier, choisissez le dossier dans lequel créer le nouveau.
4. Lisez le message qui s'affiche. Il indique où se trouve le paquet, combien de fichiers liés et de fichiers de polices y ont été ajoutés, et ce qui n'a pas pu être empaqueté.

Un **fichier ZIP** est un fichier qui contient d'autres fichiers ; il est donc facile à envoyer. Le destinataire l'ouvre pour en extraire les fichiers. Un dossier est préférable si vous voulez copier l'ensemble sur une clé USB.

## Contenu du paquet

- **Le classeur lui-même**, avec le même nom que celui qu'il porte actuellement. Il s'agit d'une copie. Votre fichier d'origine ne change pas.
- **Images liées.** Une image du classeur peut être liée à son fichier. Le paquet contient une copie de chacune d'elles. Les images insérées directement dans le classeur y sont déjà incluses.
- **Classeurs liés.** Une formule peut récupérer des nombres dans un autre classeur. Le paquet contient une copie de ce classeur également, et le lien désigne désormais la copie. Les polices concernées sont celles utilisées dans les cellules, les graphiques et les formes.
- **Polices.** Une police correspond à l'aspect des caractères. Le paquet contient les fichiers de police fournis avec le programme et utilisés par le classeur, dans un dossier nommé **fonts**. Pack and Go n'empaquette jamais une police dont la licence interdit la diffusion. Le message indique chaque police laissée de côté. Les polices que vous avez installées vous-même sur votre ordinateur ne sont pas empaquetées ; le destinataire pourrait donc en avoir besoin.

## Modification des liens

Dans la copie, chaque lien empaqueté pointe vers le fichier situé dans le paquet, sous un nom court, par exemple **linked/photo.png**. Les liens n'utilisent plus le chemin de votre ordinateur ; ils fonctionnent donc où que le paquet soit ouvert. Si deux fichiers portent le même nom, un numéro est ajouté au second.

Un lien que Pack and Go ne peut pas suivre reste tel quel, et le message le signale. Pack and Go n'empaquette pas :

- un fichier introuvable ;
- une adresse web, qui reste telle quelle ;
- un fichier situé sur un autre ordinateur du réseau ;
- un lien qui remonte hors du dossier du classeur, par exemple **../../notes.png** ;
- un document, un classeur ou un autre objet qui ne se trouve pas dans le dossier du classeur (ou dans un sous-dossier), même lorsque le lien indique son chemin complet. Seuls les images, les vidéos et les sons sont empaquetés, quel que soit leur emplacement ;
- un lien qui sort du dossier du classeur par un raccourci vers un autre dossier (lien symbolique) ;
- un lien vers le classeur lui-même ;
- un fichier qui n'est ni une image, ni une vidéo, ni un son, ni un document. Un programme n'est jamais empaqueté.

Pack and Go ne recherche pas de liens supplémentaires à l'intérieur des fichiers liés ; un fichier qui renvoie vers ce classeur ne peut donc pas créer de boucle.

## Limites

Pour éviter qu'un paquet ne devienne trop volumineux, Pack and Go vous demande confirmation lorsqu'il y a plus de 200 fichiers liés ou plus de 256 Mo de fichiers liés. Choisissez **Non** et rien n'est enregistré. Pack and Go n'empaquette jamais plus de 2 000 fichiers, ni plus de 1 Go, quelle que soit votre réponse. Les fichiers au-delà de cette limite sont laissés de côté et listés dans le message.

Si le classeur est protégé par un mot de passe, la copie contenue dans le paquet a le même mot de passe.
