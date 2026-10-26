# Règles de contenu des cellules
<!-- keywords: validation, règle, validation des données, liste, liste déroulante, menu déroulant, nombre entier, décimal, date, heure, longueur, personnalisé, arrêt, avertissement, informations, message d'erreur, alerte d'erreur, non autorisé, choix, flèche, Alt+Bas -->

Un classeur peut comporter des **règles** qui définissent ce que peuvent contenir certaines cellules : un nombre entier de 1 à 10, une date postérieure à un jour donné, un texte de cinq lettres au plus, ou un choix dans une liste. Les règles proviennent du classeur ; JeffCrunch les applique lorsque vous **tapez** une valeur dans une cellule (ou en choisissez une dans sa liste). Les valeurs que vous collez, recopiez ou déplacez ne sont pas vérifiées, comme dans les tableurs que vous connaissez : une règle définit donc ce qui peut être tapé, et non ce que contient la cellule.

## Lorsqu'une valeur n'est pas conforme

Chaque règle possède un **style**, et le message affiché est celui de la règle lorsque le classeur en définit un :

- **Arrêt** : le message s'affiche et la saisie reste dans la cellule pour que vous la modifiiez. Appuyez sur **Échap** pour l'abandonner.
- **Avertissement** : il vous est demandé si vous voulez conserver la valeur malgré tout.
- **Informations** : vous êtes averti que la valeur n'est pas conforme, et vous pouvez continuer ou abandonner.

Une valeur que vous collez n'est pas soumise à la règle : elle est insérée telle quelle, sans aucune question. Il est toujours possible d'effacer une cellule.

## Choisir dans une liste

Une cellule associée à une liste affiche, lorsqu'elle contient le curseur, un petit bouton avec une flèche sur son bord droit. Cliquez sur ce bouton, ou appuyez sur **Alt+Bas**, pour afficher les choix, puis cliquez sur l'un d'eux pour le saisir.

## Fonctionnalités non prises en charge

JeffCrunch conserve les règles du classeur, mais ne propose pas encore de commande pour en créer. Les règles qui lisent leurs choix dans une autre feuille selon la syntaxe plus récente sont conservées dans le fichier, mais ne sont pas appliquées. Les messages qui s'affichent lorsque vous sélectionnez une cellule ne sont pas affichés.
