# Mise en forme conditionnelle
<!-- keywords: mise en forme conditionnelle, règle de mise en forme, règle, surbrillance, mettre en évidence, échelle de couleurs, nuancier, barre de données, jeu d'icônes, icônes, premiers, derniers, supérieur à la moyenne, inférieur à la moyenne, doublon, unique, formule, arrêter si vrai, priorité, barres, flèches, feux tricolores, dégradé, format différentiel, bouton Mise en forme conditionnelle, effacer les règles, supérieur à, inférieur à, entre, texte qui contient, valeurs en double, top 10 -->

Une **mise en forme conditionnelle** est une règle qui modifie l'aspect d'une cellule lorsque son contenu répond à une condition. Exemple : une règle indique « colorier une cellule en rouge si elle est inférieure à 100 ». Une cellule contenant 80 est rouge, une cellule contenant 120 ne l'est pas, et si vous remplacez 80 par 150, le rouge disparaît tout seul. La mise en forme propre de la cellule n'est pas modifiée : la règle décide seulement de ce qui est affiché.

## Ce que JeffCrunch affiche

JeffCrunch affiche les règles d'un classeur :

- **Règles de mise en surbrillance** : une règle qui modifie la police, le remplissage, la bordure ou le format de nombre des cellules qui répondent à la condition. La condition peut être une comparaison avec une valeur (supérieur à, compris entre, égal à, et ainsi de suite), une formule, un texte qui contient, commence par ou se termine par certains mots, une date comme « la semaine dernière », des cellules vides ou comportant une erreur, des valeurs parmi les plus élevées ou les plus faibles (un nombre ou un pourcentage), des valeurs supérieures ou inférieures à la moyenne, ainsi que des valeurs en double ou uniques.
- **Échelles de couleurs** : chaque cellule est remplie d'une couleur qui passe entre deux ou trois couleurs selon que son nombre est faible ou élevé. Une échelle du blanc au vert rend les plus grands nombres plus verts.
- **Barres de données** : une barre dans la cellule, d'autant plus longue que le nombre est grand. Les barres peuvent être d'une couleur unie ou s'estomper, avoir une bordure, afficher un axe, et s'afficher dans le sens inverse, dans une autre couleur, pour les nombres négatifs.
- **Jeux d'icônes** : une petite image dans la cellule qui indique dans quelle partie de la plage se trouve le nombre, comme une flèche vers le haut, une flèche horizontale ou une flèche vers le bas, ou encore un feu rouge, jaune ou vert. Il existe vingt jeux.

Lorsque deux règles concernent la même cellule, c'est celle qui vient en premier dans l'ordre du classeur qui l'emporte pour ce que toutes deux définissent, et une règle marquée « arrêter si vrai » met fin à la recherche. Les valeurs d'une échelle de couleurs, d'une barre de données ou d'un jeu d'icônes sont calculées sur les cellules propres à la règle, et recalculées lorsque l'une de ces cellules change.

## Ce qui reste inchangé

- Les règles suivent les cellules auxquelles elles appartiennent. Insérez ou supprimez des lignes ou des colonnes, coupez et collez : les règles couvrent toujours les bonnes cellules. **Annuler** les rétablit.
- Une règle que JeffCrunch ne peut pas afficher est conservée dans le fichier telle quelle.
- L'enregistrement écrit les règles du classeur. Lorsque vous créez ou effacez des règles sur une feuille, les règles de cette feuille sont réécrites avec vos modifications.

## Créer une règle

1. Sélectionnez les cellules.
2. Cliquez sur **Accueil > Mise en forme conditionnelle**.
3. Sous **Règles de mise en surbrillance des cellules**, choisissez **Supérieur à**, **Inférieur à**, **Entre**, **Égal à**, **Texte qui contient** ou **Valeurs en double**. Sous **Règles haut/bas**, choisissez **10 premiers éléments**, **10 derniers éléments**, **10 % supérieurs**, **10 % inférieurs**, **Supérieur à la moyenne** ou **Inférieur à la moyenne**.
4. Saisissez la valeur (ou les deux valeurs pour **Entre**, le texte pour **Texte qui contient**, ou le nombre pour les règles haut et bas). Pour **10 premiers éléments**, « 10 » est le nombre de cellules qui reçoivent la mise en forme. Vous pouvez le remplacer par un autre nombre.
5. Choisissez l'aspect des cellules, par exemple **Remplissage rouge clair avec texte rouge foncé**, puis cliquez sur **OK**.

La règle s'affiche immédiatement. Chaque nouvelle règle est placée en premier : lorsque deux règles veulent modifier la même chose, la plus récente l'emporte. La création d'une règle constitue une étape que vous pouvez annuler.

## Effacer des règles

**Effacer les règles des cellules sélectionnées** supprime toutes les règles présentes sur l'une des cellules sélectionnées. **Effacer les règles de cette feuille** supprime toutes les règles de la feuille. Chaque commande constitue une étape que vous pouvez annuler. Une commande ne fait rien lorsqu'il n'y a aucune règle à effacer.

## Ce qui n'est pas encore disponible

- Les barres de données, les échelles de couleurs et les jeux d'icônes peuvent être affichés, mais pas encore créés. La fenêtre qui permet de lister, d'ordonner et de modifier toutes les règles (**Gérer les règles**) n'est pas encore disponible. Une règle qui utilise une formule ne peut pas encore être créée depuis le menu.
- Une feuille qui contient des barres de données provenant d'un fichier plus récent les conserve telles quelles, et JeffCrunch ne crée ni n'efface de règles sur cette feuille. Effectuez ces modifications dans le programme qui a créé le classeur.
- Un classeur ouvert depuis un format de fichier plus ancien conserve ses règles lorsque vous l'enregistrez au format de classeur .xlsx, et vous pouvez ajouter des règles ensuite.
