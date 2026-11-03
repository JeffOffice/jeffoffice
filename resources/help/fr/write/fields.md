# Champs
<!-- keywords: champ, champs, mettre à jour le champ, mise à jour des champs, F9, délier, verrouiller, codes de champ, numéro de page, date, NUMPAGES, SEQ, REF, PAGEREF, formule, calcul, Insérer un champ -->

Un **champ** est un élément de texte que le programme calcule pour vous, par exemple un numéro de page, la date du jour ou le numéro d'une figure. Il se compose de deux parties : un **code**, qui indique ce qu'il faut calculer, et un **résultat**, qui est le texte affiché. Le code d'un numéro de page est `PAGE`, et son résultat sur la troisième page est `3`.

## Insérer un champ dans le texte

1. Cliquez à l'endroit où le champ doit apparaître.
2. Cliquez sur **Insertion > Champ**.
3. Choisissez un champ dans la liste. Son code apparaît dans la zone située en dessous, et vous pouvez le modifier.
4. Cliquez sur **OK**.

Par exemple, choisissez **Date**, remplacez le code par `DATE \@ "d MMMM yyyy"`, puis cliquez sur **OK**. Le texte affiche la date sous la forme `2 novembre 2026`.

## Mettre à jour les champs

Un champ affiche le résultat obtenu lors de son dernier calcul. Appuyez sur **F9** pour recalculer le champ dans lequel se trouve le point d'insertion. Pour mettre à jour plusieurs champs à la fois, sélectionnez le texte qui les contient, puis appuyez sur **F9**. Appuyez sur **Ctrl+A** puis sur **F9** pour mettre à jour tous les champs du document.

La mise à jour constitue une seule étape : **Annuler** rétablit tous les résultats précédents. La mise à jour n'est pas une modification suivie, même lorsque le suivi des modifications est activé, car le résultat n'est pas un texte que vous avez saisi.

Un champ placé dans un en-tête ou un pied de page qui affiche le numéro de page, le nombre de pages ou un titre est toujours exact et n'a pas besoin d'être mis à jour.

## Afficher les codes

Appuyez sur **Maj+F9** pour afficher, à la place de son résultat, le code du champ dans lequel se trouve le point d'insertion, entre accolades, par exemple `{ PAGE }`. Appuyez sur **Alt+F9** pour afficher les codes de tous les champs, puis appuyez de nouveau sur cette touche pour afficher les résultats. Seul l'affichage change : le fichier est enregistré tel quel et le document n'est pas marqué comme modifié.

## Délier ou verrouiller un champ

- **Ctrl+Maj+F9** convertit le champ en texte brut. Le résultat est conservé, mais ce n'est plus un champ.
- **Ctrl+F11** verrouille un champ, pour que **F9** ne le modifie pas. **Ctrl+Maj+F11** le déverrouille.

## Formats

Ajoutez des commutateurs au code pour modifier l'apparence d'un résultat :

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` et `\* Caps` modifient la casse ou le type de nombre. `{ = 21 \* CardText }` affiche `vingt et un`.
- `\# "0.00"` définit l'apparence d'un nombre. `{ = 1234.5 \# "#,##0.00" }` affiche `1 234,50`.
- `\@ "dddd d MMMM"` définit l'apparence d'une date. `M` correspond au mois et `m` aux minutes.

## Champs calculés par JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, une formule commençant par `=`, et TOC. Un champ d'un autre type conserve son résultat et est enregistré tel quel.

DATE, TIME, FILENAME et AUTHOR dépendent de l'ordinateur et du moment, et peuvent donc changer à chaque mise à jour.
