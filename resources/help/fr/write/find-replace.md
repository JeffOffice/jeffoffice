# Rechercher et remplacer des mots
<!-- keywords: rechercher, remplacer, remplacer tout, recherche, trouver, modifier des mots, respecter la casse, mots entiers, suivant, ctrl+f, ctrl+h, modification, volet, texte masqué, annuler -->

Utilisez Rechercher pour accéder à des mots de votre document. Utilisez Remplacer pour les modifier. Les deux fonctionnent dans un même volet, à droite de la fenêtre.

## Rechercher des mots

1. Appuyez sur **Ctrl+F**, ou cliquez sur **Accueil > Modification > Rechercher**. Le volet **Rechercher et remplacer** s'ouvre, avec le curseur dans **Rechercher**.
2. Tapez les mots voulus. Appuyez ensuite sur **Entrée**, ou cliquez sur **Suivant**.

JeffWrite commence à l'emplacement de votre curseur. Il sélectionne l'endroit suivant où il trouve vos mots et fait défiler les pages pour que vous puissiez le voir. Cet endroit est appelé une **occurrence**. Cliquez de nouveau sur **Suivant** pour passer à l'occurrence suivante. Après la dernière occurrence, il revient au début du document.

Si vos mots ne figurent pas dans le document, le volet vous l'indique et rien ne bouge.

Vous pouvez continuer à travailler dans le document pendant que le volet est ouvert. Cliquez sur la page pour taper du texte. Cliquez sur le **X** en haut du volet pour le fermer.

## Affiner la recherche

- **Respecter la casse** ne trouve que les mots ayant les mêmes majuscules. Lorsque cette option est activée, « Plan » ne trouve pas « plan ».
- **Mots entiers uniquement** ignore les mots plus longs qui contiennent les vôtres. Lorsque cette option est activée, « plan » ne trouve pas « planète ».

## Remplacer des mots

1. Appuyez sur **Ctrl+H**, ou cliquez sur **Accueil > Modification > Remplacer**. Le curseur se place dans **Remplacer par**.
2. Tapez les nouveaux mots dans **Remplacer par**. Pour supprimer les mots trouvés, laissez cette zone vide.
3. Tapez les anciens mots dans **Rechercher**.
4. Cliquez sur **Remplacer** pour afficher la première occurrence. Cliquez de nouveau sur **Remplacer** pour la modifier. JeffWrite passe alors à l'occurrence suivante. Chaque modification constitue une étape : **Annuler** (Ctrl+Z) les annule donc une par une.
5. Cliquez sur **Remplacer tout** pour modifier toutes les occurrences en une seule fois. Le volet indique le nombre de modifications effectuées. **Remplacer tout** constitue une seule étape : une seule commande **Annuler** rétablit donc tous les mots.

Les nouveaux mots prennent l'apparence de la première lettre des mots qu'ils remplacent. Si vous remplacez un mot en gras, le nouveau mot est en gras.

Par exemple, pour remplacer partout « couleur » par « teinte » : tapez **couleur** dans **Rechercher**, tapez **teinte** dans **Remplacer par**, puis cliquez sur **Remplacer tout**.

## Ce que la recherche examine

- Elle examine le texte du document, y compris celui des tableaux. Un signet placé au milieu de certains mots, comme on en trouve dans certains fichiers, n'empêche pas de les trouver. **Remplacer** et **Remplacer tout** conservent le signet.
- Elle n'examine pas encore les en-têtes, pieds de page, notes de bas de page ni notes de fin. Les mots qui s'y trouvent ne sont pas trouvés, et **Remplacer tout** ne les modifie pas. Si un en-tête ou un pied de page est ouvert pour modification lorsque vous ouvrez le volet, il se ferme d'abord. Voir [En-têtes et pieds de page](headers-footers).
- Elle ignore le texte défini comme **Masqué**, car ce texte n'est pas affiché. Voir [Modifier l'apparence du texte](font).

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Utiliser JeffWrite avec le clavier](keyboard)
