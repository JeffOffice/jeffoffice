# Ouvrir des fichiers RTF
<!-- keywords: rtf, texte enrichi, rich text format, ouvrir rtf, .rtf, anciens documents, importer, importation, ouvrir, convertir, conversion -->

Le format **RTF** (Rich Text Format) est un type de fichier de document que presque tous les traitements de texte savent lire et écrire. JeffWrite ouvre les fichiers **.rtf** et conserve leur texte et leur mise en forme.

## Ouvrir un fichier RTF

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**, puis cliquez sur **Parcourir**. Choisissez **Texte enrichi (*.rtf)** dans la liste des types de fichiers pour n'afficher que ces fichiers, ou sélectionnez le fichier dans la liste de tous les documents. Vous pouvez aussi faire glisser un fichier .rtf sur une fenêtre de JeffWrite, ou indiquer son nom lorsque vous lancez JeffWrite depuis une ligne de commande.

JeffWrite reconnaît un fichier RTF à son contenu et non à son nom : un fichier portant une autre extension s'ouvre donc quand même s'il s'agit réellement d'un fichier RTF.

## Éléments repris

JeffWrite lit les éléments suivants d'un fichier RTF :

- Le texte dans n'importe quelle langue, avec ses polices, tailles, couleurs, gras, italique, soulignement, barré, exposant, indice et surlignage.
- Les paramètres de paragraphe : alignement, retraits, espacement, taquets de tabulation, bordures, trame de fond, et les options Paragraphes solidaires et Lignes solidaires.
- Les styles, les listes numérotées et les listes à puces.
- Les tableaux, y compris les tableaux imbriqués, les bordures, la trame de fond, la largeur des colonnes et les cellules fusionnées.
- La taille de la page, les marges, les colonnes, l'orientation de la page, les sections, les en-têtes et les pieds de page.
- Les notes de bas de page, les notes de fin, les champs de numéro de page, les autres champs et les signets.
- Les modifications suivies : texte ajouté, texte supprimé, sauts de paragraphe ajoutés ou supprimés, et modifications de mise en forme. Une **modification suivie** est une modification marquée par un réviseur. Consultez [Suivre les modifications, puis les accepter ou les refuser](track-changes).
- Les images aux formats PNG, JPEG, EMF et WMF, ainsi que les sauts de page et les sauts de colonne.

Si une partie du fichier ne peut pas être lue, JeffWrite vous indique ce qu'il a omis. Certains fichiers RTF contiennent des éléments qu'un document JeffWrite ne peut pas encore contenir, comme des dessins ou des équations. Ces éléments sont omis, et le texte qui les entoure est conservé.

## Enregistrer vos modifications

JeffWrite n'écrit pas de fichiers .rtf. Lorsque vous enregistrez un document issu d'un fichier .rtf, JeffWrite vous demande un nouveau nom et l'enregistre au format .docx. Le fichier .rtf reste inchangé.

## Rubriques connexes

- [Ouvrir, enregistrer, imprimer et exporter](open-save)
- [Bienvenue dans JeffWrite](start)
