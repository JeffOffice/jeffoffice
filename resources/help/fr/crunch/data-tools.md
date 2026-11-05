# Outils de données
<!-- keywords: données, onglet données, convertir du texte en colonnes, scinder, séparateur, virgule, tabulation, largeur fixe, supprimer les doublons, valeur cible, sous-total, totaux, filtre avancé, conditions, validation des données, entourer les données non valides, ovale rouge, effacer les entourages de validation, consolider, consolider des plages, autoriser, liste, liste déroulante, message de saisie, alerte d'erreur, lignes répétées, unique, fichier texte, csv, à partir de texte, colonnes, délimiteur, guillemets -->

L'onglet **Données** regroupe les outils qui travaillent sur une liste de valeurs : obtenir du texte à partir d'un fichier, mettre une liste en ordre, découper du texte en colonnes et supprimer les lignes répétées.

## À partir de texte/CSV

**À partir de texte/CSV** ouvre un fichier texte (.csv, .tsv ou .txt). La commande est la même que **Ouvrir un fichier texte** dans l'onglet Fichier. JeffCrunch demande comment le fichier est réparti en colonnes et quel type de texte il contient, puis l'affiche dans un nouveau classeur.

## Trier et filtrer

Le groupe **Trier et filtrer** contient les mêmes commandes que le menu **Trier et filtrer** de l'onglet Accueil. Voir [Trier et filtrer](sort-filter).

## Convertir du texte en colonnes

Utilisez cette commande lorsqu'une colonne contient plusieurs morceaux de texte que vous voulez placer dans des colonnes séparées, par exemple « Dupont, Anne, 31 ».

1. Sélectionnez les cellules d'**une seule** colonne.
2. Choisissez **Convertir du texte en colonnes** dans l'onglet **Données**.
3. Choisissez comment scinder le texte :
   - **Séparer par un séparateur** : cochez les signes qui se trouvent entre les morceaux : **Tabulation**, **Point-virgule**, **Virgule**, **Espace** ou **Autre**, où vous saisissez votre propre signe. **Traiter les séparateurs consécutifs comme un seul** évite qu'deux virgules à la suite créent un morceau vide. **Texte entre guillemets** indique le signe qui encadre un morceau qu'il ne faut pas scinder, comme « Lee, Bo ». À l'intérieur des guillemets, deux signes à la suite représentent un seul signe.
   - **Séparer à des positions fixes** : saisissez les positions où commence une nouvelle colonne, en comptant les caractères à partir de 1. Par exemple, 5, 12 démarre une nouvelle colonne au cinquième et au douzième caractère.
4. Choisissez le format de chaque morceau : **Standard** interprète les nombres et les dates comme si vous les aviez saisis, et **Texte** conserve chaque morceau tel quel, de sorte que « 007 » reste « 007 ».
5. Dans **Placer les parties à partir de**, saisissez la première cellule des morceaux. Elle correspond au départ à la première cellule sélectionnée.
6. La zone située sous les choix affiche les premières lignes scindées selon vos choix. Cliquez sur **OK**.

Les morceaux sont placés dans les cellules situées à droite de la première cellule et remplacent leur contenu. La conversion du texte en colonnes peut être annulée en une seule étape.

## Supprimer les doublons

Utilisez cette commande pour ne conserver que la première ligne de chaque groupe de lignes identiques.

1. Sélectionnez les cellules de la liste, ou une cellule de celle-ci. JeffCrunch prend tout le bloc de données entourant une seule cellule.
2. Choisissez **Supprimer les doublons**.
3. Cochez **Mes données ont des en-têtes** si la première ligne contient des noms. Cette ligne est conservée et n'est pas comparée.
4. Cochez les colonnes qui doivent être identiques pour que deux lignes soient considérées comme des doublons. Deux lignes ne sont des doublons que si toutes les colonnes cochées sont identiques. Les majuscules et les minuscules sont considérées comme identiques, et une formule compte pour la valeur qu'elle donne.
5. Cliquez sur **OK**. La boîte de dialogue indique combien de lignes ont été supprimées et combien en restent.

Les lignes situées sous une ligne supprimée remontent à l'intérieur des cellules sélectionnées, avec leur mise en forme et leurs formules. Les cellules situées sous la sélection ne bougent pas. Les lignes vides laissées en bas de la sélection sont effacées. La suppression des doublons peut être annulée en une seule étape.

## Valeur cible

Utilisez cette commande lorsque vous connaissez le résultat souhaité, mais pas le nombre qui le produit. Par exemple, une cellule de mensualité contient une formule qui utilise un taux d'intérêt, et vous voulez que la mensualité soit de 500. La valeur cible essaie des nombres dans la cellule du taux jusqu'à ce que la mensualité soit égale à 500.

1. Choisissez **Valeur cible** dans l'onglet **Données**.
2. Dans **Cellule contenant la formule**, saisissez la cellule qui contient la formule, par exemple B3. Elle correspond au départ à la cellule sélectionnée.
3. Dans **Valeur souhaitée**, saisissez le résultat que doit donner la formule.
4. Dans **Cellule à modifier**, saisissez la cellule que la valeur cible peut modifier, par exemple B1. Elle doit contenir un nombre, et non une formule.
5. Cliquez sur **OK**.

Si la valeur cible trouve un nombre, celui-ci reste dans la cellule à modifier, et la barre d'état indique sa valeur. Si elle n'en trouve pas, rien ne change et un message vous le signale. Essayez une autre valeur de départ dans la cellule à modifier, et vérifiez que la formule utilise bien cette cellule. La valeur cible peut être annulée en une seule étape.

## Sous-total

Utilisez cette commande pour placer une ligne de totaux sous chaque groupe de lignes qui ont la même valeur dans une colonne, ainsi qu'un total général en bas. La liste doit avoir une ligne d'en-tête et être triée selon la colonne de regroupement, afin que les valeurs identiques se suivent.

1. Sélectionnez une cellule de la liste.
2. Choisissez **Sous-total** dans le groupe **Plan** de l'onglet **Données**.
3. Dans **À chaque modification de**, choisissez la colonne qui regroupe les lignes, par exemple Région.
4. Dans **Utiliser la fonction**, choisissez le calcul à effectuer : **Somme**, **Nombre**, **Moyenne**, **Max**, **Min**, et d'autres.
5. Dans **Ajouter un sous-total à**, cochez chaque colonne à totaliser.
6. Laissez **Remplacer les sous-totaux existants** coché pour remplacer les sous-totaux déjà présents. Cliquez sur **OK**.

Chaque ligne de total affiche « Total Est » (ou la valeur à laquelle elle correspond) et contient une formule SOUS.TOTAL, qui ignore les autres lignes de total. Les lignes de chaque groupe sont regroupées, de sorte que vous pouvez utiliser les boutons 1, 2 et 3 à côté des lignes pour n'afficher que les totaux. Pour supprimer les totaux, ouvrez à nouveau **Sous-total** et cliquez sur **Supprimer tout**. L'ajout ou la suppression de sous-totaux peut être annulé en une seule étape.

## Filtre avancé

Les boutons de filtre sélectionnent les lignes colonne par colonne. **Avancé** (dans le groupe **Trier et filtrer**) vous permet d'écrire les conditions dans la feuille, si longues soient-elles.

1. À côté de la liste, copiez les en-têtes des colonnes à tester dans une ligne de cellules vides.
2. Sous chaque en-tête, écrivez une condition. Les conditions situées sur la **même ligne** doivent toutes être vraies (c'est un « et »). Les conditions situées sur des **lignes différentes** sont des solutions de rechange : une ligne de la liste doit satisfaire une seule de ces lignes (c'est un « ou »).
3. Choisissez **Avancé**. Vérifiez que **Plage de la liste** correspond à la liste avec sa ligne d'en-tête, et saisissez les cellules de vos en-têtes et conditions dans **Plage des conditions**.
4. Choisissez **Masquer les autres lignes de la liste**, ou **Copier les lignes à un autre emplacement** et saisissez la première cellule de destination, par exemple H1. Cochez **Ignorer les lignes en double** pour ne copier qu'une seule occurrence de chaque ligne.
5. Cliquez sur **OK**.

Comment écrire une condition :

- Un mot, comme **Dup**, sélectionne le texte qui commence par ce mot. Les majuscules et les minuscules sont considérées comme identiques. **?** remplace un caractère quelconque et ***** remplace une suite de caractères quelconque.
- Un nombre sélectionne les cellules qui lui sont égales.
- **=Anne** sélectionne le texte qui est exactement Anne. **=** seul sélectionne les cellules vides.
- **>**, **>=**, **<**, **<=** et **<>** (différent de) placés avant un nombre ou un mot effectuent la comparaison, par exemple **>25**. **<>** seul sélectionne les cellules non vides.

Une ligne de conditions vide sélectionne toutes les lignes. Pour afficher de nouveau les lignes masquées, choisissez **Effacer**. Une copie est créée immédiatement et ne change pas lorsque la liste change. Le filtre avancé peut être annulé en une seule étape.

## Validation des données

Utilisez cette commande pour définir ce qui peut être saisi dans certaines cellules, afin qu'une entrée erronée soit bloquée ou contestée.

1. Sélectionnez les cellules.
2. Choisissez **Validation des données** dans le groupe **Outils de données** de l'onglet **Données**.
3. Dans l'onglet **Paramètres**, choisissez ce qui est **Autoriser** : **Toute valeur** (pas de règle), **Nombre entier**, **Décimal**, **Liste**, **Date**, **Heure**, **Longueur du texte** ou **Personnalisé**.
4. Pour un nombre, une date, une heure ou une longueur de texte, choisissez **Données** (**comprise entre**, **non comprise entre**, **égal à**, etc.) et saisissez les limites. Une limite peut être une valeur ou une formule qui commence par =.
5. Pour une **Liste**, saisissez les choix séparés par des points-virgules, par exemple Rouge;Vert;Bleu, ou saisissez une plage de cellules après =, par exemple =$A$1:$A$5. La cellule affiche alors une flèche donnant accès aux choix. Désactivez **Liste déroulante dans la cellule** pour masquer la flèche.
6. Pour **Personnalisé**, saisissez une formule qui commence par =. La saisie est autorisée lorsque la formule est vraie.
7. **Ignorer les blancs** laisse passer une cellule vide.

Dans l'onglet **Message de saisie**, cochez la case et rédigez un titre et un message. Ils s'affichent à côté de la cellule chaque fois qu'elle est sélectionnée.

Dans l'onglet **Alerte d'erreur**, choisissez ce qui se passe pour une entrée non autorisée. **Arrêt** la refuse. **Avertissement** demande s'il faut la conserver. **Information** signale la saisie à l'utilisateur et la conserve. Rédigez un titre et un message pour l'alerte.

**Entourer les données non valides** (à côté de **Validation des données**) place un ovale rouge autour de chaque cellule de la feuille dont la valeur n'est pas autorisée par sa règle, par exemple un nombre saisi avant la création de la règle, ou le résultat d'une formule. Un message indique si toutes les cellules sont correctes. Une cellule que vous corrigez perd son ovale. **Effacer les entourages de validation** supprime tous les ovales.

**Effacer tout** supprime les règles des cellules sélectionnées. La nouvelle règle remplace toute règle que les cellules sélectionnées possédaient ; les autres cellules conservent les leurs. La modification des règles peut être annulée en une seule étape, et les règles sont enregistrées dans le fichier.

## Consolider

Utilisez cette commande pour combiner des plages semblables, comme un même tableau présent dans plusieurs feuilles, en un seul tableau.

1. Choisissez **Consolider** dans le groupe **Outils de données** de l'onglet **Données**.
2. Choisissez la **Fonction** à utiliser pour la combinaison, par exemple **Somme** ou **Moyenne**.
3. Saisissez une plage dans la zone et cliquez sur **Ajouter**. Utilisez **Feuil2!A1:C10** pour une plage située dans une autre feuille. Ajoutez chaque plage à combiner. Sélectionnez une plage dans la liste et cliquez sur **Supprimer** pour la retirer.
4. Si les plages ont des noms dans leur première ligne, cochez **La ligne du haut contient les noms des colonnes**. Si elles ont des noms dans leur première colonne, cochez **La colonne de gauche contient les noms des lignes**.
5. Dans **Placer le résultat à**, saisissez la première cellule du résultat.
6. Cochez **Créer des liens vers les données source** si chaque résultat doit être une formule qui lit ses cellules sources, et donc se met à jour avec elles. Sans cette case, les résultats sont des nombres simples.
7. Cliquez sur **OK**.

Si aucun nom n'est coché, la cellule située au même emplacement dans chaque plage est combinée (la première cellule de chaque plage, puis la deuxième, et ainsi de suite). Si les noms sont cochés, les cellules portant le même nom sont combinées, quel que soit leur emplacement dans chaque plage, et un nom qui n'existe que dans certaines plages donne tout de même une ligne ou une colonne. La consolidation peut être annulée en une seule étape.
