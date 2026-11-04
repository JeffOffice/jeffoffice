# Publipostage, enveloppes et étiquettes
<!-- keywords: publipostage, fusion, mailings, destinataires, liste d'adresses, lettre type, étiquettes, enveloppes, champ de fusion, bloc d'adresse, formule d'appel, règles, si alors sinon, ignorer l'enregistrement, enregistrement suivant, aperçu des résultats, terminer et fusionner, annuaire, messages électroniques, csv, xlsx, ods, vcard -->

Un **publipostage** crée de nombreux exemplaires d'une même lettre, chacun comportant les coordonnées d'une personne différente. Vous rédigez la lettre une seule fois. Vous conservez les coordonnées des personnes dans une **liste**. JeffWrite place le nom et l'adresse de la première personne dans le premier exemplaire, ceux de la deuxième personne dans le deuxième exemplaire, et ainsi de suite.

Les outils se trouvent dans l'onglet **Publipostage**. Pour une procédure guidée, cliquez sur **Publipostage > Démarrer le publipostage > Assistant Publipostage pas à pas**.

## Choisir ce que vous créez

Cliquez sur **Publipostage > Démarrer le publipostage**, puis choisissez :

- **Lettres** : un exemplaire de la page pour chaque personne. Chaque exemplaire commence sur une nouvelle page.
- **Messages électroniques** : un message pour chaque personne. Voir « Envoyer des messages électroniques » ci-dessous.
- **Enveloppes** et **Étiquettes** : une page conçue pour ces supports. Voir les sections ci-dessous.
- **Annuaire** : une liste unique de toutes les personnes, sans saut de page entre elles. Utilisez-le pour un annuaire téléphonique ou un catalogue.
- **Supprimer le type de publipostage** redonne au document un aspect de document ordinaire.

JeffWrite conserve le type de publipostage et le nom de la liste dans le fichier lors de l'enregistrement. Lorsque vous rouvrez le fichier, le publipostage est toujours en place.

## Choisir vos destinataires

Un **destinataire** est une personne qui reçoit un exemplaire. Cliquez sur **Publipostage > Sélectionner les destinataires** :

- **Saisir une nouvelle liste** : saisissez les personnes dans la fenêtre qui s'ouvre. Cliquez sur **Nouvelle entrée** pour chacune d'elles. JeffWrite vous propose ensuite d'enregistrer la liste dans un fichier .csv.
- **Utiliser une liste existante** : choisissez un fichier qui est déjà une liste. Il peut s'agir d'un fichier .csv ou d'un fichier texte avec tabulations (une ligne par personne, la première ligne contenant les noms des colonnes), d'une feuille de calcul (.xlsx, .xls ou .ods, créée avec JeffCrunch ou un autre logiciel), ou d'un fichier de contacts (.vcf).
- **Sélectionner dans les contacts** : choisissez un fichier de contacts (.vcf). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme.

Cliquez ensuite sur **Publipostage > Modifier la liste de destinataires** pour vérifier la liste :

- Décochez une ligne pour exclure la personne correspondante.
- Cliquez sur un en-tête de colonne pour trier.
- Saisissez dans **Filtrer les destinataires** pour n'afficher que les lignes qui contiennent vos mots.
- **Rechercher les doublons** décoche les répétitions.
- **Valider les adresses** colore les lignes qui n'ont pas d'adresse ou dont le code postal semble incorrect.

## Placer les coordonnées dans la lettre

Un **champ de fusion** est un emplacement réservé. Il affiche le nom d'une colonne entre chevrons, par exemple «Prénom». Dans chaque exemplaire, il est remplacé par la coordonnée de la personne concernée.

Cliquez à l'endroit où doit figurer la coordonnée, puis utilisez **Publipostage > Rédiger et insérer des champs** :

- **Insérer un champ de fusion** affiche les colonnes de votre liste. Choisissez-en une.
- **Bloc d'adresse** insère une adresse postale complète : le nom, la société, la rue et la ville. Choisissez les lignes souhaitées. Une ligne vide est omise. Vous pouvez aussi indiquer un pays à omettre, par exemple le vôtre. L'**Aperçu** affiche l'adresse de la première personne.
- **Formule d'appel** insère une formule telle que « Cher Docteur Fenwick, ». Vous choisissez les termes. Le premier choix, « Cher Docteur Fenwick, », utilise le titre de la personne. Si une personne n'a pas de titre, la formule utilise à la place le prénom et le nom, comme dans « Chère Nora Pell, ». Si une personne n'a pas de nom, la formule contient une autre expression, telle que « Madame, Monsieur, », que vous pouvez modifier.
- **Associer les champs** indique au bloc d'adresse et à la formule d'appel la colonne qui contient chaque élément, lorsque les noms de vos colonnes sont inhabituels.

## Règles

Une **règle** rend un exemplaire différent pour certaines personnes. Cliquez sur **Règles**, puis choisissez :

- **Si…Alors…Sinon** : affiche un texte lorsqu'une colonne contient une valeur, et un autre texte lorsqu'elle est vide. Par exemple, si le département est « 75 », écrivez « Bienvenue à Paris ».
- **Ignorer l'enregistrement si** : exclut une personne lorsqu'une colonne contient une valeur, par exemple lorsque le nom de famille est vide.
- **Enregistrement suivant** et **Enregistrement suivant si** : utilisent les coordonnées de la personne suivante au même emplacement. Les étiquettes s'en servent pour placer une personne différente sur chaque étiquette.
- **Numéro d'enregistrement de fusion** insère le numéro de la ligne de la personne dans la liste. **Numéro de séquence de fusion** compte les exemplaires déjà créés.
- **Demander** et **Saisie** : vous posent une question à la fin de la fusion. Demander conserve la réponse sous un nom. Saisie insère la réponse dans le texte.
- **Définir un signet** : attribue une valeur à un nom. Un renvoi vers ce nom affiche la valeur.

## Prévisualiser les résultats

Cliquez sur **Publipostage > Aperçu des résultats**. Un volet s'ouvre et affiche la page pour une personne. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou saisissez un numéro. **Rechercher un destinataire** passe à la personne suivante dont une coordonnée contient vos mots.

**Mettre en surbrillance les champs de fusion** ombre en gris les champs de fusion de votre page afin que vous puissiez les repérer. Les autres champs, comme les numéros de page, ne sont pas ombrés. L'ombrage s'affiche à l'écran et n'est jamais imprimé.

Votre document n'est pas modifié. **Vérifier les erreurs** vous signale lorsque la liste ne contient aucune colonne pour un champ, ou lorsque personne n'est inclus.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer et fusionner** :

- **Modifier les documents individuels** crée un nouveau document contenant l'exemplaire de chaque personne. Vous pouvez le modifier avant de l'imprimer.
- **Imprimer les documents** crée ce document et ouvre la page **Imprimer**.
- **Enregistrer au format PDF** enregistre tous les exemplaires dans un seul fichier PDF.
- **Envoyer les messages électroniques** crée un fichier de message .eml pour chaque personne, dans un dossier de votre choix. Choisissez la colonne qui contient les adresses e-mail, puis saisissez un objet. JeffWrite n'envoie pas les messages. Ouvrez les fichiers dans votre messagerie.

Les personnes que vous avez décochées dans la liste sont exclues.

Un fichier de message n'est créé que pour une personne dont l'adresse contient une seule adresse e-mail. Si une adresse en contient deux (séparées par une virgule ou un point-virgule), comporte un nom ou n'a pas de domaine, cette personne ne reçoit aucun message, et JeffWrite vous indique combien il y en a. Un objet long est enregistré en plusieurs parties que les messageries reconstituent.

Une fusion affiche une fenêtre de progression, et **Annuler** l'interrompt. Si une règle pose une question (Demander ou Saisie), chaque question différente vous est posée une seule fois, jusqu'à 20 questions. Ensuite, les autres valeurs prennent leur réponse par défaut. Si vous annulez une question, toute la fusion s'arrête. Une fusion s'arrête et le signale si elle devait produire plus de 10 000 pages, afin qu'un document très long associé à une liste très longue ne prenne pas des heures. Fusionnez alors une partie de la liste à la fois.

## Enveloppes

Cliquez sur **Publipostage > Enveloppes**. Saisissez l'adresse du destinataire et votre adresse, puis choisissez le format de l'enveloppe dans la liste. Les formats sont désignés par leur largeur et leur hauteur. Cliquez sur **Ajouter au document** pour placer l'enveloppe sur sa propre page, au début de votre document, ou sur **Imprimer** pour l'imprimer directement. La page a la largeur du grand côté de l'enveloppe, ce qui correspond à la manière dont la plupart des imprimantes chargent une enveloppe.

Pour créer des enveloppes dans une fusion, cliquez sur **Démarrer le publipostage > Enveloppes**. Chaque enveloppe comporte un bloc d'adresse.

## Étiquettes

Cliquez sur **Publipostage > Étiquettes**. Saisissez l'adresse et choisissez la planche dans la liste. Une planche est désignée par le nombre d'étiquettes qu'elle contient et par la taille de chaque étiquette. Si votre planche ne figure pas dans la liste, choisissez **Votre propre planche** et saisissez les dimensions : le nombre d'étiquettes en largeur et en hauteur, la taille d'une étiquette, l'espace au-dessus et à côté de la première étiquette, et la distance entre les étiquettes.

- **Page entière de la même étiquette** place la même adresse sur chaque étiquette de la planche.
- **Étiquette unique** place l'adresse sur une seule étiquette, ce qui vous permet d'utiliser une planche déjà partiellement utilisée. Indiquez la ligne et la colonne.

Cliquez sur **Nouveau document** pour créer un document que vous pouvez consulter et modifier, ou sur **Imprimer** pour l'imprimer.

Pour créer des étiquettes dans une fusion, cliquez sur **Démarrer le publipostage > Étiquettes**. La première étiquette comporte un bloc d'adresse, et chaque autre étiquette commence par une règle **Enregistrement suivant**. Si vous modifiez la première étiquette, cliquez sur **Mettre à jour les étiquettes** pour reporter la modification sur toutes les autres.

## Contenu du fichier

Le type de publipostage, le nom et l'emplacement de la liste, l'objet et la colonne des adresses e-mail sont enregistrés dans votre fichier .docx. Les personnes que vous avez décochées, ainsi que vos choix dans Associer les champs, ne sont pas enregistrés : ils vous seront demandés à nouveau après l'ouverture du fichier. Une liste est relue depuis son fichier à chaque fois. Si le fichier n'est plus à son emplacement, JeffWrite le cherche à côté de votre document, puis vous demande de le sélectionner.

Une liste peut comporter jusqu'à 1 000 colonnes et 4 000 000 de cellules au total, et une fusion utilise les 100 000 premières personnes. Si un fichier en contient davantage, JeffWrite utilise ce qui peut être traité et vous en informe. Les lignes vides sont exclues de la liste.

## Voir aussi

- [Utiliser les champs](fields)
- [Imprimer](print)
- [Créer et modifier des tableaux](tables)
