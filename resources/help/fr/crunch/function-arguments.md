# Saisir une fonction avec la boîte de dialogue Arguments de la fonction
<!-- keywords: arguments de la fonction, argument, fx, résultat de la formule, Maj+F3, sélectionner des cellules, plage, facultatif, répéter, modifier une fonction, fonction imbriquée, erreur -->

La boîte de dialogue **Arguments de la fonction** vous aide à remplir une fonction argument par argument et affiche le résultat de la formule avant que vous ne l'insériez dans la cellule.

## Ouvrir la boîte de dialogue

- Choisissez **Choisir une fonction** dans l'onglet Formules (**Maj+F3**), ou cliquez sur **fx** à côté de la barre de formule, puis choisissez une fonction. La boîte de dialogue s'ouvre pour cette fonction.
- Pour modifier une fonction déjà présente dans une formule, placez le curseur entre ses parenthèses (dans la cellule ou dans la barre de formule), puis appuyez sur **Maj+F3** ou cliquez sur **fx**. La boîte de dialogue s'ouvre pour cette fonction, avec ses arguments dans les zones. Dans une formule telle que =SOMME(A1; SI(B1>2; 5; 6)), si le curseur se trouve dans le SI, c'est le SI qui est modifié ; s'il se trouve dans la SOMME mais en dehors du SI, c'est la SOMME qui est modifiée.

## Remplir les arguments

Chaque argument occupe une ligne : son nom, une zone où le saisir et, à droite, sa valeur actuelle, par exemple 12,5, "texte", VRAI ou {1;2;3} pour une plage de cellules. Un nom en gras doit être renseigné ; un nom qui n'est pas en gras est facultatif et peut rester vide.

- **Tab** permet de passer à la zone suivante, et **Maj+Tab** à la zone précédente.
- La ligne sous les zones indique ce que fait la fonction, et la ligne suivante décrit l'argument dans lequel vous vous trouvez.
- Les fonctions telles que SOMME acceptent autant d'arguments que nécessaire : lorsque vous tapez dans la dernière zone, une nouvelle zone apparaît.
- Le bouton à droite d'une zone permet de désigner des cellules. La boîte de dialogue se réduit à une petite barre, et vous sélectionnez les cellules dans la feuille avec la souris (la zone affiche leur adresse pendant que vous faites glisser). Cliquez sur le bouton de la barre, ou appuyez sur **Entrée**, pour revenir à la boîte de dialogue.

## Le résultat

**Résultat de la formule =** affiche la valeur de la fonction entière, peu après que vous avez cessé de taper. Rien n'est écrit dans la feuille tant que vous n'avez pas appuyé sur **OK**. En cas de problème, une ligne sous le résultat l'explique en clair : par exemple, qu'une valeur est introuvable, qu'une division par zéro a lieu ou qu'un nombre n'est pas autorisé. Une formule trop longue à calculer ici n'est pas affichée ; appuyez sur **OK** et la feuille la calculera.

**Aide sur cette fonction** ouvre cette page.

## Terminer

Appuyez sur **Entrée** ou cliquez sur **OK** pour insérer la fonction. Si aucune saisie n'est en cours dans la cellule, la formule est écrite dans la cellule. Si vous êtes en train de taper une formule, la fonction est insérée à l'emplacement du curseur, ou remplace celle que vous avez modifiée, et vous pouvez continuer à taper. Appuyez sur **Échap** ou cliquez sur **Annuler** : rien n'est modifié.
