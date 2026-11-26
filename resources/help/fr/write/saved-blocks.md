# Enregistrer un bloc et le réinsérer
<!-- keywords: blocs enregistrés, texte type, modèle de texte, signature, en-tête, raccourci de texte, raccourcis de texte, réutiliser du texte, enregistrer la sélection, réinsérer, F3, bibliothèque, blocs de construction, formule de politesse -->

Un **bloc** est une partie de document que vous enregistrez pour pouvoir la réinsérer : une signature, un paragraphe que vous utilisez dans chaque lettre, un en-tête de lettre, ou une image accompagnée d'une légende. Un bloc conserve son texte avec ses polices, ses couleurs, ses styles, ses puces et numérotations, ses images, ses liens, ses champs et ses tableaux (lignes, colonnes, largeurs, bordures, trames, cellules fusionnées, ligne d'en-tête répétée et style du tableau). Il ne conserve pas les commentaires, les notes de bas de page, les notes de fin ni les sauts de section (voir Bon à savoir).

## Enregistrer un bloc

1. Sélectionnez ce que vous voulez enregistrer.
2. Choisissez **Insertion > Blocs enregistrés > Enregistrer la sélection comme bloc**.
3. Tapez un **Nom**. Vous pouvez aussi ajouter une **Catégorie** et une **Description**, qui vous aideront à retrouver le bloc plus tard.
4. Dans **Galerie**, choisissez **Blocs enregistrés** (un bloc que vous sélectionnez dans une galerie) ou **Raccourcis de texte** (un bloc que vous insérez en tapant son nom).
5. Dans **Insérer**, choisissez la manière dont le bloc est inséré dans un document : **Tel quel, au curseur**, **Dans un paragraphe distinct** ou **Sur une page distincte**.
6. Choisissez **Enregistrer**.

Si un bloc portant ce nom est déjà enregistré, JeffWrite vous demande s'il faut le remplacer.

## Insérer un bloc

1. Placez le curseur à l'endroit où le bloc doit aller. Si vous sélectionnez d'abord des mots, le bloc prend leur place.
2. Choisissez **Insertion > Blocs enregistrés**, puis cliquez sur le bloc. Tapez dans la zone de recherche pour trouver un bloc par son nom, sa catégorie ou sa description.

Insérer un bloc constitue une seule étape d'annulation. Un style utilisé par le bloc et absent de votre document est ajouté ; un style déjà présent dans votre document sous le même nom n'est jamais modifié. Un champ du bloc, comme le numéro de page, est mis à jour pour le document dans lequel il se trouve. Le premier et le dernier paragraphe d'un bloc prennent la mise en forme des paragraphes qu'ils rejoignent, comme lors d'un collage.

## Vos pages de garde, tableaux et zones de texte

Votre propre page de garde, tableau ou zone de texte peut être conservé dans une galerie à part, sous les galeries intégrées.

- **Page de garde.** Sélectionnez la page de garde et son saut de page (ou placez le curseur n'importe où sans rien sélectionner, lorsque le document comporte déjà une page de garde), puis choisissez **Insertion > Page de garde > Vos pages de garde > Enregistrer la sélection dans la galerie de pages de garde**.
- **Tableau.** Sélectionnez la totalité du tableau, puis choisissez **Insertion > Tableau > Tableaux prédéfinis > Vos tableaux > Enregistrer la sélection dans la galerie de tableaux**.
- **Zone de texte.** Cliquez sur la zone de texte pour la sélectionner, puis choisissez **Insertion > Zone de texte > Vos zones de texte > Enregistrer la sélection dans la galerie de zones de texte**.

Donnez-lui un nom, puis cliquez sur **Enregistrer**. Il apparaît alors sous **Vos pages de garde**, **Vos tableaux** ou **Vos zones de texte**. Cliquez dessus pour l'insérer. Une page de garde se place au début du document, sur une page distincte, et remplace la page de garde existante du document. Un tableau ou une zone de texte s'insère au curseur. **Gérer les blocs enregistrés** affiche aussi ces galeries, pour que vous puissiez les renommer, les modifier, les supprimer, les exporter et les importer.

## Raccourcis de texte

Un raccourci de texte est un bloc doté d'un nom court. Tapez le nom, puis appuyez sur **F3** (ou choisissez **Insertion > Blocs enregistrés > Développer le raccourci de texte**) : le nom est remplacé par le bloc. Le nom doit commencer un mot : « sig » fonctionne après une espace, mais pas à l'intérieur de « dessiné ».

## Gérer les blocs enregistrés

Choisissez **Insertion > Blocs enregistrés > Gérer les blocs enregistrés**. Vous pouvez voir chaque bloc, modifier son nom, sa catégorie, sa description ou son mode d'insertion avec **Modifier les propriétés**, et le **Supprimer**. **Rétablir la suppression** remet en place ce que vous avez supprimé tant que la boîte de dialogue reste ouverte. **Exporter** enregistre vos blocs dans un fichier, et **Importer** ajoute les blocs d'un tel fichier ; un bloc dont vous possédez déjà le nom est laissé tel quel.

## Bon à savoir

- Vos blocs enregistrés sont conservés dans un seul fichier sur cet ordinateur, pour votre seul compte : ils ne figurent pas dans vos documents et ne sont envoyés nulle part. Le fichier est `library.jblocks`, dans le dossier `Saved Blocks` situé dans votre dossier de données : `AppData\Local\JeffOffice` sous Windows, `Library/Application Support/JeffOffice` sur un Mac et `.local/share/JeffOffice` sous Linux. Tous les programmes JeffOffice de votre ordinateur partagent ce fichier, et deux fenêtres peuvent ajouter des blocs en même temps sans que l'une efface les blocs de l'autre.
- Un bloc peut atteindre 8 mégaoctets, et la bibliothèque peut contenir jusqu'à 5 000 blocs.
- Un tableau est enregistré comme tableau lorsque la sélection le contient en entier. Si la sélection ne contient qu'une partie d'un tableau, cette partie est enregistrée sous forme de texte, avec un paragraphe par cellule, et JeffWrite vous en avertit avant l'enregistrement.
- Un signet présent dans un bloc reçoit un nom propre lorsque vous insérez le bloc : si votre document comporte déjà un signet de ce nom, le nouveau est nommé avec « _2 », « _3 », et ainsi de suite.
- Les commentaires sont omis. Les notes de bas de page et les notes de fin sont omises. Les sauts de section ne sont pas conservés. Les formes et les zones de texte sont conservées et réapparaissent avec le bloc.
- Les modifications suivies sont acceptées lors de l'enregistrement du bloc : les mots insérés sont conservés, et les mots supprimés ne figurent pas dans le bloc, de sorte qu'un bloc ne contient jamais les mots suivis d'une autre personne.
- Un document n'ajoute jamais de blocs à votre bibliothèque. Seul vous le faites.
