# Dessiner des formes et des traits
<!-- keywords: forme, trait, flèche, rectangle, cercle, ovale, connecteur, courbe, forme libre, remplissage, contour, ombre, 3D, modifier les points, organigramme, dessin, tracé, lignes -->

Les formes et les traits sont des objets dessinés. Vous pouvez les remplir de couleur, leur appliquer un contour et saisir du texte à l'intérieur d'une forme.

## Dessiner une forme

1. Cliquez sur **Insertion > Illustrations > Formes**. (Vous pouvez aussi cliquer sur **Accueil > Objets > Formes**.)
2. Choisissez une forme. Les groupes sont Traits, Rectangles, Formes de base, Flèches pleines, Formes d'équation, Organigramme, Étoiles et bannières, Légendes et Boutons d'action.
3. Faites glisser le pointeur sur la page.

Pendant le glissement, maintenez la touche **Maj** pour obtenir une forme régulière : un ovale devient un cercle, et un rectangle devient un carré. Maintenez **Ctrl** pour dessiner à partir du centre. Si vous cliquez sans faire glisser le pointeur, vous obtenez une forme d'un pouce (2,54 cm) de côté. Après avoir dessiné une forme, le pointeur reprend son fonctionnement normal. Appuyez sur **Échap** pour annuler avant de dessiner.

## Dessiner des traits et des flèches

Choisissez **Trait**, **Flèche** ou **Flèche double** dans le groupe Traits de la galerie, ou dans **Format de la forme > Traits**. Faites glisser le pointeur sur la page. Maintenez la touche **Maj** pour garder le trait horizontal, vertical ou à 45 degrés. Pour modifier un trait ultérieurement, faites glisser la poignée située à l'une de ses extrémités.

Trois autres outils permettent de dessiner à main levée :

- **Courbe** : cliquez pour placer des points. Le trait passe en douceur par ces points.
- **Forme libre : forme** : cliquez pour placer des points qui forment des côtés droits, ou maintenez le bouton enfoncé et faites glisser le pointeur pour dessiner à main levée.
- **Forme libre : dessin à main levée** : maintenez le bouton enfoncé et tracez un seul trait.

Pour terminer une courbe ou une forme libre, double-cliquez ou appuyez sur **Entrée**. Cliquez sur le premier point pour fermer le tracé et obtenir une forme remplie.

## Relier des objets avec des connecteurs

Un trait peut rester attaché à d'autres objets. **Connecteur en coude** et **Connecteur courbe** dessinent des traits coudés et courbes. Lorsque vous approchez le pointeur d'une forme, d'une zone de texte, d'une image ou d'un tableau, ses points d'attache apparaissent. L'extrémité du trait se fixe au milieu d'un côté. Lorsque vous déplacez, redimensionnez ou faites pivoter l'objet, le trait le suit. Faites glisser la poignée jaune pour déplacer le coude d'un connecteur en coude. Pour détacher un trait, faites-le glisser à l'écart, seul.

## Modifier le remplissage, le contour et les effets

Sélectionnez la forme, puis ouvrez l'onglet **Format de la forme**.

- **Styles de forme** : la galerie propose des apparences prêtes à l'emploi.
- **Remplissage de forme** : choisissez une couleur, **Aucun remplissage**, **Remplissage par image…** ou **Effets de remplissage…** pour un dégradé, une texture ou un motif.
- **Contour de forme** : choisissez une couleur. La flèche située à côté permet de définir l'épaisseur, les tirets et les pointes de flèche.
- **Effets de forme** : **Ombre**, **Reflet**, **Lueur**, **Bordures adoucies**, **Biseau** et **Rotation 3D**. **Options d'ombre…** permet de régler la couleur, le flou, la distance et l'angle de l'ombre. **Rotation 3D** incline la forme en perspective.

Pour accéder à tous les réglages à la fois, cliquez sur le petit bouton situé dans le coin du groupe **Styles de forme** afin d'ouvrir la fenêtre **Mettre en forme la forme**.

## Ajouter du texte à une forme

Double-cliquez sur la forme, puis saisissez le texte. Vous pouvez aussi sélectionner la forme et appuyer sur **Entrée**, ou faire un clic droit sur la forme et choisir **Ajouter du texte**.

## Modifier la forme

- **Modifier la forme** (dans **Insérer des formes**) remplace la forme par une autre.
- Une poignée losange jaune, présente sur certaines formes, permet d'ajuster leur forme. Sur un rectangle à coins arrondis, elle règle l'arrondi des angles.
- **Modifier les points** permet de déplacer les angles. Des carrés blancs marquent chaque point. Faites glisser un point pour le déplacer. Cliquez sur un point tout en maintenant **Ctrl** pour le supprimer, ou cliquez sur un bord tout en maintenant **Ctrl** pour en ajouter un. Appuyez sur **Échap** lorsque vous avez terminé.

## Points de colle pour les connecteurs

Un connecteur se fixe aux points de connexion dont dispose une forme (le milieu de chaque côté, et davantage sur certaines formes). Pour ajouter un point personnalisé, sélectionnez la forme, cliquez sur **Modifier les points**, puis maintenez **Maj** et cliquez à l'endroit où le point doit se trouver. Un losange bleu le marque. Les connecteurs peuvent se fixer à ce point comme à n'importe quel autre point de connexion, et suivent la forme lorsqu'elle est déplacée ou redimensionnée : le point conserve sa place dans la forme (un point situé au quart de la largeur reste au quart de la largeur). Pour le supprimer, maintenez **Maj** et cliquez sur le losange ; les connecteurs qui y étaient fixés sont libérés. Une forme peut comporter jusqu'à 16 points de colle qui lui sont propres. Chaque modification correspond à une étape d'annulation, et les points sont conservés lorsque vous enregistrez le fichier au format JeffPub. Un fichier `.pub` conserve un connecteur du côté le plus proche de son point de colle.

## Fusionner des formes et convertir des contours

Sélectionnez deux formes ou plus situées sur la même page, puis cliquez sur **Format de la forme > Insérer des formes > Fusionner les formes**. La nouvelle forme reprend le remplissage et le trait de la forme que vous avez sélectionnée en premier, et se place à l'emplacement des anciennes. Les anciennes formes sont supprimées, ainsi que leur texte : la barre d'état l'indique, et si plusieurs formes contiennent du texte, il vous est demandé une seule fois de confirmer sa suppression. Un connecteur fixé à une ancienne forme est fixé à la nouvelle s'il se trouvait au milieu d'un des côtés de la nouvelle forme, et il est libéré dans le cas contraire. Chaque choix correspond à une étape d'annulation, et les nouvelles formes restent sélectionnées.

- **Union** crée une seule forme à partir de tout ce que recouvrent les formes.
- **Combiner** conserve ce que recouvre une seule des formes et découpe les zones de chevauchement.
- **Soustraire** conserve la première forme sélectionnée, sans les autres.
- **Intersection** ne conserve que ce que recouvrent toutes les formes.
- **Fragmenter** découpe les formes en toutes les parties qu'elles forment entre elles. Chaque morceau devient une forme indépendante.

Vous pouvez fusionner jusqu'à 64 formes à la fois, et un résultat qui serait découpé en plus de 256 morceaux est refusé. Les formes comportant un très grand nombre de bords qui se croisent prendraient trop de temps : elles sont refusées avec un message. Une fusion comportant plus de quelques croisements s'exécute en arrière-plan, avec une boîte dans laquelle vous pouvez cliquer sur **Annuler** (les formes restent alors inchangées). Un trou dans une forme reste un trou, quel que soit le nombre de fusions effectuées ensuite. Une forme verrouillée, située dans un groupe ou constituant une illustration reste inchangée, et la barre d'état l'indique. Une forme fusionnée est une forme dotée de points propres : **Modifier les points** permet de la remodeler, et elle est conservée dans le fichier comme n'importe quelle autre forme.

**Format de la forme > Insérer des formes > Convertir** modifie la manière dont les formes sélectionnées sont dessinées :

- **Convertir en courbe** donne à la forme un contour propre, afin que ses points puissent être déplacés (**Modifier les points** produit le même effet lorsque vous commencez à la modifier).
- **Convertir en polygone** remplace chaque courbe du contour par de courts segments droits.
- **Convertir en contour** transforme le trait de la forme en une forme indépendante, remplie de la couleur du trait. La forme doit avoir un trait.
- **Séparer** transforme une forme composée de plusieurs parties (comme l'union de formes qui ne se touchent pas) en une forme distincte pour chaque partie. Un anneau se sépare en son cercle extérieur et en son trou, chacun rempli. Le texte d'une forme reste attaché à elle : il suit la première partie de la forme séparée.

## Recalculer le tracé des connecteurs

Un connecteur est un trait dont les extrémités sont fixées à des formes (tracez-le du point de connexion d'une forme vers celui d'une autre). Sélectionnez une forme ou le connecteur, puis cliquez sur **Format de la forme > Insérer des formes > Réacheminer les connecteurs**. Chaque connecteur fixé à la forme déplace chacune de ses extrémités fixées vers le point de connexion de sa forme qui est le plus proche de l'autre extrémité, de sorte que le tracé soit le plus court possible. Cette opération correspond à une étape d'annulation. Si tous les connecteurs suivent déjà leur tracé le plus court, la barre d'état l'indique.

## Voir aussi

- [Déplacer, aligner, grouper et superposer des objets](arrange)
- [Insérer et ajuster des images](pictures)
- [Icônes, codes-barres et autres objets](insert-objects)
- [L'onglet Format de la forme](tab-shape-format)
