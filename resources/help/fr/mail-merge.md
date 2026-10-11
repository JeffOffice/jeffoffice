# Publipostage et fusion de catalogue
<!-- keywords: publipostage, fusion, destinataires, liste d'adresses, étiquettes, lettres types, mailing, csv, xlsx, vcard, champ de fusion, bloc d'adresse, catalogue, liste de produits, fusion et publipostage électronique, e-mail, aperçu -->

Le publipostage remplit une composition avec les informations d'une liste. Vous écrivez une lettre une seule fois, et JeffPub en crée une copie pour chaque personne, avec son nom et son adresse. Une fusion de catalogue répète une même disposition pour chaque élément d'une liste.

Les outils se trouvent dans l'onglet **Publipostage**. Pour être guidé pas à pas, cliquez sur **Publipostage > Démarrer > Publipostage > Assistant Publipostage pas à pas** pour ouvrir le volet **Publipostage**.

## Choisir les destinataires

Cliquez sur **Publipostage > Démarrer > Sélectionner les destinataires** :

- **Utiliser une liste existante…** : choisissez un fichier CSV, un fichier texte délimité par des tabulations, une feuille de calcul .xlsx (la première feuille) ou un fichier de contacts .vcf. Dans un fichier CSV, texte ou feuille de calcul, la première ligne doit contenir les noms des colonnes.
- **Sélectionner dans les contacts…** : choisissez un fichier de contacts (.vcf ou .vcard). La plupart des carnets d'adresses et des téléphones peuvent enregistrer leurs contacts dans ce format ; chaque contact devient un destinataire, avec des colonnes pour le nom, l'adresse, le téléphone et l'adresse e-mail.
- **Taper une nouvelle liste…** : remplissez la fenêtre **Nouvelle liste d'adresses** en utilisant **Nouvelle entrée** pour chaque personne. JeffPub propose ensuite d'enregistrer la liste dans un fichier CSV.

Pour vérifier la liste, cliquez sur **Publipostage > Démarrer > Modifier la liste de destinataires**. Dans la fenêtre **Destinataires du publipostage**, décochez une ligne pour exclure la personne correspondante, cliquez sur un en-tête de colonne pour trier et tapez du texte dans **Filtrer les destinataires** pour restreindre la liste. **Rechercher les doublons** décoche les entrées en double, et **Valider les adresses** met en évidence les lignes dont l'adresse est manquante ou dont le code postal semble incorrect.

## Ajouter des champs de fusion

Un champ de fusion est un espace réservé, par exemple «Prénom», qui est remplacé par la valeur réelle de chaque personne. Cliquez dans une zone de texte (JeffPub en ajoute une si vous ne le faites pas), puis utilisez **Publipostage > Champs d'écriture et d'insertion** :

- **Bloc d'adresse** : une adresse postale construite à partir de colonnes portant des noms tels que First Name, Last Name et Address Line 1.
- **Formule d'appel** : « Cher » suivi du nom de la personne, ou « Cher ami, » en l'absence de nom.
- **Insérer un champ de fusion** : choisissez n'importe quelle colonne.
- **Champ d'image** : choisissez une colonne contenant des noms de fichiers image. JeffPub ajoute un cadre vide et recherche les fichiers à côté de votre liste. Après avoir enregistré puis rouvert la composition, une image dont le fichier se trouve hors du dossier de la composition ne s'affiche pas tant que vous n'avez pas de nouveau choisi la liste de destinataires (**Publipostage > Sélectionner les destinataires > Utiliser une liste existante…**). Les images du dossier de la liste et de ses sous-dossiers s'affichent alors jusqu'à la fermeture de la composition. JeffPub ne le mémorise pas pour la fois suivante. Vous pouvez aussi placer les fichiers image dans le dossier de la composition ou dans l'un de ses sous-dossiers : ils s'affichent alors dès l'ouverture. Cette précaution empêche une composition que quelqu'un vous envoie de lire d'elle-même des images sur votre ordinateur. L'image de chaque enregistrement s'affiche lorsque vous affichez l'aperçu des résultats, imprimez, enregistrez un PDF ou fusionnez dans une nouvelle composition ou un catalogue.

## Afficher l'aperçu des résultats

Cliquez sur **Publipostage > Aperçu des résultats > Aperçu des résultats** pour remplacer les champs par les informations réelles. Utilisez **Premier enregistrement**, **Enregistrement précédent**, **Enregistrement suivant** et **Dernier enregistrement** pour parcourir les destinataires, ou **Rechercher un destinataire** pour atteindre directement une correspondance.

## Terminer la fusion

Cliquez sur **Publipostage > Terminer > Terminer et fusionner** :

- **Fusionner vers l'imprimante…** ouvre la page **Imprimer**. Cochez **Imprimer tous les enregistrements du publipostage**, puis lancez l'impression.
- **Fusionner dans une nouvelle composition** crée une nouvelle composition contenant une copie de vos pages pour chaque personne.
- **Fusionner vers PDF…** enregistre un seul PDF contenant les pages de toutes les personnes.
- **Fusionner vers la messagerie…** crée un fichier de message électronique .eml par personne, avec la première page sous forme d'image. Choisissez la colonne contenant les adresses e-mail, tapez un objet et choisissez un dossier. JeffPub n'envoie pas les messages : ouvrez les fichiers dans votre logiciel de messagerie.
- **Exporter la liste de destinataires…** enregistre la liste dans un fichier CSV.

## Fusion de catalogue

Cliquez sur **Insertion > Pages > Pages de catalogue** pour ouvrir le volet **Fusion de catalogue**.

1. Choisissez la liste de produits, comme indiqué ci-dessus.
2. Cliquez sur **Insérer une zone de catalogue**. Un cadre portant l'étiquette « Zone de catalogue » apparaît. Définissez ses **Lignes** et ses **Colonnes**, ainsi que sa position et sa taille. Placez les champs d'un élément dans la première cellule en double-cliquant dessus dans la liste. Ils se répètent dans les autres cellules. Pour une image, choisissez la version « (image) » du champ.
3. Affichez l'aperçu à l'aide des flèches. Chaque page affiche autant d'éléments que la zone compte de cellules. Fusionnez ensuite dans une nouvelle composition, un PDF ou vers l'imprimante.

**Supprimer la zone de catalogue** retire la zone.

## Rubriques connexes

- [Imprimer une composition](printing)
- [Ajouter et mettre en forme des zones de texte](text-boxes)
- [L'onglet Publipostage](tab-mailings)
