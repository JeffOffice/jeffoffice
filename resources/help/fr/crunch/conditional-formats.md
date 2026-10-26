# Mises en forme conditionnelles
<!-- keywords: mise en forme conditionnelle, format conditionnel, règle, surligner, mettre en évidence, nuances de couleurs, échelle de couleurs, barre de données, jeu d'icônes, icônes, valeurs les plus élevées, valeurs les plus faibles, au-dessus de la moyenne, en dessous de la moyenne, doublons, valeurs uniques, formule, interrompre si vrai, priorité, barres, flèches, feux tricolores, dégradé, format différentiel -->

Une **mise en forme conditionnelle** est une règle qui modifie l'apparence d'une cellule lorsque son contenu répond à une condition. Exemple : une règle indique « colorer une cellule en rouge si elle est inférieure à 100 ». Une cellule contenant 80 est rouge, une cellule contenant 120 ne l'est pas, et si vous remplacez 80 par 150, le rouge disparaît de lui-même. La mise en forme propre de la cellule n'est pas modifiée : la règle détermine uniquement ce qui est affiché.

## Ce qu'affiche JeffCrunch

JeffCrunch affiche les règles que contient un classeur :

- **Règles de mise en évidence** : une règle qui modifie la police, le remplissage, la bordure ou le format de nombre des cellules qui y répondent. La condition peut être une comparaison avec une valeur (supérieure à, comprise entre, égale à, etc.), une formule, un texte qui contient certains mots, commence ou se termine par ceux-ci, une date comme « la semaine dernière », des cellules vides ou contenant une erreur, des valeurs parmi les plus élevées ou les plus faibles (en nombre ou en pourcentage), des valeurs au-dessus ou en dessous de la moyenne, et des valeurs en double ou uniques.
- **Nuances de couleurs** : chaque cellule est remplie d'une couleur qui varie entre deux ou trois couleurs selon que son nombre est faible ou élevé. Avec une échelle allant du blanc au vert, les nombres les plus grands sont les plus verts.
- **Barres de données** : une barre dans la cellule, d'autant plus longue que le nombre est grand. Les barres peuvent être de couleur unie ou en dégradé, avoir une bordure, avoir un axe, et partir dans l'autre sens, dans une autre couleur, pour les nombres négatifs.
- **Jeux d'icônes** : une petite image dans la cellule qui indique dans quelle partie de la plage se situe le nombre, comme une flèche vers le haut, une flèche horizontale ou une flèche vers le bas, ou encore un feu rouge, orange ou vert. Il existe vingt jeux.

Lorsque deux règles s'appliquent à la même cellule, celle qui vient en premier dans l'ordre du classeur l'emporte pour les éléments qu'elles définissent toutes les deux, et une règle marquée « Interrompre si vrai » met fin à la recherche. Les nombres d'une nuance de couleurs, d'une barre de données ou d'un jeu d'icônes sont calculés sur les cellules de la règle uniquement, et ils sont recalculés lorsque l'une de ces cellules change.

## Ce qui reste inchangé

- Les règles suivent les cellules auxquelles elles appartiennent. Si vous insérez ou supprimez des lignes ou des colonnes, ou si vous coupez et collez, les règles couvrent toujours les bonnes cellules ; **Annuler** les rétablit.
- Une règle que JeffCrunch n'affiche pas est conservée dans le fichier telle quelle.
- L'enregistrement écrit les règles du classeur sans les modifier.

## Ce qui n'est pas encore disponible

**Vous ne pouvez pas encore créer, modifier ou supprimer une règle.** JeffCrunch ne dispose pas de commande **Mise en forme conditionnelle** : il affiche ce que contient déjà le classeur et le conserve correctement au fil de vos modifications. La modification des règles arrivera dans une version ultérieure. En attendant, créez ou modifiez les règles dans le programme qui a créé le classeur.
