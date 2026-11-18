# Publipostage, enveloppes et étiquettes
<!-- keywords: publipostage, fusion, mailings, destinataires, liste d'adresses, lettre type, étiquettes, enveloppes, champ de fusion, bloc d'adresse, formule d'appel, règles, si alors sinon, ignorer l'enregistrement, enregistrement suivant, aperçu des résultats, terminer et fusionner, répertoire, messages électroniques, csv, xlsx, ods, vcard, mailing, publipostage électronique -->

Un **publipostage** crée de nombreux exemplaires d'une même lettre, chacun contenant les coordonnées d'une personne différente. Vous rédigez la lettre une seule fois. Vous conservez les coordonnées des personnes dans une **liste**. JeffWrite place le nom et l'adresse de la première personne dans le premier exemplaire, ceux de la deuxième personne dans le deuxième exemplaire, et ainsi de suite.

Les outils se trouvent dans l'onglet **Publipostage**. Pour une marche à suivre guidée, cliquez sur **Publipostage > Démarrer le publipostage > Assistant Publipostage pas à pas**.

## Choisir ce que vous créez

Cliquez sur **Publipostage > Démarrer le publipostage**, puis choisissez :

- **Lettres** : un exemplaire de la page pour chaque personne. Chaque exemplaire commence sur une nouvelle page.
- **Messages électroniques** : un message pour chaque personne. Voir « Envoyer des messages électroniques » ci-dessous.
- **Enveloppes** et **Étiquettes** : une page conçue pour ces supports. Voir les sections ci-dessous.
- **Répertoire** : une liste unique de toutes les personnes, sans saut de page entre elles. Utilisez-le pour un annuaire téléphonique ou un catalogue.
- **Effacer le type de fusion** redonne au document son caractère de document ordinaire.

JeffWrite conserve dans le fichier le type de publipostage et le nom de la liste lorsque vous l'enregistrez. Lorsque vous rouvrez le fichier, le publipostage est toujours présent.

## Choisir vos destinataires

Un **destinataire** est une personne qui reçoit un exemplaire. Cliquez sur **Publipostage > Sélectionner les destinataires** :

- **Saisir une nouvelle liste** : saisissez les personnes dans la fenêtre qui s'ouvre. Cliquez sur **Nouvelle entrée** pour chacune d'elles. JeffWrite vous propose ensuite d'enregistrer la liste dans un fichier .csv.
- **Utiliser une liste existante** : choisissez un fichier qui est déjà une liste. Il peut s'agir d'un fichier .csv ou d'un fichier texte avec tabulations (une ligne par personne, la première ligne contenant les noms des colonnes), d'une feuille de calcul (.xlsx, .xls ou .ods, créée avec JeffCrunch ou un autre programme), ou d'un fichier de contacts (.vcf).
- **Sélectionner dans les contacts** : choisissez un fichier de contacts (.vcf). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme.

Cliquez ensuite sur **Publipostage > Modifier la liste de destinataires** pour vérifier la liste :

- Décochez une ligne pour exclure la personne correspondante.
- Cliquez sur un en-tête de colonne pour trier.
- Saisissez dans **Filtrer les destinataires** pour n'afficher que les lignes contenant vos mots.
- **Rechercher les doublons** décoche les répétitions.
- **Valider les adresses** colore les lignes qui n'ont pas d'adresse ou dont le code postal semble incorrect.

## Placer les coordonnées dans la lettre

Un **champ de fusion** est un espace réservé. Il affiche le nom d'une colonne entre doubles chevrons, par exemple «Prénom». Dans chaque exemplaire, il est remplacé par la donnée de la personne concernée.

Cliquez à l'endroit où la donnée doit figurer, puis utilisez **Publipostage > Écrire et insérer des champs** :

- **Insérer un champ de fusion** affiche les colonnes de votre liste. Choisissez-en une.
  - **Code-barres à partir d'un champ**, dans la même liste, insère un code-barres dont la valeur est celle d'une colonne, par exemple une référence de pièce ou l'ISBN à 13 chiffres d'un livre. Choisissez la colonne, puis le type, et réglez l'aspect dans la fenêtre **Insérer un code-barres** (ce que vous saisissez là n'est qu'un exemple). La fenêtre propose uniquement ce qu'un champ peut porter : le type, la taille, la hauteur des barres, les chiffres et le niveau du code QR. Elle exclut le prix d'un livre, un code complémentaire, un caractère de contrôle et le fond clair, que un champ ne peut pas décrire ; l'ISBN d'un livre est un EAN-13. Dans le document, le champ apparaît sous forme de code-barres, avec un exemple du type tant que vous n'avez pas effectué la fusion ; pour modifier le type ou l'aspect, double-cliquez dessus, ou faites un clic droit et choisissez **Modifier le code-barres**. Lors de la fusion, chaque exemplaire reçoit un code-barres correspondant à la valeur propre de la personne, dessiné sous forme d'image dans la ligne ; il s'imprime et s'enregistre en PDF comme toute image. La valeur n'est que une donnée : les guillemets ou une barre oblique inversée qu'elle contient ne changent rien. Si une valeur est vide, aucun code-barres n'est créé. Si un code ne peut pas contenir une valeur (une lettre dans un EAN-13, ou plus de 500 caractères dans le Code 128), l'exemplaire affiche à la place un court message entre crochets, sans incidence sur les autres exemplaires. Une fusion crée au maximum 2 000 codes-barres, en comptant chaque valeur qui ne peut pas être représentée ; si le programme indique qu'il y en a trop, fusionnez moins de personnes à la fois. Dans un fichier `.docx` enregistré, le champ est un champ CODEBARRE_AFFICHAGE contenant la colonne ; il est donc conservé à l'enregistrement et à la réouverture du fichier. Un champ dont la valeur est saisie directement, ou d'un type que JeffWrite ne dessine pas (UPC-E, par exemple), est conservé tel quel. Les fusions vers des messages électroniques insèrent la valeur sous forme de texte.
- **Bloc d'adresse** insère une adresse postale complète : le nom, la société, la rue et la ville. Choisissez les lignes souhaitées. Une ligne vide est omise. Vous pouvez aussi indiquer un pays à omettre, par exemple le vôtre. **Aperçu** affiche l'adresse de la première personne.
- **Formule d'appel** insère une formule telle que « Cher Dr Fenwick, ». Vous choisissez les termes. Le premier choix, « Cher Dr Fenwick, », utilise le titre de la personne. Si une personne n'a pas de titre, la formule utilise à la place son prénom et son nom, comme dans « Chère Nora Pell, ». Si une personne n'a pas de nom, la formule affiche une autre expression, comme « Madame, Monsieur, », que vous pouvez modifier.
- **Faire correspondre les champs** indique au bloc d'adresse et à la formule d'appel la colonne qui contient chaque élément, lorsque vos noms de colonnes sont inhabituels.

## Règles

Une **règle** rend un exemplaire différent pour certaines personnes. Cliquez sur **Règles** et choisissez :

- **Si…Alors…Sinon** : affiche un texte lorsqu'une colonne contient une valeur, et un autre texte lorsqu'elle est vide. Par exemple, si Région vaut « CA », écrivez « Bienvenue chez vous ».
- **Ignorer l'enregistrement si** : exclut une personne lorsqu'une colonne satisfait un test que vous définissez, par exemple lorsque le nom de famille est vide.
- **Enregistrement suivant** et **Enregistrement suivant si** : utilisent les coordonnées de la personne suivante à la même place. Les étiquettes s'en servent pour placer une personne différente sur chaque étiquette.
- **Numéro d'enregistrement de fusion** insère le numéro de la ligne de la personne dans la liste. **Numéro de séquence de fusion** compte les exemplaires déjà créés.
- **Demander** et **Saisie** : vous posent une question à la fin de la fusion. Demander conserve la réponse sous un nom. Saisie insère la réponse dans le texte.
- **Définir un signet** : attribue une valeur à un nom. Un renvoi vers ce nom affiche la valeur.

## Afficher un aperçu des résultats

Cliquez sur **Publipostage > Aperçu des résultats**. Un volet s'ouvre et affiche la page pour une personne. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou saisissez un numéro. **Rechercher un destinataire** passe à la personne suivante dont une donnée contient vos mots.

**Mettre en surbrillance les champs de fusion** ombre en gris les champs de fusion de votre page afin que vous puissiez les repérer. Les autres champs, comme les numéros de page, ne sont pas ombrés. Cet ombrage apparaît à l'écran et ne s'imprime jamais.

Votre document n'est pas modifié. **Vérifier les erreurs** vous signale lorsque la liste ne comporte pas de colonne correspondant à un champ, ou lorsque personne n'est inclus.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer et fusionner** :

- **Modifier les documents individuels** crée un nouveau document contenant l'exemplaire de chaque personne. Vous pouvez le modifier avant de l'imprimer.
- **Imprimer les documents** crée ce document et ouvre la page **Imprimer**.
- **Enregistrer au format PDF** enregistre tous les exemplaires dans un seul fichier PDF.
- **Envoyer des messages électroniques** crée un fichier de message .eml pour chaque personne, dans un dossier que vous choisissez. Choisissez la colonne qui contient les adresses e-mail et saisissez un objet. JeffWrite n'envoie pas les messages. Ouvrez les fichiers dans votre messagerie.

Les personnes que vous avez décochées dans la liste sont exclues.

Un fichier de message n'est créé que pour une personne dont l'adresse ne contient qu'une seule adresse e-mail. Si une adresse en contient deux (séparées par une virgule ou un point-virgule), comporte un nom, ou n'a pas de domaine, cette personne ne reçoit aucun message, et JeffWrite vous indique combien il y en a. Un objet long est enregistré en plusieurs parties que les messageries reconstituent.

Une fusion affiche une fenêtre de progression, et **Annuler** l'interrompt. Si une règle pose une question (Demander ou Saisie), vous êtes invité à répondre une fois à chaque question différente, jusqu'à 20 questions. Ensuite, les autres valeurs utilisent leur réponse par défaut. Si vous annulez une question, la fusion entière s'arrête. Une fusion s'arrête et l'indique si elle devait produire plus de 10 000 pages, afin qu'un document très long avec une liste très longue ne prenne pas des heures. Fusionnez alors une partie de la liste à la fois.

## Enveloppes

Cliquez sur **Publipostage > Enveloppes**. Saisissez l'adresse du destinataire et votre adresse de retour, puis choisissez le format de l'enveloppe dans la liste. Les formats sont désignés par leur largeur et leur hauteur. Cliquez sur **Ajouter au document** pour placer l'enveloppe sur sa propre page, au début de votre document, ou sur **Imprimer** pour l'imprimer. La page a la largeur du grand côté de l'enveloppe, ce qui correspond à la manière dont la plupart des imprimantes chargent les enveloppes.

Pour créer des enveloppes pour un publipostage, cliquez sur **Démarrer le publipostage > Enveloppes**. Chaque enveloppe contient un bloc d'adresse.

## Étiquettes

Cliquez sur **Publipostage > Étiquettes**. Saisissez l'adresse, puis choisissez la planche dans la liste. Une planche est désignée par le nombre d'étiquettes qu'elle contient et par la taille de chaque étiquette. Si votre planche ne figure pas dans la liste, choisissez **Votre propre planche** et saisissez les dimensions : le nombre d'étiquettes en largeur et en hauteur, la taille d'une étiquette, l'espace au-dessus et à côté de la première étiquette, et l'écart entre les étiquettes.

- **Page complète de la même étiquette** place la même adresse sur chaque étiquette de la planche.
- **Étiquette unique** place l'adresse sur une seule étiquette, ce qui vous permet d'utiliser une planche déjà partiellement employée. Indiquez la ligne et la colonne.

Cliquez sur **Nouveau document** pour créer un document que vous pouvez consulter et modifier, ou sur **Imprimer** pour l'imprimer.

Pour créer des étiquettes pour un publipostage, cliquez sur **Démarrer le publipostage > Étiquettes**. La première étiquette contient un bloc d'adresse, et chaque autre étiquette commence par une règle **Enregistrement suivant**. Si vous modifiez la première étiquette, cliquez sur **Mettre à jour les étiquettes** pour reporter la modification sur toutes les autres.

## Ce que contient le fichier

Le type de publipostage, le nom et l'emplacement de la liste, l'objet et la colonne des adresses e-mail sont enregistrés dans votre fichier .docx. Les personnes que vous avez décochées, ainsi que vos choix de Faire correspondre les champs, ne sont pas enregistrés : ils vous seront demandés après l'ouverture du fichier. Une liste est relue depuis son fichier à chaque fois. Si le fichier ne se trouve plus à son emplacement, JeffWrite le recherche à côté de votre document, puis vous demande de le choisir.

Une liste peut comporter jusqu'à 1 000 colonnes et 4 000 000 de cellules au total, et une fusion utilise les 100 000 premières personnes. Si un fichier en contient davantage, JeffWrite utilise ce qui peut être traité et vous en informe. Les lignes vides sont exclues de la liste.

## Voir aussi

- [Utiliser les champs](fields)
- [Imprimer](print)
- [Créer et modifier des tableaux](tables)
