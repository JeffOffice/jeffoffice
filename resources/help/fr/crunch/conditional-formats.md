# Mise en forme conditionnelle

<!-- keywords: mise en forme conditionnelle, règle de mise en forme, règle, surbrillance, mise en surbrillance, échelle de couleurs, barre de données, jeu d'icônes, icônes, premiers, derniers, supérieur à la moyenne, inférieur à la moyenne, doublon, unique, formule, arrêter si vrai, priorité, barres, flèches, feux de circulation, dégradé, format différentiel, bouton Mise en forme conditionnelle, effacer les règles, supérieur à, inférieur à, compris entre, texte qui contient, valeurs en double, 10 premiers -->

Une **mise en forme conditionnelle** est une règle qui modifie l'aspect d'une cellule lorsque son contenu remplit une condition. Exemple : une règle indique « colorer une cellule en rouge si elle est inférieure à 100 ». Une cellule contenant 80 apparaît en rouge, une cellule contenant 120 n'est pas colorée, et si vous remplacez 80 par 150, le rouge disparaît tout seul. La mise en forme propre de la cellule n'est pas modifiée : la règle détermine seulement ce qui est affiché.

## Ce que JeffCrunch affiche

JeffCrunch affiche les règles qu'un classeur contient :

- **Règles de surbrillance** : une règle qui modifie la police, le remplissage, la bordure ou le format de nombre des cellules qui la remplissent. La condition peut être une comparaison avec une valeur (supérieur à, compris entre, égal à, et ainsi de suite), une formule, un texte qui contient, commence par ou se termine par certains mots, une date comme « la semaine dernière », des cellules vides ou comportant une erreur, des valeurs parmi les plus élevées ou les plus faibles (un nombre ou un pourcentage), des valeurs supérieures ou inférieures à la moyenne, et des valeurs en double ou uniques.
- **Échelles de couleurs** : chaque cellule est remplie d'une couleur qui passe d'une à deux ou trois couleurs selon que son nombre est faible ou élevé. Une échelle du blanc au vert rend les plus grands nombres les plus verts.
- **Barres de données** : une barre dans la cellule, d'autant plus longue que le nombre est grand. Les barres peuvent être d'une couleur unie ou se estomper, peuvent avoir une bordure, un axe, et se dessiner dans l'autre sens, dans une autre couleur, pour les nombres négatifs.
- **Jeux d'icônes** : une petite image dans la cellule indiquant dans quelle partie de la plage se situe le nombre, comme une flèche vers le haut, une flèche horizontale ou une flèche vers le bas ; ou un feu rouge, jaune ou vert. Il existe vingt jeux.

Lorsque deux règles concernent la même cellule, c'est celle qui vient en premier dans l'ordre du classeur qui l'emporte pour ce que toutes deux définissent, et une règle marquée « arrêter si vrai » met fin à la recherche. Les nombres d'une échelle de couleurs, d'une barre de données ou d'un jeu d'icônes sont calculés sur les cellules propres à la règle, et recalculés lorsque l'une de ces cellules change.

## Ce qui reste inchangé

- Les règles suivent les cellules auxquelles elles appartiennent. Insérez ou supprimez des lignes ou des colonnes, couper et coller : les règles couvrent toujours les bonnes cellules ; **Annuler** les rétablit.
- Une règle que JeffCrunch ne peut pas afficher est conservée dans le fichier telle quelle.
- L'enregistrement écrit les règles que le classeur contenait. Lorsque vous créez ou effacez des règles sur une feuille, les règles de cette feuille sont réécrites avec vos modifications.

## Créer une règle

1. Sélectionnez les cellules. Vous pouvez sélectionner des colonnes entières ou la feuille entière ; une règle est conservée pour la plage, et non pour chaque cellule.
2. Cliquez sur **Accueil > Mise en forme conditionnelle**.
3. Sous **Règles de mise en surbrillance des cellules**, choisissez **Supérieur à**, **Inférieur à**, **Compris entre**, **Égal à**, **Texte qui contient**, **Une date qui survient** ou **Valeurs en double**. Sous **Règles pour les valeurs supérieures et inférieures**, choisissez **10 premiers éléments**, **10 derniers éléments**, **10 % supérieurs**, **10 % inférieurs**, **Supérieur à la moyenne** ou **Inférieur à la moyenne**.
4. Saisissez la valeur (ou les deux valeurs pour **Compris entre**, le texte pour **Texte qui contient**, ou le nombre pour les règles des valeurs supérieures et inférieures), ou choisissez les dates. Pour **10 premiers éléments**, « 10 » indique combien de cellules reçoivent la mise en forme. Vous pouvez le remplacer par n'importe quel nombre.
5. Choisissez l'aspect des cellules, par exemple **Remplissage rouge clair avec texte rouge foncé**, puis cliquez sur **OK**. L'exemple à côté de la liste montre l'aspect obtenu.

Pour créer un aspect personnalisé, choisissez **Format personnalisé...** en bas de la liste. Une boîte de dialogue s'ouvre avec quatre onglets : **Nombre**, **Police** (le style, le soulignement, le barré et la couleur), **Bordure** (uniquement les traits fins) et **Remplissage** (une couleur, ou un motif à deux couleurs). La boîte part de l'aspect que vous aviez choisi, et une partie que vous ne modifiez pas reste inchangée. Cliquez sur **OK** et l'exemple affiche votre aspect.

La règle s'affiche immédiatement. Chaque nouvelle règle passe en premier : lorsque deux règles veulent modifier la même chose, la plus récente l'emporte. La création d'une règle est une étape que vous pouvez annuler.

## Barres de données, échelles de couleurs et jeux d'icônes

Cliquez sur **Accueil > Mise en forme conditionnelle**, puis sur **Barres de données**, **Échelles de couleurs** ou **Jeux d'icônes**, et cliquez sur un choix. La règle s'applique immédiatement aux cellules sélectionnées, sans fenêtre à remplir, comme une étape que vous pouvez annuler.

- Les **barres de données** existent en six couleurs, chacune en remplissage en dégradé ou en remplissage uni.
- Les **échelles de couleurs** comptent douze choix, à deux ou trois couleurs. Le nom indique d'abord la couleur des nombres les plus élevés.
- Les **jeux d'icônes** comptent vingt jeux : directionnels (flèches et triangles), formes (feux de circulation et panneaux), indicateurs (symboles et drapeaux) et évaluations (étoiles, quarts et cases). Les icônes répartissent la plage en tranches égales.

**Nouvelle règle**, en bas du menu, ouvre une seule fenêtre pour chaque type de règle, y compris une règle avec votre propre formule, ainsi qu'une barre de données, une échelle de couleurs ou un jeu d'icônes avec vos propres paramètres.

## Gérer les règles

**Accueil > Mise en forme conditionnelle > Gérer les règles** affiche les règles de la feuille dans l'ordre où elles agissent. Utilisez **Nouvelle règle**, **Modifier la règle** et **Supprimer la règle** ; **Monter** et **Descendre** modifient l'ordre ; saisissez d'autres cellules dans **S'applique à** ; et cochez **Arrêter si vrai**. Modifier la règle change tout ce que la règle contient :

- pour une **barre de données** : ce que représentent les barres les plus courtes et les plus longues (valeur la plus faible ou la plus élevée, un nombre, un pourcentage, un centile ou une formule), remplissage en dégradé ou uni, les couleurs, une bordure, la couleur des barres négatives, la position de l'axe, le sens des barres, et **Afficher uniquement la barre** ;
- pour une **échelle de couleurs** : deux ou trois couleurs, et ce que représente chacune ;
- pour un **jeu d'icônes** : le style d'icônes, la valeur à partir de laquelle chaque icône commence, **Inverser l'ordre des icônes**, et **Afficher uniquement l'icône** ;
- pour les autres règles : la valeur, le texte, les dates ou la formule, et l'aspect. Une règle qu'un fichier a apportée avec un aspect propre apparaît avec l'aspect **Format personnalisé**, et l'exemple montre son rendu réel. Si vous modifiez autre chose dans la règle, son aspect reste exactement tel que le fichier le contenait. Choisissez **Format personnalisé...** pour modifier l'aspect.

Cliquer sur **OK** applique toutes vos modifications en une seule étape que vous pouvez annuler. Une règle que vous n'avez pas modifiée conserve exactement ce que contenait le fichier, quelle que soit sa place dans la liste ; une règle modifiée est réécrite. La formule d'une règle est lue dans la cellule située en haut à gauche de sa plage, et une formule saisie pour une nouvelle règle est lue comme si elle était écrite dans la cellule où se trouve le curseur. Une feuille protégée qui ne permet pas de mettre en forme les cellules n'accepte pas de règles.

## Ce que conservent les autres types de fichiers

Seul un classeur .xlsx conserve les mises en forme conditionnelles. L'enregistrement dans un type de fichier plus ancien (.xls ou .ods) les supprime, et **Vérifier la compatibilité** le signale en citant les barres de données, les échelles de couleurs et les jeux d'icônes lorsque la feuille en contient. Un fichier .csv ne conserve aucune mise en forme. Les mises en forme conditionnelles sont imprimées telles qu'elles apparaissent à l'écran.

## Effacer les règles

**Effacer les règles des cellules sélectionnées** supprime toutes les règles présentes sur l'une des cellules sélectionnées. **Effacer les règles de cette feuille** supprime toutes les règles de la feuille. **Effacer les règles de ce tableau** et **Effacer les règles de ce tableau croisé dynamique** suppriment les règles présentes sur le tableau (ou le tableau croisé dynamique) où se trouvent les cellules sélectionnées, et ne sont disponibles que lorsque la sélection se trouve dans l'un de ces tableaux. Chaque commande est une étape que vous pouvez annuler. Un choix ne fait rien lorsqu'il n'y a aucune règle à effacer.

## Ce qui n'est pas encore pris en charge

- Une feuille qui contient des barres de données créées avec une version plus récente du format de fichier les conserve telles quelles, et JeffCrunch ne crée, ne modifie ni n'efface de règles sur cette feuille. Effectuez ces modifications dans le programme qui a créé le classeur.
- Un classeur ouvert depuis un type de fichier plus ancien conserve ses règles lorsque vous l'enregistrez au format de classeur .xlsx, et vous pouvez ajouter des règles ensuite.
- Un aspect personnalisé ne peut pas comporter de trait moyen, épais, double ou incliné, de trait diagonal, de remplissage en dégradé, ni de police ou de taille de police : ces éléments ne font pas partie de ce qu'une règle peut modifier.
