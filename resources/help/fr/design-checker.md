# Vérifier votre conception
<!-- keywords: vérificateur de conception, vérifier, contrôler, erreurs, problèmes, basse résolution, faible résolution, texte en excès, débordement, le texte ne tient pas, zone de texte vide, hors de la page, contrôle en amont, vérification finale, épreuve, image liée, police manquante -->

Le **Vérificateur de conception** examine votre composition à la recherche des problèmes les plus fréquents. Il détecte le texte qui ne tient pas, les zones vides, les objets qui dépassent de la page et les images qui s'imprimeront mal. Exécutez-le avant d'imprimer ou d'envoyer un fichier à un imprimeur.

## Exécuter le vérificateur de conception

Cliquez sur **Révision > Vérifier > Exécuter le vérificateur de conception**. Le volet **Vérificateur de conception** s'ouvre à droite. Vous pouvez aussi cliquer sur **Fichier > Propriétés**, puis sur **Exécuter le vérificateur de conception**.

Le volet comporte deux cases à cocher, toutes deux activées au départ :

- **Exécuter les vérifications générales de conception** recherche les problèmes de mise en page.
- **Exécuter les vérifications finales de publication** recherche les éléments importants pour l'impression.

Laissez les deux cases cochées pour voir tous les types de problèmes. La liste se met à jour au fur et à mesure de votre travail. S'il n'y a rien à corriger, elle indique « Aucun problème détecté. »

## Corriger un problème

1. Cliquez sur un élément de la liste sous **Sélectionnez un élément à corriger**.
2. Cliquez sur **Atteindre l'élément**, ou double-cliquez sur l'élément.

JeffPub affiche la page et sélectionne l'objet. Corrigez-le : l'élément disparaît de la liste.

## Ce qu'il détecte

| Message | Signification | Que faire |
|---|---|---|
| La page n'a pas de contenu | La page est vide. | Ajoutez du contenu ou supprimez la page. |
| L'objet dépasse partiellement de la page | Une partie d'un objet se trouve hors de la page. | Déplacez-le, ou laissez-le s'il s'agit d'un fond perdu. |
| La zone de texte est vide | Une zone de texte ne contient aucun texte. | Tapez du texte dedans ou supprimez-la. |
| Article avec du texte en excès | Un article est le texte qui circule dans des zones liées. Celui-ci contient plus de texte que les zones ne peuvent en afficher. | Agrandissez une zone, réduisez le texte ou [liez une autre zone](linked-text). |
| Le texte est fortement réduit pour tenir | La réduction du texte pour l'ajuster à sa zone l'a ramené à moins de la moitié de sa taille. | Agrandissez la zone ou raccourcissez le texte. |
| L'espace réservé pour l'image est vide | Un cadre d'image ne contient aucune image. | Cliquez dessus pour en ajouter une, ou supprimez-le. |
| L'image n'est pas mise à l'échelle proportionnellement | L'image est étirée ou écrasée. | Redimensionnez-la pour rétablir ses proportions. |
| L'image est liée, non incorporée | L'image est rattachée à un fichier et seul un petit aperçu est stocké dans la composition. Une image insérée avec **Insérer et lier** est stockée en entier et n'apparaît pas dans la liste. | Conservez le fichier d'origine avec votre composition, ou cliquez sur **Incorporer l'image** dans le [Gestionnaire de graphiques](graphics-manager). |
| L'image liée est introuvable | Le fichier vers lequel pointe une image liée n'existe plus : la page n'affiche que son petit aperçu, et c'est lui qui serait imprimé. | Dans le Gestionnaire de graphiques, cliquez sur **Modifier le lien…** pour retrouver le fichier, ou remettez-le en place et cliquez sur **Mettre à jour le lien**. |
| L'image liée n'affiche que son aperçu jusqu'à la mise à jour du lien | L'image est liée à un fichier situé hors du dossier de la composition, que JeffPub n'ouvre pas de lui-même. La page affiche le petit aperçu, et c'est lui qui serait imprimé. | Dans le Gestionnaire de graphiques, sélectionnez l'image et cliquez sur **Mettre à jour le lien**. |
| L'image a une faible résolution | L'image fait moins de 150 ppi (pixels par pouce) à sa taille d'impression. | Utilisez une image plus grande ou imprimez-la plus petite. |
| L'objet comporte de la transparence ou des effets (impression possiblement différente) | Les zones transparentes et les effets peuvent avoir un rendu différent sur papier. | Imprimez une page de test. |
| La police « nom » n'est pas installée | Une police utilisée dans votre texte n'est pas présente sur cet ordinateur ; une autre la remplace. | Installez la police ou choisissez-en une autre. |

Chaque message se termine par le numéro de page, par exemple « (Page 2) ».

## Avant de préparer un fichier pour l'imprimeur

Lorsque vous créez un fichier PDF/X, JeffPub vous avertit des images en basse résolution et des polices manquantes, et propose d'ouvrir le vérificateur de conception. Voir [Fichiers PDF et imprimeurs](pdf).

## Rubriques connexes

- [Fichiers PDF et imprimeurs](pdf)
- [Imprimer une composition](printing)
- [Faire circuler le texte entre des zones de texte](linked-text)
- [Ajouter et ajuster des images](pictures)
- [Gestionnaire de graphiques](graphics-manager)
