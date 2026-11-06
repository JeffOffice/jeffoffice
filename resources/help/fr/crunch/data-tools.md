# Outils de données
<!-- keywords: gestionnaire de scénarios, scénarios, simulation, que se passe-t-il si, synthèse des scénarios, données, onglet données, convertir du texte en colonnes, scinder, séparateur, virgule, tabulation, largeur fixe, supprimer les doublons, valeur cible, sous-total, totaux, filtre avancé, conditions, validation des données, entourer les données non valides, ovale rouge, effacer les entourages de validation, consolider, combiner des plages, autoriser, liste, liste déroulante, message de saisie, alerte d'erreur, lignes répétées, unique, fichier texte, csv, à partir de texte, colonnes, délimiteur, guillemets, modifier les liens, liens, externe, source, rompre le lien, vérifier l'état, ouvrir la source -->

L'onglet **Données** contient les outils qui travaillent sur une liste de valeurs : obtenir du texte depuis un fichier, mettre une liste en ordre, répartir un texte en colonnes et supprimer les lignes répétées.

## À partir de texte/CSV

**À partir de texte/CSV** ouvre un fichier texte (.csv, .tsv ou .txt). La commande est identique à **Ouvrir un fichier texte** dans l'onglet **Fichier**. JeffCrunch vous demande comment le fichier est découpé en colonnes et quel type de texte il contient, puis l'affiche dans un nouveau classeur.

## Modifier les liens

Si un classeur contient des formules qui lisent un autre classeur (comme ='[Prix.xlsx]Feuil1'!A1), **Modifier les liens**, dans le groupe **Obtenir des données**, affiche la liste de ces autres classeurs, appelés sources. Chaque ligne indique où se trouve la source et son état. Tant que le classeur n'a pas été ouvert à nouveau avec ses sources, il affiche les nombres enregistrés dans son fichier.

- **Vérifier l'état** cherche chaque source sur cet ordinateur et indique **Trouvée**, ou qu'elle est introuvable. Une source sur le web n'est pas récupérée : son état indique qu'elle n'a pas été vérifiée.
- **Ouvrir la source** ouvre la source choisie dans une nouvelle fenêtre, si elle peut être trouvée.
- **Rompre le lien** remplace chaque cellule qui lit la source choisie par la valeur qu'elle affiche actuellement, et un nom qui désignait un emplacement dans cette source par **#REF!**. Cette action est irréversible, donc JeffCrunch vous demande confirmation. Lorsqu'il ne reste plus aucun lien, le fichier est enregistré sans les parties qui les contenaient.

## Trier et filtrer

Le groupe **Trier et filtrer** contient les mêmes commandes que le menu **Trier et filtrer les lignes** de l'onglet **Accueil**. Voir [Trier et filtrer](sort-filter).

## Convertir du texte en colonnes

Utilisez cette commande lorsqu'une colonne contient plusieurs éléments de texte que vous voulez placer dans des colonnes séparées, comme « Dupont, Anne, 31 ».

1. Sélectionnez les cellules d'**une seule** colonne.
2. Choisissez **Convertir du texte en colonnes** dans l'onglet **Données**.
3. Choisissez comment scinder le texte :
   - **Scinder à un séparateur** : cochez les signes qui séparent les éléments : **Tabulation**, **Point-virgule**, **Virgule**, **Espace** ou **Autre**, où vous saisissez votre propre signe. **Traiter les séparateurs consécutifs comme un seul** évite qu'deux virgules à la suite créent un élément vide. **Texte entre guillemets** indique le signe qui encadre un élément à ne pas scinder, comme « Lee, Bo ». À l'intérieur des guillemets, deux signes à la suite représentent un seul signe.
   - **Scinder à des positions fixes** : saisissez les positions où commence une nouvelle colonne, en comptant les caractères à partir de 1. Par exemple, 5, 12 commence une nouvelle colonne au cinquième et au douzième caractère.
4. Choisissez le type de chaque élément : **Standard** interprète les nombres et les dates comme si vous les aviez saisis, et **Texte** conserve chaque élément tel quel, de sorte que « 007 » reste « 007 ».
5. Dans **Placer les parties à partir de**, saisissez la première cellule des éléments. Elle correspond d'abord à la première cellule sélectionnée.
6. La zone sous les choix affiche les premières lignes scindées selon vos options. Cliquez sur **OK**.

Les éléments sont placés dans les cellules situées à droite de la première cellule, et remplacent ce qui s'y trouvait. Convertir du texte en colonnes peut être annulé en une seule étape.

## Supprimer les doublons

Utilisez cette commande pour ne garder que la première ligne de chaque groupe de lignes répétées.

1. Sélectionnez les cellules de la liste, ou une seule cellule de celle-ci. JeffCrunch prend le bloc de données entier autour d'une cellule.
2. Choisissez **Supprimer les doublons**.
3. Cochez **Mes données ont des en-têtes** si la première ligne contient des noms. Cette ligne est conservée et n'est pas comparée.
4. Cochez les colonnes qui doivent être identiques pour que deux lignes soient considérées comme répétées. Deux lignes ne sont répétées que si toutes les colonnes cochées sont identiques. Les majuscules et les minuscules sont considérées comme identiques, et une formule est comparée à la valeur qu'elle donne.
5. Cliquez sur **OK**. La boîte de dialogue indique combien de lignes ont été supprimées et combien en restent.

Les lignes situées sous une ligne supprimée remontent à l'intérieur des cellules sélectionnées, avec leur mise en forme et leurs formules. Les cellules situées sous la sélection ne bougent pas. Les lignes vides laissées en bas de la sélection sont effacées. Supprimer les doublons peut être annulé en une seule étape.

## Valeur cible

Utilisez cette commande lorsque vous connaissez le résultat voulu, mais pas le nombre qui le produit. Par exemple, une cellule de mensualité contient une formule qui utilise un taux d'intérêt, et vous voulez que la mensualité soit de 500. La valeur cible essaie des nombres dans la cellule du taux jusqu'à ce que la mensualité soit égale à 500.

1. Choisissez **Valeur cible** dans l'onglet **Données**.
2. Dans **Cellule contenant la formule**, saisissez la cellule qui contient la formule, par exemple B3. Elle correspond d'abord à la cellule que vous avez sélectionnée.
3. Dans **Valeur souhaitée**, saisissez le résultat que vous voulez obtenir avec la formule.
4. Dans **Cellule à modifier**, saisissez la cellule que la valeur cible peut modifier, par exemple B1. Elle doit contenir un nombre, et non une formule.
5. Cliquez sur **OK**.

Si la valeur cible trouve un nombre, celui-ci reste dans la cellule à modifier, et la barre d'état indique sa valeur. Si elle n'en trouve pas, rien n'est modifié et un message vous le signale. Essayez un autre nombre de départ dans la cellule à modifier, et vérifiez que la formule utilise bien cette cellule. Valeur cible peut être annulée en une seule étape.

## Gestionnaire de scénarios

Un scénario est un ensemble nommé de valeurs pour certaines cellules. Utilisez les scénarios pour tester des hypothèses « que se passe-t-il si » sur une même feuille. Par exemple, conservez un scénario « Ventes faibles » et un scénario « Ventes fortes » pour les mêmes cellules de données, et affichez celui que vous voulez examiner.

1. Choisissez **Gestionnaire de scénarios** dans l'onglet **Données**.
2. Cliquez sur **Ajouter**. Saisissez un nom, par exemple Ventes fortes. Dans **Cellules modifiables**, saisissez les cellules que le scénario remplit, par exemple B1:B3 ou D5. Un scénario peut modifier jusqu'à 32 cellules. Les cellules que vous avez sélectionnées sont remplies pour vous. Chaque cellule commence avec la valeur qu'elle contient actuellement. Saisissez dans le tableau la valeur que ce scénario doit utiliser, puis cliquez sur **OK**.
3. Sélectionnez un scénario dans la liste et cliquez sur **Afficher** pour placer ses valeurs dans les cellules. Un double-clic sur un nom produit le même effet. Cliquez sur **Modifier** pour changer son nom, son commentaire ou ses valeurs. Cliquez sur **Supprimer** pour le retirer.

Afficher, Ajouter, Modifier et Supprimer peuvent chacun être annulés en une étape.

**Synthèse** place tous les scénarios côte à côte sur une nouvelle feuille nommée Synthèse des scénarios. Elle demande les cellules de résultat, c'est-à-dire les cellules que vous voulez comparer, par exemple le total en B10. Pour chaque scénario, la synthèse affiche les valeurs des cellules modifiables et le résultat des cellules de résultat, calculé avec les valeurs de ce scénario. Une colonne nommée Valeurs actuelles indique l'état des cellules au moment où vous avez créé la synthèse. Vos propres cellules restent inchangées.

Les scénarios sont enregistrés dans le classeur. Un classeur créé par un autre logiciel conserve ses scénarios, que vous voyez ici. Une copie enregistrée au format .xls ancien ou au format .ods ne les conserve pas, et l'enregistrement vous en avertit.

## Sous-total

Utilisez cette commande pour placer une ligne de totaux sous chaque groupe de lignes ayant la même valeur dans une colonne, ainsi qu'un total général en bas. La liste doit avoir une ligne d'en-tête et être triée par la colonne de regroupement, afin que les valeurs identiques soient côte à côte.

1. Sélectionnez une cellule de la liste.
2. Choisissez **Sous-total** dans le groupe **Plan** de l'onglet **Données**.
3. Dans **À chaque modification de**, choisissez la colonne qui regroupe les lignes, par exemple Région.
4. Dans **Utiliser la fonction**, choisissez le calcul à effectuer : **Somme**, **Nombre**, **Moyenne**, **Max**, **Min**, et d'autres.
5. Dans **Ajouter un sous-total à**, cochez chaque colonne à totaliser.
6. Laissez **Remplacer les sous-totaux existants** coché pour remplacer les sous-totaux déjà présents. Cliquez sur **OK**.

Chaque ligne de total indique « Total Est » (ou la valeur qu'elle concerne) et contient une formule SOUS.TOTAL, qui ignore donc les autres lignes de total. Les lignes de chaque groupe sont regroupées : vous pouvez utiliser les boutons 1, 2 et 3 à côté des lignes pour n'afficher que les totaux. Pour supprimer les totaux, ouvrez à nouveau **Sous-total** et cliquez sur **Tout supprimer**. Ajouter ou supprimer des sous-totaux peut être annulé en une seule étape.

## Filtre avancé

Les boutons de filtre sélectionnent les lignes une colonne à la fois. **Avancé** (dans le groupe **Trier et filtrer**) vous permet d'écrire les conditions dans la feuille, si longues soient-elles.

1. À côté de la liste, copiez les en-têtes des colonnes à tester dans une ligne de cellules vides.
2. Sous chaque en-tête, écrivez une condition. Les conditions situées sur la **même ligne** doivent toutes être vraies (c'est un « et »). Les conditions situées sur des **lignes différentes** sont des solutions de rechange : une ligne de la liste doit satisfaire une seule de ces lignes (c'est un « ou »).
3. Choisissez **Avancé**. Vérifiez que **Plage de la liste** correspond à la liste avec sa ligne d'en-tête, puis saisissez les cellules de vos en-têtes et conditions dans **Plage des conditions**.
4. Choisissez **Masquer les autres lignes de la liste**, ou **Copier les lignes à un autre emplacement** et saisissez la première cellule de destination, par exemple H1. Cochez **Ignorer les lignes en double** pour ne copier qu'une occurrence de chaque ligne.
5. Cliquez sur **OK**.

Comment écrire une condition :

- Un mot, par exemple **Du**, sélectionne le texte qui commence par ce mot. Les majuscules et les minuscules sont considérées comme identiques. **?** remplace un caractère quelconque et ***** remplace une suite quelconque de caractères.
- Un nombre sélectionne les cellules qui lui sont égales.
- **=Anne** sélectionne le texte qui est exactement Anne. **=** seul sélectionne les cellules vides.
- **>**, **>=**, **<**, **<=** et **<>** (différent de), placés devant un nombre ou un mot, permettent la comparaison, par exemple **>25**. **<>** seul sélectionne les cellules non vides.

Une ligne de conditions vide sélectionne toutes les lignes. Pour afficher à nouveau les lignes masquées, choisissez **Effacer**. Une copie est créée immédiatement et ne change pas lorsque la liste change. Filtre avancé peut être annulé en une seule étape.

## Validation des données

Utilisez cette commande pour définir ce qui peut être saisi dans certaines cellules, afin qu'une saisie erronée soit bloquée ou contestée.

1. Sélectionnez les cellules.
2. Choisissez **Validation des données** dans le groupe **Outils de données** de l'onglet **Données**.
3. Dans l'onglet **Paramètres**, choisissez ce qu'il faut **Autoriser** : **Toute valeur** (aucune règle), **Nombre entier**, **Décimal**, **Liste**, **Date**, **Heure**, **Longueur du texte** ou **Personnalisé**.
4. Pour un nombre, une date, une heure ou une longueur de texte, choisissez **Données** (**compris entre**, **non compris entre**, **égal à**, etc.) et saisissez les limites. Une limite peut être une valeur ou une formule commençant par =.
5. Pour une **Liste**, saisissez les choix séparés par des virgules, par exemple Rouge,Vert,Bleu, ou saisissez une plage de cellules après =, par exemple =$A$1:$A$5. La cellule affiche alors une flèche donnant accès aux choix. Désactivez **Liste déroulante dans la cellule** pour masquer la flèche.
6. Pour **Personnalisé**, saisissez une formule commençant par =. La saisie est autorisée lorsque la formule est vraie.
7. **Ignorer les blancs** laisse passer une cellule vide.

Dans l'onglet **Message de saisie**, cochez la case et saisissez un titre et un message. Ils s'affichent à côté de la cellule chaque fois qu'elle est sélectionnée.

Dans l'onglet **Alerte d'erreur**, choisissez ce qui se passe pour une saisie non autorisée. **Arrêt** refuse la saisie. **Avertissement** demande si la saisie doit être conservée. **Information** informe l'utilisateur et conserve la saisie. Saisissez un titre et un message pour l'alerte.

**Entourer les données non valides** (à côté de **Validation des données**) place un ovale rouge autour de chaque cellule de la feuille dont la valeur n'est pas autorisée par sa règle, par exemple un nombre saisi avant l'application de la règle, ou le résultat d'une formule. Un message vous indique si toutes les cellules sont conformes. Une cellule que vous corrigez perd son ovale. **Effacer les entourages de validation** supprime tous les ovales.

**Effacer tout** supprime les règles des cellules sélectionnées. La nouvelle règle remplace toute règle que les cellules sélectionnées possédaient ; les autres cellules conservent les leurs. La modification des règles peut être annulée en une seule étape, et les règles sont enregistrées dans le fichier.

## Consolider

Utilisez cette commande pour combiner des plages semblables, comme le même tableau provenant de plusieurs feuilles, en un seul tableau.

1. Choisissez **Consolider** dans le groupe **Outils de données** de l'onglet **Données**.
2. Choisissez la **Fonction** à appliquer, par exemple **Somme** ou **Moyenne**.
3. Saisissez une plage dans la zone et cliquez sur **Ajouter**. Utilisez **Feuil2!A1:C10** pour une plage située sur une autre feuille. Ajoutez chaque plage à combiner. Sélectionnez une plage dans la liste et cliquez sur **Supprimer** pour la retirer.
4. Si les plages ont des noms dans leur première ligne, cochez **La ligne du haut contient les noms des colonnes**. Si elles ont des noms dans leur première colonne, cochez **La colonne de gauche contient les noms des lignes**.
5. Dans **Placer le résultat à**, saisissez la première cellule du résultat.
6. Cochez **Créer des liens vers les données source** si chaque résultat doit être une formule qui lit ses cellules source, et donc se met à jour avec elles. Sans cette option, les résultats sont de simples nombres.
7. Cliquez sur **OK**.

Si aucun nom n'est coché, la cellule située au même emplacement dans chaque plage est combinée (la première cellule de chaque plage, puis la deuxième, et ainsi de suite). Si les noms sont cochés, les cellules portant le même nom sont combinées, où qu'elles se trouvent dans chaque plage, et un nom présent dans certaines plages seulement donne tout de même une ligne ou une colonne. Consolider peut être annulé en une seule étape.
