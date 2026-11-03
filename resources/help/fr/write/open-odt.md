# Ouvrir et enregistrer des fichiers OpenDocument
<!-- keywords: odt, fodt, ott, opendocument, open document, ouvrir, enregistrer, enregistrer sous, format, type de fichier, autres programmes, autres logiciels, échanger des documents, partager, compatibilité, suivi des modifications, modifications suivies, éléments omis, modèle -->

JeffWrite ouvre et enregistre le texte OpenDocument, le type de fichier dont le nom se termine par **.odt**. De nombreux programmes gratuits le lisent et l'écrivent : c'est donc un bon moyen d'échanger un document avec une personne qui n'utilise pas JeffWrite.

## Ouvrir un fichier OpenDocument

Appuyez sur **Ctrl+O**, ou cliquez sur **Fichier > Ouvrir**, puis choisissez un fichier :

- **.odt** est un document.
- **.ott** est un modèle. Il s'ouvre en tant que nouveau document créé à partir de ce modèle, qui reste donc inchangé.
- **.fodt** est le même type de document, conservé dans un seul fichier XML brut. JeffWrite n'écrit pas de fichiers .fodt : **Enregistrer** sur l'un d'eux demande un nouveau nom et l'enregistre en tant que fichier .odt.

JeffWrite identifie un fichier d'après son contenu, et non d'après son nom : un fichier dont l'extension est incorrecte s'ouvre donc aussi. Vous pouvez également indiquer le nom d'un fichier au démarrage de JeffWrite, ou le faire glisser sur le programme depuis votre gestionnaire de fichiers, comme pour un fichier .docx.

JeffWrite lit le texte et sa mise en forme, les titres, les styles et les styles sur lesquels ils sont basés, les listes à puces et numérotées, les tableaux (avec cellules fusionnées, largeurs de colonne, bordures, trames et tableaux imbriqués), les sections et colonnes, la taille, les marges et l'orientation du texte de la page, les en-têtes et pieds de page (première page, pages de gauche et de droite), les notes de bas de page et notes de fin, les champs (numéros de page, nombre de pages, titre, auteur, nom du fichier, renvois et numéros incrémentés), les signets, les liens, une table des matières, les images, les zones de texte, les taquets de tabulation, les commentaires et le suivi des modifications, qui s'affiche sous forme de marques que vous pouvez accepter ou refuser.

Certains éléments ne peuvent pas encore être affichés par JeffWrite : les formes dessinées sans texte, les formules, les graphiques et les autres objets placés dans le texte. Si un fichier en contient, JeffWrite vous indique lesquels à l'ouverture. Il n'enregistre alors pas par-dessus ce fichier : **Enregistrer** demande un nouveau nom, afin que l'original conserve ce que la copie a perdu. Un document protégé par un mot de passe ne s'ouvre pas. Ouvrez-le dans le programme qui l'a créé et enregistrez-en une copie sans mot de passe.

## Enregistrer un fichier OpenDocument

Cliquez sur **Fichier > Enregistrer sous**, puis choisissez **Texte OpenDocument (*.odt)** dans la liste des types de fichiers. Ensuite, **Ctrl+S** enregistre de nouveau dans le même format.

Le fichier contient les éléments énumérés ci-dessus, avec ses commentaires et le suivi de ses modifications : texte ajouté et supprimé, mise en forme modifiée, et sauts de paragraphe ajoutés ou supprimés. Pour quelques éléments, le format ne prévoit aucun emplacement, et JeffWrite les indique après l'enregistrement : ils sont enregistrés dans leur état actuel ou omis. Il s'agit des modifications suivies apportées aux tableaux, aux colonnes et à la mise en page, et des modifications suivies dans les en-têtes, pieds de page et notes (enregistrées dans leur état actuel) ; des champs de formulaire (enregistrés sous forme du texte affiché) ; des équations (enregistrées en texte brut) ; ainsi que des dessins, formes, graphiques et objets incorporés qui ne peuvent pas être conservés. Si vous enregistrez de nouveau après une modification qui entraîne l'omission d'un nouvel élément, JeffWrite vous le signale aussi, sans répéter ce qu'il vous a déjà indiqué.

Les autres programmes qui lisent le format OpenDocument ouvrent le fichier sans proposer de le réparer.

## Voir aussi

- [Ouvrir, enregistrer, imprimer et exporter](open-save)
- [Suivre les modifications, puis les accepter ou les refuser](track-changes)
- [Bienvenue dans JeffWrite](start)
