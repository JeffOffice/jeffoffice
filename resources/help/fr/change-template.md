# Changer le modèle d'une composition
<!-- keywords: changer de modèle, modifier le modèle, appliquer un modèle, autre conception, nouvelle conception, nouveau design, remplacer le modèle, contenu supplémentaire, texte restant, images restantes, espace réservé, rôles, créer une composition, déplacer texte et images -->

**Modifier le modèle** donne à la composition ouverte l'apparence d'une autre conception et y transfère votre texte et vos images. Vous n'avez rien à retaper. L'opération se fait en une seule étape, que vous pouvez annuler.

## Changer le modèle

1. Cliquez sur **Création de page > Modèle > Modifier le modèle**. Le mode Fichier s'ouvre sur une galerie de conceptions intitulée **Modifier le modèle**.
2. Choisissez une conception comme pour une nouvelle composition : sélectionnez un groupe à gauche ou tapez du texte dans la zone **Rechercher des modèles**, puis cliquez sur une conception. Consultez [Créer une composition](create).
3. Si vous le souhaitez, utilisez le volet de droite. **Jeu de couleurs** et **Jeu de polices** sont réglés sur **(conserver l'actuel)**, ce qui conserve vos couleurs et vos polices. Choisissez un jeu uniquement si vous voulez que la nouvelle conception les modifie. **Informations professionnelles** sélectionne le jeu de coordonnées que la conception remplit. **Inclure le logo** et **Inclure l'adresse postale** ne s'affichent que pour les conceptions qui les utilisent.
4. Cliquez sur **Modifier le modèle**. Vous pouvez aussi double-cliquer sur la conception.
5. Choisissez où appliquer la nouvelle conception, puis cliquez sur **OK** :
   - **Appliquer le modèle à la composition actuelle** modifie la composition ouverte.
   - **Créer une nouvelle composition avec mon texte et mes graphismes** ouvre le résultat dans une seconde fenêtre. La composition de départ reste inchangée.

La galerie ne présente que des conceptions. Pour changer le format de papier d'une composition sans en modifier la conception, utilisez la [mise en page](page-setup).

## Où vont votre texte et vos images

Un **article** est le texte d'une zone de texte, ou d'une chaîne de zones de texte liées. Chaque zone de texte d'une conception a un **rôle**, qui indique à quoi elle sert. Les rôles sont les suivants :

- Titre (le gros titre) et sous-titre
- Intertitre, pour les grands caractères plus loin dans la composition
- Corps de texte
- Date, qui contient aussi un lieu ou une heure
- Adresse et coordonnées : adresse postale, téléphone, e-mail et adresse web
- Organisation, c'est-à-dire le nom et le slogan de votre organisation
- Personne, c'est-à-dire le nom et la fonction d'une personne
- Légende et citation en exergue
- Adresse d'envoi, le bloc rempli par le publipostage
- Texte court, pour les petites étiquettes

Les conceptions fournies avec JeffPub comportent déjà ces rôles. Pour une zone que vous avez dessinée vous-même, ou provenant d'un fichier .pub, JeffPub déduit le rôle de son texte (une date, un numéro de téléphone), de son style (Légende ou Citation) et de sa taille.

Chacun de vos articles va dans la zone de la nouvelle conception qui a le même rôle. Par exemple, l'article rédigé dans le corps d'un prospectus va dans la zone de corps de texte d'un bulletin, et le nom de votre organisation va dans l'en-tête de titre. Si la nouvelle conception comporte plusieurs zones ayant le même rôle, vos articles les remplissent dans l'ordre de lecture : page par page, de haut en bas et de gauche à droite.

Votre article conserve ses mots, ainsi que le gras, l'italique, les liens et les champs. Il adopte les caractères, la couleur et l'espacement de la nouvelle conception, pour s'harmoniser avec le reste. Le titre d'une conception qui utilise du texte artistique reprend le premier paragraphe de votre article de titre. Les paragraphes suivants vont dans Contenu supplémentaire.

Vos images vont dans les **images d'espace réservé** de la conception, c'est-à-dire les images provisoires qu'elle affiche tant que vous n'avez pas ajouté les vôtres. Votre première image prend le premier espace réservé dans l'ordre de lecture, l'image suivante le suivant, et ainsi de suite. Chaque image est rognée pour remplir son cadre et conserve sa luminosité, son contraste et sa recoloration. Les espaces réservés pour lesquels vous n'avez pas d'image conservent l'image provisoire de la conception.

Certains éléments ne sont pas transférés :

- Le texte que vous n'avez jamais remplacé est le texte d'exemple de l'ancienne conception. Il n'est pas repris, et la nouvelle conception affiche son propre texte d'exemple à cet endroit.
- La taille de la page, les marges, les pages et les pages maîtres proviennent de la nouvelle conception. Si la composition compte plus de pages que la conception, elle conserve les pages de la conception, et le reste de votre contenu va dans Contenu supplémentaire. JeffPub n'ajoute pas de pages pour ce contenu.
- Les tableaux, formes, traits, textes artistiques et groupes d'objets créés par l'ancienne conception, que vous n'avez pas modifiés, restent liés à celle-ci et ne sont pas repris. Tout ce que vous avez dessiné et tout ce que vous avez modifié (un tableau dont vous avez rempli une cellule, une forme dont vous avez modifié le texte, un trait que vous avez déplacé) va dans Contenu supplémentaire, en entier.
- Vos informations professionnelles, les propriétés du document, les styles, la liste de publipostage et les objets de la zone de travail restent dans la composition.

## Contenu supplémentaire

Rien de ce que vous avez créé n'est perdu. Le contenu qui n'a pas de place dans la nouvelle conception attend dans le volet **Contenu supplémentaire**, qui s'ouvre à côté de la page une fois le changement effectué. Il répertorie chaque article, image, tableau, forme, trait et groupe restant.

- Cliquez sur un élément, puis sur **Placer sur la page** pour le placer sur la page affichée. Un article devient une zone de texte ajustée à son texte, un tableau ou un groupe est restitué en entier, et une image ou un objet trop grand est réduit pour tenir sur la page. Vous pouvez aussi double-cliquer sur un élément ou le faire glisser sur la page.
- Cliquez sur un élément, puis sur **Ignorer** pour le supprimer définitivement. (Vous pouvez toujours annuler.)

Le volet reste associé à la composition jusqu'à sa fermeture et il est enregistré dans le fichier de la composition. (Un fichier .pub ne peut pas le contenir : l'enregistrement au format .pub l'omet donc.) Pour le rouvrir, cliquez sur **Création de page > Modèle > Contenu supplémentaire**. Le bouton est grisé lorsqu'il ne reste aucun contenu.

## Annuler

**Annuler** (Ctrl+Z) rétablit l'ancienne conception et retire de nouveau les éléments restants de Contenu supplémentaire. Placer ou ignorer un élément constitue une étape distincte.

## Voir aussi

- [Créer une composition](create)
- [Jeux de couleurs et de polices](color-schemes)
- [Taille de la page, marges et disposition](page-setup)
- [L'onglet Création de page](tab-page-design)
