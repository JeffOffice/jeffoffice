# Imprimer, PDF et images des diapositives
<!-- keywords: imprimer, imprimante, impression, copies, assembler, assemblées, sélection, dernière page en premier, inverser, zoom, diapositives, pages de commentaires, documents, plan, pdf, exporter, image, png, jpeg, jpg, image, plage, diapositives masquées, cadre, nuances de gris, noir et blanc, aperçu avant impression, papier, a4, lettre, mot de passe, chiffrer, protéger par un mot de passe -->

Vous pouvez imprimer votre présentation, l'enregistrer au format PDF ou enregistrer une diapositive sous forme d'image. Les trois options se trouvent dans l'écran Fichier.

## Imprimer

1. Appuyez sur Ctrl+P, ou cliquez sur **Fichier > Imprimer**.
2. À côté du bouton **Imprimer**, définissez les **Copies**. Choisissez une **Imprimante** : la liste se termine par **Enregistrer au format PDF**, qui crée un fichier PDF à la place d'une impression sur papier. Avec plusieurs copies, **Assembler** imprime l'ensemble complet pour chaque copie (1, 2, 3, puis 1, 2, 3), et lorsque cette option est désactivée, les copies de chaque page sortent ensemble (1, 1, 2, 2, 3, 3). Un fichier PDF est créé une seule fois, quel que soit le nombre de copies. **Propriétés de l'imprimante…** ouvre la fenêtre d'impression du système pour cette imprimante, avec ses propres options (le bac, la qualité) ; les réglages effectués là sont conservés pour le travail, tandis que les copies, l'assemblage, le papier et les nuances de gris reviennent sur cette page. **Papier** définit le format de la feuille ; **Comme indiqué dans les diapositives** est le choix habituel.
3. Sous **Imprimer**, choisissez ce qui doit sortir : **Diapositives**, **Pages de commentaires**, **Plan**, ou **Documents** avec 1, 2, 3, 4, 6 ou 9 diapositives par page.
4. Pour les documents de 4 diapositives ou plus, **Ordre** indique si les diapositives se suivent d'abord horizontalement puis verticalement (**Horizontal**), ou verticalement puis horizontalement (**Vertical**).
5. Sous **Plage**, choisissez **Toutes les diapositives**, **Diapositive active** ou **Sélection** (les diapositives sélectionnées dans la trieuse de diapositives, ou la diapositive active lorsque vous n'êtes pas dans la trieuse), ou choisissez **Diapositives** et saisissez les numéros dans la zone : numéros de diapositives et plages séparés par des virgules, par exemple 1-3, 5.
6. Vérifiez les options dont vous avez besoin : **Dernière page en premier** (les feuilles sortent dans l'ordre inverse, si bien qu'une pile sort dans l'ordre ; en recto verso, un recto reste avec son verso, de sorte que les diapositives 1 à 4 sortent sous la forme 3 et 4, puis 1 et 2 ; l'option est grisée lorsque vous imprimez recto verso manuellement), **Imprimer les diapositives masquées**, **Encadrer les diapositives** (un fin trait autour de chaque diapositive) et **Nuances de gris**.
7. Sous **Recto verso**, choisissez **Imprimer en recto**, **Imprimer recto verso, retournement sur le grand côté** (pages qui se tournent comme un livre), **Imprimer recto verso, retournement sur le petit côté** (pages qui se tournent comme un bloc-notes) ou **Imprimer recto verso manuellement**. Les deux premières options sont grisées si votre imprimante ne peut pas imprimer recto verso ; pointez-en une pour voir la raison. **Manuellement** convient à ce type d'imprimante : JeffDeck imprime les pages 1, 3, 5, et ainsi de suite, puis un message vous demande de retourner la pile et de la replacer dans l'imprimante. Lorsque vous cliquez sur **OK**, il imprime les autres pages, de la dernière à la première. Avec une seule page, il n'y a rien à retourner. JeffDeck mémorise votre choix.
8. Cliquez sur **Imprimer**. Pour un fichier PDF, JeffDeck vous demande un nom. Un fichier PDF n'a pas de faces.

L'aperçu à droite affiche chaque page qui sortira, avec des flèches pour passer d'une page à l'autre. **Zoom**, au-dessus, agrandit la page au-delà de la taille qui tient à l'écran ; faites défiler pour voir le reste. L'aperçu est dessiné de la même façon que la page est imprimée. JeffDeck mémorise vos choix pour la prochaine fois.

## Ce que chaque choix imprime

- **Diapositives** imprime une diapositive par page, dans sa propre forme. Le papier passe en orientation paysage pour une diapositive large.
- **Pages de commentaires** place une diapositive en haut d'une page, avec ses commentaires du présentateur en dessous.
- **Documents** placent une grille de diapositives sur chaque page, avec un en-tête et un pied de page contenant le titre, la date et le numéro de page. Avec 3 par page, des lignes sont prévues à côté de chaque diapositive pour écrire.
- **Plan** imprime le numéro et le titre de chaque diapositive, puis son texte, en retrait selon le niveau.

Les pages de commentaires, les documents et le plan s'impriment sur du papier A4 ou Lettre en orientation portrait, selon le format utilisé dans votre région. Les diapositives masquées ne sont pas imprimées, sauf si vous cochez **Imprimer les diapositives masquées**.

## Enregistrer au format PDF

Cliquez sur **Fichier > Exporter > Exporter au format PDF**. Une fenêtre demande ce qu'il faut inclure : les mêmes choix **Imprimer**, **Ordre** et **Plage** que pour l'impression, ainsi que les trois cases à cocher. Elle indique le nombre de pages du fichier. Cliquez sur **Exporter…**, puis choisissez un nom et un emplacement.

Le PDF contient du texte réel, avec les polices incorporées ; il s'affiche donc de la même façon sur tous les ordinateurs. Son titre est celui de la présentation, ou le nom du fichier.

**Balises de structure du document pour l'accessibilité** (activé par défaut) crée un PDF *balisé* : le titre de chaque diapositive devient un titre, ses objets sont placés dans l'ordre de lecture de la diapositive, les images et les formes ont leur texte de remplacement, et un tableau reste un tableau. **Créer des signets** (activé par défaut) ajoute un signet dans le PDF pour chaque diapositive, avec le titre de la diapositive, ou « Diapositive 3 » si elle n'en a pas. La disposition Plan n'a ni balises ni signets.

Cochez **PDF/A pour une conservation à long terme** pour créer un fichier PDF/A-1b. Toutes les polices sont incluses dans le fichier, et les effets de transparence sont aplatis, de sorte que les diapositives restent identiques pendant des années. Les balises et les signets sont conservés. Un PDF protégé par un mot de passe ne peut pas être un fichier PDF/A ; le choix d'un mot de passe désactive donc PDF/A.

## Envoyer une présentation par courriel

Cliquez sur **Fichier > Exporter > Créer un courriel**. Choisissez la présentation sous forme de fichier, ou sous forme de PDF. JeffDeck crée un fichier de message (.eml) auquel elle est jointe, puis demande où l'enregistrer. Rien n'est envoyé. Ouvrez le fichier dans votre logiciel de messagerie, ajoutez le ou les destinataires, puis envoyez-le. Une présentation protégée par un mot de passe est jointe avec ce mot de passe toujours actif. Un PDF peut avoir son propre mot de passe.

## Protéger un PDF par un mot de passe

Dans la fenêtre **Exporter au format PDF**, cliquez sur **Chiffrer avec un mot de passe…**. Saisissez le mot de passe deux fois. Toute personne qui ouvre le PDF devra alors le saisir. Le mot de passe protège réellement le fichier : les pages sont brouillées par un chiffrement fort (AES-256), et personne ne peut les lire sans lui.

Vous pouvez aussi cocher **Restreindre l'impression, la copie et la modification**, saisir un second mot de passe et choisir ce qui est autorisé. Ces restrictions ne sont qu'une demande. La plupart des lecteurs PDF les respectent, mais elles n'empêchent pas une personne déterminée de les contourner ; ne comptez donc pas dessus pour garder un fichier privé.

Un mot de passe oublié ne peut pas être récupéré : conservez-le en lieu sûr. Le mot de passe sert uniquement pour ce PDF et n'est jamais enregistré.

Lorsque vous insérez un PDF protégé par un mot de passe comme image, JeffDeck demande le mot de passe. S'il est incorrect, il le redemande ; cliquez sur **Annuler** pour arrêter. La diapositive reçoit une copie de la page ne nécessitant aucun mot de passe. Si le propriétaire du PDF demande qu'il ne soit pas copié, JeffDeck l'insère néanmoins et affiche une note à ce sujet ; cette demande n'est pas un verrou.

## Enregistrer une diapositive sous forme d'image

Cliquez sur **Fichier > Exporter > Exporter la diapositive en tant qu'image**. JeffDeck enregistre la diapositive active sous forme d'image PNG ou JPEG, large de 1920 pixels, sur fond blanc. Il propose comme nom de fichier celui de la présentation suivi du numéro de la diapositive.

## Voir aussi

- [L'écran Fichier](file-screen)
- [Notes du présentateur](notes)
- [Ajouter, modifier et supprimer des diapositives](slides)
