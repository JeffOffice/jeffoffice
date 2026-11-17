# Notes de bas de page et de fin

<!-- keywords: marque personnalisée, sous le texte, colonnes, symbole, note de bas de page, note de fin, note, notes, renvoi, référence, citation, source, insérer une note de bas de page, insérer une note de fin, onglet Références, numéro, bas de page, fin du document, légende, Ctrl+Alt+F, Ctrl+Alt+D, format de numérotation, commencer à, recommencer, convertir, permuter, séparateur, avis de suite, numérotation, emplacement, chiffres romains, lettres, symboles, options des notes de bas de page et de fin -->

Une **note** apporte des précisions sur certains mots sans interrompre la phrase. Un petit chiffre en exposant dans le texte, appelé le **renvoi**, indique la note qui lui correspond. Une **note de bas de page** est placée au bas de la page qui contient son renvoi. Une **note de fin** est placée après le dernier texte du document, ou à la fin d'une section lorsque le fichier le demande.

## Ajouter une note de bas de page

1. Cliquez à l'endroit où doit figurer le renvoi, juste après les mots auxquels la note se rapporte.
2. Cliquez sur **Références > Insérer une note de bas de page**, ou appuyez sur **Ctrl+Alt+F**.

JeffWrite place le renvoi dans le texte et crée une nouvelle note au bas de la page, avec le même numéro. Le point d'insertion passe dans la note, et vous pouvez la saisir immédiatement.

Les notes sont numérotées dans l'ordre de leurs renvois dans le texte. Si vous ajoutez une note de bas de page avant d'autres notes, chaque note suivante passe au numéro suivant, et le renvoi dans le texte affiche également le nouveau numéro.

## Utiliser une marque personnalisée

Ouvrez la boîte de dialogue depuis **Références > Note de bas de page et note de fin**, et regardez l'option **Nouvelle note** en bas. Choisissez **Note de bas de page** ou **Note de fin**, saisissez une marque, comme \* ou ††, dans **Marque personnalisée** (ou cliquez sur **Symbole...** pour en choisir une), puis cliquez sur **Insérer**. Le renvoi dans le texte et la note commencent alors par votre marque au lieu d'un numéro. Une note qui a sa propre marque ne prend pas de numéro, et les notes qui la suivent ne sont pas comptées différemment. Si **Marque personnalisée** reste vide, **Insérer** numérote la note comme d'habitude. L'insertion se fait en une seule étape : **Annuler** supprime en une fois le renvoi, la marque et la note.

## Ajouter une note de fin

Cliquez à l'endroit où doit figurer le renvoi, puis cliquez sur **Références > Insérer une note de fin**, ou appuyez sur **Ctrl+Alt+D**. JeffWrite place le renvoi dans le texte et crée une nouvelle note de fin après le dernier texte, avec son numéro. Le point d'insertion passe dans la note. Les notes de fin ont leur propre numérotation, selon le style demandé par le fichier.

## Modifier la numérotation des notes

Cliquez sur la petite flèche dans le coin du groupe **Notes de bas de page** de l'onglet **Références**. La boîte de dialogue comporte une partie pour les notes de bas de page et une partie pour les notes de fin :

- **Format de numérotation** définit l'apparence des numéros : 1, 2, 3 ; a, b, c ; A, B, C ; i, ii, iii ; I, II, III ; ou les marques \*, †, ‡, §.
- **Commencer à** indique le numéro de la première note. Si vous choisissez 3 avec le format i, ii, iii, la première note affiche iii.
- **Numérotation** indique quand les numéros repartent de 1 : jamais (**Continue**), à chaque nouvelle section ou, pour les notes de bas de page, en haut de chaque page.
- **Emplacement** indique où les notes sont placées. Pour les notes de bas de page, **Bas de page** les place au bas de la page, et **Sous le texte** les place juste après la dernière ligne de texte de la page, de sorte qu'une page courte a ses notes proches des mots. Pour les notes de fin, il s'agit de la fin de chaque section ou de la fin du document.
- **Colonnes** (notes de bas de page uniquement) répartit les notes sur deux colonnes ou plus, côte à côte au bas de la page. Chaque note reste entière dans une colonne, et une note trop haute pour sa colonne n'est pas coupée.

Les notes de bas de page et les notes de fin changent immédiatement, et un seul **Annuler** rétablit l'état précédent, avec les marques et les notes comme elles étaient. La modification est enregistrée avec le fichier, de sorte que les autres programmes affichent les mêmes numéros.

## Transformer les notes de bas de page en notes de fin, ou inversement

Cliquez sur **Références > Convertir...** (un petit bouton à côté des boutons des notes). La boîte de dialogue propose quatre choix :

- **Convertir toutes les notes de bas de page en notes de fin**
- **Convertir toutes les notes de fin en notes de bas de page**
- **Permuter les notes de bas de page et les notes de fin** : chaque note de bas de page devient une note de fin, et chaque note de fin devient une note de bas de page.
- **Convertir cette note** : ce choix est disponible lorsque le point d'insertion se trouve dans une note, et il ne modifie que celle-ci.

Le texte de chaque note le suit. Chaque note prend l'apparence de son nouveau type, et les notes sont à nouveau numérotées. L'opération se fait en une seule étape : **Annuler** rétablit toutes les notes.

## Modifier le trait au-dessus des notes

Une note comporte trois éléments qui ne sont pas du texte :

- Le **séparateur** est le trait entre le texte et les notes.
- Le **séparateur de suite** est le trait au-dessus d'une note qui se poursuit depuis la page précédente.
- L'**avis de suite** est le texte placé sous une note qui se poursuit sur la page suivante, par exemple « Suite page suivante ».

Cliquez sur **Références > Séparateurs...**. Pour chaque élément, choisissez **Trait**, **Texte** (puis saisissez le texte) ou **Aucun**. Les notes de fin n'ont que le séparateur, car elles se poursuivent dans le texte et ne sont pas coupées au bas d'une page ; les deux autres éléments d'une note de fin, lorsque le fichier en contient, sont conservés tels quels. Un élément qui contient plus qu'un trait ou quelques mots, comme une image, affiche **Comme dans le fichier**, et JeffWrite le conserve tel quel. La boîte de dialogue exige au moins une note du type concerné. Lorsque vous modifiez un élément d'une note de fin, JeffWrite relit le document, comme pour la numérotation, et la liste des modifications annulables repart à zéro.

## Modifier une note

Cliquez dans le texte d'une note au bas de la page, ou après le dernier texte pour une note de fin. Le point d'insertion s'y place. Saisissez, sélectionnez, coupez, copiez, collez et modifiez l'apparence des mots comme dans le texte, et choisissez un style pour son paragraphe : le paragraphe reste une partie de la note. Une note peut comporter plusieurs paragraphes.

Deux éléments d'une note ne sont pas du texte, et aucune modification ne peut les changer : le numéro qui commence la note et le court trait au-dessus des notes. Si vous appuyez sur **Retour arrière** juste après le numéro, rien ne se passe. Une sélection qui irait d'une note à une autre s'arrête à la fin de la note où vous avez commencé.

Pour quitter une note, appuyez sur **Échap**, appuyez sur **Haut** à sa première ligne ou sur **Bas** à sa dernière ligne, ou cliquez dans le texte de la page. Le point d'insertion revient dans le texte, juste après le renvoi de la note. Une note longue peut se poursuivre au bas de la page suivante ; les touches fléchées passent d'une partie à l'autre, et la note n'est quittée qu'à sa toute première et sa toute dernière ligne. Une note qui comporte deux renvois est placée sur les deux pages, et c'est la page sur laquelle vous avez cliqué que vous modifiez.

## Supprimer ou déplacer une note

- Supprimez un renvoi, et sa note disparaît avec lui. Les notes qui suivent sont à nouveau numérotées. **Annuler** rétablit les deux.
- Coupez un renvoi et collez-le ailleurs : sa note le suit.
- Copiez un renvoi et collez-le : vous obtenez une seconde note avec le même texte, que vous pouvez modifier indépendamment.
- Certains fichiers comportent deux renvois vers une même note. Supprimez l'un des deux, et la note reste. Supprimez le dernier, et la note disparaît.

## Ce que JeffWrite conserve

- Une partie du fichier que vous n'avez pas modifiée est enregistrée exactement comme elle était. Lorsque vous modifiez une note, seules les notes modifiées sont réécrites, et tout ce qu'elles contiennent et que JeffWrite ne peut pas modifier (une image, un signet, un lien, un champ) reste à sa place.
- Le trait au-dessus des notes, et celui qui précède une note qui se poursuit depuis la page précédente, sont conservés tels que le fichier les contient, jusqu'à ce que vous les modifiiez dans **Séparateurs de notes**.
- Si le fichier ne contient aucune note, la première que vous ajoutez crée aussi ce dont une partie des notes a besoin, afin que les autres programmes ouvrent le fichier sans message.
- Une note reste dans le fichier tant qu'un élément la cite : le texte, un en-tête ou un pied de page, une zone de texte ou une autre note. Lorsque le dernier élément qui la cite la libère, la note disparaît.
- Un document peut contenir jusqu'à 5 000 notes de chaque type citées par son texte. Lorsque ce nombre est atteint, **Insérer une note de bas de page** et **Insérer une note de fin** sont désactivées, et la barre d'état l'indique. Les notes ajoutées puis annulées ne comptent pas.
- **Rechercher et remplacer** et le volet **Orthographe** (F7) examinent le texte du document, et non ses notes.

## Annuler

**Annuler** (Ctrl+Z) rétablit la dernière modification effectuée, où qu'elle se trouve : dans le texte, dans un en-tête ou un pied de page, ou dans une note. S'il s'agissait d'une note de bas de page, le point d'insertion s'y place pour vous le montrer. **Rétablir** la refait.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [En-têtes et pieds de page](headers-footers)
- [Vérifier l'orthographe](spelling)
- [Utiliser JeffWrite au clavier](keyboard)
