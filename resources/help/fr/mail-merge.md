# Fusion de publipostage et fusion de catalogue
<!-- keywords: publipostage, fusion, destinataires, liste d'adresses, étiquettes, lettres types, csv, xlsx, vcard, champ de fusion, bloc d'adresse, catalogue, liste de produits, fusion par e-mail, aperçu -->

Le publipostage remplit une composition avec des informations provenant d'une liste. Vous rédigez une lettre une seule fois, et JeffPub en crée une copie pour chaque personne, avec son nom et son adresse à la place des champs. Une fusion de catalogue répète une même mise en page pour chaque article d'une liste.

Les outils se trouvent dans l'onglet **Publipostage**. Pour suivre une procédure guidée, cliquez sur **Publipostage > Démarrer le publipostage > Assistant Publipostage pas à pas** pour ouvrir le volet **Publipostage**.

## Choisir les destinataires

Cliquez sur **Publipostage > Démarrer le publipostage > Sélectionner les destinataires** :

- **Utiliser une liste existante…** : choisissez un fichier CSV, un fichier texte séparé par des tabulations, un classeur .xlsx (la première feuille) ou un fichier de contacts .vcf. Dans un fichier CSV, texte ou tableur, la première ligne doit contenir les noms des colonnes.
- **Sélectionner dans les contacts…** : choisissez un fichier de contacts (.vcf ou .vcard). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme ; chaque contact devient un destinataire, avec des colonnes pour le nom, l'adresse, le téléphone et l'e-mail.
- **Saisir une nouvelle liste…** : remplissez la fenêtre **Nouvelle liste d'adresses**, en utilisant **Nouvelle entrée** pour chaque personne. JeffPub vous propose ensuite d'enregistrer la liste dans un fichier CSV.

Pour vérifier la liste, cliquez sur **Publipostage > Démarrer le publipostage > Modifier la liste de destinataires**. Dans la fenêtre **Destinataires du publipostage**, décochez une ligne pour ignorer une personne, cliquez sur un en-tête pour trier, et saisissez dans **Filtrer les destinataires** pour réduire la liste. **Rechercher les doublons** décoche les répétitions, et **Valider les adresses** met en évidence les lignes dont l'adresse est absente ou dont le code postal semble incorrect.

## Ajouter des champs de fusion

Un champ de fusion est un espace réservé, comme « Prénom », qui est remplacé par une valeur réelle pour chaque personne. Cliquez dans une zone de texte (JeffPub en ajoute une si vous n'en avez pas), puis utilisez **Publipostage > Écrire et insérer des champs** :

- **Bloc d'adresse** : une adresse postale construite à partir de colonnes nommées par exemple Prénom, Nom et Adresse ligne 1.
- **Formule d'appel** : « Dear » suivi du nom de la personne, ou « Dear Friend, » si aucun nom n'est indiqué.
- **Insérer un champ de fusion** : choisissez n'importe quelle colonne.
- **Champ d'image** : choisissez une colonne contenant des noms de fichiers image. JeffPub ajoute un cadre vide et recherche les fichiers à côté de votre liste. Après avoir enregistré la composition puis l'avoir rouverte, une image dont le fichier se trouve en dehors du dossier de la composition ne s'affiche pas tant que vous n'avez pas à nouveau choisi la liste de destinataires (**Publipostage > Sélectionner les destinataires > Utiliser une liste existante…**). Les images du dossier de la liste, et de ses sous-dossiers, s'affichent alors jusqu'à la fermeture de la composition. JeffPub ne retient pas ce choix pour la prochaine fois. Vous pouvez aussi placer les fichiers image dans le dossier de la composition, ou dans un sous-dossier de celui-ci : ils s'afficheront dès l'ouverture. Cela évite qu'une composition que l'on vous envoie lise, de sa propre initiative, des images présentes sur votre ordinateur. L'image de chaque enregistrement s'affiche lorsque vous prévisualisez les résultats, imprimez, enregistrez un PDF, ou fusionnez vers une nouvelle composition ou un catalogue.

## Afficher un nombre, un prix, une date ou une heure à votre façon

Un champ de fusion affiche la valeur telle qu'elle figure dans votre liste. Pour la modifier, cliquez à côté du champ (ou sélectionnez-le, ou sélectionnez plusieurs champs), puis cliquez sur **Publipostage > Écrire et insérer des champs > Mettre en forme le champ de fusion…**. Sous **Afficher la valeur sous forme de**, choisissez :

- **Nombre**, avec de 0 à 4 **Décimales**. Par exemple, 1234,5 avec 1 décimale s'affiche 1 234,5.
- **Monétaire**, avec le symbole monétaire de votre région : 1234,5 s'affiche 1 234,50 € si tel est le symbole de votre région.
- **Pourcentage**, où 0,256 s'affiche 25,6 % avec 1 décimale.
- **Date**, dans un **Style** tel que Court, Long, Année, Mois, jour, Mois et jour, ou Mois et année.
- **Heure**, dans un **Style** Court, sur 12 heures ou sur 24 heures.

La ligne **Exemple** montre le résultat de votre choix. Les valeurs sont lues et affichées selon la langue du texte à l'emplacement du champ (anglais américain si aucune langue n'est définie), et non selon la langue de votre ordinateur. En anglais américain, 03/04/2026 correspond au 4 mars ; dans une langue où le jour précède le mois, comme l'allemand ou l'anglais britannique, il correspond au 3 avril. En allemand, 1.234,5 est un nombre avec une virgule décimale. Une année sur deux chiffres est lue de 2000 à 2029 pour 00 à 29, et de 1930 à 1999 pour 30 à 99. Un nombre entre parenthèses, comme (500), est négatif. Une valeur qui n'est ni un nombre, ni une date, ni une heure du type choisi s'affiche exactement comme dans votre liste : un mot, « nan » ou « inf » (une personne peut s'appeler Nan), une valeur vide, une heure avec un format de date, ou une date avec un format d'heure. L'aperçu et la fusion finale affichent tous deux la valeur mise en forme. Ce choix constitue une étape d'Annuler, et il est enregistré avec la composition.

## Prévisualiser les résultats

Cliquez sur **Publipostage > Aperçu des résultats > Aperçu des résultats** pour voir les vraies informations à la place des champs. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou **Rechercher un destinataire** pour aller directement à une correspondance.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer > Terminer et fusionner** :

- **Fusionner vers l'imprimante…** ouvre la page **Imprimer**. Cochez **Imprimer tous les enregistrements du publipostage**, puis imprimez.
- **Fusionner dans une nouvelle composition** crée une nouvelle composition contenant une copie de vos pages pour chaque personne.
- **Fusionner en PDF…** enregistre un seul PDF contenant les pages de toutes les personnes.
- **Fusionner vers la messagerie…** crée un fichier e-mail .eml par personne, avec la première page sous forme d'image. Choisissez la colonne d'e-mail, saisissez un objet, puis choisissez un dossier. JeffPub n'envoie pas les messages : ouvrez les fichiers dans votre programme de messagerie.
- **Exporter la liste de destinataires…** enregistre la liste dans un fichier CSV.

## Fusion de catalogue

Cliquez sur **Insertion > Pages > Pages de catalogue** pour ouvrir le volet **Fusion de catalogue**.

1. Choisissez la liste de produits, comme indiqué ci-dessus.
2. Cliquez sur **Insérer une zone de catalogue**. Une zone étiquetée « Zone de catalogue » apparaît. Réglez ses **Lignes** et ses **Colonnes**, ainsi que sa position et sa taille. Placez les champs d'un article dans la première cellule en double-cliquant dessus dans la liste ; ils se répètent dans les autres cellules. Choisissez la version « (image) » d'un champ pour une image.
3. Prévisualisez avec les flèches. Chaque page affiche autant d'articles que la zone comporte de cellules. Fusionnez ensuite vers une nouvelle composition, un PDF ou l'imprimante.

**Supprimer la zone de catalogue** retire la zone.

## Voir aussi

- [Imprimer une composition](printing)
- [Ajouter et mettre en forme des zones de texte](text-boxes)
- [L'onglet Publipostage](tab-mailings)
