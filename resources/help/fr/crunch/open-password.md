# Ouvrir des classeurs protégés par mot de passe
<!-- keywords: mot de passe, protégé, chiffré, chiffrement, ouvrir, verrouillé, mauvais mot de passe, annuler, xlsx, xls, ods, protection du fichier, mot de passe par défaut, privé -->

Certains classeurs sont protégés par un mot de passe lors de leur enregistrement : sans ce mot de passe, aucun programme ne peut lire ce qui se trouve dans le fichier. Ce n'est pas la même chose que **Protéger la feuille** et **Protéger le classeur** (voir [Protéger une feuille ou le classeur](protect)), qui empêchent seulement les modifications. Un classeur protégé par un mot de passe ne peut pas du tout être ouvert tant que le mot de passe n'a pas été saisi. Cette page explique comment en ouvrir un et comment en créer un.

## Ouvrir un classeur protégé

Ouvrez le fichier comme n'importe quel autre : appuyez sur **{key:Open}**, choisissez-le, ou faites-le glisser dans la fenêtre. JeffCrunch détecte que le fichier est protégé et demande le mot de passe avant de lire quoi que ce soit :

1. Saisissez le mot de passe dans la zone qui indique **Ce fichier est protégé par un mot de passe.** Les caractères apparaissent sous forme de points.
2. Cliquez sur **Ouvrir**.

Si le mot de passe n'est pas le bon, JeffCrunch le signale et redemande le mot de passe, autant de fois que nécessaire. Cliquez sur **Annuler** pour arrêter : rien n'est ouvert, et les classeurs déjà ouverts restent tels quels. Les mots de passe sont exacts : les majuscules, les espaces et les accents comptent.

JeffCrunch n'écrit le mot de passe nulle part. Il le conserve en mémoire pendant que le classeur est ouvert, afin que **Enregistrer** puisse protéger de nouveau le fichier, puis l'efface à la fermeture du classeur. La prochaine fois que vous ouvrirez le fichier, il le redemandera.

## Ce qui peut être ouvert

- **Classeurs .xlsx et .xlsm** protégés selon l'un des deux modes utilisés par le programme d'origine depuis 2007 : le plus récent (clé de 128, 192 ou 256 bits, et JeffCrunch vérifie aussi que le fichier n'a pas été modifié après sa protection) et l'ancien (AES avec une clé de 128, 192 ou 256 bits).
- **Classeurs .xls** protégés selon l'un des trois modes de ce format : le simple brouillage des plus anciens fichiers, le mode à clé de 40 bits, et le mode plus solide qui utilise le service de clés de l'ordinateur.
- **Feuilles de calcul .ods** protégées par un mot de passe, selon les trois modes utilisés par LibreOffice et OpenOffice : le plus récent (LibreOffice 25 et versions ultérieures verrouillent tout le fichier avec une clé de 256 bits et dérivent la clé du mot de passe par une méthode volontairement lente et gourmande en mémoire ; l'ouverture d'un tel fichier prend donc un peu plus de temps), le mode à 256 bits des versions précédentes, et le plus ancien.

Les anciens fichiers .xls imposent des limites au mot de passe lui-même. Le simple brouillage des plus anciens fichiers n'utilise que les 15 premiers caractères d'un mot de passe et ne peut contenir que des lettres, des chiffres et des signes du jeu de caractères latin occidental (Latin-1) : un mot de passe contenant d'autres lettres (grecques ou chinoises, par exemple) n'est jamais accepté dans ce mode. Le mode à 40 bits accepte toutes les lettres, mais seulement les 16 premiers caractères. JeffCrunch écrit les copies .xls protégées par mot de passe dans le mode le plus solide (celui qui utilise le service de clés de l'ordinateur, avec une clé de 128 bits), car les deux autres sont plus faibles. Ce mode utilise les 255 premiers caractères du mot de passe, qui peuvent être de n'importe quelles lettres.

Certains fichiers sont verrouillés par un mot de passe que tout le monde connaît, afin de pouvoir être ouverts par tous mais ne pas être modifiés par accident. JeffCrunch les ouvre sans demander de mot de passe.

## Quand un fichier ne peut pas être ouvert

- **« Le mot de passe n'est pas correct. »** Le mot de passe saisi n'est pas celui avec lequel le fichier a été protégé. Essayez de nouveau en faisant attention aux majuscules et aux espaces ; JeffCrunch ne peut pas récupérer un mot de passe perdu.
- **« Ce fichier est protégé d'une manière que ce programme ne peut pas ouvrir. »** Le fichier est verrouillé par autre chose qu'un mot de passe (par exemple une clé détenue par un certificat), ou sous une forme que JeffCrunch ne sait pas lire.
- **« La protection par mot de passe de ce fichier demande plus de mémoire ou de temps que ce programme ne l'autorise : il n'est donc pas ouvert. »** Un fichier créé par LibreOffice 25 ou une version ultérieure fixe lui-même la quantité de mémoire et de temps nécessaire pour vérifier le mot de passe. JeffCrunch accepte jusqu'à dix fois la valeur fixée par LibreOffice (640 Mio de mémoire), afin qu'un fichier conçu pour monopoliser votre ordinateur ne puisse pas être ouvert. Un fichier enregistré par LibreOffice lui-même reste nettement en deçà de cette limite.
- **« Le fichier est endommagé, ou il a été modifié après sa protection. »** Le mot de passe était bon, mais le contenu du fichier ne correspond pas à ce qui a été verrouillé : il a été tronqué, endommagé ou modifié par un autre programme. Copiez-le de nouveau depuis l'original.

## Enregistrer avec un mot de passe

Cliquez sur **Fichier**, puis sur **Protéger avec un mot de passe** (ou **Révision > Protéger avec un mot de passe**), saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le classeur est enregistré protégé à votre prochain enregistrement : quiconque ouvre le fichier doit saisir le mot de passe. Pour changer de mot de passe, choisissez la même commande et saisissez-en un nouveau ; pour le supprimer, cliquez sur **Supprimer le mot de passe**, puis enregistrez.

Un classeur ouvert avec un mot de passe le conserve pendant qu'il est ouvert : **Enregistrer** réécrit donc le fichier protégé avec le même mot de passe. JeffCrunch ne garde le mot de passe en mémoire que jusqu'à la fermeture du classeur, puis l'efface. Le fichier ouvert avec le mot de passe ne se retrouve pas non protégé par inadvertance : une copie enregistrée au format .xls reste protégée par le même mot de passe, selon le mode plus solide du format .xls, et une copie enregistrée au format .ods ou .ots reste protégée selon la méthode décrite par la norme OpenDocument, de sorte que les autres programmes qui suivent ces normes peuvent ouvrir la copie avec le mot de passe. Une copie au format texte ne peut pas être protégée : JeffCrunch vous le signale d'abord, et vous pouvez annuler.

Choisissez un mot de passe long et conservez-le en lieu sûr : personne, nous compris, ne pourra ouvrir le fichier sans lui.
