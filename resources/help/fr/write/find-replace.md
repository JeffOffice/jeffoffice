# Rechercher et remplacer des mots
<!-- keywords: recherche avancée, sonorité proche, soundex, formes d'un mot, toutes les formes d'un mot, ignorer la ponctuation, ignorer les espaces, spécial, marque de paragraphe, tabulation, rechercher par mise en forme, mise en forme, gras, police, style, caractères génériques, caractère générique, motif, expression régulière, groupes, nombre d'occurrences, surligner toutes les occurrences, remplacer, tout remplacer, rechercher, rechercher des mots, modifier des mots, respecter la casse, mots entiers, rechercher suivant, ctrl+f, ctrl+h, modification, volet, texte masqué, annuler, recherche, trouver, chercher, remplacement -->

Utilisez **Rechercher** pour accéder rapidement aux mots de votre document. Utilisez **Remplacer** pour les modifier. Les deux fonctions s'utilisent dans un seul volet, à droite de la fenêtre.

## Rechercher des mots

1. Appuyez sur **Ctrl+F**, ou cliquez sur **Accueil > Modification > Rechercher**. Le volet **Rechercher et remplacer** s'ouvre, avec le curseur dans **Rechercher**.
2. Saisissez les mots recherchés. Appuyez ensuite sur **Entrée**, ou cliquez sur **Rechercher suivant**.

JeffWrite commence à l'endroit où se trouve votre curseur. Il sélectionne l'occurrence suivante de vos mots et fait défiler les pages pour que vous la voyiez. Chaque endroit trouvé s'appelle une **occurrence**. Cliquez de nouveau sur **Rechercher suivant** pour passer à l'occurrence suivante. Après la dernière, la recherche reprend au début du document.

Pendant la saisie, le volet compte les occurrences, par exemple « 9 occurrences », et chacune reçoit une bande jaune sur les pages. Les bandes apparaissent aussi dans les cellules de tableau, les en-têtes, les pieds de page et les notes de bas de page. Après **Rechercher suivant**, le volet indique l'occurrence en cours, par exemple « Occurrence 2 sur 4 ». Le nombre et les bandes changent lorsque vous cliquez sur **Respecter la casse** ou sur **Mots entiers uniquement**, ou lorsque vous modifiez le texte. Les bandes restent affichées sur les pages lorsque vous fermez le volet et lorsque vous cliquez sur **Mode Lecture**, afin que vous puissiez lire avec toutes les occurrences visibles. Le document n'est pas modifié, et les bandes ne sont jamais imprimées ni enregistrées. Elles disparaissent lorsque vous modifiez le document, lorsque vous rouvrez le volet, ou lorsque vous cliquez sur **Accueil > Modification > Effacer les surlignages de recherche**.

## Rechercher avec des caractères génériques

Cochez **Utiliser les caractères génériques** pour rechercher un motif et non seulement des mots. Un **caractère générique** est un signe qui remplace d'autres lettres. Voici ces signes :

- **?** représente une lettre quelconque. **c?t** trouve « chat » et « cot ».
- **\*** représente une suite quelconque de lettres. **c\*t** trouve « chat » et « chant ».
- **<** marque le début d'un mot, et **>** la fin d'un mot. **<le>** trouve « le » mais pas « lent ».
- **[abc]** représente l'une de ces lettres, **[a-c]** représente une lettre de a à c, et **[!abc]** représente une lettre autre que celles-ci.
- **{2}** répète deux fois la lettre qui précède. **{1,3}** la répète de une à trois fois.
- **@** signifie une ou plusieurs fois la lettre qui précède.
- **\\** placé devant un signe trouve le signe lui-même. **\\?** trouve un point d'interrogation.
- **^p** est une marque de paragraphe, **^t** une tabulation, **^#** un chiffre, **^$** une lettre, et **^w** un espace blanc.

Placez une partie du motif entre **( )** pour former un **groupe**. Dans **Remplacer par**, **\1** désigne le premier groupe, **\2** le deuxième, et ainsi de suite jusqu'à **\9**. **^&** désigne tout ce qui a été trouvé. Par exemple, recherchez **(<[A-Z]@>), (<[A-Z]@>)** et remplacez par **\2 \1** pour transformer « Dupont, Jean » en « Jean Dupont ». **Mots entiers uniquement** est grisé lorsque les caractères génériques sont activés, car **<** et **>** remplissent ce rôle.

Si le motif comporte une erreur, le volet indique laquelle.

Si vos mots ne figurent pas dans le document, le volet l'indique, et rien ne bouge.

Vous pouvez continuer à travailler dans le document pendant que le volet est ouvert. Cliquez dans la page pour écrire. Cliquez sur la croix **X** en haut du volet pour le fermer.

## Rechercher avec des expressions régulières

Cochez **Utiliser les expressions régulières** pour des motifs plus élaborés que les caractères génériques. Une **expression régulière** est un motif court, composé de lettres et de signes. Cette option s'active à la place de **Utiliser les caractères génériques** : une seule des deux peut être activée. Voici les signes les plus courants :

- **.** représente un caractère quelconque, **\d** un chiffre, **\w** une lettre, un chiffre ou un tiret bas, et **\s** un espace ou une tabulation. **[a-c]** représente une lettre de a à c, et **[^abc]** un caractère autre que ceux-ci.
- **^** marque le début d'un paragraphe, **$** la fin d'un paragraphe, et **\b** la limite d'un mot. **\bchat\b** trouve « chat » mais pas « chatte ».
- **\*** signifie zéro ou plusieurs fois ce qui précède, **+** une ou plusieurs fois, et **?** zéro ou une fois. **{2}** signifie exactement deux fois, et **{2,5}** de deux à cinq fois ; un nombre de répétitions peut aller jusqu'à 200. Ajoutez **?** après une répétition, comme dans **.+?**, pour trouver la plus courte suite qui convient.
- **|** signifie « ou » : **chat|chien** trouve l'un ou l'autre mot.
- **( )** forme un **groupe**. Utilisez **(?: )** pour un groupe dont vous n'avez pas besoin dans **Remplacer par**.
- **\\** placé devant un signe trouve le signe lui-même : **\.** trouve un point. **\t** représente une tabulation.

**Respecter la casse** fonctionne comme pour les mots. Les lettres accentuées et les lettres d'autres alphabets sont considérées comme des lettres.

Un motif est recherché paragraphe par paragraphe, donc **^** et **$** désignent le début et la fin de chaque paragraphe. Pour rechercher au-delà de la fin d'un paragraphe, placez **\n** (une marque de paragraphe) dans le motif : **accueil\.\nLee** trouve « accueil. » à la fin d'un paragraphe et « Lee » au début du suivant.

Dans **Remplacer par**, **$1** ou **\1** désigne le premier groupe, **$2** ou **\2** le deuxième, et ainsi de suite jusqu'à neuf groupes. **$0** ou **\0** désigne tout ce qui a été trouvé. **$$** représente un signe dollar, **\\** une barre oblique inverse, et **\t** une tabulation. Par exemple, recherchez **(\w+), (\w+)** et remplacez par **$2 $1** pour transformer « Dupont, Jean » en « Jean Dupont ».

**Tout remplacer** modifie toutes les occurrences du texte, des en-têtes, des pieds de page, des notes et des zones de texte en une seule action, de sorte qu'un seul **Annuler** les rétablit toutes. Lorsque le **Suivi des modifications** est activé, chaque modification est suivie, comme pour les mots. Le texte masqué n'est pas modifié.

Si le motif comporte une erreur, le volet indique près de quel caractère. Un motif peut ne correspondre à aucun caractère, comme **^** (le début d'un paragraphe) ou **$** (la fin) : **Remplacer par** insère alors son texte à cet endroit. Ainsi, **^** avec **> ** ajoute ces signes au début de chaque paragraphe. Un motif qui demanderait beaucoup trop de temps sur votre texte, comme **(a+)+$** sur une longue ligne de a, est arrêté par JeffWrite, qui le signale ; rien n'est modifié. Une recherche longue affiche une boîte **Recherche en cours** avec un bouton **Annuler** ; annuler laisse votre document tel qu'il était, et vous ne pouvez pas écrire dans le document tant que la recherche n'est pas terminée. Les motifs qui regardent vers l'avant ou vers l'arrière sans consommer ce qu'ils vérifient, comme **x(?=\d)**, et les motifs en mode étendu, comme **(?x)**, ne sont pas pris en charge ; JeffWrite le signale. Un paragraphe très long est recherché par parties, et chaque occurrence est celle qu'une recherche sur l'ensemble du paragraphe trouverait. Un motif qui correspond à une très longue portion d'un paragraphe très long (des dizaines de milliers de caractères) peut être arrêté avec le message « travail trop important », et une occurrence de plus de 200 000 caractères n'est pas remplacée : JeffWrite s'arrête, le signale, et ne modifie rien.

## Recherche avancée

Ces options se trouvent dans le volet, sous **Utiliser les expressions régulières**. Chacune modifie la manière dont JeffWrite lit les mots saisis dans **Rechercher**.

- **Sonorité proche** trouve les mots qui se prononcent comme votre mot. Il attribue à chaque mot un code issu de ses sons, et trouve les mots ayant le même code. Si vous saisissez **Dupont**, il trouve aussi « Dupond ».
- **Toutes les formes d'un mot** trouve toutes les formes du mot. Si vous saisissez **courir**, il trouve « courons », « court » et « couru ». Cette option agit sur des mots entiers. Si vous saisissez deux mots, comme **grand chien**, il les trouve côte à côte, sous toutes leurs formes.
- **Ignorer les signes de ponctuation** ignore les signes comme les tirets et les points placés entre les lettres. Si vous saisissez **courriel**, il trouve aussi « courri-el ».
- **Ignorer les espaces** ignore les espaces et les tabulations placés entre les lettres. Si vous saisissez **cour riel**, il trouve « courriel ».

Une seule option parmi **Sonorité proche**, **Toutes les formes d'un mot**, **Utiliser les caractères génériques** et **Utiliser les expressions régulières** peut être activée à la fois. **Sonorité proche** et **Toutes les formes d'un mot** trouvent toujours des mots entiers ; **Mots entiers uniquement** et **Respecter la casse** sont donc grisés.

### Spécial

Cliquez sur **Spécial** pour insérer un code dans **Rechercher** : **Marque de paragraphe** (^p), **Caractère de tabulation** (^t), **Tout chiffre** (^#), **Toute lettre** (^$), **Espace blanc** (^w), **Saut de section** (^b), **Champ** (^d), **Appel de note de bas de page** (^f), **Appel de note de fin** (^e), ou **Caractère circonflexe** (^^). Ces codes fonctionnent sans **Utiliser les caractères génériques**. Un **saut de section** est la marque qui termine une section. Un **champ** est un élément de texte que le programme calcule, comme un numéro de page ou une date ; **^d** trouve le début de chacun. Les appels de notes sont les petits chiffres du texte qui renvoient à une note de bas de page ou de fin. Ces quatre marques sont trouvées, mais **Remplacer** et **Tout remplacer** ne les modifient pas, car leur remplacement ferait perdre ce qu'elles représentent. Les autres signes de la liste, comme **?** et **\***, sont alors de simples caractères.

### Mise en forme

Cliquez sur **Mise en forme** pour ne rechercher que le texte présentant un certain aspect : **Gras**, **Italique**, **Souligné**, **Barré**, **Surligné**, une **Police**, ou un **Style** de paragraphe. Le volet affiche sous les boutons les aspects choisis. Choisissez **Aucune mise en forme** pour les effacer.

Vous pouvez laisser **Rechercher** vide. Chaque passage de texte ayant cet aspect devient alors une occurrence. Si vous saisissez des mots dans **Rechercher**, seuls les mots ayant cet aspect sont trouvés. **Tout remplacer** ne modifie que ces mots.

## Affiner la recherche

- **Respecter la casse** ne trouve que les mots ayant les mêmes majuscules. Avec cette option, « Plan » ne trouve pas « plan ».
- **Mots entiers uniquement** ignore les mots plus longs qui contiennent le vôtre. Avec cette option, « plan » ne trouve pas « planète ».
- **Sens** détermine dans quelle direction **Rechercher suivant** avance à partir du curseur. **Bas** va jusqu'à la fin du document, et **Haut** jusqu'au début. Chacun s'arrête à cet endroit et le signale. **Tout** poursuit la recherche jusqu'à l'autre extrémité du document et est le choix habituel. **Tout remplacer** modifie toujours le document entier.

## Remplacer des mots

1. Appuyez sur **Ctrl+H**, ou cliquez sur **Accueil > Modification > Remplacer**. Le curseur passe dans **Remplacer par**.
2. Saisissez les nouveaux mots dans **Remplacer par**. Pour supprimer les mots trouvés, laissez ce champ vide.
3. Saisissez les anciens mots dans **Rechercher**.
4. Cliquez sur **Remplacer** pour voir la première occurrence. Cliquez de nouveau sur **Remplacer** pour la modifier. JeffWrite passe alors à l'occurrence suivante. Chaque modification constitue une action distincte, de sorte que **Annuler** (Ctrl+Z) les annule une par une.
5. Cliquez sur **Tout remplacer** pour modifier toutes les occurrences d'un coup. Le volet indique combien ont été modifiées. **Tout remplacer** modifie aussi les mots des zones de texte et des formes, et constitue une seule action : un seul **Annuler** rétablit tous les mots. Lorsque le **Suivi des modifications** est activé, chaque remplacement est enregistré comme une modification suivie, dans le texte comme dans les zones. Une zone trop grande pour être modifiée est comptée dans le message. **Tout remplacer** n'atteint pas les zones de texte des en-têtes et des pieds de page, les formes groupées, ni les zones liées entre elles. **Rechercher suivant** n'examine pas encore les zones de texte.

Les nouveaux mots reprennent la majuscule initiale des mots qu'ils remplacent. Si vous remplacez un mot en gras, le nouveau mot est également en gras.

Par exemple, pour remplacer « color » par « colour » partout : saisissez **color** dans **Rechercher**, saisissez **colour** dans **Remplacer par**, puis cliquez sur **Tout remplacer**.

## Ce que la recherche examine

- Elle examine le texte du document, y compris celui des tableaux. Un signet placé au milieu de certains mots, présent dans certains fichiers, n'empêche pas de les trouver. **Remplacer** et **Tout remplacer** conservent le signet.
- **Rechercher suivant** examine d'abord le texte. Il passe ensuite aux en-têtes, puis aux pieds de page, puis aux notes de bas de page, et revient enfin au début du texte. Lorsqu'une occurrence se trouve dans l'un de ces éléments, JeffWrite l'ouvre pour modification et la sélectionne. Le volet indique où elle se trouve, par exemple « Trouvé dans l'en-tête de la page 3 ». **Remplacer** remplace l'occurrence sélectionnée, dans le texte ou dans un en-tête, un pied de page ou une note de bas de page. **Tout remplacer** modifie toutes les occurrences de tous ces éléments, et un seul **Annuler** les rétablit toutes. Les notes de fin font partie du texte et sont donc recherchées avec lui. Voir [En-têtes et pieds de page](headers-footers).
- Elle ignore le texte défini comme **Masqué**, car ce texte n'est pas affiché. Voir [Modifier l'apparence du texte](font).

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Utiliser JeffWrite au clavier](keyboard)
