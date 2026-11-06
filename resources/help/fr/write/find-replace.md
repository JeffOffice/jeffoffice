# Rechercher et remplacer des mots
<!-- keywords: recherche avancée, son proche, soundex, formes de mot, toutes les formes d'un mot, ignorer la ponctuation, ignorer les espaces, spécial, marque de paragraphe, tabulation, rechercher par mise en forme, mise en forme, gras, police, style, caractères génériques, caractère générique, motif, expression régulière, groupes, nombre d'occurrences, surligner toutes les occurrences, remplacer, remplacer tout, rechercher, chercher, changer des mots, respecter la casse, mots entiers, suivant, ctrl+f, ctrl+h, modification, volet, texte masqué, annuler -->

Utilisez Rechercher pour atteindre des mots dans votre document. Utilisez Remplacer pour les modifier. Les deux fonctions fonctionnent dans un même volet à droite de la fenêtre.

## Rechercher des mots

1. Appuyez sur **Ctrl+F**, ou cliquez sur **Accueil > Modification > Rechercher**. Le volet **Rechercher et remplacer** s'ouvre, avec le curseur dans **Rechercher**.
2. Tapez les mots recherchés. Appuyez ensuite sur **Entrée**, ou cliquez sur **Suivant**.

JeffWrite commence à l'emplacement de votre curseur. Il sélectionne l'endroit suivant où il trouve vos mots, et fait défiler les pages pour que vous puissiez le voir. Cet endroit s'appelle une **occurrence**. Cliquez de nouveau sur **Suivant** pour passer à l'occurrence suivante. Après la dernière occurrence, le programme revient au début du document.

Pendant la saisie, le volet compte les occurrences, par exemple « 9 occurrences », et chaque occurrence reçoit une bande jaune sur les pages. Les bandes s'affichent aussi dans les cellules de tableau, les en-têtes, les pieds de page et les notes de bas de page. Après **Suivant**, le volet indique laquelle vous êtes en train de voir, par exemple « Occurrence 2 sur 4 ». Le nombre et les bandes changent lorsque vous cliquez sur **Respecter la casse** ou **Mots entiers uniquement**, ou lorsque vous modifiez le texte. Ils disparaissent à la fermeture du volet.

## Rechercher avec des caractères génériques

Cochez **Utiliser les caractères génériques** pour rechercher un motif et non seulement des mots. Un **caractère générique** est un signe qui représente d'autres lettres. Voici ces signes :

- **?** représente une lettre quelconque. **c?t** trouve « cat » et « cot ».
- **\*** représente une suite quelconque de lettres. **c\*t** trouve « cat » et « cart ».
- **<** marque le début d'un mot, et **>** la fin. **<the>** trouve « the » mais pas « other ».
- **[abc]** représente une de ces lettres, **[a-c]** représente une lettre de a à c, et **[!abc]** représente une lettre autre que celles-ci.
- **{2}** répète deux fois la lettre qui précède. **{1,3}** la répète de une à trois fois.
- **@** signifie une ou plusieurs fois la lettre qui précède.
- **\\** placé devant un signe trouve le signe lui-même. **\\?** trouve un point d'interrogation.
- **^p** est une marque de paragraphe, **^t** une tabulation, **^#** un chiffre, **^$** une lettre, et **^w** un espace blanc.

Placez une partie du motif entre **( )** pour créer un **groupe**. Dans **Remplacer par**, **\1** désigne le premier groupe, **\2** le deuxième, et ainsi de suite jusqu'à **\9**. **^&** désigne tout ce qui a été trouvé. Par exemple, recherchez **(<[A-Z]@>), (<[A-Z]@>)** et remplacez par **\2 \1** pour transformer « Dupont, Jean » en « Jean Dupont ». **Mots entiers uniquement** est grisé lorsque les caractères génériques sont activés, car **<** et **>** remplissent cette fonction.

Si le motif comporte une erreur, le volet indique laquelle.

Si vos mots ne figurent pas dans le document, le volet le signale, et rien ne bouge.

Vous pouvez continuer à travailler dans le document pendant que le volet est ouvert. Cliquez dans la page pour écrire. Cliquez sur la croix **X** en haut du volet pour le fermer.

## Recherche avancée

Ces options se trouvent dans le volet, sous **Utiliser les caractères génériques**. Chacune modifie la manière dont JeffWrite lit les mots saisis dans **Rechercher**.

- **Son proche** trouve les mots prononcés comme le vôtre. Il attribue à chaque mot un code tiré de ses sons, et trouve les mots ayant le même code. Tapez **Dupond**, et il trouve aussi « Dupon ».
- **Rechercher toutes les formes du mot** trouve toutes les formes du mot. Tapez **courir**, et il trouve « cours », « courait » et « courra ». Cette option porte sur des mots entiers. Si vous tapez deux mots, par exemple **grand chien**, il les trouve côte à côte, sous n'importe laquelle de leurs formes.
- **Ignorer les caractères de ponctuation** ignore les signes tels que les tirets et les points placés entre les lettres. Tapez **courriel**, et il trouve aussi « cour-riel ».
- **Ignorer les caractères d'espacement** ignore les espaces et les tabulations placés entre les lettres. Tapez **cour riel**, et il trouve « courriel ».

Une seule des options **Son proche**, **Rechercher toutes les formes du mot** et **Utiliser les caractères génériques** peut être activée à la fois. **Son proche** et **Rechercher toutes les formes du mot** trouvent toujours des mots entiers ; **Mots entiers uniquement** et **Respecter la casse** sont donc grisés.

### Spécial

Cliquez sur **Spécial** pour placer un code dans **Rechercher** : **Marque de paragraphe** (^p), **Caractère de tabulation** (^t), **Tout chiffre** (^#), **Toute lettre** (^$), **Espace blanc** (^w), **Saut de section** (^b), **Champ** (^d), **Appel de note de bas de page** (^f), **Appel de note de fin** (^e) ou **Caractère circonflexe** (^^). Les codes fonctionnent sans **Utiliser les caractères génériques**. Un **saut de section** est la marque qui termine une section. Un **champ** est un élément de texte que le programme calcule, comme un numéro de page ou une date, et **^d** trouve le début de chacun d'eux. Les appels de notes sont les petits chiffres du texte qui renvoient à une note de bas de page ou de fin. Ces quatre marques sont trouvées, mais **Remplacer** et **Remplacer tout** les laissent intactes, car les modifier ferait perdre ce qu'elles représentent. Les autres signes de la liste, comme **?** et **\***, sont alors de simples caractères.

### Mise en forme

Cliquez sur **Mise en forme** pour ne rechercher que le texte présentant un certain aspect : **Gras**, **Italique**, **Souligné**, **Barré**, **Surligné**, une **Police** ou un **Style** de paragraphe. Le volet affiche sous les boutons les mises en forme que vous avez choisies. Choisissez **Aucune mise en forme** pour les effacer.

Vous pouvez laisser **Rechercher** vide. Chaque passage de texte ayant cette mise en forme devient alors une occurrence. Si vous saisissez des mots dans **Rechercher**, seuls les mots ayant cette mise en forme sont trouvés. **Remplacer tout** ne modifie que ces mots.

## Affiner la recherche

- **Respecter la casse** ne trouve que les mots ayant les mêmes majuscules. Avec cette option, « Plan » ne trouve pas « plan ».
- **Mots entiers uniquement** ignore les mots plus longs qui contiennent le vôtre. Avec cette option, « plan » ne trouve pas « planète ».

## Remplacer des mots

1. Appuyez sur **Ctrl+H**, ou cliquez sur **Accueil > Modification > Remplacer**. Le curseur passe dans **Remplacer par**.
2. Tapez les nouveaux mots dans **Remplacer par**. Pour supprimer les mots trouvés, laissez ce champ vide.
3. Tapez les anciens mots dans **Rechercher**.
4. Cliquez sur **Remplacer** pour voir la première occurrence. Cliquez de nouveau sur **Remplacer** pour la modifier. JeffWrite passe alors à l'occurrence suivante. Chaque modification correspond à une étape, si bien que **Annuler** (Ctrl+Z) les retire une à une.
5. Cliquez sur **Remplacer tout** pour modifier toutes les occurrences d'un seul coup. Le volet indique combien il en a modifié. **Remplacer tout** correspond à une seule étape : un seul **Annuler** rétablit tous les mots.

Les nouveaux mots prennent la mise en forme du premier caractère des mots remplacés. Si vous remplacez un mot en gras, le nouveau mot est en gras.

Par exemple, pour remplacer « couleur » par « colour » partout : tapez **couleur** dans **Rechercher**, tapez **colour** dans **Remplacer par**, puis cliquez sur **Remplacer tout**.

## Ce que la recherche examine

- Elle examine le texte du document, y compris le texte des tableaux. Un signet placé au milieu de certains mots, présent dans certains fichiers, n'empêche pas de les trouver. **Remplacer** et **Remplacer tout** conservent le signet.
- **Suivant** examine d'abord le texte. Il passe ensuite aux en-têtes, puis aux pieds de page, puis aux notes de bas de page, et revient enfin au début du texte. Lorsqu'une occurrence se trouve dans l'une de ces zones, JeffWrite l'ouvre pour modification et sélectionne l'occurrence. Le volet indique où elle se trouve, par exemple « Trouvé dans l'en-tête de la page 3 ». **Remplacer** modifie l'occurrence sélectionnée, qu'elle se trouve dans le texte, un en-tête, un pied de page ou une note de bas de page. **Remplacer tout** modifie toutes les occurrences de toutes ces zones, et un seul **Annuler** les rétablit toutes. Les notes de fin font partie du texte ; elles sont donc recherchées avec lui. Voir [En-têtes et pieds de page](headers-footers).
- Il ignore le texte défini comme **Masqué**, car ce texte n'est pas affiché. Voir [Modifier l'apparence du texte](font).

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Utiliser JeffWrite au clavier](keyboard)
