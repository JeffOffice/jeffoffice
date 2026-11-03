# Bienvenue dans JeffCrunch
<!-- keywords: bienvenue, prise en main, démarrage, premiers pas, introduction, débutant, démarrage rapide, présentation, vue d'ensemble, qu'est-ce que JeffCrunch, notions de base, bêta, tableur, feuille de calcul, classeur, feuille, cellules, fonctionnalités, ce qui fonctionne, pas encore, limites -->

JeffCrunch est un tableur. Il est libre et open source. Il n'y a ni frais, ni abonnement, ni compte à créer. C'est l'une des applications JeffOffice, et il fonctionne sous Windows, macOS et Linux.

Le fichier sur lequel vous travaillez s'appelle un **classeur**. Un classeur contient une ou plusieurs **feuilles**, et chaque feuille est une grille de **cellules**. JeffCrunch enregistre les classeurs au format .xlsx. Il ouvre aussi les classeurs .xlsx et .xlsm créés par d'autres programmes, ainsi que les anciens fichiers .xls, les feuilles de calcul OpenDocument et les fichiers texte de valeurs. Il conserve ce qu'il ne sait pas encore modifier.

## JeffCrunch est une version bêta

Cette version de JeffCrunch est une version **bêta**. Elle est prête pour un usage réel, mais elle ne fait pas encore tout. Conservez une copie de tout classeur important.

**Ce qui fonctionne**

- L'ouverture des classeurs : .xlsx et .xlsm, anciens fichiers .xls, feuilles de calcul OpenDocument (.ods et .fods) et fichiers texte de valeurs (.csv, .tsv et .txt). L'enregistrement au format .xlsx, et de copies au format .xls ou .ods. Consultez [Ouvrir et enregistrer des classeurs](open-save).
- L'ouverture des classeurs protégés par un mot de passe (.xlsx, .xls et .ods), et la protection d'un classeur .xlsx par un mot de passe. Consultez [Ouvrir des classeurs protégés par mot de passe](open-password).
- La saisie de nombres, de texte et de formules dans les cellules, avec annulation et rétablissement. Les formules sont calculées au fur et à mesure de la frappe, avec plus de 200 fonctions. Pendant la saisie d'une formule, vous pouvez cliquer sur des cellules ou vous y rendre avec les touches de direction, et la boîte de dialogue **Arguments de la fonction** remplit les arguments d'une fonction en affichant le résultat en direct. Consultez [Saisir dans les cellules](edit-cells), [Affichage et calcul des formules](formulas) et [Entrer une fonction avec la boîte de dialogue Arguments de la fonction](function-arguments).
- La mise en forme des cellules : polices, couleurs, bordures, formats de nombre, styles de cellules et cellules fusionnées. Consultez [Mettre en forme les cellules](format-cells).
- Couper, copier et coller, recopier des cellules, les additionner, rechercher et remplacer, trier, filtrer, les tableaux et les tableaux croisés dynamiques. Consultez [Tableaux croisés dynamiques](pivot-tables).
- Les lignes, les colonnes et les feuilles : insertion, suppression, dimensionnement, masquage et déplacement.
- Les graphiques, les images et les formes, ainsi que les notes et les liens.
- La protection d'une feuille ou du classeur.
- La mise en page, l'aperçu avant impression, l'impression et la création d'un PDF. Consultez [Imprimer et créer un PDF](printing).

**Ce qui ne fonctionne pas encore**

- Vous ne pouvez pas créer, modifier ni supprimer des règles de mise en forme conditionnelle ou de validation des données. JeffCrunch affiche celles que contient un classeur et les conserve.
- Vous ne pouvez pas modifier la définition d'un style de cellule, ni en créer un nouveau.
- Une fonction que JeffCrunch ne connaît pas encore laisse une nouvelle formule sans valeur ; une formule fournie avec le classeur conserve le résultat enregistré dans le fichier.
- Les macros sont conservées et réenregistrées, mais jamais exécutées.
- Les tableaux croisés dynamiques ont quelques limites, répertoriées dans [Tableaux croisés dynamiques](pivot-tables).
- Un classeur ouvert à partir d'un fichier .xls, .ods ou texte est enregistré comme nouveau fichier .xlsx, sauf si vous choisissez une copie .xls ou .ods. Une copie ne peut pas tout contenir, et JeffCrunch indique ce qu'il va omettre avant d'enregistrer.
- Un mot de passe peut protéger un classeur .xlsx, mais pas une copie .xls ou .ods.
- Une table de données (table d'hypothèses) peut être calculée, mais pas créée.

## Commencez par ces cinq étapes

1. **Ouvrez un classeur.** Appuyez sur **{key:Open}**, ou cliquez sur **Fichier**, puis sur **Ouvrir**. Les premières lignes s'affichent immédiatement, même dans un très grand classeur, pendant que JeffCrunch lit le reste.
2. **Explorez.** Cliquez sur une cellule pour la sélectionner. Son adresse figure dans la **Zone Nom** en haut à gauche, et son contenu dans la **barre de formule** à côté. Utilisez les touches de direction, la molette de la souris et les barres de défilement pour vous déplacer. Consultez [Sélectionner des cellules et se déplacer](select-and-move).
3. **Modifiez une cellule.** Sélectionnez une cellule et tapez. Appuyez sur **Entrée** pour valider votre saisie. Consultez [Saisir dans les cellules](edit-cells).
4. **Mettez en forme des cellules.** Sélectionnez des cellules et utilisez l'onglet **Accueil** pour modifier la police, les couleurs, les bordures et le format de nombre. Consultez [Mettre en forme les cellules](format-cells).
5. **Modifiez l'affichage.** Utilisez l'onglet **Affichage** pour zoomer, afficher ou masquer le quadrillage et les en-têtes, et figer les premières lignes ou les premières colonnes. Consultez [Modifier l'affichage](view).

## Ce que vous voyez

Un classeur s'affiche tel qu'il a été enregistré : largeur des colonnes et hauteur des lignes, polices, couleurs, remplissages et bordures, cellules fusionnées, volets figés, et formats de nombre qui transforment un nombre enregistré en date, en pourcentage ou en prix. Un texte trop long pour sa cellule déborde dans les cellules vides voisines. Un nombre trop large pour sa cellule s'affiche sous forme d'une suite de signes # ; élargissez la colonne, ou sélectionnez la cellule pour lire le nombre dans la barre de formule. Les nombres sont écrits selon la langue de votre ordinateur (consultez [Formats de nombre et langue](number-formats)), et les mises en forme conditionnelles d'un classeur, telles que les nuances de couleurs, les barres de données et les icônes, sont affichées (consultez [Mises en forme conditionnelles](conditional-formats)).

Enregistrez votre travail avec **{key:Save}**, qui écrit vos modifications dans le classeur que vous avez ouvert, ou avec **Enregistrer sous**, qui les écrit dans un nouveau fichier. Consultez [Ouvrir et enregistrer des classeurs](open-save).
