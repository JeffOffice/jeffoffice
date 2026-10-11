# Faire circuler le texte entre des zones de texte
<!-- keywords: lier des zones de texte, zones de texte liées, créer un lien, rompre un lien, chaînage, enchaînement du texte, suite de l'article, texte en excès, débordement, pichet, suite page, suite de la page, relier des zones de texte, chaîne, zone suivante, zone précédente, chaînage automatique -->

Lorsqu'un article est trop long pour une seule zone de texte, vous pouvez lier cette zone à une deuxième. Le reste du texte s'écoule alors dans la deuxième zone. Les zones liées forment une chaîne, et tout le texte d'une chaîne constitue un seul article. Modifiez le texte dans n'importe quelle zone : les mots se répartissent de nouveau dans toute la chaîne.

## Lier deux zones

La deuxième zone doit être vide et ne pas être déjà liée. Dessinez-la d'abord avec **Accueil > Objets > Dessiner une zone de texte**.

1. Cliquez sur la première zone pour la sélectionner. Elle doit être la dernière zone de sa chaîne.
2. Cliquez sur **Zone de texte > Liaison > Créer un lien**. Le pointeur prend la forme d'un pichet.
3. Cliquez sur la zone vide. Le texte en excès s'y déverse.

Vous pouvez aussi cliquer sur l'indicateur de texte en excès **A…** rouge, dans le coin inférieur droit d'une zone pleine. Le même pichet apparaît.

Si vous cliquez plutôt sur un emplacement vide de la page, JeffPub y dessine une nouvelle zone, de la même taille que la première, et la lie.

La zone vide peut se trouver sur une autre page. Pendant que le pichet est affiché, cliquez sur cette page dans la liste des pages, puis cliquez sur la zone.

Si la liaison échoue, la barre d'état en indique la raison : soit la zone sur laquelle vous avez cliqué contient du texte, soit elle est déjà liée.

## Circulation du texte

Le texte remplit la première zone, puis la zone suivante dans l'ordre où vous les avez liées. Cet ordre ne dépend pas de la position des zones sur la page. Si une zone comporte des colonnes, les colonnes se remplissent d'abord. Si la dernière zone est encore trop petite, l'indicateur **A…** rouge apparaît et le texte restant demeure masqué.

## Passer d'une zone liée à l'autre

Sélectionnez une zone liée. Un petit bouton fléché dans son coin inférieur droit permet d'atteindre la zone suivante, et un autre dans son coin supérieur gauche, la zone précédente. Vous pouvez aussi utiliser **Zone de texte > Liaison > Suivant** et **Zone de texte > Liaison > Précédent**. JeffPub change de page automatiquement.

## Rompre un lien

1. Sélectionnez la zone située juste avant la rupture.
2. Cliquez sur **Zone de texte > Liaison > Rompre**.

Tout le texte reste dans la première partie de la chaîne. Les zones situées après la rupture deviennent vides, et vous pouvez les lier de nouveau.

## Afficher les mentions « Suite page » et « Suite de la page »

Une mention indique au lecteur où se poursuit l'article.

1. Cliquez avec le bouton droit sur une zone liée et choisissez **Format de l'objet…**.
2. Ouvrez l'onglet **Zone de texte**.
3. Cochez **Inclure « Suite page… »** pour ajouter une mention en bas de la zone. Cochez **Inclure « Suite de la page… »** pour en ajouter une en haut.

La mention s'affiche sous la forme « (Suite page 4) » ou « (Suite de la page 2) », en petits caractères italiques. JeffPub insère le numéro de page. La première n'apparaît que si une zone suit, et la seconde que si une zone précède.

## Laisser JeffPub ajouter des pages

Si vous utilisez **Insertion > Texte > Insérer un fichier** et que le texte est trop long, JeffPub vous demande si vous voulez le poursuivre sur de nouvelles pages. Choisissez **Oui**. Des pages sont ajoutées, chacune avec une zone liée, au même emplacement et de la même taille que la dernière zone.

## Voir aussi

- [Ajouter et mettre en forme des zones de texte](text-boxes)
- [L'onglet Zone de texte](tab-text-box)
- [Ajouter, déplacer, supprimer et nommer des pages](pages)
