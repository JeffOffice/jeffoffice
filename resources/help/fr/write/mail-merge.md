# Fusion, enveloppes et étiquettes
<!-- keywords: fusion, publipostage, mailings, destinataires, liste d'adresses, lettre type, étiquettes, enveloppes, champ de fusion, bloc d'adresse, formule d'appel, règles, si alors sinon, ignorer l'enregistrement, enregistrement suivant, aperçu des résultats, terminer et fusionner, répertoire, messages électroniques, csv, xlsx, ods, vcard, carte de visite, publipostage, liste de diffusion, mailing, adresses, courriel -->

Un **publipostage** crée de nombreuses copies d'une même lettre, chacune contenant les coordonnées d'une personne différente. Vous rédigez la lettre une seule fois. Vous conservez les coordonnées des personnes dans une **liste**. JeffWrite place le nom et l'adresse de la première personne dans la première copie, ceux de la deuxième personne dans la deuxième copie, et ainsi de suite.

Les outils se trouvent sous l'onglet **Publipostage**. Pour suivre un parcours guidé, cliquez sur **Publipostage > Démarrer le publipostage > Assistant Publipostage pas à pas**.

## Choisir ce que vous créez

Cliquez sur **Publipostage > Démarrer le publipostage**, puis choisissez :

- **Lettres** : une copie de la page pour chaque personne. Chaque copie commence sur une nouvelle page.
- **Messages électroniques** : un message pour chaque personne. Voir « Envoyer des messages électroniques » ci-dessous.
- **Enveloppes** et **Étiquettes** : une page conçue pour ces supports. Voir les sections ci-dessous.
- **Répertoire** : une seule liste de tout le monde, sans saut de page entre les personnes. Utilisez-le pour un annuaire téléphonique ou un catalogue.
- **Effacer le type de fusion** redonne au document son caractère de document ordinaire.

JeffWrite conserve dans le fichier le type de fusion et le nom de la liste lorsque vous l'enregistrez. À la réouverture du fichier, la fusion est toujours là.

## Choisir vos destinataires

Un **destinataire** est une personne qui reçoit une copie. Cliquez sur **Publipostage > Sélectionner les destinataires** :

- **Saisir une nouvelle liste** : saisissez les personnes dans la fenêtre qui s'ouvre. Cliquez sur **Nouvelle entrée** pour chacune d'elles. JeffWrite vous propose alors d'enregistrer la liste sous forme de fichier .csv.
- **Utiliser une liste existante** : choisissez un fichier qui est déjà une liste. Il peut s'agir d'un fichier .csv ou d'un fichier texte avec des tabulations (une ligne par personne, la première ligne contenant les noms des colonnes), d'une feuille de calcul (.xlsx, .xls ou .ods, créée avec JeffCrunch ou un autre logiciel), ou d'un fichier de contacts (.vcf).
- **Sélectionner dans les contacts** : choisissez un fichier de contacts (.vcf). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme.

Cliquez ensuite sur **Publipostage > Modifier la liste de destinataires** pour vérifier la liste :

- Décochez une ligne pour exclure la personne correspondante.
- Cliquez sur un en-tête de colonne pour trier.
- Saisissez dans **Filtrer les destinataires** pour n'afficher que les lignes contenant vos mots.
- **Rechercher les doublons** décoche les répétitions.
- **Valider les adresses** colore les lignes qui n'ont pas d'adresse ou dont le code postal semble incorrect.

## Placer les coordonnées dans la lettre

Un **champ de fusion** est un emplacement réservé. Il affiche le nom d'une colonne entre guillemets français, par exemple « Prénom ». Dans chaque copie, il est remplacé par la valeur correspondante de la personne.

Cliquez à l'endroit où doit figurer la donnée, puis utilisez **Publipostage > Rédiger et insérer des champs** :

- **Insérer un champ de fusion** affiche les colonnes de votre liste. Choisissez-en une.
  - **Code-barres à partir du champ**, dans la même liste, insère un code-barres dont la valeur est une colonne, comme une référence d'article ou l'ISBN à 13 chiffres d'un livre. Choisissez la colonne, puis le type, et regardez dans la fenêtre **Insérer un code-barres** (ce que vous y tapez n'est qu'un exemple). La fenêtre n'propose que ce qu'un champ peut porter : le type, la taille, la hauteur des barres, les chiffres et le niveau du QR code. Elle omet le prix d'un livre, un code complémentaire, un caractère de contrôle et le fond clair, qu'un champ ne peut pas indiquer ; l'ISBN d'un livre est un EAN-13. Dans le document, le champ affiche le nom de la colonne entre guillemets français. Lors de la fusion, la copie de chaque personne reçoit un code-barres de sa propre valeur, dessiné comme une image dans la ligne ; il s'imprime et s'enregistre en PDF comme n'importe quelle image. La valeur n'est qu'une donnée : les guillemets ou une barre oblique inverse qu'elle contient ne changent rien. Si une valeur est vide, il n'y a pas de code-barres. Si un code ne peut pas contenir la valeur (une lettre dans un EAN-13, ou plus de 500 caractères en Code 128), la copie affiche à la place un court message entre crochets ; les autres copies ne sont pas touchées. Une fusion crée au maximum 2 000 codes-barres, y compris ceux dont la valeur ne peut pas être codée ; si le message indique qu'il y en a trop, fusionnez moins de personnes. Dans un fichier `.docx` enregistré, le champ est un champ DISPLAYBARCODE qui contient la colonne, de sorte qu'il est conservé à l'enregistrement et à la réouverture du fichier. Un champ dont la valeur est saisie, ou d'un type que JeffWrite ne dessine pas (UPC-E, par exemple), reste tel quel. Les fusions de messages électroniques insèrent la valeur sous forme de texte.
- **Bloc d'adresse** insère une adresse postale complète : le nom, la société, la rue et la ville. Choisissez les lignes souhaitées. Une ligne vide est omise. Vous pouvez aussi indiquer un pays à omettre, comme le vôtre. **Aperçu** affiche l'adresse de la première personne.
- **Formule d'appel** insère une formule telle que « Chère Dre Fenwick, ». Vous choisissez les termes. Le premier choix, « Chère Dre Fenwick, », utilise le titre de la personne. Si une personne n'a pas de titre, la formule utilise à la place son prénom et son nom, comme dans « Cher Nora Pell, ». Si une personne n'a pas de nom, la formule dit autre chose, comme « Madame, Monsieur, », que vous pouvez modifier.
- **Faire correspondre les champs** indique au bloc d'adresse et à la formule d'appel la colonne qui contient chaque élément, lorsque vos noms de colonnes sont inhabituels.

## Règles

Une **règle** modifie une copie pour certaines personnes. Cliquez sur **Règles** et choisissez :

- **Si…Alors…Sinon** : affiche un texte lorsqu'une colonne contient une valeur, et un autre texte lorsqu'elle est vide. Par exemple, si Région vaut « CA », écrivez « Bienvenue chez vous ».
- **Ignorer l'enregistrement si** : exclut une personne lorsqu'une colonne est vide, par exemple lorsque le nom de famille n'est pas renseigné.
- **Enregistrement suivant** et **Enregistrement suivant si** : utilisent les coordonnées de la personne suivante au même endroit. Les étiquettes s'en servent pour placer une personne différente sur chaque étiquette.
- **Numéro d'enregistrement de fusion** insère le numéro de la ligne de la personne dans la liste. **Numéro de séquence de fusion** compte les copies déjà créées.
- **Demander** et **Saisie** : vous posent une question à la fin de la fusion. Demander conserve la réponse sous un nom. Saisie insère la réponse dans le texte.
- **Définir un signet** : attribue une valeur à un nom. Un renvoi vers ce nom affiche cette valeur.

## Afficher l'aperçu des résultats

Cliquez sur **Publipostage > Aperçu des résultats**. Un volet s'ouvre et affiche la page pour une personne. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou saisissez un numéro. **Rechercher un destinataire** passe à la personne suivante dont une donnée contient vos mots.

**Mettre en surbrillance les champs de fusion** ombre en gris les champs de fusion de votre page afin de les repérer. Les autres champs, comme les numéros de page, ne sont pas ombrés. L'ombrage s'affiche à l'écran et n'est jamais imprimé.

Votre document d'origine n'est pas modifié. **Vérifier les erreurs** vous indique lorsque la liste ne comporte pas de colonne pour un champ, ou lorsque aucune personne n'est incluse.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer et fusionner** :

- **Modifier les documents individuels** crée un nouveau document contenant la copie de chaque personne. Vous pouvez le modifier avant de l'imprimer.
- **Imprimer les documents** crée ce document et ouvre la page **Imprimer**.
- **Enregistrer au format PDF** enregistre toutes les copies dans un seul fichier PDF.
- **Envoyer les messages électroniques** crée un fichier de message .eml pour chaque personne, dans un dossier de votre choix. Choisissez la colonne qui contient les adresses électroniques, puis saisissez un objet. JeffWrite n'envoie pas les messages. Ouvrez les fichiers dans votre messagerie.

Les personnes décochées dans la liste sont exclues.

Un fichier de message n'est créé que pour une personne dont l'adresse ne contient qu'une seule adresse électronique. Si une adresse en contient deux (séparées par une virgule ou un point-virgule), comporte un nom, ou n'a pas de domaine, cette personne ne reçoit aucun message, et JeffWrite vous indique combien il y en a. Un objet long est enregistré en plusieurs parties que les messageries reconstituent.

Une fusion affiche une fenêtre de progression, et **Annuler** l'interrompt. Si une règle pose une question (Demander ou Saisie), vous êtes interrogé une fois pour chaque question différente, jusqu'à 20 questions. Ensuite, les autres réponses prennent leurs valeurs par défaut. Si vous annulez une question, la fusion entière s'arrête. Une fusion s'arrête et le signale si elle produirait plus de 10 000 pages, afin qu'un très long document avec une très longue liste ne prenne pas des heures. Fusionnez alors une partie de la liste à la fois.

## Enveloppes

Cliquez sur **Publipostage > Enveloppes**. Saisissez l'adresse du destinataire et votre adresse, puis choisissez le format de l'enveloppe dans la liste. Les formats sont désignés par leur largeur et leur hauteur. Cliquez sur **Ajouter au document** pour placer l'enveloppe sur sa propre page, au début de votre document, ou sur **Imprimer** pour l'imprimer. La page a pour largeur le grand côté de l'enveloppe, ce que la plupart des imprimantes attendent pour une enveloppe.

Pour créer des enveloppes pour une fusion, cliquez sur **Démarrer le publipostage > Enveloppes**. Chaque enveloppe comporte un bloc d'adresse.

## Étiquettes

Cliquez sur **Publipostage > Étiquettes**. Saisissez l'adresse et choisissez la planche dans la liste. Une planche est désignée par le nombre d'étiquettes qu'elle comporte et par la taille de chaque étiquette. Si votre planche ne figure pas dans la liste, choisissez **Votre propre planche** et saisissez les dimensions : le nombre d'étiquettes en largeur et en hauteur, la taille d'une étiquette, l'espace au-dessus et à côté de la première étiquette, et la distance entre les étiquettes.

- **Page complète de la même étiquette** place la même adresse sur chaque étiquette de la planche.
- **Étiquette unique** place l'adresse sur une seule étiquette, afin d'utiliser une planche déjà partiellement employée. Indiquez la ligne et la colonne.

Cliquez sur **Nouveau document** pour créer un document que vous pouvez consulter et modifier, ou sur **Imprimer** pour l'imprimer.

Pour créer des étiquettes pour une fusion, cliquez sur **Démarrer le publipostage > Étiquettes**. La première étiquette comporte un bloc d'adresse, et chaque autre étiquette commence par une règle **Enregistrement suivant**. Si vous modifiez la première étiquette, cliquez sur **Mettre à jour les étiquettes** pour répercuter la modification sur toutes les autres.

## Contenu du fichier

Le type de fusion, le nom et l'emplacement de la liste, l'objet et la colonne des messages électroniques sont enregistrés dans votre fichier .docx. Les personnes que vous avez décochées, ainsi que vos choix dans Faire correspondre les champs, ne sont pas enregistrés. Ils vous seront demandés de nouveau après la réouverture du fichier. Une liste est relue depuis son fichier à chaque utilisation. Si le fichier n'est plus à son emplacement, JeffWrite le recherche à côté de votre document, puis vous demande de le choisir.

Une liste peut comporter jusqu'à 1 000 colonnes et 4 000 000 de cellules au total, et une fusion utilise les 100 000 premières personnes. Si un fichier en contient davantage, JeffWrite utilise ce qui peut l'être et vous en informe. Les lignes vides de la liste sont ignorées.

## Voir aussi

- [Utiliser les champs](fields)
- [Imprimer](print)
- [Créer et modifier des tableaux](tables)
