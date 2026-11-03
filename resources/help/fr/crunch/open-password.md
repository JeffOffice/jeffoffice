# Ouvrir des classeurs protégés par mot de passe
<!-- keywords: mot de passe, protégé, chiffré, chiffrement, cryptage, ouvrir, verrouillé, mot de passe incorrect, annuler, xlsx, xls, ods, protection du fichier, mot de passe par défaut, privé, confidentiel -->

Certains classeurs sont protégés par un mot de passe au moment de leur enregistrement : sans ce mot de passe, aucun programme ne peut lire le contenu du fichier. Cette protection est différente de **Protéger la feuille** et **Protéger le classeur** (voir [Protéger une feuille ou le classeur](protect)), qui empêchent seulement les modifications. Un classeur protégé par mot de passe ne peut pas du tout être ouvert tant que le mot de passe n'a pas été tapé. Cette page explique comment ouvrir un tel classeur et comment en créer un.

## Ouvrir un classeur protégé

Ouvrez le fichier comme n'importe quel autre : appuyez sur **{key:Open}**, choisissez-le, ou faites-le glisser sur la fenêtre. JeffCrunch détecte que le fichier est protégé et demande le mot de passe avant de lire quoi que ce soit :

1. Tapez le mot de passe dans la zone **Ce fichier est protégé par un mot de passe.** Les caractères s'affichent sous forme de points.
2. Cliquez sur **Ouvrir**.

Si le mot de passe est incorrect, JeffCrunch vous l'indique et vous le redemande, autant de fois que nécessaire. Cliquez sur **Annuler** pour abandonner : rien n'est ouvert, et les classeurs déjà ouverts restent tels quels. Les mots de passe doivent être exacts : les majuscules, les espaces et les accents comptent.

JeffCrunch n'enregistre le mot de passe nulle part. Il le conserve en mémoire tant que le classeur est ouvert, afin que **Enregistrer** puisse protéger de nouveau le fichier, puis l'efface lorsque vous fermez le classeur. À la prochaine ouverture du fichier, le mot de passe vous est redemandé.

## Fichiers pris en charge

- **Classeurs .xlsx et .xlsm** protégés selon l'une des deux méthodes utilisées par le tableur d'origine depuis 2007 : la plus récente (clé de 128, 192 ou 256 bits ; JeffCrunch vérifie en outre que personne n'a modifié le fichier après sa protection) et la plus ancienne (AES avec une clé de 128, 192 ou 256 bits).
- **Classeurs .xls** protégés selon l'une des trois méthodes de ce format : le simple brouillage des fichiers les plus anciens, le chiffrement 40 bits, et le chiffrement plus robuste qui utilise le service de chiffrement de l'ordinateur.
- **Feuilles de calcul .ods** protégées par mot de passe, selon les trois méthodes employées par LibreOffice et OpenOffice : la plus récente (LibreOffice 25 et versions ultérieures verrouillent l'ensemble du fichier avec une clé de 256 bits, et calculent la clé à partir du mot de passe avec une méthode volontairement lente et gourmande en mémoire : l'ouverture d'un tel fichier prend donc un peu plus de temps), le chiffrement 256 bits des versions précédentes, et la méthode la plus ancienne.

Les anciens fichiers .xls imposent des limites au mot de passe lui-même. Le simple brouillage des fichiers les plus anciens n'utilise que les 15 premiers caractères du mot de passe et n'accepte que les lettres, chiffres et signes du jeu de caractères occidental (Latin-1) : un mot de passe contenant d'autres caractères (grecs ou chinois, par exemple) n'est jamais reconnu comme correct. Le chiffrement 40 bits accepte tous les caractères, mais seulement les 16 premiers. JeffCrunch n'enregistre pas de fichiers .xls protégés par mot de passe.

Certains fichiers sont verrouillés par un mot de passe connu de tous, afin que chacun puisse les ouvrir sans risquer de les modifier par erreur. JeffCrunch les ouvre sans rien demander.

## Si un fichier ne s'ouvre pas

- **« Le mot de passe est incorrect. »** Le mot de passe tapé n'est pas celui qui a servi à protéger le fichier. Réessayez en faisant attention aux majuscules et aux espaces ; JeffCrunch ne peut pas récupérer un mot de passe perdu.
- **« Ce fichier est protégé d'une manière que ce programme ne peut pas ouvrir. »** Le fichier est verrouillé par autre chose qu'un mot de passe (par exemple une clé liée à un certificat), ou sous une forme que JeffCrunch ne lit pas.
- **« La protection par mot de passe de ce fichier demande plus de mémoire ou de temps que ce programme ne l'autorise ; il n'est donc pas ouvert. »** Un fichier créé avec LibreOffice 25 ou une version ultérieure fixe la quantité de mémoire et le temps nécessaires à la vérification du mot de passe. JeffCrunch accepte jusqu'à dix fois les valeurs fixées par LibreOffice (640 Mio de mémoire), afin qu'un fichier conçu pour bloquer votre ordinateur ne puisse pas y parvenir. Un fichier enregistré par LibreOffice reste largement en deçà de cette limite.
- **« Le fichier est endommagé ou a été modifié après sa protection. »** Le mot de passe est correct, mais le contenu du fichier ne correspond pas à ce qui a été verrouillé : il est tronqué, endommagé ou a été modifié par un autre programme. Copiez-le de nouveau à partir de l'original.

## Enregistrer avec un mot de passe

Cliquez sur **Fichier**, puis sur **Protéger par mot de passe** (ou **Révision > Protéger par mot de passe**), tapez le mot de passe deux fois, puis cliquez sur **Protéger**. Le classeur est protégé lors du prochain enregistrement : toute personne qui ouvre le fichier doit taper le mot de passe. Pour changer le mot de passe, choisissez la même commande et tapez-en un nouveau ; pour le supprimer, cliquez sur **Supprimer le mot de passe**, puis enregistrez.

Un classeur ouvert avec un mot de passe le conserve tant qu'il reste ouvert : **Enregistrer** protège donc de nouveau le fichier avec le même mot de passe. JeffCrunch ne garde le mot de passe en mémoire que jusqu'à la fermeture du classeur, puis l'efface. Le fichier ouvert avec un mot de passe ne perd pas sa protection par inadvertance : si vous en enregistrez une copie au format .xls ou .ods, que JeffCrunch ne peut pas protéger, il vous prévient d'abord que la copie ne sera pas protégée, et vous pouvez annuler.

Choisissez un mot de passe long et conservez-le en lieu sûr : personne, pas même nous, ne peut ouvrir le fichier sans lui.
