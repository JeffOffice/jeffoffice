# Trier et filtrer
<!-- keywords: trier, tri, filtrer, filtre, filtre automatique, croissant, décroissant, A-Z, règles, en-tête, couleur, liste, mois, jours de la semaine, majuscules, casse, colonnes, plus grandes valeurs, plus petites valeurs, moyenne, entre, commence par, contient, date, effacer, réappliquer, liste déroulante, lignes masquées, tableau, Ctrl+Maj+L -->

## Trier

Sélectionnez une cellule de vos données et choisissez **Trier de A à Z** ou **Trier de Z à A** (**Trier et filtrer** sous l'onglet Accueil). JeffCrunch prend tout le bloc de données autour de la cellule, conserve une ligne d'en-tête s'il en trouve une (du texte au-dessus de nombres) et classe les lignes selon la colonne dans laquelle vous vous trouvez. Si plusieurs cellules sont sélectionnées, seules celles-ci sont triées ; si des données se trouvent à côté, JeffCrunch vous demande s'il faut les inclure.

Dans l'ordre de tri, les nombres viennent en premier, puis le texte, puis VRAI et FAUX, puis les erreurs ; les cellules vides sont toujours placées en dernier. Les majuscules et les minuscules sont considérées comme identiques, sauf indication contraire. Le texte est trié de la même façon partout : selon l'alphabet anglais, une lettre accentuée étant classée avec sa lettre non accentuée, quelle que soit la langue de l'ordinateur.

Choisissez **Trier…** pour ouvrir la boîte de dialogue Trier :

- Ajoutez une règle pour chaque colonne de tri. Une règle suivante départage les égalités des règles précédentes. Utilisez **Monter** et **Descendre** pour modifier l'ordre des règles, et **Dupliquer la règle** pour partir d'une règle existante.
- Triez **Selon** les valeurs, la **couleur de remplissage** des cellules ou leur **couleur de texte**. Pour une couleur, choisissez si elle se place **En haut** ou **En bas** ; les autres lignes conservent leur ordre. Une couleur est comparée selon son rendu final : une couleur choisie dans la palette et une couleur ayant les mêmes valeurs de rouge, vert et bleu sont identiques.
- Pour les valeurs, l'ordre peut être **A–Z**, **Z–A** ou une liste : les jours de la semaine et les mois (en entier et abrégés, en anglais et dans votre langue), ou une liste que vous tapez, en séparant les valeurs par des virgules. Les valeurs absentes de la liste sont placées après celles qui y figurent.
- **La première ligne contient les noms des colonnes** exclut la première ligne du tri. **Respecter la casse** distingue les majuscules des minuscules (les minuscules en premier). **Trier les colonnes et non les lignes** classe les colonnes selon les valeurs d'une ligne.

Une formule se déplace avec sa ligne, et ses références à des cellules de la même ligne restent dans la ligne. Une référence à une cellule d'une autre ligne est modifiée comme si la formule était copiée. Ce qui est lié à une cellule l'accompagne : son format, sa note et son lien sont déplacés vers le nouvel emplacement de la cellule. Les règles qui s'appliquent à une plage de cellules (validation des données et mises en forme conditionnelles) restent en place, car elles concernent les emplacements et non les cellules. Un tri constitue une seule action que vous pouvez annuler, et **Échap** interrompt un tri volumineux ; un tri volumineux s'effectue par étapes.

Certaines cellules ne peuvent pas être triées, et la raison vous est indiquée : cellules verrouillées d'une feuille protégée, cellules faisant partie d'une matrice, cellules fusionnées de tailles différentes (ou s'étendant sur plusieurs lignes), cellules fusionnées coupées par le bord de la sélection et ligne d'en-tête d'un tableau.

## Filtrer

Sélectionnez une cellule de vos données et choisissez **Filtrer** (**Ctrl+Maj+L**). Un bouton apparaît sur chaque cellule de la première ligne. Cliquez sur l'un d'eux pour afficher :

- **Trier de A à Z** et **Trier de Z à A**, ainsi que **Trier par couleur**.
- La liste des valeurs de la colonne avec des cases à cocher, une zone de recherche et **(Tout)**. Une colonne de dates est présentée par année, mois et jour, et les valeurs qui ne sont pas des dates sont répertoriées en dessous. Décochez ce que vous ne voulez pas, puis cliquez sur **OK**. La recherche affiche les valeurs contenant tous vos mots ; **(Tous les résultats trouvés)** coche ou décoche alors uniquement celles-ci. La liste porte sur la colonne entière. Si la colonne contient plus de valeurs distinctes que la liste ne peut en afficher, un message l'indique et la liste est tronquée ; utilisez une condition pour le reste.
- **Conserver une couleur**, selon la couleur de remplissage ou la couleur du texte.
- **Conditions numériques**, **Conditions de texte** ou **Conditions de date** : est égal à, supérieur à, au moins, inférieur à, au plus, entre, les plus grandes ou plus petites valeurs (éléments ou pourcentage), au-dessus ou en dessous de la moyenne, commence par, se termine par, contient, dates d'une période (aujourd'hui, cette semaine, mois prochain, trimestre dernier, année à ce jour, un trimestre ou un mois d'une année quelconque), et **Condition personnalisée** composée de deux comparaisons reliées par Et ou Ou. Dans le texte d'une condition, ? et \* remplacent respectivement un caractère quelconque et une suite quelconque de caractères.
- **Afficher tout** de la colonne.

Un bouton portant un entonnoir signale une colonne filtrée. Les lignes qui ne correspondent pas sont masquées et conservent leur numéro de ligne. **Effacer** (dans le menu **Trier et filtrer**) supprime toutes les conditions et affiche les lignes ; **Réappliquer** recalcule les conditions après la modification d'une valeur. Choisir de nouveau **Filtrer** désactive les boutons.

Les lignes masquées par un filtre sont distinguées des lignes que vous avez masquées vous-même : **Afficher les lignes** n'affiche pas une ligne masquée par le filtre, et l'effacement du filtre n'affiche pas une ligne que vous avez masquée. Le tri d'un bloc filtré ne classe que les lignes visibles, entre elles.

Un filtre s'applique à une feuille entière, jusqu'à 1 048 576 lignes, par étapes ; **Effacer** et la désactivation du filtre fonctionnent quelle que soit la taille. Le travail demandé à un filtre est limité à 40 millions de vérifications (le nombre de lignes multiplié par le nombre de colonnes comportant une condition, plus une part pour les valeurs listées par les conditions) ; un filtre qui en demande davantage le signale et n'est pas créé : vous pouvez alors filtrer moins de lignes ou utiliser moins de conditions. Un filtre de la feuille ne couvre pas les cellules d'un tableau : un tableau possède ses propres boutons, qui fonctionnent de la même façon (voir [Tableaux](tables)).

Une liste de valeurs dans une condition est conservée telle que le fichier l'écrit, sans format de langue pour les nombres et les dates (1,234.50), et une valeur correspond à une cellule qui l'affiche dans votre langue ou sous cette forme ; un fichier filtré dans une langue filtre les mêmes cellules dans une autre.

Le filtre est enregistré dans le fichier avec ses conditions et les lignes qu'il masque, si bien que le fichier s'ouvre avec les mêmes lignes affichées. À la réouverture d'un fichier, l'entonnoir apparaît sur chaque colonne comportant une condition ; les lignes masquées dans la plage du filtre sont considérées comme masquées par le filtre.
