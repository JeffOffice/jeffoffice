# Vérifier votre conception

<!-- keywords: vérificateur de conception, vérifier, erreurs, problèmes, basse résolution, débordement, texte ne tient pas, zone de texte vide, hors de la page, contrôle avant impression, vérification finale, épreuve, image liée, police manquante, police absente, contrôle, vérificateur -->

Le **Vérificateur de conception** parcourt votre composition à la recherche d'erreurs fréquentes. Il repère le texte qui ne tient pas dans son cadre, les zones de texte vides, les objets qui dépassent de la page et les images qui seront mal imprimées. Exécutez-le avant d'imprimer ou d'envoyer un fichier à un imprimeur.

## Exécuter le Vérificateur de conception

Cliquez sur **Révision > Vérifier > Exécuter le vérificateur de conception**. Le volet **Vérificateur de conception** s'ouvre à droite. Vous pouvez aussi cliquer sur **Fichier > Informations**, puis sur **Exécuter le vérificateur de conception**.

Le volet comporte deux cases à cocher, toutes deux activées au départ :

- **Exécuter les vérifications générales de conception** recherche les problèmes de mise en page.
- **Exécuter les vérifications finales de publication** recherche les points importants pour l'impression.

Laissez les deux cases cochées pour voir tous les types de problèmes. La liste se met à jour au fur et à mesure de votre travail. S'il n'y a rien à corriger, le message affiché est « Aucun problème détecté ».

## Corriger un problème

1. Cliquez sur un élément de la liste sous **Sélectionnez un élément à corriger**.
2. Cliquez sur **Atteindre l'élément**. Vous pouvez aussi double-cliquer sur l'élément.

JeffPub affiche la page et sélectionne l'objet. Corrigez le problème : l'élément disparaît de la liste.

## Ce qu'il détecte

| Message | Signification | Que faire |
|---|---|---|
| La page n'a pas de contenu | La page est vide. | Ajoutez un élément, ou supprimez la page. |
| L'objet dépasse partiellement de la page | Une partie de l'objet se trouve en dehors de la page. | Déplacez-le, ou laissez-le s'il s'agit d'un fond perdu. |
| La zone de texte est vide | Une zone de texte ne contient aucun texte. | Saisissez du texte, ou supprimez-la. |
| Article avec du texte en excès | Un article est le texte qui circule dans des zones de texte liées. Celui-ci contient plus de texte que l'espace disponible. | Agrandissez une zone, raccourcissez le texte, ou [liez une autre zone](linked-text). |
| Texte fortement réduit pour tenir | La réduction du texte pour l'ajuster à sa zone l'a fait passer à moins de la moitié de sa taille. | Agrandissez la zone ou utilisez moins de texte. |
| L'espace réservé pour l'image est vide | Un cadre d'image ne contient aucune image. | Cliquez dessus pour ajouter une image, ou supprimez-le. |
| L'image n'est pas mise à l'échelle proportionnellement | L'image est étirée ou écrasée. | Redimensionnez-la pour rétablir ses proportions. |
| L'image est liée, non incorporée | L'image est reliée à un fichier, et la composition ne contient qu'un petit aperçu. Une image insérée avec **Insérer et lier** est enregistrée entièrement et n'apparaît pas dans la liste. | Conservez le fichier original avec votre composition, ou cliquez sur **Incorporer l'image** dans le [Gestionnaire de graphiques](graphics-manager). |
| L'image liée est introuvable | Le fichier auquel l'image liée renvoie n'existe plus : la page n'affiche que son petit aperçu, qui serait imprimé. | Dans le Gestionnaire de graphiques, cliquez sur **Modifier le lien…** pour retrouver le fichier, ou remettez-le en place puis cliquez sur **Mettre à jour le lien**. |
| L'image liée n'affiche que son aperçu jusqu'à Mettre à jour le lien | L'image est liée à un fichier situé en dehors du dossier de la composition, que JeffPub n'ouvre pas automatiquement. La page affiche le petit aperçu, qui serait imprimé. | Dans le Gestionnaire de graphiques, sélectionnez l'image et cliquez sur **Mettre à jour le lien**. |
| L'image a une faible résolution | L'image a moins de 150 ppp (pixels par pouce) à sa taille d'impression. | Utilisez une image plus grande, ou imprimez-la plus petite. |
| L'objet comporte de la transparence ou des effets (impression possiblement différente) | Les zones transparentes et les effets peuvent rendre différemment sur papier. | Imprimez une page d'essai. |
| La police « nom » n'est pas installée | Une police utilisée dans votre texte n'est pas installée sur cet ordinateur ; une autre la remplace. | Installez la police, ou choisissez-en une autre. |

Chaque message se termine par le numéro de page, par exemple « (page 2) ».

## Avant un fichier pour un imprimeur

Lorsque vous créez un fichier PDF/X, JeffPub vous avertit des images à basse résolution et des polices manquantes, et vous propose d'ouvrir le Vérificateur de conception. Voir [Fichiers PDF et imprimeurs](pdf).

## Voir aussi

- [Fichiers PDF et imprimeurs](pdf)
- [Imprimer une composition](printing)
- [Faire circuler le texte entre des zones de texte](linked-text)
- [Ajouter et ajuster des images](pictures)
- [Gestionnaire de graphiques](graphics-manager)
