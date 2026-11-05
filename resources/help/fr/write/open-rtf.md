# Ouvrir des fichiers RTF
<!-- keywords: rtf, texte enrichi, rich text format, ouvrir rtf, .rtf, anciens documents, importer, ouvrir, convertir -->

**RTF** (Rich Text Format) est un type de fichier de document que presque tous les traitements de texte savent lire et écrire. JeffWrite ouvre les fichiers **.rtf** et conserve leur texte ainsi que leur mise en forme.

## Ouvrir un fichier RTF

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**, puis sur **Parcourir**. Choisissez **Texte enrichi (*.rtf)** dans la liste des types de fichiers pour n'afficher que ces fichiers, ou sélectionnez le fichier dans la liste de tous les documents. Vous pouvez aussi faire glisser un fichier .rtf sur une fenêtre de JeffWrite, ou indiquer son nom lorsque vous lancez JeffWrite depuis une ligne de commande.

JeffWrite reconnaît un fichier RTF à son contenu, et non à son nom : un fichier portant une autre extension s'ouvre donc aussi, s'il s'agit bien d'un fichier RTF.

## Ce qui est repris

JeffWrite lit les éléments suivants d'un fichier RTF :

- Texte dans n'importe quelle langue, avec ses polices, tailles, couleurs, gras, italique, soulignement, barré, exposant, indice et surlignage.
- Paramètres de paragraphe : alignement, retraits, espacements, taquets de tabulation, bordures, trames, ainsi que les paramètres paragraphes solidaires et lignes solidaires.
- Styles, listes numérotées et listes à puces.
- Tableaux, y compris les tableaux imbriqués, les bordures, les trames, les largeurs de colonnes et les cellules fusionnées.
- Taille de page, marges, colonnes, orientation de la page, sections, en-têtes et pieds de page.
- Notes de bas de page, notes de fin, champs de numérotation de page, autres champs et signets.
- Modifications suivies : texte ajouté, texte supprimé, sauts de paragraphe ajoutés ou supprimés, et mises en forme modifiées. Une **modification suivie** est une modification signalée par un réviseur. Consultez [Suivre les modifications, puis les accepter ou les refuser](track-changes).
- Images au format PNG, JPEG, EMF et WMF, ainsi que les sauts de page et de colonne.

Si une partie du fichier ne peut pas être lue, JeffWrite vous indique ce qu'il a omis. Certains fichiers RTF contiennent des éléments qu'un document JeffWrite ne peut pas encore accueillir, comme des dessins ou des équations. Ils sont omis, mais le texte qui les entoure est conservé.

## Enregistrer vos modifications

Vous pouvez enregistrer un document au format .rtf : choisissez **Texte enrichi (*.rtf)** dans **Enregistrer sous**, ou appuyez sur **Ctrl+S** pour un document provenant d'un fichier .rtf. Le fichier conserve le texte et sa mise en forme, les styles, les listes, les tableaux, la mise en page, les en-têtes et pieds de page, les liens et les images (PNG, JPEG et EMF). Il ne conserve pas les notes de bas de page, les notes de fin, les commentaires, les modifications suivies, les équations, les formes ni les zones de texte. JeffWrite vous indique ce qu'il a omis lors de l'enregistrement.

## Voir aussi

- [Ouvrir, enregistrer, imprimer et exporter](open-save)
- [Bienvenue dans JeffWrite](start)
