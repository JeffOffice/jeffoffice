# Publipostage, enveloppes et étiquettes
<!-- keywords: publipostage, fusion, mailings, destinataires, liste d'adresses, lettre type, étiquettes, enveloppes, champ de fusion, bloc d'adresse, formule d'appel, règles, si alors sinon, ignorer l'enregistrement, enregistrement suivant, aperçu des résultats, terminer et fusionner, annuaire, messages électroniques, csv, xlsx, ods, vcard -->

Un **publipostage** crée de nombreuses copies d'une même lettre, chacune avec les coordonnées d'une personne différente. Vous rédigez la lettre une seule fois. Vous conservez les coordonnées des personnes dans une **liste**. JeffWrite place le nom et l'adresse de la première personne dans la première copie, ceux de la deuxième personne dans la deuxième copie, et ainsi de suite.

Les outils se trouvent dans l'onglet **Publipostage**. Pour suivre une procédure guidée, cliquez sur **Publipostage > Démarrer le publipostage > Assistant Publipostage pas à pas**.

## Choisir ce que vous créez

Cliquez sur **Publipostage > Démarrer le publipostage**, puis choisissez :

- **Lettres** : une copie de la page pour chaque personne. Chaque copie commence sur une nouvelle page.
- **Messages électroniques** : un message pour chaque personne. Voir « Envoyer des messages électroniques » ci-dessous.
- **Enveloppes** et **Étiquettes** : une page préparée pour ces supports. Voir les sections ci-dessous.
- **Annuaire** : une seule liste de tout le monde, sans saut de page entre les personnes. Utilisez-le pour une liste de téléphones ou un catalogue.
- **Effacer le type de fusion** redonne au document son caractère de document ordinaire.

JeffWrite enregistre dans le fichier le type de fusion et le nom de la liste lorsque vous enregistrez le document. Lorsque vous rouvrez le fichier, la fusion est toujours là.

## Choisir vos destinataires

Un **destinataire** est une personne qui reçoit une copie. Cliquez sur **Publipostage > Sélectionner les destinataires** :

- **Saisir une nouvelle liste** : saisissez les personnes dans la fenêtre qui s'ouvre. Cliquez sur **Nouvelle entrée** pour chacune. JeffWrite vous propose ensuite d'enregistrer la liste dans un fichier .csv.
- **Utiliser une liste existante** : choisissez un fichier qui est déjà une liste. Il peut s'agir d'un fichier .csv ou d'un fichier texte avec des tabulations (une ligne par personne, la première ligne contenant les noms des colonnes), d'une feuille de calcul (.xlsx, .xls ou .ods, créée avec JeffCrunch ou un autre programme), ou d'un fichier de contacts (.vcf).
- **Sélectionner dans les contacts** : choisissez un fichier de contacts (.vcf). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme.

Cliquez ensuite sur **Publipostage > Modifier la liste de destinataires** pour vérifier la liste :

- Décochez une ligne pour exclure une personne.
- Cliquez sur un en-tête de colonne pour trier.
- Saisissez dans **Filtrer les destinataires** pour n'afficher que les lignes contenant vos mots.
- **Rechercher les doublons** décoche les répétitions.
- **Valider les adresses** colore les lignes qui n'ont pas d'adresse ou dont le code postal semble incorrect.

## Placer les informations dans la lettre

Un **champ de fusion** est un emplacement réservé. Il affiche le nom d'une colonne entre chevrons, comme "«Prénom»". Dans chaque copie, il est remplacé par l'information de la personne.

Cliquez à l'endroit où l'information doit apparaître, puis utilisez **Publipostage > Rédiger et insérer des champs** :

- **Insérer un champ de fusion** affiche les colonnes de votre liste. Choisissez-en une.
  - **Code-barres à partir d'un champ**, dans la même liste, insère un code-barres dont la valeur provient d'une colonne, comme une référence d'article ou un ISBN à 13 chiffres. Choisissez la colonne, puis le type, et réglez l'aspect dans la fenêtre **Insérer un code-barres** (ce que vous saisissez là n'est qu'un exemple). La fenêtre n'propose que ce qu'un champ peut porter : le type, la taille, la hauteur des barres, les chiffres et le niveau du code QR. Elle omet le prix d'un livre, un complément, un caractère de contrôle et le fond clair, qu'un champ ne peut pas indiquer ; l'ISBN d'un livre est un EAN-13. Dans le document, le champ est dessiné comme un code-barres, avec un exemple du type tant que vous n'avez pas fusionné ; pour modifier le type ou l'aspect, double-cliquez dessus, ou faites un clic droit et choisissez **Modifier le code-barres**. Lors de la fusion, la copie de chaque personne reçoit un code-barres de sa propre valeur, dessiné comme une image dans la ligne ; il s'imprime et s'enregistre en PDF comme toute image. La valeur n'est que de la donnée : des guillemets ou une barre oblique inverse qu'elle contient ne changent rien. Si une valeur est vide, il n'y a pas de code-barres. Si un code ne peut pas contenir la valeur (une lettre dans un EAN-13, ou plus de 500 caractères dans le Code 128), la copie affiche à la place un court message entre crochets, sans effet sur les autres copies. Une fusion crée au plus 2 000 codes-barres, en comptant chaque valeur qui ne peut pas être codée ; si le message indique qu'il y en a trop, fusionnez moins de personnes. Dans un fichier `.docx` enregistré, le champ est un champ DISPLAYBARCODE contenant le nom de la colonne ; il est donc conservé lors de l'enregistrement et de la réouverture du fichier. Un champ dont la valeur a été saisie directement, ou d'un type que JeffWrite ne dessine pas (UPC-E, par exemple), reste tel quel. Les fusions de courriels insèrent la valeur sous forme de texte.
- **Bloc d'adresse** insère une adresse postale complète : le nom, la société, la rue et la ville. Choisissez les lignes souhaitées. Une ligne vide est omise. Vous pouvez aussi indiquer un pays à omettre, comme le vôtre. **Aperçu** affiche l'adresse de la première personne.
- **Formule d'appel** insère une ligne telle que « Cher Docteur Fenwick, ». Vous choisissez les mots. Le premier choix, « Cher Docteur Fenwick, », utilise le titre de la personne. Si une personne n'a pas de titre, la formule utilise à la place son prénom et son nom, comme dans « Chère Nora Pell, ». Si une personne n'a pas de nom, la formule est autre, par exemple « Madame, Monsieur, », que vous pouvez modifier.
- **Associer les champs** indique au bloc d'adresse et à la formule d'appel la colonne qui contient chaque élément, lorsque les noms de vos colonnes sont inhabituels.

## Règles

Une **règle** rend une copie différente pour certaines personnes. Cliquez sur **Règles** et choisissez :

- **Si…Alors…Sinon** : affiche un texte lorsqu'une colonne contient une valeur, et un autre texte lorsqu'elle est vide. Par exemple, si le département est « 75 », écrivez « Bienvenue à Paris ».
- **Ignorer l'enregistrement si** : exclut une personne lorsqu'une colonne satisfait un test que vous définissez, par exemple lorsque le nom de famille est vide.
- **Enregistrement suivant** et **Enregistrement suivant si** : utilisent les coordonnées de la personne suivante au même endroit. Les étiquettes s'en servent pour placer une personne différente sur chaque étiquette.
- **Numéro d'enregistrement de fusion** insère le numéro de la ligne de la personne dans la liste. **Numéro de séquence de fusion** compte les copies déjà créées.
- **Demander** et **Saisie** : vous posent une question lorsque vous terminez la fusion. Demander conserve la réponse sous un nom. Saisie insère la réponse dans le texte.
- **Définir un signet** : attribue une valeur à un nom. Un renvoi vers ce nom affiche la valeur.

## Aperçu des résultats

Cliquez sur **Publipostage > Aperçu des résultats**. Un volet s'ouvre et affiche la page pour une personne. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou saisissez un numéro. **Rechercher un destinataire** passe à la personne suivante dont une information contient vos mots.

**Mettre en surbrillance les champs de fusion** ombre en gris les champs de fusion de votre page afin de repérer leur emplacement. Les autres champs, comme les numéros de page, ne sont pas ombrés. L'ombrage s'affiche à l'écran et n'est jamais imprimé.

Votre document d'origine n'est pas modifié. **Vérifier les erreurs** vous indique lorsque la liste ne contient pas de colonne pour un champ, ou lorsque aucune personne n'est incluse.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer et fusionner** :

- **Modifier les documents individuels** crée un nouveau document contenant la copie de chaque personne. Vous pouvez le modifier avant de l'imprimer.
- **Imprimer les documents** crée ce document et ouvre la page **Imprimer**.
- **Enregistrer au format PDF** enregistre toutes les copies dans un seul fichier PDF.
- **Envoyer les messages électroniques** crée un fichier de message .eml pour chaque personne, dans un dossier que vous choisissez. Choisissez la colonne qui contient les adresses électroniques et saisissez un objet. JeffWrite n'envoie pas les messages. Ouvrez les fichiers dans votre messagerie.

Les personnes décochées dans la liste sont exclues.

Un fichier de message n'est créé que pour une personne dont l'adresse contient une seule adresse électronique. Si une adresse en contient deux (séparées par une virgule ou un point-virgule), comporte un nom, ou n'a pas de domaine, cette personne ne reçoit pas de message, et JeffWrite vous indique combien de personnes sont concernées. Un objet long est enregistré en plusieurs parties que les messageries rassemblent ensuite.

Une fenêtre de progression s'affiche pendant la fusion, et **Annuler** l'interrompt. Si une règle pose une question (Demander ou Saisie), vous êtes interrogé une fois pour chaque question différente, jusqu'à 20 questions. Ensuite, les autres utilisent leurs réponses par défaut. Si vous annulez une question, la fusion entière s'arrête. Une fusion s'arrête et le signale si elle devait produire plus de 10 000 pages, afin qu'un document très long avec une très longue liste ne prenne pas des heures. Fusionnez alors une partie de la liste à la fois.

## Enveloppes

Cliquez sur **Publipostage > Enveloppes**. Saisissez l'adresse du destinataire et votre adresse d'expéditeur, puis choisissez le format de l'enveloppe dans la liste. Les formats sont désignés par leur largeur et leur hauteur. Cliquez sur **Ajouter au document** pour placer l'enveloppe sur sa propre page, au début de votre document, ou sur **Imprimer** pour l'imprimer directement. La page a la largeur du grand côté de l'enveloppe, ce qui correspond à la façon dont la plupart des imprimantes chargent une enveloppe.

Pour créer des enveloppes pour une fusion, cliquez sur **Démarrer le publipostage > Enveloppes**. Chaque enveloppe comporte un bloc d'adresse.

## Étiquettes

Cliquez sur **Publipostage > Étiquettes**. Saisissez l'adresse et choisissez la planche dans la liste. Une planche est désignée par le nombre d'étiquettes qu'elle porte et par la taille de chaque étiquette. Si votre planche ne figure pas dans la liste, choisissez **Votre propre planche** et saisissez les dimensions : le nombre d'étiquettes en largeur et en hauteur, la taille d'une étiquette, l'espace au-dessus et à côté de la première étiquette, ainsi que l'écart entre les étiquettes.

- **Page entière de la même étiquette** place la même adresse sur chaque étiquette de la planche.
- **Étiquette unique** place l'adresse sur une seule étiquette, afin d'utiliser une planche déjà partiellement employée. Indiquez la ligne et la colonne.

Cliquez sur **Nouveau document** pour créer un document que vous pouvez consulter et modifier, ou sur **Imprimer** pour l'imprimer directement.

Pour créer des étiquettes pour une fusion, cliquez sur **Démarrer le publipostage > Étiquettes**. La première étiquette comporte un bloc d'adresse, et chaque autre étiquette commence par une règle **Enregistrement suivant**. Si vous modifiez la première étiquette, cliquez sur **Mettre à jour les étiquettes** pour reporter la modification sur toutes les autres.

## Contenu du fichier

Le type de fusion, le nom et l'emplacement de la liste, l'objet et la colonne des adresses électroniques sont enregistrés dans votre fichier .docx. Les personnes décochées et vos choix dans Associer les champs ne sont pas enregistrés ; ils vous seront de nouveau demandés après l'ouverture du fichier. Une liste est relue depuis son fichier à chaque fois. Si le fichier n'est plus à son emplacement, JeffWrite le cherche à côté de votre document, puis vous demande de le choisir.

Une liste peut comporter jusqu'à 1 000 colonnes et 4 000 000 de cellules au total, et une fusion utilise les 100 000 premières personnes. Si un fichier en contient davantage, JeffWrite utilise ce qui tient et vous en informe. Les lignes vides de la liste sont ignorées.

## Voir aussi

- [Utiliser des champs](fields)
- [Imprimer](print)
- [Créer et modifier des tableaux](tables)
