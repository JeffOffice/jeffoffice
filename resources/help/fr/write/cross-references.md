# Renvois
<!-- keywords: renvoi, référence croisée, REF, PAGEREF, NOTEREF, voir page, voir figure, ci-dessus, ci-dessous, numéro de titre, lien -->

Un **renvoi** est un texte de votre document qui fait référence à un autre endroit, par exemple « voir Figure 2 page 7 ». Comme il s'agit d'un champ, lorsque la figure ou la page change, il suffit de mettre à jour le champ pour que le texte suive.

## Insérer un renvoi

1. Cliquez à l'endroit où le texte doit figurer.
2. Cliquez sur **Références > Renvoi**.
3. Choisissez la **Catégorie** : un titre, un signet, une note de bas de page, une note de fin, un élément numéroté ou une légende comme Figure ou Tableau.
4. Choisissez l'option voulue dans **Insérer un renvoi à** : le texte, le numéro, le numéro de page, ou « ci-dessus » et « ci-dessous ».
5. Choisissez l'élément dans la liste, laissez **Insérer en tant que lien hypertexte** coché si vous voulez pouvoir y accéder directement, puis cliquez sur **Insérer**.

Par exemple, pour écrire « Voir le titre Résultats page 4 » : tapez `Voir le titre `, insérez un renvoi au titre avec **Texte du titre**, tapez ` page `, puis insérez un second renvoi au même titre avec **Numéro de page**.

## Suivre un renvoi

Maintenez la touche **Ctrl** enfoncée et cliquez sur un renvoi inséré en tant que lien hypertexte. Le curseur se place à l'endroit indiqué.

## Garder le texte à jour

Après avoir ajouté des pages, modifié des titres ou déplacé des figures, sélectionnez le texte et appuyez sur **F9**. Chaque renvoi recalcule son texte. Si l'endroit visé par un renvoi a été supprimé, le champ affiche `Erreur : le signet auquel ce champ fait référence est introuvable.`, et vous pouvez choisir une nouvelle cible.

## Fonctionnement en coulisses

JeffWrite marque la cible avec un signet masqué (dont le nom commence par `_Ref`) et crée un champ qui y fait référence : `REF` pour le texte et les numéros, `PAGEREF` pour les numéros de page et `NOTEREF` pour les notes. Un autre programme ouvre le fichier avec des renvois toujours fonctionnels.
