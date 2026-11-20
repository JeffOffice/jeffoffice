# Vérifier l'orthographe
<!-- keywords: orthographe, vérifier l'orthographe, correcteur orthographique, correcteur, faute d'orthographe, mot mal orthographié, trait rouge, trait ondulé, soulignement ondulé, suggestions, ignorer, ignorer tout, ajouter au dictionnaire, dictionnaire personnalisé, dictionnaire, langue, vérification, f7, révision, vérifier l'orthographe pendant la frappe, majuscules, nombres, grammaire, vérifier la grammaire, style, trait bleu, trait pointillé, vérifier la grammaire pendant la frappe -->

JeffWrite vérifie l'orthographe au fur et à mesure que vous tapez. Un mot absent du dictionnaire est souligné d'un trait rouge ondulé. La vérification s'effectue sur votre ordinateur, sans connexion à Internet, et elle connaît 31 langues et variantes régionales.

## Pendant la frappe

- Un trait rouge ondulé signifie que le dictionnaire ne contient pas le mot. Il peut tout de même être correct. Les noms propres et les mots nouveaux sont souvent absents. Les noms des applications JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail et JeffBase) ainsi que LLC ne sont jamais signalés, dans aucune langue, avec ou sans « s ».
- Le mot en cours de saisie n'est pas signalé tant que vous n'avez pas quitté ce mot, afin qu'un mot en cours de saisie ne clignote pas.
- Les traits s'affichent uniquement à l'écran. Ils ne s'impriment jamais et n'apparaissent jamais dans un PDF.

Pour corriger un mot, faites un **clic droit** dessus. Le menu affiche les choix suivants, au-dessus des commandes habituelles :

- Jusqu'à six **suggestions**, la meilleure en premier, en gras. Cliquez sur l'une d'elles pour la substituer au mot. Il s'agit d'une seule étape, donc **Ctrl+Z** l'annule.
- **Ignorer tout** laisse ce mot tel quel, à chaque endroit où il apparaît, jusqu'à la fermeture de JeffWrite.
- **Ajouter au dictionnaire** enregistre définitivement le mot dans le dictionnaire. Il est placé dans un petit fichier, dans votre dossier JeffOffice, et toutes les applications JeffOffice l'utilisent. Pour afficher ou modifier votre liste de mots, consultez la section Vos propres mots, plus bas.

Si le dictionnaire ne propose aucune suggestion, le menu l'indique.

## Parcourir les fautes d'orthographe

Appuyez sur **F7**, ou cliquez sur **Révision > Orthographe**. Le volet Orthographe s'ouvre à droite. Il affiche le premier mot mal orthographié situé après le point d'insertion, sélectionné dans votre texte, avec la ligne où il se trouve et une liste de suggestions.

- **Modifier** remplace le mot par la suggestion choisie. Double-cliquez sur une suggestion pour obtenir le même effet.
- **Tout modifier** effectue ce remplacement pour chaque occurrence du mot dans les emplacements parcourus par le volet, énumérés ci-dessous. Il s'agit d'une seule étape, donc **Ctrl+Z** les annule toutes, où qu'elles se trouvent. Une fois l'opération terminée, le volet indique le nombre de mots modifiés et le nombre d'emplacements concernés. Le texte, chaque zone de texte, chaque note et chaque en-tête ou pied de page comptent chacun pour un emplacement.
- **Ignorer** passe ce mot. Son trait rouge reste affiché.
- **Ignorer tout** et **Ajouter au dictionnaire** fonctionnent comme dans le menu du clic droit.

Le volet commence à l'emplacement du point d'insertion : dans le texte, ou dans la zone de texte, la note de bas de page, l'en-tête ou le pied de page que vous modifiez. Il parcourt d'abord le texte du document, y compris les tableaux, puis les zones de texte, puis les notes de bas de page, puis les en-têtes et pieds de page. Lorsqu'un mot se trouve dans une zone de texte, une note de bas de page, un en-tête ou un pied de page, JeffWrite l'ouvre pour modification, comme le fait Rechercher, afin que vous voyiez le mot à sa place. Un en-tête ou un pied de page qu'aucune page n'affiche est ignoré, et **Tout modifier** le laisse également tel quel. À partir du point d'insertion, le volet va jusqu'à la fin de cet emplacement, puis continue dans les autres dans l'ordre indiqué, puis reprend depuis le début jusqu'à l'emplacement où il a commencé. **Modifier** est une seule étape annulable, et **Tout modifier** parcourt chaque emplacement visé par le volet et constitue également une seule étape. Un mot figurant dans une note de fin peut être ignoré ou ajouté, mais pas modifié. Lorsqu'il ne reste plus de fautes, le volet indique que la vérification orthographique est terminée. Une fois l'orthographe vérifiée, il passe aux problèmes de grammaire et de style (voir « Vérifier la grammaire », plus bas).

## Vos propres mots

Cliquez sur **Fichier > Options > Vérification > Dictionnaire personnalisé** pour afficher et modifier vos mots. La boîte de dialogue comporte deux listes, chacune avec une zone **Rechercher** pour les filtrer :

- **Mots ajoutés** sont les mots que vous avez enseignés à JeffWrite. Ils sont acceptés dans toutes les langues.
- **Mots exclus** sont les mots que vous voulez voir signalés comme mal orthographiés, bien qu'un dictionnaire les accepte, par exemple un mot que vous tapez toujours de travers. Ils sont signalés à chaque endroit où ils apparaissent, quelles que soient les majuscules, et ne sont jamais proposés comme suggestion. Un mot exclu désigne ce mot exact, et non ses autres formes : exclure « maison » ne signale pas « maisons ». Ajouter un mot à votre dictionnaire, depuis le volet, le menu du clic droit ou la boîte de dialogue, le retire de la liste des mots exclus. **Ignorer tout** fonctionne toujours sur un mot exclu jusqu'à la fermeture du programme.

Saisissez un mot et cliquez sur **Ajouter**, ou sélectionnez des mots et cliquez sur **Supprimer**. Un mot ne peut figurer que dans une liste à la fois : l'ajouter à l'une le retire de l'autre. **Importer** reprend les mots d'un fichier texte brut, à raison d'un mot par ligne, et **Exporter** enregistre la liste dans le même type de fichier (UTF-8). Une ligne contenant un espace ou un caractère de commande, ou comptant plus de 100 lettres, est omise, et la boîte de dialogue indique combien de lignes ont été omises. Une liste peut atteindre 1 Mo de texte. Rien ne change tant que vous n'avez pas cliqué sur **OK**. Les listes vous appartiennent, sont identiques dans toutes les applications JeffOffice et ne sont jamais enregistrées dans un document.

## Lorsque la correction automatique modifie un mot

Lorsque la correction automatique modifie un mot pendant la frappe, un petit bouton **Options de correction automatique** apparaît sous le point d'insertion. Il propose trois choix. Un lecteur d'écran annonce la présence de ces choix lorsque le bouton apparaît, et la description du bouton indique comment les ouvrir. Vous pouvez aussi appuyer sur **Alt+Maj+F10** pour les ouvrir. Il s'agit de la commande **Menu des options de correction automatique** ; elle figure sur la page Clavier des options, avec les autres commandes, et vous pouvez lui attribuer une autre touche à cet endroit.

- **Annuler la correction automatique** rétablit le mot tel que vous l'avez saisi. Il s'agit d'une seule étape, donc **Rétablir** applique de nouveau la modification.
- Un choix qui empêche la modification. Il dépend de ce qui a modifié le mot : **Arrêter le remplacement** retire l'entrée de votre liste de remplacement, **Conserver tel quel** ajoute à la liste des mots conservés un mot comportant deux majuscules (« THursday »), et **Arrêter la mise en majuscule** désactive partout la majuscule des noms de jours, de la première lettre des phrases ou de la première lettre des cellules de tableau, et non seulement pour ce mot, et le signale (il s'agit du même paramètre que dans la boîte de dialogue Correction automatique). Le mot est aussi rétabli tel que vous l'avez saisi.
- **Options de correction automatique...** ouvre la boîte de dialogue Correction automatique.

Le bouton disparaît lorsque vous tapez, modifiez le texte ou déplacez le point d'insertion.

## Activer ou désactiver les traits rouges

Cliquez sur **Révision > Vérifier l'orthographe pendant la frappe**. Ce même choix se trouve dans **Fichier > Options > Vérification**, et il s'agit du seul choix commun à toutes les applications JeffOffice. Vous pouvez aussi choisir d'ignorer les mots en MAJUSCULES et les mots contenant des chiffres, comme 3e. Ces deux catégories sont ignorées par défaut.

## Vérifier la grammaire pendant la frappe

JeffWrite recherche également les fautes de grammaire et de style évidentes dans le texte. Un problème est souligné d'un trait bleu pointillé. L'orthographe conserve son trait rouge ondulé, et un mot mal orthographié n'est jamais signalé comme problème de grammaire. Voici les fautes recherchées :

- un mot répété (« le le »)
- « a » et « an » devant le mauvais son (« an test », « a hour »)
- une phrase qui commence par une minuscule après un point
- « its » et « it's », « their » et « there », « your » et « you're », « then » et « than », « lose » et « loose », lorsque les mots qui les entourent rendent la faute évidente
- « could of » pour « could have »
- deux espaces de suite à l'intérieur d'une phrase
- une espace avant une virgule, un point ou un autre signe qui se rattache au mot précédent
- aucune espace après la fin d'une phrase
- « alot », et « everyday » pour « every day » (« almost everyday »)
- des verbes qui ne s'accordent pas avec leur sujet dans les cas évidents (« she don't », « they was », « it have », « everyone are », « there is many reasons »)
- une mauvaise forme de verbe (« did went », « has went », « can't hardly », « don't have no »)
- des mots confondus (« who's car », « an affect on », « less people », « irregardless », « can't breath », « weather » pour « whether », « chose » pour « choose », « advise » pour « advice »)
- « between you and I », « Me and him went », « more better » et « these thing »
- une virgule après l'année d'une date (« On June 5, 2020, we left »), une virgule après une ville et un État (« in Austin, Texas, the heat »), et un trait d'union dans un composé placé devant un nom (« a well-known author »)
- un « i » minuscule pour le mot « I », et les jours et les mois en minuscules

JeffWrite préfère manquer une faute plutôt que de signaler un texte correct. Une phrase sans trait bleu n'est donc pas garantie comme correcte.

Faites un clic droit sur le problème. Le menu indique la nature de la faute, puis propose une ou plusieurs suggestions. Cliquez sur une suggestion pour l'insérer dans le texte, et appuyez sur Ctrl+Z pour l'annuler. **Ignorer ce problème** retire le trait à cet endroit précis, jusqu'à la fermeture de JeffWrite. Les mêmes mots ailleurs conservent leur trait, et la modification d'autres parties du paragraphe ne le fait pas réapparaître. **Ignorer tout** retire le trait de tous les problèmes qui présentent ce message sur ces mots.

La vérification s'effectue sur votre ordinateur, sans connexion à Internet, en arrière-plan : la frappe n'est jamais ralentie, et un paragraphe est vérifié peu après que vous avez cessé de le modifier. Un paragraphe est vérifié jusqu'à ses 100 000 premiers caractères. Un document dont la vérification dépasse cette limite cesse d'être vérifié, et la barre d'état le signale une seule fois ; la vérification de l'orthographe continue de fonctionner.

**Langues vérifiées.** L'anglais, le français, l'allemand et l'espagnol, chacun avec ses propres règles. Pour le français, la vérification porte sur l'espace insécable avant ; : ! ? et à l'intérieur des « », l'apostrophe dans des mots comme « l'ami » et « qu'il », ainsi que les confusions comme « a » et « à ». Pour l'allemand, elle porte sur « das » et « dass », « seid » et « seit », « wider » et « wieder », la virgule devant « dass » et « weil », et les majuscules sur les noms communs. Pour l'espagnol, elle recherche un « ¿ » ou un « ¡ » manquant, les accents sur les mots interrogatifs, « a ver » et « haber », et « del » et « al ». Un paragraphe est vérifié dans la langue qui lui est attribuée, et un texte sans langue est vérifié en anglais. Dans un paragraphe comportant plusieurs langues, la première langue vérifiable est utilisée et les autres sont ignorées. Un texte qu'un fichier marque comme « ne pas vérifier » est également ignoré. La grammaire est vérifiée dans le texte du document, mais pas encore dans les tableaux, les en-têtes, les pieds de page ou les notes.

Pour la désactiver, cliquez sur **Révision > Vérifier la grammaire pendant la frappe**. Ce choix se trouve aussi dans **Fichier > Options > Vérification**. Il s'agit d'un choix unique pour toutes les applications JeffOffice, identique dans toutes les fenêtres. Lorsqu'il est activé, le volet Orthographe (F7) parcourt les problèmes de grammaire après les fautes d'orthographe.

**Choisir les problèmes recherchés.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les catégories de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme de verbe, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union superflus ou manquants), **Majuscules**, **Mots confondus** (mots qui se prononcent de la même manière), **Répétition** et **Style**. **Style** est désactivé tant que vous ne l'activez pas. Il formule des suggestions et ne signale pas de fautes : phrases de plus de 40 mots, voix passive, trois phrases de suite commençant par le même mot, et formulations lourdes (« in order to », « due to the fact that »). Le choix est conservé lorsque vous cliquez sur **OK** dans les options. Il s'applique immédiatement aux traits bleus et au volet Orthographe (F7) dans les fenêtres ouvertes de cette application, et aux autres applications JeffOffice à leur prochain démarrage.

## Langues

JeffWrite vérifie chaque partie du texte dans sa propre langue. Un fichier peut indiquer la langue de son texte, jusqu'à un seul mot, et JeffWrite en tient compte. Un texte qui n'indique rien est vérifié dans la langue du document. Le fichier donne cette langue pour l'ensemble du document, et un nouveau document utilise l'anglais américain.

Un texte dans une langue pour laquelle JeffWrite ne dispose d'aucun dictionnaire n'est pas vérifié. Un texte qu'un fichier marque comme « ne pas vérifier l'orthographe » ne l'est pas non plus. Vous ne pouvez pas encore modifier la langue d'un texte dans JeffWrite, mais la langue indiquée par le fichier est conservée.

Les 31 dictionnaires sont ceux de l'anglais (américain, britannique, canadien et australien), de l'espagnol (21 régions, dont le Mexique et l'Espagne), du français, de l'allemand, de l'italien, du néerlandais et du portugais (Brésil et Portugal).

## Ce qui n'est pas encore vérifié

Les traits rouges apparaissent dans le texte, dans les cellules des tableaux, ainsi que dans un en-tête, un pied de page ou une note de bas de page pendant que vous les modifiez. Ils n'apparaissent pas encore dans les zones de texte, mais le volet Orthographe les parcourt.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
