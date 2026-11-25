# Publipostage, enveloppes et étiquettes
<!-- keywords: publipostage, fusion, mailings, destinataires, liste d'adresses, lettre type, étiquettes, enveloppes, champ de fusion, bloc d'adresse, formule d'appel, règles, si alors sinon, ignorer l'enregistrement, enregistrement suivant, aperçu des résultats, terminer et fusionner, répertoire, messages électroniques, csv, xlsx, ods, vcard, carte de visite, contacts, code-barres -->

Un **publipostage** crée de nombreux exemplaires d'une même lettre, chacun contenant les coordonnées d'une personne différente. Vous rédigez la lettre une seule fois. Vous conservez les coordonnées des personnes dans une **liste**. JeffWrite place le nom et l'adresse de la première personne dans le premier exemplaire, ceux de la deuxième personne dans le deuxième exemplaire, et ainsi de suite.

Les outils se trouvent sous l'onglet **Publipostage**. Pour un parcours guidé, cliquez sur **Publipostage > Démarrer le publipostage > Assistant Publipostage pas à pas**.

## Choisir ce que vous créez

Cliquez sur **Publipostage > Démarrer le publipostage**, puis choisissez :

- **Lettres** : un exemplaire de la page pour chaque personne. Chaque exemplaire commence sur une nouvelle page.
- **Messages électroniques** : un message pour chaque personne. Voir « Envoyer des messages électroniques » ci-dessous.
- **Enveloppes** et **Étiquettes** : une page conçue pour ces supports. Voir les sections ci-dessous.
- **Répertoire** : une liste de tout le monde, sans saut de page entre les personnes. Utilisez-le pour un annuaire téléphonique ou un catalogue.
- **Effacer le type de fusion** redonne au document son caractère de document ordinaire.

JeffWrite conserve dans le fichier, à l'enregistrement, le type de fusion et le nom de la liste. Lorsque vous rouvrez le fichier, le publipostage est toujours là.

## Choisir vos destinataires

Un **destinataire** est une personne qui reçoit un exemplaire. Cliquez sur **Publipostage > Sélectionner les destinataires** :

- **Saisir une nouvelle liste** : saisissez les personnes dans la fenêtre qui s'ouvre. Cliquez sur **Nouvelle entrée** pour chacune. JeffWrite vous propose ensuite d'enregistrer la liste dans un fichier .csv.
- **Utiliser une liste existante** : choisissez un fichier qui est déjà une liste. Il peut s'agir d'un fichier .csv ou d'un fichier texte avec des tabulations (une ligne par personne, la première ligne contenant les noms des colonnes), d'une feuille de calcul (.xlsx, .xls ou .ods, issue de JeffCrunch ou d'un autre programme), ou d'un fichier de contacts (.vcf).
- **Sélectionner dans les contacts** : choisissez un fichier de contacts (.vcf). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme.

Cliquez ensuite sur **Publipostage > Modifier la liste de destinataires** pour vérifier la liste :

- Décochez une ligne pour exclure la personne correspondante.
- Cliquez sur un en-tête de colonne pour trier.
- Saisissez dans **Filtrer les destinataires** pour n'afficher que les lignes contenant vos mots.
- **Distinguer les destinataires par** choisit la colonne qui permet de différencier une personne d'une autre, par exemple une adresse e-mail. Lorsque vous rouvrez le document et que la liste est relue, les personnes décochées restent exclues, même si la liste a un ordre différent ou de nouvelles personnes. Avec **Placer dans la liste**, elles sont repérées par le numéro de leur ligne dans le fichier de liste, même si vous avez trié ou filtré les lignes dans cette fenêtre. Si la liste a changé au point que cela ne correspond plus, JeffWrite vous en avertit et inclut toutes les personnes qu'il n'a pas pu faire correspondre. Le document conserve une forme codée de chaque adresse, et non l'adresse elle-même ; toutefois, une personne qui possède le document et devine une adresse peut tester cette supposition. **Inspecter le document** (Fichier > Infos) supprime la trace des personnes exclues.
- **Rechercher les doublons** décoche les répétitions.
- **Valider les adresses** colore les lignes sans adresse ou dont le code postal semble incorrect.

## Placer les coordonnées dans la lettre

Un **champ de fusion** est un espace réservé. Il affiche le nom d'une colonne entre chevrons, par exemple «Prénom». Dans chaque exemplaire, il est remplacé par la donnée de la personne concernée.

Cliquez à l'endroit où doit figurer la donnée, puis utilisez **Publipostage > Rédiger et insérer des champs** :

- **Insérer un champ de fusion** affiche les colonnes de votre liste. Choisissez-en une.
  - **Code-barres à partir d'un champ**, dans la même liste, insère un code-barres dont la valeur est celle d'une colonne, par exemple une référence de pièce ou l'ISBN à 13 chiffres d'un livre. Choisissez la colonne, puis le type, et réglez l'aspect dans la fenêtre **Insérer un code-barres** (ce que vous saisissez dans cette fenêtre n'est qu'un exemple). La fenêtre ne propose que ce qu'un champ peut porter : le type, la taille, la hauteur des barres, les chiffres et le niveau du QR code. Elle exclut le prix d'un livre, un code complémentaire, un caractère de contrôle et le fond clair, qu'un champ ne peut pas indiquer ; l'ISBN d'un livre est un EAN-13. Dans le document, le champ est dessiné sous forme de code-barres, avec un exemple du type choisi jusqu'à la fusion ; pour changer le type ou l'aspect, double-cliquez dessus, ou faites un clic droit et choisissez **Modifier le code-barres**. Lors de la fusion, l'exemplaire de chaque personne reçoit un code-barres de sa propre valeur, dessiné comme une image dans la ligne ; il s'imprime et s'enregistre en PDF comme toute image. La valeur n'est qu'une donnée : les guillemets ou une barre oblique inverse qu'elle contient ne changent rien. Si une valeur est vide, il n'y a pas de code-barres. Si un code ne peut pas contenir la valeur (une lettre dans un EAN-13, ou plus de 500 caractères dans un Code 128), l'exemplaire affiche à la place un court message entre crochets, et les autres exemplaires ne sont pas touchés. Une fusion crée au plus 2 000 codes-barres, en comptant chaque valeur qui ne peut pas être codée ; si le programme signale qu'il y en a trop, fusionnez moins de personnes à la fois. Dans un fichier `.docx` enregistré, le champ est un champ DISPLAYBARCODE contenant la colonne, et il est conservé lors de l'enregistrement puis de la réouverture du fichier. Un champ auquel une valeur fixe a été donnée, ou d'un type que JeffWrite ne dessine pas (UPC-E, par exemple), reste tel quel. Les fusions par e-mail insèrent la valeur sous forme de texte.
  - **Bloc d'adresse** insère une adresse postale complète : le nom, la société, la rue et la ville. Choisissez les lignes à afficher. Une ligne vide est omise. Vous pouvez aussi indiquer un pays à omettre, par exemple le vôtre. L'**Aperçu** affiche l'adresse de la première personne.
  - **Formule d'appel** insère une formule telle que « Chère Madame Fenwick, ». Vous choisissez les termes. Le premier choix, « Chère Madame Fenwick, », utilise le titre de la personne. Si une personne n'a pas de titre, la formule utilise le prénom et le nom, par exemple « Chère Nora Pell, ». Si une personne n'a pas de nom, la formule est différente, par exemple « Madame, Monsieur, », que vous pouvez modifier.
  - **Associer les champs** indique au bloc d'adresse et à la formule d'appel quelle colonne contient chaque élément, lorsque vos noms de colonnes sont inhabituels.

## Règles

Une **règle** rend un exemplaire différent pour certaines personnes. Cliquez sur **Règles** et choisissez :

- **Si…Alors…Sinon** : afficher un texte lorsqu'une colonne contient une valeur, et un autre texte lorsqu'elle est vide. Par exemple, si Région vaut « CA », écrire « Bienvenue chez vous ».
- **Ignorer l'enregistrement si** : exclure une personne lorsqu'une colonne satisfait un test que vous définissez, par exemple lorsque le nom de famille est vide.
- **Enregistrement suivant** et **Enregistrement suivant si** : utiliser les coordonnées de la personne suivante au même emplacement. Les étiquettes s'en servent pour placer une personne différente sur chaque étiquette.
- **Numéro d'enregistrement de fusion** insère le numéro de la ligne de la personne dans la liste. **Numéro de séquence de fusion** compte les exemplaires déjà créés.
- **Demander** et **Saisie** : vous poser une question à la fin de la fusion. Demander conserve la réponse sous un nom. Saisie insère la réponse dans le texte.
- **Définir un signet** : attribue une valeur à un nom. Un renvoi vers ce nom affiche la valeur.

## Prévisualiser les résultats

Cliquez sur **Publipostage > Aperçu des résultats**. Un volet s'ouvre et affiche la page pour une personne. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou saisissez un numéro. **Rechercher un destinataire** passe à la personne suivante dont une donnée contient vos mots.

**Mettre en surbrillance les champs de fusion** grise les champs de fusion de votre page afin de les repérer. Les autres champs, comme les numéros de page, ne sont pas grisés. Ce grisé s'affiche à l'écran et n'est jamais imprimé.

Votre document n'est pas modifié. **Vérifier les erreurs** vous signale lorsque la liste ne comporte pas de colonne pour un champ, ou lorsque aucune personne n'est incluse.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer et fusionner** :

- **Modifier les documents individuels** crée un nouveau document contenant l'exemplaire de chaque personne. Vous pouvez le modifier avant de l'imprimer.
- **Imprimer les documents** crée ce document et ouvre la page **Imprimer**.
- **Enregistrer au format PDF** enregistre tous les exemplaires dans un seul fichier PDF.
- **Envoyer des messages électroniques** crée un fichier de message .eml pour chaque personne, dans un dossier que vous choisissez. Choisissez la colonne qui contient les adresses e-mail, et saisissez un objet. JeffWrite n'envoie pas les messages. Ouvrez les fichiers dans votre messagerie.
- **Enregistrer chacun dans un fichier** crée un fichier pour chaque personne, dans un dossier que vous choisissez, au format .docx ou PDF. Nommez les fichiers à l'aide de colonnes entre accolades, par exemple {Nom}, {Prénom} ; un nom utilisé deux fois reçoit un numéro. Une boîte affiche les prénoms et le nombre de fichiers qui seront créés. Une confirmation est demandée au-delà de 200 fichiers, et une fois si des fichiers portant les mêmes noms existent déjà dans le dossier. Cliquez sur **Annuler** pour arrêter entre deux fichiers : les fichiers déjà créés sont conservés ; fermer la fenêtre a le même effet. Un fichier qui est un lien vers un autre fichier est remplacé par un nouveau fichier, et le fichier vers lequel il pointait n'est pas modifié. Les fichiers ne contiennent que la lettre d'une personne : ils ne contiennent ni la liste, ni son emplacement, ni la liste des personnes exclues. Une fusion crée au plus 100 000 fichiers.

Les personnes décochées dans la liste sont exclues.

Un fichier de message n'est créé que pour une personne dont l'adresse est une seule adresse e-mail. Si une adresse en contient deux (séparées par une virgule ou un point-virgule), comporte un nom, ou n'a pas de domaine, cette personne ne reçoit aucun message, et JeffWrite indique combien il y en a. Un objet long est enregistré en plusieurs parties que les programmes de messagerie reconstituent.

Une fusion affiche une fenêtre de progression, et **Annuler** l'interrompt. Si une règle pose une question (Demander ou Saisie), vous êtes interrogé une fois pour chaque question différente, jusqu'à 20 questions. Ensuite, les autres prennent leurs réponses par défaut. Si vous annulez une question, la fusion entière s'arrête. Une fusion s'arrête, en le signalant, si elle devait produire plus de 10 000 pages, afin qu'un très long document avec une très longue liste ne prenne pas des heures. Fusionnez alors la liste par parties.

## Enveloppes

Cliquez sur **Publipostage > Enveloppes**. Saisissez l'adresse du destinataire et votre adresse d'expédition, puis choisissez le format de l'enveloppe dans la liste. Les formats sont désignés par leur largeur et leur hauteur. Cliquez sur **Ajouter au document** pour placer l'enveloppe sur sa propre page, au début de votre document, ou sur **Imprimer** pour l'imprimer directement. La page a la largeur du grand côté de l'enveloppe, ce qui correspond à la façon dont la plupart des imprimantes prennent une enveloppe.

Pour créer des enveloppes pour une fusion, cliquez sur **Publipostage > Démarrer le publipostage > Enveloppes**. Chaque enveloppe comporte un bloc d'adresse.

## Étiquettes

Cliquez sur **Publipostage > Étiquettes**. Saisissez l'adresse et choisissez la planche dans la liste. Une planche est désignée par le nombre d'étiquettes qu'elle porte et par la taille de chaque étiquette. Si votre planche ne figure pas dans la liste, choisissez **Votre propre planche** et saisissez les dimensions : le nombre d'étiquettes en largeur et en hauteur, la taille d'une étiquette, l'espace au-dessus et à côté de la première étiquette, et l'écart entre les étiquettes. Ces dimensions sont exprimées en centimètres (ou en millimètres pour les petites valeurs).

- **Page complète de la même étiquette** place la même adresse sur chaque étiquette de la planche.
- **Étiquette unique** place l'adresse sur une seule étiquette, afin d'utiliser une planche partiellement employée. Indiquez la ligne et la colonne.

Cliquez sur **Nouveau document** pour créer un document que vous pouvez consulter et modifier, ou sur **Imprimer** pour l'imprimer.

Pour créer des étiquettes pour une fusion, cliquez sur **Publipostage > Démarrer le publipostage > Étiquettes**. La première étiquette comporte un bloc d'adresse, et chaque autre étiquette commence par une règle **Enregistrement suivant**. Si vous modifiez la première étiquette, cliquez sur **Actualiser les étiquettes** pour copier la modification sur toutes les autres.

## Contenu du fichier

Le type de fusion, le nom et l'emplacement de la liste, l'objet et la colonne des adresses e-mail sont enregistrés dans votre fichier .docx. Les personnes que vous avez décochées le sont aussi, sous forme de codes abrégés de la colonne choisie, et non sous forme de noms ou d'adresses (seul un fichier .docx conserve cette information ; Vérifier la compatibilité le signale pour les autres formats). Vos choix dans Associer les champs sont également enregistrés. Une liste est relue depuis son fichier à chaque ouverture. Si le fichier ne se trouve plus à son emplacement, JeffWrite le cherche à côté de votre document, puis vous demande de le choisir.

Une liste peut comporter jusqu'à 1 000 colonnes et 4 000 000 de cellules au total, et une fusion utilise les 100 000 premières personnes. Si un fichier en contient davantage, JeffWrite utilise ce qui peut l'être et vous en informe. Les lignes vides de la liste sont ignorées.

## Voir aussi

- [Utiliser les champs](fields)
- [Imprimer](print)
- [Créer et modifier des tableaux](tables)
