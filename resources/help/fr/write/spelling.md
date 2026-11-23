# Vérifier l'orthographe
<!-- keywords: orthographe, vérification orthographique, correcteur orthographique, correcteur, mot mal orthographié, ligne rouge, ligne ondulée, ondulation, suggestions, ignorer, ignorer tout, ajouter au dictionnaire, dictionnaire personnalisé, dictionnaire, langue, vérification, f7, révision, vérifier l'orthographe pendant la frappe, majuscules, nombres, grammaire, vérification grammaticale, style, ligne bleue, ligne pointillée, vérifier la grammaire pendant la frappe -->

JeffWrite vérifie l'orthographe pendant que vous tapez. Un mot absent du dictionnaire est souligné d'une ligne ondulée rouge. La vérification se fait sur votre ordinateur, sans connexion à Internet, et elle connaît 31 langues et variantes régionales.

## Pendant la frappe

- Une ligne ondulée rouge signifie que le dictionnaire ne contient pas le mot. Le mot peut tout de même être correct. Les noms propres et les mots récents manquent souvent. Les noms des applications JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail et JeffBase) ainsi que « LLC » ne sont jamais signalés, dans aucune langue, avec ou sans « s ».
- Le mot que vous êtes en train de taper n'est pas signalé tant que vous n'avez pas quitté ce mot, afin qu'un mot à moitié tapé ne clignote pas.
- Les lignes ne s'affichent qu'à l'écran. Elles ne sont jamais imprimées et n'apparaissent jamais dans un PDF.

Pour corriger un mot, faites un **clic droit** dessus. Le menu affiche les choix suivants, au-dessus des choix habituels :

- Jusqu'à six **suggestions**, la meilleure en premier, en **gras**. Cliquez sur l'une d'elles pour la mettre à la place du mot. Il s'agit d'une seule étape : **Ctrl+Z** l'annule.
- **Ignorer tout** laisse ce mot tranquille, partout où il apparaît, jusqu'à la fermeture de JeffWrite.
- **Ajouter au dictionnaire** apprend définitivement le mot à JeffWrite. Le mot est enregistré dans un petit fichier, dans votre dossier JeffOffice, et toutes les applications JeffOffice l'utilisent. Pour voir ou modifier votre liste de mots, reportez-vous à la section Vos propres mots, plus bas.

Si le dictionnaire ne propose aucune suggestion, le menu l'indique.

## Parcourir les fautes d'orthographe

Appuyez sur **F7**, ou cliquez sur **Révision > Orthographe**. Le volet Orthographe s'ouvre à droite. Il affiche le premier mot mal orthographié après le point d'insertion, sélectionné dans votre texte, avec la ligne où il se trouve et une liste de suggestions.

- **Modifier** remplace le mot par la suggestion choisie. Double-cliquez sur une suggestion pour faire de même.
- **Tout modifier** fait la même chose pour chaque occurrence du mot dans les emplacements parcourus par le volet, énumérés ci-dessous. Il s'agit d'une seule étape : **Ctrl+Z** annule toutes les modifications, où qu'elles se trouvent. Une fois l'opération terminée, le volet indique combien de mots ont été modifiés et à combien d'endroits. Le texte, chaque zone de texte, chaque note et chaque en-tête ou pied de page comptent chacun pour un endroit.
- **Ignorer** passe ce mot. Sa ligne rouge reste en place.
- **Ignorer tout** et **Ajouter au dictionnaire** fonctionnent comme dans le menu du clic droit.

Le volet commence à l'endroit où se trouve le point d'insertion : dans le texte, ou dans la zone de texte, la note de bas de page, l'en-tête ou le pied de page que vous modifiez. Il parcourt d'abord le texte du document, tableaux compris, puis les zones de texte, puis les notes de bas de page, puis les en-têtes et pieds de page. Lorsqu'un mot se trouve dans une zone de texte, une note de bas de page, un en-tête ou un pied de page, JeffWrite l'ouvre pour modification, comme le fait Rechercher, afin que vous voyiez le mot à sa place. Un en-tête ou un pied de page qu'aucune page n'affiche est ignoré, et **Tout modifier** le laisse également intact. Depuis le point d'insertion, le volet va jusqu'à la fin de cet endroit, puis continue dans les autres selon cet ordre, puis reprend depuis le début, jusqu'à l'endroit où il a commencé. **Modifier** est une seule étape annulable, et **Tout modifier** parcourt chaque endroit visité par le volet, également en une seule étape. Un mot figurant dans une note de fin peut être ignoré ou ajouté, mais pas modifié. Lorsqu'il ne reste plus aucun mot, le volet indique que la vérification orthographique est terminée. Une fois l'orthographe vérifiée, il passe aux problèmes de grammaire et de style (voir Vérifier la grammaire, plus bas).

## Compter les erreurs dans la barre d'état

La barre d'état indique le nombre d'erreurs d'orthographe et de grammaire du document, par exemple **3 erreurs de vérification**. Cliquez dessus pour sélectionner l'erreur suivante après le point d'insertion. Lorsque vous atteignez la dernière, un clic vous ramène à la première.

JeffWrite compte les erreurs pendant que vous travaillez, peu à peu, et ne vérifie de nouveau que les paragraphes que vous modifiez. Dans un long document, le compte affiche un signe plus, par exemple **120+ erreurs de vérification**, tant que chaque paragraphe n'a pas été contrôlé. Un mot que la vérification orthographique et la vérification grammaticale signalent toutes deux est compté deux fois. Les erreurs de grammaire sont comptées pour les paragraphes déjà examinés par la vérification grammaticale, de sorte que ce nombre peut augmenter au fil du défilement. Le compte est masqué lorsque la vérification de l'orthographe et celle de la grammaire sont toutes deux désactivées.

## Vos propres mots

Cliquez sur **Fichier > Options > Vérification > Dictionnaire personnalisé** pour voir et modifier vos mots. La boîte de dialogue comporte deux listes, chacune avec une zone **Rechercher** pour la réduire :

- **Mots ajoutés** sont les mots que vous avez appris à JeffWrite. Ils sont acceptés dans toutes les langues.
- **Mots exclus** sont les mots que vous voulez voir signalés comme mal orthographiés, bien qu'un dictionnaire les accepte, comme un mot que vous tapez toujours de travers. Ils sont signalés partout où ils apparaissent, quelle que soit la casse, et ne sont jamais proposés comme suggestion. Un mot exclu désigne ce mot exact, et non ses autres formes : exclure « maison » ne signale pas « maisons ». Ajouter un mot à votre dictionnaire, depuis le volet, le menu du clic droit ou la boîte de dialogue, le retire de la liste des mots exclus. **Ignorer tout** fonctionne toujours sur un mot exclu jusqu'à la fermeture du programme.

Saisissez un mot puis cliquez sur **Ajouter**, ou sélectionnez des mots puis cliquez sur **Supprimer**. Un mot ne peut figurer que dans une liste à la fois : l'ajouter à l'une le retire de l'autre. **Importer** reprend les mots d'un fichier texte brut comportant un mot par ligne, et **Exporter** écrit la liste dans le même type de fichier (UTF-8). Une ligne contenant une espace ou un caractère de contrôle, ou comptant plus de 100 lettres, est omise, et la boîte de dialogue vous indique combien de lignes ont été omises. Une liste peut atteindre 1 Mo de texte. Rien ne change tant que vous ne cliquez pas sur **OK**. Les listes vous appartiennent, sont identiques dans toutes les applications JeffOffice et ne sont jamais enregistrées dans un document.

## Lorsque la correction automatique modifie un mot

Lorsque la correction automatique modifie un mot pendant la frappe, un petit bouton **Options de correction automatique** apparaît sous le point d'insertion. Il propose trois choix. Un lecteur d'écran annonce la présence de choix lorsque le bouton apparaît, et la description du bouton indique comment les ouvrir. Vous pouvez aussi appuyer sur **Alt+Maj+F10** pour les ouvrir. Il s'agit de la commande **Menu des options de correction automatique**, qui se trouve sur la page Clavier des options avec les autres commandes, et vous pouvez lui attribuer une autre touche à cet endroit.

- **Annuler la correction automatique** remet le mot tel que vous l'aviez tapé. Il s'agit d'une seule étape, de sorte que **Rétablir** refait la modification.
- Un choix qui empêche la modification. Il dépend de ce qui a changé le mot : **Arrêter le remplacement** retire l'entrée de votre liste de remplacement, **Conserver tel que saisi** ajoute à la liste des mots conservant leurs majuscules un mot comportant deux majuscules (« THursday »), et **Arrêter la mise en majuscule** désactive partout la majuscule pour les noms de jours, la première lettre des phrases ou la première lettre des cellules de tableau, et non seulement pour ce mot, et l'indique. Il s'agit du même réglage que dans la boîte de dialogue Correction automatique. Le mot est également remis tel que vous l'avez tapé.
- **Options de correction automatique...** ouvre la boîte de dialogue Correction automatique.

Le bouton disparaît lorsque vous tapez, modifiez le texte ou déplacez le point d'insertion.

## Activer ou désactiver les lignes rouges

Cliquez sur **Révision > Vérifier l'orthographe pendant la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**, et c'est le seul choix que partagent toutes les applications JeffOffice. Vous pouvez aussi choisir d'ignorer les mots en MAJUSCULES et les mots contenant des chiffres, comme 3e. Les deux sont ignorés par défaut.

## Vérifier la grammaire pendant la frappe

JeffWrite recherche aussi les fautes de grammaire et de style évidentes dans le texte. Un problème est souligné d'une ligne pointillée bleue. L'orthographe conserve sa ligne ondulée rouge, et un mot mal orthographié n'est jamais signalé comme problème de grammaire. Voici les fautes recherchées :

- un mot répété (« le le »)
- « a » et « an » devant le mauvais son (« an test », « a hour » ; en anglais)
- une phrase qui commence par une minuscule après un point
- « its » et « it's », « their » et « there », « your » et « you're », « then » et « than », « lose » et « loose », lorsque les mots qui les entourent rendent la faute évidente
- « could of » pour « could have »
- deux espaces de suite à l'intérieur d'une phrase
- une espace avant une virgule, un point ou un autre signe qui se rattache au mot précédent
- aucune espace après la fin d'une phrase
- « alot », et « everyday » pour « every day » (« almost everyday »)
- des verbes qui ne s'accordent pas avec leur sujet dans les cas évidents (« she don't », « they was », « it have », « everyone are », « there is many reasons »)
- la mauvaise forme d'un verbe (« did went », « has went », « can't hardly », « don't have no »)
- des mots confondus (« who's car », « an affect on », « less people », « irregardless », « can't breath », « weather » pour « whether », « chose » pour « choose », « advise » pour « advice »)
- « between you and I », « Me and him went », « more better » et « these thing »
- une virgule après l'année d'une date (« On June 5, 2020, we left »), une virgule après une ville et un État (« in Austin, Texas, the heat »), et un trait d'union dans un composé placé devant un nom (« a well-known author »)
- un « i » minuscule pour le mot « I », et les jours et les mois en minuscules

JeffWrite préfère laisser passer une faute plutôt que de signaler un texte correct. Une phrase sans ligne bleue n'est donc pas une garantie qu'elle est juste.

Faites un clic droit sur le problème. Le menu indique ce qui ne va pas, puis propose une ou plusieurs suggestions. Cliquez sur une suggestion pour l'insérer dans le texte, et appuyez sur Ctrl+Z pour l'annuler. **Ignorer ce problème** retire la ligne à cet endroit seulement, jusqu'à la fermeture de JeffWrite. Les mêmes mots ailleurs conservent leur ligne, et une modification ailleurs dans le paragraphe ne la fait pas réapparaître. **Ignorer tout** retire la ligne de tous les problèmes ayant ce message sur ces mots.

La vérification se fait sur votre ordinateur, sans connexion à Internet, en arrière-plan : la frappe n'est jamais ralentie, et un paragraphe est contrôlé peu après que vous avez arrêté de le taper. Un paragraphe est contrôlé jusqu'à ses 100 000 premiers caractères. Un document qui demande plus de vérifications que la limite cesse d'être contrôlé, et la barre d'état vous le signale une fois ; la vérification de l'orthographe continue de fonctionner.

**Langues contrôlées.** L'anglais, le français, l'allemand et l'espagnol, chacun avec ses propres règles. Le français contrôle l'espace insécable avant ; : ! ? et à l'intérieur de « », l'apostrophe dans des mots comme « l'ami » et « qu'il », ainsi que les confusions entre « a » et « à ». L'allemand contrôle « das » et « dass », « seid » et « seit », « wider » et « wieder », la virgule devant « dass » et « weil », ainsi que les majuscules des noms communs. L'espagnol contrôle l'absence de ¿ ou de ¡, les accents sur les mots interrogatifs, « a ver » et « haber », ainsi que « del » et « al ». Un paragraphe est contrôlé dans la langue qui lui est attribuée, et le texte sans langue est contrôlé en anglais. Dans un paragraphe qui comporte plusieurs langues, c'est la première langue contrôlée qui est utilisée, et les autres sont ignorées. Le texte qu'un fichier marque « ne pas vérifier » est lui aussi ignoré. La grammaire est vérifiée dans le texte du document, mais pas encore dans les tableaux, les en-têtes, les pieds de page ni les notes.

Pour la désactiver, cliquez sur **Révision > Vérifier la grammaire pendant la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**. C'est un réglage unique pour toutes les applications JeffOffice, identique dans toutes les fenêtres. Lorsqu'il est activé, le volet Orthographe (F7) parcourt les problèmes de grammaire après les fautes d'orthographe.

**Choisir ce que la vérification recherche.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les types de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme d'un verbe, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union superflus ou manquants), **Majuscules**, **Mots confondus** (mots qui se prononcent de la même façon), **Répétition** et **Style**. **Style** est désactivé tant que vous ne l'activez pas. Il formule des suggestions, et ne signale pas des fautes : phrases de plus de 40 mots, voix passive, trois phrases de suite commençant par le même mot, et formules verbeuses (« in order to », « due to the fact that »). Le choix est conservé lorsque vous cliquez sur **OK** dans les options. Il s'applique immédiatement aux lignes bleues et au volet Orthographe (F7) dans les fenêtres ouvertes de cette application, et aux autres applications JeffOffice à leur prochain démarrage.

## Langues

JeffWrite vérifie chaque passage de texte dans sa propre langue. Un fichier peut indiquer la langue de son texte, jusqu'à un seul mot, et JeffWrite en tient compte. Le texte qui n'indique aucune langue est vérifié dans la langue du document. Le fichier donne cette langue pour l'ensemble du document, et un nouveau document utilise l'anglais américain.

Le texte dans une langue pour laquelle JeffWrite ne dispose d'aucun dictionnaire n'est pas vérifié, pas plus que le texte qu'un fichier marque « ne pas vérifier l'orthographe ». Vous ne pouvez pas encore modifier la langue d'un texte dans JeffWrite, mais celle indiquée par le fichier est conservée.

Les 31 dictionnaires sont : l'anglais (américain, britannique, canadien et australien), l'espagnol (21 variantes régionales, dont le Mexique et l'Espagne), le français, l'allemand, l'italien, le néerlandais et le portugais (Brésil et Portugal).

## Ce qui n'est pas encore vérifié

Les lignes rouges apparaissent dans le texte, dans les cellules des tableaux, ainsi que dans un en-tête, un pied de page ou une note de bas de page pendant que vous les modifiez. Elles n'apparaissent pas encore dans les zones de texte, mais le volet Orthographe les parcourt.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
