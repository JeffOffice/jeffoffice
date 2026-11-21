# Rechercher et remplacer des mots
<!-- keywords: recherche avancée, sonorité proche, soundex, formes du mot, toutes les formes d'un mot, ignorer la ponctuation, ignorer les espaces, spécial, marque de paragraphe, tabulation, rechercher par mise en forme, mise en forme, gras, police, style, caractères génériques, caractère générique, motif, expression régulière, groupes, nombre d'occurrences, surligner toutes les occurrences, remplacer, remplacer tout, rechercher, chercher, changer des mots, respecter la casse, mots entiers, rechercher suivant, ctrl+f, ctrl+h, modification, volet, texte masqué, annuler -->

Utilisez la recherche pour atteindre des mots dans votre document. Utilisez le remplacement pour les modifier. Les deux fonctions fonctionnent dans un même volet à droite de la fenêtre.

## Rechercher des mots

1. Appuyez sur **Ctrl+F**, ou cliquez sur **Accueil > Modification > Rechercher**. Le volet **Rechercher et remplacer** s'ouvre, avec le curseur dans **Rechercher**.
2. Tapez les mots recherchés. Appuyez ensuite sur **Entrée**, ou cliquez sur **Rechercher suivant**.

JeffWrite commence à l'endroit où se trouve votre curseur. Il sélectionne l'emplacement suivant où il trouve vos mots, et fait défiler les pages pour que vous puissiez le voir. Cet emplacement s'appelle une **occurrence**. Cliquez de nouveau sur **Rechercher suivant** pour passer à l'occurrence suivante. Après la dernière occurrence, le programme revient au début du document.

Pendant la saisie, le volet compte les occurrences, par exemple « 9 occurrences », et chaque occurrence reçoit une bande jaune sur les pages. Les bandes apparaissent aussi dans les cellules de tableau, les en-têtes, les pieds de page et les notes de bas de page. Après **Rechercher suivant**, le volet indique l'occurrence sur laquelle vous vous trouvez, par exemple « Occurrence 2 sur 4 ». Le compte et les bandes changent lorsque vous cliquez sur **Respecter la casse** ou **Mots entiers uniquement**, ou lorsque vous modifiez le texte. Ils disparaissent lorsque vous fermez le volet.

## Rechercher avec des caractères génériques

Cochez **Utiliser les caractères génériques** pour rechercher un motif, et non seulement des mots. Un **caractère générique** est un signe qui remplace d'autres lettres. Voici les signes disponibles :

- **?** remplace une lettre quelconque. **c?t** trouve « cat » et « cot ».
- **\*** remplace une suite quelconque de lettres. **c\*t** trouve « cat » et « chat ».
- **<** marque le début d'un mot, et **>** en marque la fin. **<the>** trouve « the » mais pas « other ».
- **[abc]** désigne une seule de ces lettres, **[a-c]** désigne une lettre de a à c, et **[!abc]** désigne une lettre autre que celles-ci.
- **{2}** répète deux fois la lettre qui précède. **{1,3}** la répète de une à trois fois.
- **@** signifie une ou plusieurs fois la lettre qui précède.
- **\\** placé devant un signe trouve le signe lui-même. **\\?** trouve un point d'interrogation.
- **^p** est une marque de paragraphe, **^t** une tabulation, **^#** un chiffre, **^$** une lettre et **^w** un espace blanc.

Placez une partie du motif entre **( )** pour créer un **groupe**. Dans **Remplacer par**, **\1** désigne le premier groupe, **\2** le deuxième, et ainsi de suite jusqu'à **\9**. **^&** désigne tout ce qui a été trouvé. Par exemple, recherchez **(<[A-Z]@>), (<[A-Z]@>)** et remplacez par **\2 \1** pour transformer « Dupont, Jean » en « Jean Dupont ». **Mots entiers uniquement** est grisé tant que les caractères génériques sont activés, car **<** et **>** remplissent ce rôle.

Si le motif contient une erreur, le volet indique laquelle.

Si vos mots ne figurent pas dans le document, le volet le signale, et rien ne se déplace.

Vous pouvez continuer à travailler dans le document pendant que le volet est ouvert. Cliquez dans la page pour écrire. Cliquez sur la croix **X** en haut du volet pour le fermer.

## Recherche avancée

Ces options se trouvent dans le volet, sous **Utiliser les caractères génériques**. Chacune modifie la manière dont JeffWrite lit les mots de **Rechercher**.

- **Sonorité proche** trouve les mots qui se prononcent comme le vôtre. Il attribue à chaque mot un code fondé sur ses sons, et trouve les mots ayant le même code. Si vous tapez **Martin**, il trouve aussi « Martein ».
- **Rechercher toutes les formes du mot** trouve toutes les formes d'un mot. Si vous tapez **courir**, il trouve « cours », « courons » et « courant ». Cette option agit sur des mots entiers. Si vous tapez deux mots, par exemple **grand chien**, il les trouve côte à côte, sous n'importe laquelle de leurs formes.
- **Ignorer les caractères de ponctuation** ignore les signes tels que les traits d'union et les points placés entre les lettres. Si vous tapez **courriel**, il trouve aussi « cour-riel ».
- **Ignorer les caractères d'espacement** ignore les espaces et les tabulations placées entre les lettres. Si vous tapez **cour riel**, il trouve « courriel ».

Une seule des options **Sonorité proche**, **Rechercher toutes les formes du mot** et **Utiliser les caractères génériques** peut être activée à la fois. **Sonorité proche** et **Rechercher toutes les formes du mot** trouvent toujours des mots entiers ; **Mots entiers uniquement** et **Respecter la casse** sont donc grisés.

### Spécial

Cliquez sur **Spécial** pour placer un code dans **Rechercher** : **Marque de paragraphe** (^p), **Caractère de tabulation** (^t), **Tout chiffre** (^#), **Toute lettre** (^$), **Espace blanc** (^w), **Saut de section** (^b), **Champ** (^d), **Appel de note de bas de page** (^f), **Appel de note de fin** (^e) ou **Caractère circonflexe** (^^). Les codes fonctionnent sans **Utiliser les caractères génériques**. Un **saut de section** est la marque qui termine une section. Un **champ** est un élément de texte que le programme calcule, comme un numéro de page ou une date ; **^d** trouve le début de chacun d'eux. Les appels de note sont les petits chiffres du texte qui renvoient à une note de bas de page ou à une note de fin. Ces quatre éléments peuvent être trouvés, mais **Remplacer** et **Remplacer tout** ne les modifient pas, car changer l'un d'eux ferait perdre ce qu'il représente. Les autres signes de la liste, comme **?** et **\***, sont alors de simples caractères.

### Mise en forme

Cliquez sur **Format** pour ne rechercher que le texte ayant un aspect donné : **Gras**, **Italique**, **Souligné**, **Barré**, **Surligné**, une **Police** ou un **Style de paragraphe**. Le volet affiche sous les boutons les aspects que vous avez choisis. Sélectionnez **Aucune mise en forme** pour les effacer.

Vous pouvez laisser **Rechercher** vide. Chaque passage de texte ayant cet aspect devient alors une occurrence. Si des mots figurent dans **Rechercher**, seuls les mots ayant cet aspect sont trouvés. **Remplacer tout** ne modifie que ces mots.

## Affiner la recherche

- **Respecter la casse** ne trouve que les mots ayant les mêmes majuscules. Avec cette option activée, « Plan » ne trouve pas « plan ».
- **Mots entiers uniquement** ignore les mots plus longs qui contiennent le vôtre. Avec cette option activée, « plan » ne trouve pas « planète ».
- **Rechercher** détermine le sens de **Rechercher suivant** à partir du curseur. **Bas** va jusqu'à la fin du document, et **Haut** jusqu'au début. Chacun s'arrête à cet endroit et l'indique. **Tous** continue jusqu'à l'autre extrémité du document et est le choix habituel. **Remplacer tout** modifie toujours tout le document.

## Remplacer des mots

1. Appuyez sur **Ctrl+H**, ou cliquez sur **Accueil > Modification > Remplacer**. Le curseur passe dans **Remplacer par**.
2. Tapez les nouveaux mots dans **Remplacer par**. Pour supprimer les mots trouvés, laissez ce champ vide.
3. Tapez les anciens mots dans **Rechercher**.
4. Cliquez sur **Remplacer** pour voir la première occurrence. Cliquez de nouveau sur **Remplacer** pour la modifier. JeffWrite passe alors à l'occurrence suivante. Chaque modification constitue une étape, de sorte que **Annuler** (Ctrl+Z) les retire une à une.
5. Cliquez sur **Remplacer tout** pour modifier toutes les occurrences en une fois. Le volet indique combien il en a modifié. **Remplacer tout** modifie aussi les mots des zones de texte et des formes, et constitue une seule étape : un seul **Annuler** rétablit tous les mots. Lorsque **Suivi des modifications** est activé, chaque remplacement est enregistré comme une modification suivie, dans le texte comme dans les zones de texte. Une zone de texte trop grande pour être modifiée est comptée dans le message. **Remplacer tout** n'atteint pas les zones de texte des en-têtes et pieds de page, les formes groupées, ni les zones de texte liées entre elles. **Rechercher suivant** ne parcourt pas encore les zones de texte.

Les nouveaux mots reprennent la mise en forme du premier caractère des mots qu'ils remplacent. Si vous remplacez un mot en gras, le nouveau mot est en gras.

Par exemple, pour remplacer « couleur » par « coloris » partout : tapez **couleur** dans **Rechercher**, tapez **coloris** dans **Remplacer par**, puis cliquez sur **Remplacer tout**.

## Ce que la recherche examine

- Elle examine le texte du document, y compris le texte des tableaux. Un signet placé au milieu de certains mots, présent dans certains fichiers, n'empêche pas de les trouver. **Remplacer** et **Remplacer tout** conservent le signet.
- **Rechercher suivant** examine d'abord le texte. Il passe ensuite aux en-têtes, puis aux pieds de page, aux notes de bas de page, et revient enfin au début du texte. Lorsqu'une occurrence se trouve dans l'une de ces zones, JeffWrite l'ouvre pour la modifier et sélectionne l'occurrence. Le volet indique où elle se trouve, par exemple « Trouvé dans l'en-tête de la page 3 ». **Remplacer** modifie l'occurrence sur laquelle vous vous trouvez, qu'elle soit dans le texte, un en-tête, un pied de page ou une note de bas de page. **Remplacer tout** modifie toutes les occurrences de toutes ces zones, et un seul **Annuler** les rétablit toutes. Les notes de fin font partie du texte et sont donc recherchées avec lui. Voir [En-têtes et pieds de page](headers-footers).
- Il ignore le texte défini comme **Masqué**, car ce texte n'est pas affiché. Voir [Modifier l'apparence du texte](font).

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Utiliser JeffWrite au clavier](keyboard)
