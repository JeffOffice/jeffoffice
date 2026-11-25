# Publipostage et fusion de catalogue
<!-- keywords: publipostage, fusion, destinataires, liste d'adresses, étiquettes, lettres types, csv, xlsx, vcard, champ de fusion, bloc d'adresse, catalogue, liste de produits, fusion par e-mail, aperçu -->

Le publipostage remplit une composition avec les informations d'une liste. Vous rédigez une lettre une seule fois, et JeffPub en crée une copie pour chaque personne, avec le nom et l'adresse de cette personne à la place des champs. La fusion de catalogue reprend une même mise en page pour chaque article d'une liste.

Les outils se trouvent dans l'onglet **Publipostage**. Pour suivre une démarche guidée, cliquez sur **Publipostage > Démarrer > Démarrer le publipostage > Assistant Publipostage pas à pas** pour ouvrir le volet **Publipostage**.

## Choisir vos destinataires

Cliquez sur **Publipostage > Démarrer > Sélectionner les destinataires** :

- **Utiliser une liste existante…** : choisissez un fichier CSV, un fichier texte à tabulations, un classeur .xlsx (la première feuille) ou un fichier de contacts .vcf. Dans un fichier CSV, texte ou tableur, la première ligne doit contenir les noms des colonnes.
- **Sélectionner dans les contacts…** : choisissez un fichier de contacts (.vcf ou .vcard). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts sous cette forme ; chaque contact devient un destinataire, avec des colonnes pour le nom, l'adresse, le téléphone et l'e-mail.
- **Saisir une nouvelle liste…** : remplissez la fenêtre **Nouvelle liste d'adresses**, en utilisant **Nouvelle entrée** pour chaque personne. JeffPub vous propose ensuite d'enregistrer la liste sous forme de fichier CSV.

Pour vérifier la liste, cliquez sur **Publipostage > Démarrer > Modifier la liste de destinataires**. Dans la fenêtre **Destinataires du publipostage**, décochez une ligne pour ignorer cette personne, cliquez sur un en-tête pour trier, et saisissez dans **Filtrer les destinataires** pour réduire la liste. **Distinguer les destinataires par** choisit une colonne qui différencie les personnes ; la liste est conservée avec ses lignes décochées dans votre composition, et lorsque vous sélectionnez de nouveau le même fichier de liste, les mêmes personnes sont à nouveau exclues, même si vous avez trié les lignes entre-temps. **Rechercher les doublons** décoche les lignes répétées, et **Valider les adresses** met en évidence les lignes dont l'adresse est manquante ou dont le code postal semble incorrect.

## Ajouter des champs de fusion

Un champ de fusion est un espace réservé, comme « Prénom », qui est remplacé par une valeur réelle pour chaque personne. Cliquez dans une zone de texte (JeffPub en ajoute une si vous n'en avez pas), puis utilisez **Publipostage > Écrire et insérer des champs** :

- **Bloc d'adresse** : une adresse postale construite à partir des colonnes portant des noms comme Prénom, Nom et Adresse ligne 1.
- **Formule d'appel** : « Cher » ou « Chère » suivi du nom de la personne, ou « Cher ami, » s'il n'y a pas de nom.
- **Insérer un champ de fusion** : choisissez n'importe quelle colonne.
- **Champ image** : choisissez une colonne contenant les noms de fichiers d'images. JeffPub ajoute un cadre vide et recherche les fichiers à côté de votre liste. Une fois la composition enregistrée puis rouverte, une image dont le fichier se trouve en dehors du dossier de la composition ne s'affiche pas tant que vous n'avez pas à nouveau choisi la liste de destinataires (**Publipostage > Sélectionner les destinataires > Utiliser une liste existante…**). Les images situées dans le dossier de la liste, et dans les dossiers qu'il contient, s'affichent alors jusqu'à la fermeture de la composition. JeffPub ne garde pas ce choix pour la fois suivante. Vous pouvez aussi placer les fichiers d'images dans le dossier de la composition, ou dans un dossier qu'il contient ; ils s'affichent dès son ouverture. Cela évite qu'une composition reçue d'un tiers lise des images de votre ordinateur à votre insu. L'image de chaque enregistrement s'affiche lors de l'aperçu des résultats, de l'impression, de l'enregistrement en PDF, ou de la fusion vers une nouvelle composition ou un catalogue.

## Afficher un nombre, un prix, une date ou une heure à votre façon

Un champ de fusion affiche la valeur telle qu'elle figure dans votre liste. Pour la modifier, cliquez à côté du champ (ou sélectionnez-le, ou sélectionnez plusieurs champs), puis cliquez sur **Publipostage > Écrire et insérer des champs > Mettre en forme le champ de fusion…**. Sous **Afficher la valeur sous forme de**, choisissez :

- **Nombre**, avec 0 à 4 **décimales**. Par exemple, 1234,5 avec 1 décimale s'affiche 1 234,5.
- **Monétaire**, avec le symbole monétaire de votre région : 1234,5 s'affiche 1 234,50 € dans une région où ce symbole est utilisé.
- **Pourcentage**, où 0,256 s'affiche 25,6 % avec 1 décimale.
- **Date**, dans un **Style** tel que Court, Long, Année, Mois, jour, Mois et jour, ou Mois et année.
- **Heure**, dans un **Style** Court, 12 heures ou 24 heures.

La ligne **Exemple** montre le résultat de votre choix. Les valeurs sont lues et affichées selon la langue du texte à l'emplacement du champ (anglais américain à défaut), et non selon la langue de votre ordinateur. En anglais américain, 03/04/2026 correspond au 4 mars, alors qu'en allemand ou en anglais britannique, il correspond au 3 avril ; en allemand, 1.234,5 est un nombre avec une virgule décimale. Une année à deux chiffres est lue de 2000 à 2029 pour 00 à 29, et de 1930 à 1999 pour 30 à 99. Un nombre entre parenthèses, comme (500), est négatif. Une valeur qui n'est ni un nombre, ni une date, ni une heure du type choisi s'affiche exactement comme dans votre liste : un mot, « nan » ou « inf » (une personne peut s'appeler Nan), une valeur vide, une heure avec un format de date, ou une date avec un format d'heure. L'aperçu et la fusion finale affichent tous deux la valeur mise en forme. Ce choix correspond à une étape d'annulation et il est enregistré avec la composition.

## Prévisualiser les résultats

Cliquez sur **Publipostage > Aperçu des résultats > Aperçu des résultats** pour voir les vraies informations à la place des champs. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les personnes, ou **Rechercher un destinataire** pour atteindre une correspondance.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer > Terminer et fusionner** :

- **Fusionner vers l'imprimante…** ouvre la page **Imprimer**. Cochez **Imprimer tous les enregistrements du publipostage**, puis imprimez.
- **Fusionner dans une nouvelle composition** crée une nouvelle composition contenant une copie de vos pages pour chaque personne.
- **Fusionner en PDF…** enregistre un seul PDF contenant les pages de chaque personne.
- **Fusionner vers la messagerie…** crée un fichier e-mail .eml par personne, avec la première page sous forme d'image. Choisissez la colonne de l'adresse e-mail, saisissez un objet et sélectionnez un dossier. JeffPub n'envoie pas les messages ; ouvrez les fichiers dans votre messagerie.
- **Enregistrer chacun dans un fichier…** crée une composition .jpub pour chaque personne, dans un dossier de votre choix. Nommez les fichiers à l'aide de colonnes entre accolades, par exemple {Last Name}, {First Name}. Une confirmation vous est demandée au-delà de 200 fichiers, et une fois si des fichiers portant les mêmes noms existent déjà. Cliquez sur **Annuler** pour arrêter entre deux fichiers.
- **Exporter la liste de destinataires…** enregistre la liste sous forme de fichier CSV.

## Fusion de catalogue

Cliquez sur **Insertion > Pages > Pages de catalogue** pour ouvrir le volet **Fusion de catalogue**.

1. Choisissez la liste de produits, comme indiqué plus haut.
2. Cliquez sur **Insérer une zone de catalogue**. Une zone étiquetée « Zone de catalogue » apparaît. Définissez son **Lignes** et ses **Colonnes**, ainsi que sa position et sa taille. Placez les champs d'un article dans la première cellule en double-cliquant dessus dans la liste ; ils se répètent dans les autres cellules. Choisissez la version « (image) » d'un champ pour insérer une image.
3. Prévisualisez avec les flèches. Chaque page affiche autant d'articles que la zone compte de cellules. Fusionnez ensuite vers une nouvelle composition, un PDF ou l'imprimante.

**Supprimer la zone de catalogue** retire la zone.

## Voir aussi

- [Imprimer une composition](printing)
- [Ajouter et mettre en forme des zones de texte](text-boxes)
- [L'onglet Publipostage](tab-mailings)
