# Outils de données
<!-- keywords: données, onglet données, convertir texte en colonnes, scinder, séparateur, virgule, tabulation, largeur fixe, supprimer les doublons, lignes répétées, unique, fichier texte, csv, à partir de texte, colonnes, délimiteur, guillemets -->

L'onglet **Données** contient les outils qui agissent sur une liste de valeurs : obtenir du texte depuis un fichier, mettre une liste en ordre, répartir du texte en colonnes et supprimer les lignes répétées.

## À partir de texte/CSV

**À partir de texte/CSV** ouvre un fichier texte (.csv, .tsv ou .txt). La commande est identique à **Ouvrir un fichier texte** dans l'onglet **Fichier**. JeffCrunch demande comment le fichier est réparti en colonnes et quel type de texte il contient, puis l'affiche dans un nouveau classeur.

## Trier et filtrer

Le groupe **Trier et filtrer** contient les mêmes commandes que le menu **Trier et filtrer** de l'onglet **Accueil**. Voir [Trier et filtrer](sort-filter).

## Convertir du texte en colonnes

Utilisez cette commande lorsqu'une colonne contient plusieurs éléments de texte que vous voulez placer dans des colonnes distinctes, par exemple « Martin, Anne, 31 ».

1. Sélectionnez les cellules d'**une seule** colonne.
2. Choisissez **Convertir du texte en colonnes** dans l'onglet **Données**.
3. Choisissez comment scinder le texte :
   - **Séparer par un séparateur** : cochez les signes qui se trouvent entre les éléments : **Tabulation**, **Point-virgule**, **Virgule**, **Espace** ou **Autre**, où vous saisissez votre propre signe. **Traiter les séparateurs consécutifs comme un seul** évite qu'deux virgules qui se suivent créent un élément vide. **Texte entre guillemets** indique le signe qui encadre un élément à ne pas scinder, comme « Lee, Bo ». À l'intérieur des guillemets, deux signes qui se suivent comptent pour un seul.
   - **Séparer à des positions fixes** : saisissez les positions où commence une nouvelle colonne, en comptant les caractères à partir de 1. Par exemple, 5, 12 démarre une nouvelle colonne au cinquième et au douzième caractère.
4. Choisissez le type de chaque élément : **Standard** interprète les nombres et les dates comme si vous les aviez saisis, et **Texte** conserve chaque élément tel quel, de sorte que « 007 » reste « 007 ».
5. Dans **Placer les éléments à partir de**, saisissez la première cellule de destination. Par défaut, il s'agit de la première cellule sélectionnée.
6. La zone située sous les choix affiche les premières lignes scindées selon vos réglages. Cliquez sur **OK**.

Les éléments sont placés dans les cellules situées à droite de la première cellule et remplacent le contenu qui s'y trouvait. La conversion du texte en colonnes est une seule étape que vous pouvez annuler.

## Supprimer les doublons

Utilisez cette commande pour ne conserver que la première ligne de chaque groupe de lignes identiques.

1. Sélectionnez les cellules de la liste, ou une cellule de celle-ci. JeffCrunch prend en compte tout le bloc de données entourant une cellule unique.
2. Choisissez **Supprimer les doublons**.
3. Cochez **Mes données ont des en-têtes** si la première ligne contient des noms. Cette ligne est conservée et n'est pas comparée.
4. Cochez les colonnes qui doivent être identiques pour que deux lignes soient considérées comme des doublons. Deux lignes ne sont des doublons que si toutes les colonnes cochées sont identiques. Les majuscules et les minuscules sont considérées comme équivalentes, et une formule est comptée d'après la valeur qu'elle donne.
5. Cliquez sur **OK**. La boîte de dialogue indique combien de lignes ont été supprimées et combien en restent.

Les lignes situées sous une ligne supprimée remontent à l'intérieur des cellules sélectionnées, avec leur mise en forme et leurs formules. Les cellules situées sous la sélection ne bougent pas. Les lignes vides laissées en bas de la sélection sont effacées. La suppression des doublons est une seule étape que vous pouvez annuler.
