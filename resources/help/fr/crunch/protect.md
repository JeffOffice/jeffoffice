# Protéger une feuille ou le classeur
<!-- keywords: protéger, protection, mot de passe, verrouiller, cellules verrouillées, déverrouiller, ôter la protection, déprotéger, structure, feuilles, lecture seule, autoriser, format de cellule, insérer des lignes, trier, filtrer, hachage -->

## Protéger une feuille

**Révision > Protéger la feuille** empêche de modifier la feuille par accident. Dans une feuille protégée, les cellules **verrouillées** ne peuvent pas être modifiées. Toutes les cellules sont verrouillées tant que vous n'en décidez pas autrement : sélectionnez les cellules qui pourront être modifiées, ouvrez **Format de cellule > Protection** et décochez **Verrouillée**. (Ce réglage ne compte qu'une fois la feuille protégée.)

La boîte de dialogue demande ce que chacun peut encore faire sur la feuille : sélectionner les cellules verrouillées ou déverrouillées, mettre en forme les cellules, modifier les colonnes ou les lignes, insérer ou supprimer des lignes et des colonnes, insérer des liens, trier, utiliser des filtres, et modifier les objets de dessin et les notes. Tout ce que vous laissez décoché est refusé, avec un message qui indique ce qui a été refusé.

Le **mot de passe** est facultatif. Si vous en indiquez un, la protection de la feuille ne peut être ôtée qu'avec lui. JeffCrunch conserve une forme brouillée du mot de passe (un hachage salé, répété 100 000 fois) et jamais le mot de passe lui-même : il ne peut donc pas vous le montrer plus tard. Conservez-le en lieu sûr. Les mots de passe créés par d'autres programmes, sous leur forme ancienne comme sous leur forme récente, sont vérifiés de la même façon.

La protection et sa suppression sont des actions que vous pouvez annuler. Pour ôter la protection, utilisez **Ôter la protection de la feuille**, puis tapez le mot de passe s'il y en a un.

Sur une feuille protégée, certaines actions restent interdites quelles que soient les autorisations : fusionner des cellules et supprimer des lignes ou des colonnes qui contiennent des cellules verrouillées.

## Protéger le classeur

**Révision > Protéger le classeur** verrouille la **structure** : aucune feuille ne peut être ajoutée, supprimée, renommée, déplacée, masquée ou affichée. Cette commande peut aussi demander que les fenêtres soient conservées en l'état. Elle peut également comporter un mot de passe. **Ôter la protection du classeur** la supprime.
