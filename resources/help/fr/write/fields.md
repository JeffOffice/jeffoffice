# Champs

<!-- keywords: champ, champs, INDEX, TOA, CITATION, BIBLIOGRAPHIE, XE, TA, TC, mettre à jour un champ, F9, dissocier, verrouiller, codes de champ, numéro de page, date, NUMPAGES, SEQ, REF, PAGEREF, formule, Insérer un champ, modifier un champ, clic droit -->

Un **champ** est un élément de texte que le programme calcule à votre place, comme un numéro de page, la date du jour ou le numéro d'une figure. Il comporte deux parties : un **code**, qui indique ce qu'il faut calculer, et un **résultat**, c'est-à-dire le texte que vous voyez. Le code d'un numéro de page est `PAGE`, et son résultat sur la troisième page est `3`.

## Insérer un champ dans le texte

1. Cliquez à l'endroit où le champ doit apparaître.
2. Cliquez sur **Insertion > Champ**.
3. Choisissez un champ dans la liste. Son code apparaît dans la zone ci-dessous, et vous pouvez le modifier.
4. Cliquez sur **OK**.

Par exemple, choisissez **Date**, remplacez le code par `DATE \@ "MMMM d, yyyy"`, puis cliquez sur **OK**. Le texte affiche la date sous la forme `2 novembre 2026`.

## Modifier le code d'un champ

Faites un clic droit sur un champ, puis choisissez **Modifier le champ**. Une boîte affiche le code. Modifiez-le, puis cliquez sur **OK** : le champ est recalculé. **Annuler** rétablit l'ancien champ. Un champ verrouillé ne peut pas être modifié tant que vous ne l'avez pas déverrouillé. Le menu du clic droit contient aussi **Mettre à jour le champ**, **Basculer le code de champ**, **Verrouiller le champ**, **Déverrouiller le champ** et **Délier le champ**.

## Mettre à jour les champs

Un champ affiche le résultat qu'il avait lors de son dernier calcul. Appuyez sur **F9** pour recalculer le champ où se trouve le point d'insertion. Pour mettre à jour plusieurs champs à la fois, sélectionnez le texte qui les contient, puis appuyez sur **F9**. Appuyez sur **Ctrl+A**, puis sur **F9**, pour mettre à jour tous les champs du document.

La mise à jour se fait en une seule étape : **Annuler** rétablit tous les résultats précédents. La mise à jour n'est pas une modification suivie, même lorsque le suivi des modifications est activé, car le résultat n'est pas un texte que vous avez saisi.

Un champ d'en-tête ou de pied de page qui affiche le numéro de page, le nombre de pages ou un titre est toujours exact et n'a pas besoin d'être mis à jour.

## Afficher les codes

Appuyez sur **Maj+F9** pour afficher le code du champ où se trouve le point d'insertion, entre accolades, par exemple `{ PAGE }`, à la place de son résultat. Appuyez sur **Alt+F9** pour afficher les codes de tous les champs, puis appuyez de nouveau dessus pour afficher les résultats. Cette opération ne modifie que l'affichage : le fichier est enregistré tel quel, et le document n'est pas marqué comme modifié.

## Ombrer les champs

Pour repérer les champs, ouvrez **Fichier > Options > Affichage** et choisissez **Ombrage des champs**. **Jamais** n'affiche aucun ombrage. **Toujours** ombre tous les champs en gris clair. **Lorsqu'il est sélectionné** n'ombre que le champ où se trouve le curseur. L'ombrage est visible à l'écran, mais n'est jamais imprimé ni enregistré dans un fichier PDF.

## Mettre à jour les champs avant l'impression

Cochez **Mettre à jour les champs avant l'impression** dans **Fichier > Options > Affichage** pour recalculer tous les champs à chaque impression ou enregistrement au format PDF, afin que les renvois de page, les dates et les totaux soient exacts sur le papier. Il s'agit d'une étape que vous pouvez annuler. Cette mise à jour n'est pas effectuée pour un document ouvert en lecture seule.

## Dissocier ou verrouiller un champ

- **Ctrl+Maj+F9** transforme le champ en texte ordinaire. Le résultat est conservé, mais ce n'est plus un champ.
- **Ctrl+F11** verrouille un champ, de sorte que **F9** ne le modifie pas. **Ctrl+Maj+F11** le déverrouille.

## Formats

Ajoutez des commutateurs au code pour modifier l'apparence du résultat :

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` et `\* Caps` modifient la casse ou le type de nombre. `{ = 21 \* CardText }` affiche `vingt-et-un`.
- `\# "0.00"` définit l'apparence d'un nombre. `{ = 1234.5 \# "#,##0.00" }` affiche `1 234,50`.
- `\@ "dddd, MMMM d"` définit l'apparence d'une date. `M` désigne le mois, et `m` désigne la minute.

## Champs calculés par JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, une formule commençant par `=`, TOC (également utilisé pour une table des illustrations), INDEX, TOA, CITATION et BIBLIOGRAPHY. Les marques masquées TC, XE et TA n'ont pas de résultat à calculer : elles indiquent à un tableau, à un index ou à une table des sources juridiques ce qu'il faut répertorier. Un champ d'un autre type conserve le résultat qu'il a et est enregistré tel quel.

DATE, TIME, FILENAME et AUTHOR dépendent de l'ordinateur et du moment : leur résultat peut donc changer à chaque mise à jour.
