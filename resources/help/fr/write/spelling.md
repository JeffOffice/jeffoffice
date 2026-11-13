# Vérifier l'orthographe
<!-- keywords: orthographe, correcteur orthographique, vérification orthographique, mot mal orthographié, ligne rouge, ligne ondulée, soulignement ondulé, suggestions, ignorer, ignorer tout, ajouter au dictionnaire, dictionnaire personnalisé, dictionnaire, langue, vérification, F7, révision, vérifier l'orthographe pendant la frappe, majuscules, chiffres, grammaire, vérification grammaticale, style, ligne bleue, ligne pointillée, vérifier la grammaire pendant la frappe -->

JeffWrite vérifie l'orthographe au fur et à mesure que vous tapez. Un mot absent du dictionnaire est souligné d'une ligne ondulée rouge. La vérification s'effectue sur votre ordinateur, sans connexion Internet, et elle connaît 31 langues et variantes régionales.

## Pendant la frappe

- Une ligne ondulée rouge signifie que le dictionnaire ne contient pas le mot. Le mot peut tout de même être correct. Les noms propres et les mots récents manquent souvent. Les noms des applications JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail et JeffBase) ainsi que LLC ne sont jamais signalés, quelle que soit la langue, avec ou sans « s ».
- Le mot que vous êtes en train de taper n'est pas signalé tant que vous ne passez pas à la suite, de sorte qu'un mot en cours de saisie ne clignote pas.
- Les lignes ne s'affichent qu'à l'écran. Elles ne sont jamais imprimées et n'apparaissent jamais dans un PDF.

Pour corriger un mot, faites un **clic droit** dessus. Le menu présente les choix suivants, au-dessus des commandes habituelles :

- Jusqu'à six **suggestions**, la meilleure en premier, en gras. Cliquez sur une suggestion pour la mettre à la place du mot. Il s'agit d'une seule action, que **Ctrl+Z** annule.
- **Ignorer tout** laisse ce mot tranquille, à chacune de ses occurrences, jusqu'à la fermeture de JeffWrite.
- **Ajouter au dictionnaire** apprend définitivement le mot à JeffWrite. Le mot est enregistré dans un petit fichier placé dans le dossier de JeffWrite, de sorte que vous pouvez lire ou modifier votre liste de mots ajoutés à cet endroit.

Si le dictionnaire ne propose aucune suggestion, le menu l'indique.

## Parcourir les fautes d'orthographe

Appuyez sur **F7**, ou cliquez sur **Révision > Orthographe**. Le volet Orthographe s'ouvre à droite. Il affiche le premier mot mal orthographié après le point d'insertion, sélectionné dans votre texte, avec la ligne où il se trouve et une liste de suggestions.

- **Remplacer** met la suggestion choisie à la place du mot. Double-cliquez sur une suggestion pour obtenir le même résultat.
- **Remplacer tout** fait de même pour chaque occurrence du mot dans le document. Il s'agit d'une seule action, que **Ctrl+Z** annule pour toutes à la fois.
- **Ignorer** passe ce mot sans le modifier. Sa ligne rouge reste affichée.
- **Ignorer tout** et **Ajouter au dictionnaire** fonctionnent comme dans le menu du clic droit.

Le volet va jusqu'à la fin du document, puis reprend depuis le début, jusqu'à l'endroit où il a commencé. Lorsqu'il n'en reste plus, il indique que la vérification orthographique est terminée. Une fois l'orthographe vérifiée, le volet passe aux problèmes de grammaire et de style (voir Vérifier la grammaire, ci-dessous).

## Activer ou désactiver les lignes rouges

Cliquez sur **Révision > Vérifier l'orthographe lors de la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**, et c'est le seul choix commun à toutes les applications JeffOffice. Vous pouvez aussi choisir d'ignorer les mots en MAJUSCULES et les mots contenant des chiffres, comme 3e. Les deux sont ignorés par défaut.

## Vérifier la grammaire pendant la frappe

JeffWrite recherche aussi les erreurs de grammaire et de style évidentes dans un texte en anglais. Un problème est souligné d'une ligne pointillée bleue. L'orthographe conserve sa ligne ondulée rouge, et un mot mal orthographié n'est jamais signalé comme un problème de grammaire. Voici les erreurs recherchées :

- un mot répété (« the the »)
- « a » et « an » devant un son inadapté (« an test », « a hour »)
- une phrase qui commence par une minuscule après un point
- « its » et « it's », « their » et « there », « your » et « you're », « then » et « than », ainsi que « lose » et « loose », lorsque les mots qui les entourent rendent l'erreur évidente
- « could of » pour « could have »
- deux espaces consécutifs à l'intérieur d'une phrase
- une espace avant une virgule, un point ou un autre signe qui se rattache au mot précédent
- aucune espace après la fin d'une phrase
- « alot », et « everyday » pour « every day » (« almost everyday »)

JeffWrite préfère manquer une erreur plutôt que de signaler un texte correct. Une phrase sans ligne bleue n'est donc pas garantie correcte.

Faites un clic droit sur le problème. Le menu indique ce qui ne va pas, puis propose une ou plusieurs suggestions. Cliquez sur une suggestion pour l'insérer dans le texte, et appuyez sur Ctrl+Z pour l'annuler. **Ignorer ce problème** supprime la ligne à cet endroit uniquement, jusqu'à la fermeture de JeffWrite. Les mêmes mots ailleurs conservent leur ligne, et la modification d'un autre endroit du paragraphe ne la fait pas réapparaître. **Ignorer tout** supprime la ligne de tous les problèmes portant ce message sur ces mots.

La vérification s'effectue sur votre ordinateur, sans connexion Internet, en arrière-plan : la frappe n'est jamais interrompue, et un paragraphe est vérifié peu de temps après que vous avez cessé de le modifier. Un paragraphe est vérifié jusqu'à ses 100 000 premiers caractères. Un document dont la vérification dépasse cette limite cesse d'être vérifié pour la grammaire, et la barre d'état vous le signale une seule fois ; la vérification orthographique continue de fonctionner.

Seul l'anglais est vérifié. Dans un paragraphe comportant plusieurs langues, le texte anglais est vérifié et le reste est ignoré, de même que le texte qu'un fichier marque « ne pas vérifier ». La grammaire est vérifiée dans le corps du document, et pas encore dans les tableaux, les en-têtes, les pieds de page ni les notes.

Pour désactiver cette vérification, cliquez sur **Révision > Vérifier la grammaire lors de la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**. Seul JeffWrite le propose, et il est identique dans toutes les fenêtres de JeffWrite. Lorsque cette option est active, le volet Orthographe (F7) parcourt les problèmes de grammaire après les fautes d'orthographe.

## Langues

JeffWrite vérifie chaque passage de texte dans sa propre langue. Un fichier peut indiquer la langue de son texte, jusqu'à un seul mot, et JeffWrite en tient compte. Le texte qui ne précise rien est vérifié dans la langue du document. Le fichier indique cette langue pour l'ensemble du document, et un nouveau document utilise l'anglais américain.

Le texte dans une langue pour laquelle JeffWrite ne dispose d'aucun dictionnaire n'est pas vérifié. Il en va de même pour le texte qu'un fichier marque « ne pas vérifier l'orthographe ». Vous ne pouvez pas encore changer la langue d'un texte dans JeffWrite, mais la langue indiquée par le fichier est conservée.

Les 31 dictionnaires sont ceux de l'anglais (américain, britannique, canadien et australien), de l'espagnol (21 régions, dont le Mexique et l'Espagne), du français, de l'allemand, de l'italien, du néerlandais et du portugais (Brésil et Portugal).

## Ce qui n'est pas encore vérifié

Les lignes rouges s'affichent dans le texte et dans les cellules des tableaux. Elles ne s'affichent pas encore dans les en-têtes, les pieds de page, les notes ni les zones de texte, et le volet Orthographe ne les vérifie pas.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
