# Enregistrer un bloc et le réinsérer
<!-- keywords: blocs enregistrés, texte standard, signature, en-tête, raccourci de texte, raccourcis de texte, réutiliser du texte, enregistrer la sélection, insérer à nouveau, F3, bibliothèque -->

Un **bloc** est un élément de document que vous enregistrez pour pouvoir le réinsérer : une signature, un paragraphe que vous utilisez dans chaque lettre, un en-tête de lettre, ou une image avec sa légende. Un bloc conserve son texte avec ses polices, ses couleurs, ses styles, ses puces et sa numérotation, ses images, ses liens, ses champs et ses tableaux (lignes, colonnes, largeurs, bordures, trames, cellules fusionnées, ligne d'en-tête répétée et style du tableau). Il ne conserve pas les commentaires, les notes de bas de page, les notes de fin ni les sauts de section (voir Bon à savoir).

## Enregistrer un bloc

1. Sélectionnez ce que vous voulez enregistrer.
2. Choisissez **Insertion > Blocs enregistrés > Enregistrer la sélection comme bloc**.
3. Saisissez un **Nom**. Vous pouvez aussi ajouter une **Catégorie** et une **Description**, qui facilitent la recherche du bloc plus tard.
4. Dans **Galerie**, choisissez **Blocs enregistrés** (un bloc que vous sélectionnez dans une galerie) ou **Raccourcis de texte** (un bloc que vous insérez en saisissant son nom).
5. Dans **Insérer dans**, choisissez la manière dont le bloc s'insère dans un document : **Tel quel, au curseur**, **Dans un paragraphe distinct** ou **Sur une page distincte**.
6. Choisissez **Enregistrer**.

Si un bloc portant ce nom est déjà enregistré, JeffWrite vous demande s'il faut le remplacer.

## Insérer un bloc

1. Placez le curseur à l'endroit où le bloc doit aller. Si vous sélectionnez d'abord des mots, le bloc les remplace.
2. Choisissez **Insertion > Blocs enregistrés**, puis cliquez sur le bloc. Saisissez dans la zone de recherche pour trouver un bloc par son nom, sa catégorie ou sa description.

Insérer un bloc correspond à une seule étape d'annulation. Un style utilisé par le bloc et absent de votre document est ajouté ; un style déjà présent dans votre document sous le même nom n'est jamais modifié. Un champ du bloc, tel que le numéro de page, est mis à jour pour le document dans lequel il se trouve. Les premier et dernier paragraphes d'un bloc prennent la mise en forme des paragraphes auxquels ils se raccordent, comme lors d'un collage.

## Raccourcis de texte

Un raccourci de texte est un bloc dont le nom est court. Saisissez le nom, puis appuyez sur **F3** (ou choisissez **Insertion > Blocs enregistrés > Développer le raccourci de texte**) : le nom est remplacé par le bloc. Le nom doit commencer un mot : « sig » fonctionne après une espace, mais pas à l'intérieur de « design ».

## Gérer les blocs enregistrés

Choisissez **Insertion > Blocs enregistrés > Gérer les blocs enregistrés**. Vous voyez chaque bloc et pouvez modifier son nom, sa catégorie, sa description ou son mode d'insertion avec **Modifier les propriétés**, et le **Supprimer**. **Annuler la suppression** rétablit ce que vous avez supprimé tant que vous n'avez pas fermé la boîte de dialogue. **Exporter** enregistre vos blocs dans un fichier, et **Importer** ajoute les blocs d'un tel fichier ; un bloc dont vous avez déjà le nom reste inchangé.

## Bon à savoir

- Vos blocs enregistrés sont conservés dans un seul fichier sur cet ordinateur, pour votre seul compte : ils ne figurent pas dans vos documents et ne sont envoyés nulle part. Le fichier est `library.jblocks`, dans le dossier `Saved Blocks` situé dans votre dossier de données : `AppData\Local\JeffOffice` sous Windows, `Library/Application Support/JeffOffice` sur un Mac et `.local/share/JeffOffice` sous Linux. Tous les programmes JeffOffice de votre ordinateur partagent ce fichier, et deux fenêtres peuvent ajouter des blocs en même temps sans que l'une perde ceux de l'autre.
- Un bloc peut atteindre 8 mégaoctets, et la bibliothèque peut contenir jusqu'à 5 000 blocs.
- Un tableau est enregistré comme tableau lorsque la sélection le contient en entier. Si la sélection ne contient qu'une partie d'un tableau, cette partie est enregistrée sous forme de texte, avec un paragraphe par cellule, et JeffWrite vous en informe avant l'enregistrement.
- Un signet présent dans un bloc reçoit un nom propre lors de l'insertion du bloc : si votre document comporte déjà un signet portant ce nom, le nouveau est nommé avec « _2 », « _3 », etc. ajoutés à la suite.
- Les commentaires ne sont pas conservés. Les notes de bas de page et les notes de fin ne sont pas conservées. Les sauts de section ne sont pas conservés. Les formes et les zones de texte sont conservées et réapparaissent avec le bloc.
- Les modifications suivies sont acceptées lors de l'enregistrement du bloc : les mots insérés sont conservés, et les mots supprimés ne figurent pas dans le bloc. Un bloc ne contient donc jamais de mots suivis appartenant à une autre personne.
- Un document n'ajoute jamais de blocs à votre bibliothèque. Seul vous le faites.
