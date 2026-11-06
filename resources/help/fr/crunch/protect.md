# Protéger une feuille ou le classeur
<!-- keywords: protéger, protection, mot de passe, verrouiller, cellules verrouillées, déverrouiller, ôter la protection, structure, feuilles, lecture seule, autoriser, mettre en forme les cellules, insérer des lignes, trier, filtrer, hachage, autoriser la modification des plages, plage modifiable, mot de passe de la plage -->

## Protéger une feuille

**Révision > Protéger la feuille** empêche les utilisateurs de modifier la feuille par erreur. Sur une feuille protégée, les cellules **verrouillées** ne peuvent pas être modifiées. Toutes les cellules sont verrouillées tant que vous ne modifiez pas ce réglage : sélectionnez les cellules qui peuvent être modifiées, ouvrez **Format de cellule > Protection**, puis décochez **Verrouillée**. (Cela ne compte qu'une fois la feuille protégée.)

La boîte de dialogue demande ce que tout le monde peut encore faire sur la feuille : sélectionner des cellules verrouillées ou déverrouillées, mettre en forme les cellules, modifier les colonnes ou les lignes, insérer ou supprimer des lignes et des colonnes, insérer des liens, trier, utiliser des filtres, et modifier les objets de dessin et les notes. Ce que vous ne cochez pas est refusé, avec un message qui indique ce qui a été refusé.

Un **mot de passe** est facultatif. Si vous en définissez un, la feuille ne peut être déprotégée qu'avec celui-ci. JeffCrunch conserve une forme brouillée du mot de passe (un haché salé, répété 100 000 fois), et jamais le mot de passe lui-même. Il ne peut donc pas vous le réafficher plus tard : conservez-le en lieu sûr. Les mots de passe créés par d'autres logiciels, sous leur forme ancienne ou récente, sont vérifiés de la même manière.

Protéger et déprotéger sont des actions que vous pouvez annuler. Pour supprimer la protection, utilisez **Ôter la protection de la feuille**, puis saisissez le mot de passe s'il en existe un.

Certaines opérations restent interdites sur une feuille protégée, quelles que soient les autorisations : fusionner des cellules, et supprimer des lignes ou des colonnes contenant des cellules verrouillées.

## Autoriser la modification de certaines cellules : Autoriser la modification des plages

**Révision > Autoriser la modification des plages** répertorie les **plages** que les utilisateurs peuvent modifier même lorsque la feuille est protégée. Une plage est un bloc de cellules, comme A1:C5. Cliquez sur **Nouveau** pour en ajouter une. Donnez-lui un **titre**, saisissez les cellules qu'elle couvre (utilisez un espace entre les blocs), et, si vous le souhaitez, un **mot de passe**. Une plage sans mot de passe peut être modifiée par tous. Une plage avec mot de passe ne peut être modifiée qu'après que la personne a saisi le mot de passe la première fois qu'elle tente de modifier l'une de ses cellules. Ensuite, la plage reste accessible jusqu'à la fermeture du classeur.

**Modifier** change une plage, et **Supprimer** la retire. Vous pouvez définir les plages avant de protéger la feuille. Elles sont enregistrées avec le classeur, et les mots de passe sont conservés sous la même forme brouillée que le mot de passe d'une feuille. Autoriser la modification des plages est une action que vous pouvez annuler. Lorsque vous insérez ou supprimez des lignes, des colonnes ou des cellules, les plages se déplacent avec leurs cellules, rétrécissent ou s'agrandissent, ou disparaissent si toutes leurs cellules sont supprimées. Annuler rétablit les plages.

## Protéger le classeur

**Révision > Protéger le classeur** verrouille la **structure** : aucune feuille ne peut être ajoutée, supprimée, renommée, déplacée, masquée ou affichée. Vous pouvez aussi demander que les fenêtres restent telles quelles. Une protection par mot de passe est possible. **Ôter la protection du classeur** la supprime.
