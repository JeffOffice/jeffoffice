# Créer des diapositives à partir d'un plan
<!-- keywords: diapositives à partir du plan, importer un plan, plan, markdown, plan texte, rtf, docx, titres en diapositives, liste à puces en diapositives, démarrer une présentation à partir d'un texte, enregistrer le plan, exporter le plan, créer des diapositives à partir d'un texte, importer du texte, titre, texte, enregistrer un plan -->

Vous pouvez rédiger une présentation sous forme de texte brut, et laisser JeffDeck la transformer en diapositives. Chaque diapositive a un titre et un corps de texte, dans la disposition Titre et contenu. Le mode Plan, la commande Réinitialiser la diapositive et les thèmes fonctionnent donc sur elle comme d'habitude.

## Ajouter les diapositives à la présentation ouverte

1. Cliquez sur **Accueil > Diapositives**, puis sur la petite flèche sous **Nouvelle diapositive**, et choisissez **Diapositives à partir du plan…**.
2. Choisissez un fichier. Il peut s'agir d'un fichier texte (.txt), d'un fichier Markdown (.md), d'un fichier texte enrichi (.rtf) ou d'un fichier de document (.docx).
3. Les diapositives sont ajoutées après la diapositive active. Elles forment une seule action : un seul Ctrl+Z les annule toutes.

## Ou démarrer une nouvelle présentation à partir d'un plan

Choisissez **Fichier > Ouvrir**, puis sélectionnez le fichier de plan. JeffDeck crée une nouvelle présentation à partir de celui-ci. Le fichier n'est jamais modifié, et **Enregistrer** demande un nom.

## Comment le fichier est lu

**Fichiers texte.** Une ligne qui commence au bord gauche est le titre d'une diapositive. Les lignes situées sous elle constituent le texte de la diapositive. Une ligne en retrait par rapport à la précédente passe d'un niveau à l'intérieur (utilisez des tabulations ou des espaces). Une puce (-, * ou +) ou un numéro en début de ligne est supprimé, car la diapositive dispose de ses propres puces.

**Fichiers Markdown.** Le niveau de titre le plus élevé utilisé dans le fichier crée les diapositives : # s'il y en a un, sinon ##. Les titres de niveau inférieur deviennent des lignes du texte de la diapositive, avec un niveau de plus à chaque étape. Une ligne de trois tirets (---) commence une nouvelle diapositive. Les listes deviennent des niveaux. Le **gras**, l'*italique*, le `code` et les liens conservent leur mise en forme. Un bloc de code placé entre deux lignes de trois apostrophes inversées (```) devient une zone de texte en police à chasse fixe. Une image située sur une ligne qui lui est propre, écrite dans la forme d'image de Markdown (un point d'exclamation, la description entre crochets et le nom du fichier entre parenthèses), est placée sur la diapositive, mais uniquement si le fichier image se trouve dans le même dossier que le plan, ou dans un dossier contenu dans celui-ci. Les images provenant d'Internet ou d'autres dossiers ne sont jamais chargées.

**Fichiers RTF et .docx.** Les paragraphes dans un style de titre créent les diapositives : le style de titre le plus élevé utilisé dans le fichier sert de titre, les titres de niveau inférieur deviennent des lignes du texte, et chaque autre paragraphe est une ligne placée sous le titre qui le précède.

## Limites

Ces limites s'appliquent à tous les types de fichiers de plan, texte, Markdown, RTF et .docx :

- 5 000 diapositives au maximum. Tout ce qui suit est omis.
- 4 000 caractères au maximum par ligne. Une ligne plus longue est coupée, de même qu'un paragraphe Markdown dont les lignes forment un ensemble plus long que cette limite.
- 500 lignes de texte au maximum sur une diapositive, et 100 000 lignes au total.
- Le texte d'une diapositive comporte jusqu'à 9 niveaux. Une ligne en retrait plus profond est affichée au niveau le plus bas.
- 200 lignes de code au maximum sur une diapositive.
- 100 images au maximum au total, et 4 par diapositive. Un fichier image peut peser jusqu'à 32 Mo, et l'ensemble des fichiers image jusqu'à 64 Mo. Une image doit être un fichier PNG, JPEG, GIF, BMP, WebP ou TIFF, quel que soit son nom, comporter au maximum 16 millions de pixels (4 000 × 4 000), et l'ensemble des images au maximum 64 millions de pixels.

La taille du fichier lui-même est également limitée. Un fichier texte ou Markdown peut peser jusqu'à 10 Mo, en UTF-8 ou UTF-16. Un fichier RTF ou .docx peut peser jusqu'à 96 Mo. Un fichier qui n'est pas un texte est refusé. Les images contenues dans un fichier RTF ou .docx ne sont pas placées sur les diapositives, et JeffDeck vous indique combien ont été omises.

Lorsque des éléments sont omis, comme une image qui n'a pas pu être utilisée ou un texte qui dépasse une limite, JeffDeck vous indique lesquels. Pendant la lecture d'un gros fichier, JeffDeck continue de fonctionner, et la fermeture de la fenêtre interrompt la lecture.

## Enregistrer une copie au format plan

Choisissez **Fichier > Enregistrer une copie**, puis sélectionnez **Plan Markdown (*.md)**. Le fichier contient le titre et le texte de chaque diapositive, avec le gras, l'italique et les liens. Il ne conserve ni les images, ni les formes, ni les tableaux, ni les graphiques, ni les autres zones de texte, ni les notes du présentateur, ni l'aspect des diapositives. JeffDeck vous le signale lors de l'enregistrement, et **Fichier > Informations > Vérifier la compatibilité** le répertorie également.

## Voir aussi

- [Ajouter, modifier et supprimer des diapositives](slides)
- [Réutiliser des diapositives d'une autre présentation](reuse)
- [Affichages](views)
