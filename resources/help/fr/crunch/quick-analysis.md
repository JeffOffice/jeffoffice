# Analyse rapide
<!-- keywords: analyse rapide, analyser, analyse, graphique instantané, graphique rapide, totaux rapides, totaux, total cumulé, pourcentage du total, barres de données, échelle de couleurs, jeu d'icônes, sparklines, tableau, tableau croisé dynamique, Ctrl+Q, aperçu -->

**Analyse rapide** regroupe dans une petite boîte les moyens les plus courants d'examiner un bloc de nombres : colorer les cellules, les représenter par un graphique, les totaliser, en faire un tableau, ou dessiner pour chaque ligne un minuscule graphique. Vous choisissez ce que vous voulez, et vous le voyez dans votre feuille avant de décider.

## Ouvrir la boîte

1. Sélectionnez deux cellules ou plus contenant des données. Un petit bouton apparaît juste en dessous et à droite de la dernière cellule sélectionnée.
2. Cliquez sur le bouton, appuyez sur **Ctrl+Q** (sur Mac, **Contrôle+Q**), ou cliquez sur **Accueil > Modification > Analyse rapide**.
3. Choisissez un onglet, puis une option.

Le bouton ne prend jamais le clavier à la feuille. Il disparaît pendant la modification d'une cellule, lorsque vous ne sélectionnez qu'une seule cellule ou des cellules vides, ou lorsque la feuille est protégée de façon à interdire tout ce que la boîte propose.

## Voir avant de choisir

Pointez une option, ou appuyez sur les touches **Flèche gauche** et **Flèche droite** pour parcourir les options d'un onglet (les touches **Flèche haut** et **Flèche bas** passent d'un onglet à l'autre). La feuille montre ce que ferait l'option, sans rien modifier. Pour les totaux portant sur un très grand nombre de cellules, un message indique que l'aperçu est impossible ; l'option fonctionne tout de même lorsque vous cliquez dessus. Il n'y a aucune étape à annuler : un fichier enregistré à ce moment-là n'en contient aucune trace. Appuyez sur **Échap** ou cliquez ailleurs, et l'aperçu disparaît. Cliquez sur une option, ou appuyez sur **Entrée**, pour l'appliquer. Chaque option appliquée constitue une étape de **Annuler**.

## Les onglets

- **Mise en forme** : **Barres de données**, **Échelle de couleurs** et **Jeu d'icônes** montrent l'importance de chaque nombre. **Supérieur à** colore les cellules dont le nombre est supérieur à la moyenne de tous les nombres. **Top 10 %** colore le dixième le plus élevé. **Effacer la mise en forme** supprime la mise en forme conditionnelle des cellules sélectionnées. Ce sont les mêmes règles que dans **Accueil > Mise en forme conditionnelle**, où vous pouvez les modifier.
- **Graphiques** : jusqu'à quatre types de graphiques adaptés à la forme de vos cellules, les mêmes que ceux proposés par **Insertion > Graphiques recommandés**. Un aperçu montre le graphique dans le coin de la feuille. Choisir une option place le graphique sur la feuille.
- **Totaux** : **Somme**, **Moyenne**, **Nombre**, **% du total** et **Total cumulé**, écrits sous forme de formules ordinaires. La première rangée d'options place un total sous chaque colonne de nombres. La seconde rangée (**Somme par ligne**, et ainsi de suite) place un total à droite de chaque ligne. Une première ligne ou une première colonne ne contenant que du texte est considérée comme un ensemble de noms et n'est pas prise en compte ; une première ligne de nombres, comme des années, est additionnée comme les autres. Les lignes masquées et filtrées sont incluses. **% du total** affiche la part de chaque colonne (ou ligne) dans l'ensemble des nombres, en pourcentage. **Total cumulé** additionne au fur et à mesure, dans de nouvelles cellules placées à côté de vos cellules (ou dessous). Si l'une des cellules où figurent les totaux contient déjà quelque chose, il vous est demandé confirmation avant son remplacement. Les totaux ne sont pas écrits par-dessus les cellules occupées par un résultat qui déborde, ni par-dessus les cellules fusionnées : Analyse rapide vous le signale et ne modifie rien.
- **Tableaux** : **Tableau** transforme les cellules en tableau, avec des boutons de tri et de filtre. **Tableau croisé dynamique** crée sur une nouvelle feuille un tableau croisé dynamique qui totalise les données ; la première ligne doit contenir les noms des colonnes.
- **Sparklines** : **Ligne**, **Colonne** et **Gain/perte** dessinent un petit graphique de chaque ligne dans la cellule située à droite de celle-ci.

## Limites

Analyse rapide examine jusqu'à 250 000 cellules pour les totaux et les sparklines. Si vous en sélectionnez davantage, un message le signale et rien n'est modifié. Une option qui ne peut pas être appliquée (par exemple faute de place sous les cellules) le signale également.
