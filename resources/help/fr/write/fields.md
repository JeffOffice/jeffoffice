# Champs

<!-- keywords: champ, champs, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, mettre à jour le champ, F9, délier, verrouiller, codes de champs, numéro de page, date, NUMPAGES, SEQ, REF, PAGEREF, formule, Insérer un champ -->

Un **champ** est un élément de texte que le programme calcule pour vous, par exemple un numéro de page, la date du jour ou le numéro d'une figure. Il comporte deux parties : un **code**, qui indique ce qu'il faut calculer, et un **résultat**, qui est le texte que vous voyez. Le code d'un numéro de page est `PAGE`, et son résultat sur la troisième page est `3`.

## Insérer un champ dans le texte

1. Cliquez à l'endroit où le champ doit apparaître.
2. Cliquez sur **Insertion > Champ**.
3. Choisissez un champ dans la liste. Son code apparaît dans la zone ci-dessous, et vous pouvez le modifier.
4. Cliquez sur **OK**.

Par exemple, choisissez **Date**, remplacez le code par `DATE \@ "MMMM d, yyyy"`, puis cliquez sur **OK**. Le texte affiche la date sous la forme `2 novembre 2026`.

## Mettre à jour les champs

Un champ affiche le résultat qu'il avait lors de son dernier calcul. Appuyez sur **F9** pour recalculer le champ où se trouve le curseur. Pour mettre à jour plusieurs champs à la fois, sélectionnez le texte qui les contient, puis appuyez sur **F9**. Appuyez sur **Ctrl+A**, puis sur **F9**, pour mettre à jour tous les champs du document.

La mise à jour compte pour une seule étape : **Annuler** rétablit tous les résultats précédents. La mise à jour n'est pas une modification suivie, même lorsque le suivi des modifications est activé, car le résultat n'est pas quelque chose que vous avez saisi.

Un champ placé dans un en-tête ou un pied de page qui affiche le numéro de page, le nombre de pages ou un titre est toujours exact et ne nécessite aucune mise à jour.

## Afficher les codes

Appuyez sur **Maj+F9** pour afficher, à la place de son résultat, le code du champ où se trouve le curseur, entre accolades, comme `{ PAGE }`. Appuyez sur **Alt+F9** pour afficher les codes de tous les champs, puis appuyez de nouveau dessus pour afficher les résultats. Cette opération modifie seulement ce que vous voyez : le fichier est enregistré tel quel, et le document n'est pas marqué comme modifié.

## Délier ou verrouiller un champ

- **Ctrl+Maj+F9** transforme le champ en texte ordinaire. Le résultat reste, mais ce n'est plus un champ.
- **Ctrl+F11** verrouille un champ, de sorte que **F9** ne le modifie pas. **Ctrl+Maj+F11** le déverrouille.

## Formats

Ajoutez des commutateurs au code pour modifier l'apparence d'un résultat :

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` et `\* Caps` modifient la casse ou le type de nombre. `{ = 21 \* CardText }` affiche `vingt et un`.
- `\# "0.00"` définit l'apparence d'un nombre. `{ = 1234.5 \# "#,##0.00" }` affiche `1 234,50`.
- `\@ "dddd, MMMM d"` définit l'apparence d'une date. `M` désigne le mois, et `m` désigne la minute.

## Champs calculés par JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, une formule commençant par `=`, TOC (également une table des illustrations), INDEX, TOA, CITATION et BIBLIOGRAPHY. Les marques masquées TC, XE et TA n'ont pas de résultat à calculer : elles indiquent à une table des matières, à un index ou à une table des sources juridiques ce qu'il faut répertorier. Un champ d'un autre type conserve le résultat qu'il a et est enregistré tel quel.

DATE, TIME, FILENAME et AUTHOR dépendent de l'ordinateur et du moment ; leur résultat peut donc changer à chaque mise à jour.
