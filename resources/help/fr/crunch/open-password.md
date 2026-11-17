# Ouvrir des classeurs protégés par un mot de passe

<!-- keywords: mot de passe, protégé, chiffré, chiffrement, ouvrir, verrouillé, mauvais mot de passe, annuler, xlsx, xls, ods, protection du fichier, mot de passe par défaut, privé -->

Certains classeurs sont protégés par un mot de passe lors de leur enregistrement : sans ce mot de passe, aucun programme ne peut lire ce que contient le fichier. Ce n'est pas la même chose que **Protéger la feuille** et **Protéger le classeur** (voir [Protéger une feuille ou le classeur](protect)), qui empêchent seulement les modifications. Un classeur protégé par un mot de passe ne peut pas être ouvert tant que le mot de passe n'a pas été saisi. Cette page explique comment ouvrir un tel classeur et comment en créer un.

## Ouvrir un classeur

Ouvrez le fichier comme n'importe quel autre : appuyez sur **{key:Open}**, choisissez-le, ou faites-le glisser sur la fenêtre. JeffCrunch détecte que le fichier est protégé et demande le mot de passe avant de lire quoi que ce soit :

1. Saisissez le mot de passe dans la zone « Ce fichier est protégé par un mot de passe. ». Les caractères apparaissent sous forme de points.
2. Cliquez sur **Ouvrir**.

Si le mot de passe n'est pas le bon, JeffCrunch le signale et demande à nouveau, autant de fois que nécessaire. Cliquez sur **Annuler** pour arrêter : rien n'est ouvert, et les classeurs déjà ouverts restent tels quels. Les mots de passe sont exacts : les majuscules, les espaces et les accents comptent.

JeffCrunch n'écrit le mot de passe nulle part. Il le conserve en mémoire pendant que le classeur est ouvert, afin que **Enregistrer** puisse protéger à nouveau le fichier, puis l'efface à la fermeture du classeur. La prochaine fois que vous ouvrirez le fichier, il demandera de nouveau le mot de passe.

## Fichiers pouvant être ouverts

- **Classeurs .xlsx et .xlsm** protégés selon l'un des deux procédés utilisés par le programme d'origine depuis 2007 : le plus récent (la clé fait 128, 192 ou 256 bits, et JeffCrunch vérifie en plus que personne n'a modifié le fichier après sa protection) et le plus ancien (AES avec une clé de 128, 192 ou 256 bits).
- **Classeurs .xls** protégés selon l'un des trois procédés de ce format : le brouillage simple des plus anciens fichiers, le procédé à clé de 40 bits, et le procédé plus robuste qui utilise le service de clés de l'ordinateur.
- **Feuilles de calcul .ods** protégées par un mot de passe, selon les trois méthodes qu'ont utilisées LibreOffice et OpenOffice : la plus récente (LibreOffice 25 et versions ultérieures verrouillent tout le fichier avec une clé de 256 bits et calculent la clé à partir du mot de passe par une méthode volontairement lente et gourmande en mémoire ; l'ouverture de ces fichiers prend donc un peu plus de temps), celle à 256 bits des versions précédentes, et la plus ancienne.

Les anciens fichiers .xls imposent des limites au mot de passe lui-même. Le brouillage simple des plus anciens fichiers ne retient que les 15 premiers caractères d'un mot de passe et ne peut contenir que des lettres, des chiffres et des signes du jeu de caractères occidental (Latin-1) : un mot de passe comportant d'autres lettres (grecques ou chinoises, par exemple) n'est jamais reconnu comme valide dans ce cas. Le procédé à 40 bits accepte toutes les lettres, mais ne retient que les 16 premiers caractères. JeffCrunch n'enregistre pas de fichiers .xls avec mot de passe.

Certains fichiers sont verrouillés par un mot de passe que tout le monde connaît, afin de pouvoir être ouverts par tous sans être modifiés par accident. JeffCrunch les ouvre sans rien demander.

## Quand un fichier ne peut pas être ouvert

- **« Le mot de passe n'est pas correct. »** Le mot de passe saisi n'est pas celui qui a servi à protéger le fichier. Essayez à nouveau en faisant attention aux majuscules et aux espaces ; JeffCrunch ne peut pas récupérer un mot de passe perdu.
- **« Ce fichier est protégé d'une manière que ce programme ne peut pas ouvrir. »** Le fichier est verrouillé par un autre moyen qu'un mot de passe (une clé détenue par un certificat, par exemple), ou selon une forme que JeffCrunch ne lit pas.
- **« La protection par mot de passe de ce fichier exige plus de mémoire ou de temps que ce programme ne l'autorise, il n'est donc pas ouvert. »** Un fichier créé par LibreOffice 25 ou une version ultérieure fixe la quantité de mémoire et de temps nécessaire à la vérification du mot de passe. JeffCrunch accepte jusqu'à dix fois ce que LibreOffice fixe lui-même (640 Mio de mémoire), afin qu'un fichier conçu pour monopoliser votre ordinateur ne puisse pas être ouvert. Un fichier enregistré par LibreOffice reste bien en dessous de cette limite.
- **« Le fichier est endommagé, ou il a été modifié après sa protection. »** Le mot de passe était correct, mais le contenu du fichier ne correspond pas à ce qui a été verrouillé : il a été tronqué, endommagé ou modifié par un autre programme. Copiez-le à nouveau depuis l'original.

## Enregistrer avec un mot de passe

Cliquez sur **Fichier**, puis sur **Protéger par mot de passe** (ou **Révision > Protéger par mot de passe**), saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le classeur est enregistré protégé à votre prochain enregistrement : quiconque ouvre le fichier doit saisir le mot de passe. Pour changer de mot de passe, utilisez la même commande et saisissez-en un nouveau ; pour le supprimer, cliquez sur **Supprimer le mot de passe**, puis enregistrez.

Un classeur ouvert avec un mot de passe le conserve tant qu'il est ouvert : **Enregistrer** réécrit donc le fichier protégé avec le même mot de passe. JeffCrunch ne garde le mot de passe en mémoire que jusqu'à la fermeture du classeur, puis l'efface. Le fichier ouvert avec le mot de passe n'est pas laissé sans protection par mégarde : une copie enregistrée au format .ods ou .ots est protégée par le même mot de passe, selon la méthode décrite par la norme OpenDocument, de sorte que les autres programmes respectant cette norme peuvent l'ouvrir avec le mot de passe. Une copie au format .xls ou en texte ne peut pas être protégée : JeffCrunch vous en avertit d'abord, et vous pouvez annuler.

Choisissez un mot de passe long et conservez-le en lieu sûr : personne, nous compris, ne peut ouvrir le fichier sans lui.
