# Bienvenue dans JeffCrunch
<!-- keywords: bienvenue, premiers pas, introduction, débutant, prise en main, démarrage rapide, présentation, qu'est-ce que JeffCrunch, bases, bêta, tableur, classeur, feuille, cellules, ce qui fonctionne, pas encore, limites -->

JeffCrunch est un logiciel de tableur. Il est gratuit et libre. Aucun paiement, aucun abonnement et aucun compte à créer ne sont nécessaires. Il fait partie des applications JeffOffice et fonctionne sous Windows, macOS et Linux.

Le fichier sur lequel vous travaillez s'appelle un **classeur**. Un classeur contient une ou plusieurs **feuilles**, et chaque feuille est une grille de **cellules**. JeffCrunch enregistre les classeurs au format .xlsx. Il ouvre aussi les classeurs .xlsx et .xlsm créés par d'autres logiciels, les anciens fichiers .xls, les feuilles de calcul OpenDocument et les fichiers texte de valeurs. Il conserve ce qu'il ne peut pas encore modifier.

## JeffCrunch est une version bêta

Cette version de JeffCrunch est une version **bêta**. Elle peut servir à un travail réel, mais elle ne fait pas encore tout. Conservez une copie de tout classeur important.

**Ce qui fonctionne**

- Ouverture de classeurs : .xlsx et .xlsm, anciens .xls, feuilles de calcul OpenDocument (.ods et .fods), et fichiers texte de valeurs (.csv, .tsv et .txt). Enregistrement au format .xlsx, et copies au format .xls ou .ods. Voir [Ouvrir et enregistrer des classeurs](open-save).
- Ouverture de classeurs protégés par un mot de passe (.xlsx, .xls et .ods), et protection d'un classeur .xlsx par un mot de passe. Voir [Ouvrir un classeur protégé par un mot de passe](open-password).
- Saisie de nombres, de texte et de formules dans les cellules, avec annulation et rétablissement. Les formules sont calculées pendant la saisie, avec plus de 200 fonctions. Vous pouvez cliquer sur des cellules ou utiliser les flèches pendant la saisie d'une formule, et la boîte de dialogue **Arguments de la fonction** remplit les arguments d'une fonction avec un résultat en direct. Voir [Saisir dans les cellules](edit-cells), [Affichage et calcul des formules](formulas) et [Saisir une fonction avec la boîte de dialogue Arguments de la fonction](function-arguments).
- Mise en forme des cellules : polices, couleurs, bordures, formats de nombre, styles de cellule et cellules fusionnées. Voir [Mettre en forme les cellules](format-cells).
- Couper, copier et coller, recopier des cellules, faire la somme, rechercher et remplacer, trier, filtrer, tableaux et tableaux croisés dynamiques. Voir [Tableaux croisés dynamiques](pivot-tables).
- Lignes, colonnes et feuilles : insertion, suppression, dimensionnement, masquage et déplacement.
- Graphiques, images et formes, ainsi que notes et liens.
- Protection d'une feuille ou du classeur.
- Mise en page, aperçu avant impression, impression et création d'un PDF. Voir [Imprimer et créer un PDF](printing).

**Ce qui ne fonctionne pas encore**

- Vous ne pouvez pas créer, modifier ni supprimer les règles de mise en forme conditionnelle ou de validation des données. JeffCrunch affiche celles qui existent dans un classeur et les conserve.
- Vous ne pouvez pas modifier la définition d'un style de cellule, ni en créer un nouveau.
- Une fonction que JeffCrunch ne connaît pas encore laisse une nouvelle formule sans valeur ; une formule présente dans le classeur d'origine conserve le résultat enregistré dans le fichier.
- Les macros sont conservées et réenregistrées, mais jamais exécutées.
- Les tableaux croisés dynamiques ont quelques limites, que [Tableaux croisés dynamiques](pivot-tables) énumère.
- Un classeur ouvert depuis un .xls, un .ods ou un fichier texte est enregistré comme nouveau fichier .xlsx, sauf si vous choisissez une copie .xls ou .ods. Une copie ne peut pas tout contenir : JeffCrunch indique ce qu'elle omettra avant d'enregistrer.
- Un mot de passe peut protéger un classeur .xlsx ainsi qu'une copie .xls, .ods ou .ots de celui-ci, mais pas une copie en texte.
- Une table de données (table d'hypothèses) peut être calculée, mais pas créée.

## Faites d'abord ces cinq choses

1. **Ouvrir un classeur.** Appuyez sur **{key:Ouvrir}**, ou cliquez sur **Fichier** puis sur **Ouvrir**. Les premières lignes apparaissent immédiatement, même dans un très grand classeur, pendant que JeffCrunch lit le reste.
2. **Se repérer.** Cliquez sur une cellule pour la sélectionner. Son adresse figure dans la **zone Nom**, en haut à gauche, et son contenu dans la **barre de formule** à côté. Utilisez les touches fléchées, la molette de la souris et les barres de défilement pour vous déplacer. Voir [Sélectionner des cellules et se déplacer](select-and-move).
3. **Modifier une cellule.** Sélectionnez une cellule et saisissez du texte. Appuyez sur **Entrée** pour valider votre saisie. Voir [Saisir dans les cellules](edit-cells).
4. **Mettre en forme les cellules.** Sélectionnez des cellules et utilisez l'onglet **Accueil** pour changer la police, les couleurs, les bordures et le format de nombre. Voir [Mettre en forme les cellules](format-cells).
5. **Modifier l'affichage.** Utilisez l'onglet **Affichage** pour faire un zoom, afficher ou masquer le quadrillage et les en-têtes, et figer les premières lignes ou les premières colonnes. Voir [Modifier l'affichage](view).

## Ce que vous voyez

Un classeur s'affiche tel qu'il a été enregistré : largeurs des colonnes et hauteurs des lignes, polices, couleurs, remplissages et bordures, cellules fusionnées, volets figés, et formats de nombre qui transforment un nombre stocké en date, en pourcentage ou en prix. Un texte trop long pour sa cellule déborde sur les cellules vides voisines. Un nombre trop large pour sa cellule s'affiche sous forme de signes # ; élargissez la colonne, ou sélectionnez la cellule pour lire le nombre dans la barre de formule. Les nombres sont écrits selon les conventions de la langue de votre ordinateur (voir [Formats de nombre et langue](number-formats)), et les mises en forme conditionnelles d'un classeur, comme les échelles de couleurs, les barres de données et les icônes, sont affichées (voir [Mises en forme conditionnelles](conditional-formats)).

Enregistrez votre travail avec **{key:Enregistrer}**, qui écrit vos modifications dans le classeur ouvert, ou avec **Enregistrer sous**, qui les écrit dans un nouveau fichier. Voir [Ouvrir et enregistrer des classeurs](open-save).
