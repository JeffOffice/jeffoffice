# Vérifier l'orthographe
<!-- keywords: orthographe, vérification orthographique, correcteur orthographique, mot mal orthographié, trait rouge, ligne ondulée, soulignement ondulé, suggestions, ignorer, ignorer tout, ajouter au dictionnaire, dictionnaire personnalisé, dictionnaire, langue, vérification, F7, révision, vérifier l'orthographe pendant la frappe, majuscules, chiffres, grammaire, vérification grammaticale, style, trait bleu, trait pointillé, vérifier la grammaire pendant la frappe, correction, faute, fautes -->

JeffWrite vérifie votre orthographe au fur et à mesure que vous tapez. Un mot absent du dictionnaire est souligné d'un trait rouge ondulé. La vérification s'effectue sur votre ordinateur, sans connexion à Internet, et couvre 31 langues et variantes régionales.

## Pendant la frappe

- Un trait rouge ondulé signifie que le mot n'est pas dans le dictionnaire. Il peut tout de même être correct. Les noms propres et les mots récents manquent souvent au dictionnaire. Les noms des applications JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail et JeffBase) ainsi que LLC ne sont jamais signalés, dans aucune langue, avec ou sans « s ».
- Le mot que vous êtes en train de taper n'est pas signalé tant que vous n'avez pas quitté ce mot, afin qu'un mot à moitié saisi ne clignote pas.
- Les traits n'apparaissent qu'à l'écran. Ils ne sont jamais imprimés ni inclus dans un PDF.

Pour corriger un mot, faites un **clic droit** dessus. Le menu affiche ces choix en plus des choix habituels :

- Jusqu'à six **suggestions**, la meilleure en premier, en **gras**. Cliquez sur l'une d'elles pour la mettre à la place du mot. Il s'agit d'une seule étape : **Ctrl+Z** l'annule.
- **Ignorer tout** laisse ce mot tel quel, partout où il apparaît, jusqu'à la fermeture de JeffWrite.
- **Ajouter au dictionnaire** enregistre le mot de façon définitive. Il est placé dans un petit fichier de votre dossier JeffOffice, et toutes les applications JeffOffice l'utilisent. Pour consulter ou modifier votre liste de mots, reportez-vous à la section *Vos propres mots*, plus bas.

Si le dictionnaire ne propose aucune suggestion, le menu l'indique.

## Parcourir les fautes d'orthographe

Appuyez sur **F7**, ou cliquez sur **Révision > Orthographe**. Le volet Orthographe s'ouvre à droite. Il affiche le premier mot mal orthographié après le curseur, sélectionné dans votre texte, avec la ligne où il se trouve et une liste de suggestions.

- **Remplacer** met la suggestion choisie à la place du mot. Double-cliquez sur une suggestion pour obtenir le même résultat.
- **Remplacer tout** fait de même pour chaque occurrence du mot dans les emplacements parcourus par le volet, énumérés ci-dessous. Il s'agit d'une seule étape : **Ctrl+Z** annule toutes les modifications, où qu'elles se trouvent. Une fois l'opération terminée, le volet indique le nombre de mots modifiés et le nombre d'emplacements concernés. Le texte, chaque zone de texte, chaque note et chaque en-tête ou pied de page comptent chacun pour un emplacement.
- **Ignorer** passe ce mot. Son trait rouge reste affiché.
- **Ignorer tout** et **Ajouter au dictionnaire** fonctionnent comme dans le menu du clic droit.

Le volet commence à la position du curseur : dans le texte, ou dans la zone de texte, la note de bas de page, l'en-tête ou le pied de page que vous modifiez. Il parcourt d'abord le texte du document, tableaux compris, puis les zones de texte, puis les notes de bas de page, puis les en-têtes et pieds de page. Lorsqu'un mot se trouve dans une zone de texte, une note de bas de page, un en-tête ou un pied de page, JeffWrite ouvre cet élément en modification, comme le fait Rechercher, afin que vous voyiez le mot à sa place. Un en-tête ou un pied de page qu'aucune page n'affiche est ignoré, et **Remplacer tout** le laisse également intact. Depuis le curseur, le volet va jusqu'à la fin de cet emplacement, puis passe aux autres dans cet ordre, puis reprend depuis le début jusqu'à l'emplacement de départ. **Remplacer** est une étape annulable, et **Remplacer tout** porte sur chaque emplacement parcouru par le volet et constitue également une seule étape. Un mot figurant dans une note de fin peut être ignoré ou ajouté au dictionnaire, mais pas modifié. Lorsqu'il ne reste plus aucune faute, le volet indique que la vérification orthographique est terminée. Une fois l'orthographe vérifiée, il passe aux problèmes de grammaire et de style (voir *Vérifier la grammaire*, plus bas).

## Compter les erreurs dans la barre d'état

La barre d'état indique le nombre d'erreurs d'orthographe et de grammaire du document, par exemple **3 erreurs de vérification**. Cliquez dessus pour sélectionner l'erreur suivante après le curseur. Lorsque vous atteignez la dernière, un clic vous ramène à la première.

JeffWrite compte les erreurs au fil de votre travail, par petites portions, et ne revérifie que les paragraphes que vous modifiez. Dans un long document, le compteur affiche un signe plus, par exemple **120+ erreurs de vérification**, tant que chaque paragraphe n'a pas été vérifié. Un mot signalé à la fois par la vérification orthographique et par la vérification grammaticale est compté deux fois. Les erreurs de grammaire sont comptées pour les paragraphes déjà examinés, de sorte que ce nombre peut augmenter au fil du défilement. Le compteur est masqué lorsque les vérifications d'orthographe et de grammaire sont toutes deux désactivées.

## Vos propres mots

Cliquez sur **Fichier > Options > Vérification > Dictionnaire personnalisé** pour consulter et modifier vos mots. La boîte de dialogue comporte deux listes, chacune avec une zone **Rechercher** pour les filtrer :

- Les **Mots ajoutés** sont les mots que vous avez enseignés à JeffWrite. Ils sont acceptés dans toutes les langues.
- Les **Mots exclus** sont des mots que vous souhaitez voir signalés comme fautifs, même si un dictionnaire les accepte, par exemple un mot que vous tapez toujours mal. Ils sont signalés partout où ils apparaissent, quelle que soit la casse, et ne sont jamais proposés comme suggestion. Un mot exclu désigne ce mot exact, et non ses autres formes : exclure « maison » ne signale pas « maisons ». Ajouter un mot à votre dictionnaire, depuis le volet, le menu du clic droit ou la boîte de dialogue, le retire de la liste des mots exclus. **Ignorer tout** fonctionne toujours sur un mot exclu jusqu'à la fermeture du programme.

Saisissez un mot puis cliquez sur **Ajouter**, ou sélectionnez des mots puis cliquez sur **Supprimer**. Un mot ne peut figurer que dans une seule liste à la fois : l'ajouter à l'une le retire de l'autre. **Importer** prend les mots d'un fichier texte brut comportant un mot par ligne, et **Exporter** écrit la liste dans le même type de fichier (UTF-8). Une ligne contenant un espace ou un caractère de contrôle, ou comptant plus de 100 lettres, est omise, et la boîte de dialogue indique combien de lignes ont été ignorées. Une liste peut atteindre 1 Mo de texte. Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ces listes vous appartiennent, sont identiques dans toutes les applications JeffOffice et ne sont jamais enregistrées dans un document.

## Quand la correction automatique modifie un mot

Lorsque la correction automatique modifie un mot pendant la frappe, un petit bouton **Options de correction automatique** apparaît sous le curseur. Il propose trois choix. Un lecteur d'écran annonce la présence de choix lorsque le bouton apparaît, et la description du bouton indique comment les ouvrir. Vous pouvez aussi appuyer sur **Alt+Maj+F10** pour les ouvrir. Il s'agit de la commande **Menu des options de correction automatique**, qui se trouve dans la page Clavier des options avec les autres commandes, et vous pouvez lui attribuer une autre touche à cet endroit.

- **Annuler la correction automatique** rétablit le mot tel que vous l'avez tapé. Il s'agit d'une seule étape, donc **Rétablir** refait la modification.
- Un choix qui arrête la modification. Il dépend de ce qui a modifié le mot : **Arrêter le remplacement** retire l'entrée de votre liste de remplacement, **Conserver tel quel** ajoute un mot comportant deux majuscules (« THursday ») à la liste des mots qui les conservent, et **Arrêter la mise en majuscule** désactive la majuscule pour les noms de jours, la première lettre des phrases ou la première lettre des cellules de tableau, partout et non pour ce seul mot, et l'indique. Il s'agit de la même option que dans la boîte de dialogue Correction automatique. Il rétablit aussi le mot tel que vous l'avez tapé.
- **Options de correction automatique...** ouvre la boîte de dialogue Correction automatique.

L'onglet **Symboles mathématiques** de la boîte de dialogue Correction automatique contient une option, **Remplacer les noms de symboles mathématiques pendant la frappe**, désactivée par défaut. Lorsqu'elle est activée, une barre oblique inverse suivie d'un nom, comme \alpha, devient le symbole (α) dès que vous tapez une espace ou un signe après lui. Le nom doit commencer un mot, de sorte qu'un chemin de dossier tel que C:\temp\pi reste tel que vous l'avez tapé. L'onglet énumère tous les noms. Lorsqu'un nom a été remplacé, le choix du bouton est **Arrêter le remplacement des noms mathématiques**, qui désactive l'option. JeffDeck et JeffPub les modifient également ; JeffCrunch ne modifie rien dans une cellule.

Le bouton disparaît dès que vous tapez, modifiez le texte ou déplacez le curseur.

## Activer ou désactiver les traits rouges

Cliquez sur **Révision > Vérifier l'orthographe pendant la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**, et c'est le seul choix que toutes les applications JeffOffice partagent. Vous pouvez aussi choisir d'ignorer les mots en MAJUSCULES et les mots contenant des chiffres, comme 3e. Ces deux options sont activées par défaut.

## Vérifier la grammaire pendant la frappe

JeffWrite repère aussi les erreurs de grammaire et de style évidentes dans le texte. Un problème est souligné d'un trait bleu pointillé. L'orthographe conserve son trait rouge ondulé, et un mot mal orthographié n'est jamais signalé comme problème de grammaire. Voici les erreurs recherchées :

- un mot répété (« the the »)
- « a » et « an » devant un son inapproprié (« an test », « a hour »)
- une phrase commençant par une minuscule après un point
- « its » et « it's », « their » et « there », « your » et « you're », « then » et « than », « lose » et « loose », lorsque les mots qui les entourent rendent la faute évidente
- « could of » pour « could have »
- deux espaces consécutives à l'intérieur d'une phrase
- une espace avant une virgule, un point ou un autre signe qui se rattache au mot précédent
- aucune espace après la fin d'une phrase
- « alot », et « everyday » pour « every day » (« almost everyday »)
- des verbes qui ne s'accordent pas avec leur sujet dans les cas évidents (« she don't », « they was », « it have », « everyone are », « there is many reasons »)
- une mauvaise forme de verbe (« did went », « has went », « can't hardly », « don't have no »)
- des mots confondus (« who's car », « an affect on », « less people », « irregardless », « can't breath », « weather » pour « whether », « chose » pour « choose », « advise » pour « advice »)
- « between you and I », « Me and him went », « more better » et « these thing »
- une virgule après l'année d'une date (« On June 5, 2020, we left »), une virgule après une ville et un État (« in Austin, Texas, the heat »), et un trait d'union dans un composé placé devant un nom (« a well-known author »)
- un « i » minuscule pour le mot « I », et les jours et les mois écrits en minuscules

JeffWrite préfère manquer une faute plutôt que de signaler un texte correct. Une phrase sans trait bleu n'est donc pas une garantie qu'elle est exacte.

Faites un clic droit sur le problème. Le menu indique ce qui ne va pas, puis propose une ou plusieurs suggestions. Cliquez sur une suggestion pour la mettre dans le texte, et appuyez sur Ctrl+Z pour l'annuler. **Ignorer ce problème** retire le trait pour cet endroit seulement, jusqu'à la fermeture de JeffWrite. Les mêmes mots ailleurs conservent leur trait, et la modification d'autres parties du paragraphe ne le fait pas réapparaître. **Ignorer tout** retire le trait de tous les problèmes ayant ce message sur ces mots.

La vérification s'effectue sur votre ordinateur, sans connexion à Internet, en arrière-plan : la frappe n'est jamais ralentie, et un paragraphe est vérifié peu après que vous avez cessé de le modifier. Un paragraphe est vérifié jusqu'à ses 100 000 premiers caractères. Un document dont la vérification dépasse cette limite cesse d'être vérifié, et la barre d'état l'indique une seule fois ; la vérification orthographique continue de fonctionner.

**Langues vérifiées.** La grammaire n'est vérifiée qu'en anglais, dans toutes ses variantes régionales (américain, britannique, canadien, australien et autres). Un texte dans une autre langue n'est vérifié que pour l'orthographe et ne reçoit aucune marque de grammaire. Un paragraphe est vérifié dans la langue qui lui est attribuée, et un texte sans langue est vérifié en anglais. Dans un paragraphe comportant plusieurs langues, seul l'anglais est vérifié et le reste est ignoré. Le texte qu'un fichier marque « ne pas vérifier » est également ignoré, de même que les mots supprimés par une modification suivie, le texte masqué et les résultats des champs (comme les numéros de page). La grammaire est vérifiée dans le texte du document, mais pas encore dans les tableaux, les en-têtes, les pieds de page ni les notes.

Pour la désactiver, cliquez sur **Révision > Vérifier la grammaire pendant la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**. C'est un choix unique pour toutes les applications JeffOffice, identique dans toutes les fenêtres. Le volet Orthographe (F7) parcourt les problèmes de grammaire après l'orthographe, lorsque cette option est activée.

**Choisir les types de problèmes recherchés.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les types de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme de verbe, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union superflus ou manquants), **Majuscules**, **Mots confondus** (mots qui se prononcent de la même façon), **Répétitions** et **Style**. Le choix **Style** est désactivé tant que vous ne l'activez pas. Il formule des suggestions et ne signale pas de fautes : phrases de plus de 40 mots, voix passive, trois phrases consécutives commençant par le même mot, et formules verbeuses (« in order to », « due to the fact that »). Le choix est conservé lorsque vous cliquez sur **OK** dans les options. Il s'applique immédiatement aux traits bleus et au volet Orthographe (F7) dans les fenêtres ouvertes de cette application, et aux autres applications JeffOffice à leur prochain démarrage.

## Langues

JeffWrite vérifie chaque passage de texte dans sa propre langue. Un fichier peut indiquer la langue de son texte, jusqu'à un seul mot, et JeffWrite en tient compte. Un texte qui ne précise rien est vérifié dans la langue du document. Le fichier donne cette langue pour l'ensemble du document, et un nouveau document utilise l'anglais américain.

Un texte dans une langue pour laquelle JeffWrite ne dispose d'aucun dictionnaire n'est pas vérifié. Il en va de même pour un texte que le fichier marque « ne pas vérifier l'orthographe ». Vous ne pouvez pas encore changer la langue d'un texte dans JeffWrite, mais la langue indiquée par le fichier est conservée.

Les 31 dictionnaires sont ceux de l'anglais (américain, britannique, canadien et australien), de l'espagnol (21 régions, dont le Mexique et l'Espagne), du français, de l'allemand, de l'italien, du néerlandais et du portugais (Brésil et Portugal).

## Ce qui n'est pas encore vérifié

Les traits rouges apparaissent dans le texte, dans les cellules des tableaux, et dans un en-tête, un pied de page ou une note de bas de page pendant que vous les modifiez. Ils n'apparaissent pas encore dans les zones de texte, mais le volet Orthographe les parcourt.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
