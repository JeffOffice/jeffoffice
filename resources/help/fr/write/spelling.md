# Vérifier l'orthographe
<!-- keywords: orthographe, vérification orthographique, correcteur orthographique, faute d'orthographe, trait rouge, trait ondulé, ligne ondulée, suggestions, ignorer, ignorer tout, ajouter au dictionnaire, dictionnaire personnalisé, dictionnaire, langue, vérification, F7, révision, vérifier l'orthographe pendant la frappe, majuscules, chiffres, grammaire, vérification grammaticale, style, trait bleu, trait pointillé, vérifier la grammaire pendant la frappe -->

JeffWrite vérifie l'orthographe pendant que vous tapez. Un mot absent du dictionnaire est souligné d'un trait rouge ondulé. La vérification se fait sur votre ordinateur, sans connexion à Internet, et elle connaît 31 langues et variantes régionales.

## Pendant la frappe

- Un trait rouge ondulé signifie que le dictionnaire ne contient pas le mot. Le mot peut tout de même être correct. Les noms propres et les mots récents sont souvent absents. Les noms des applications JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail et JeffBase) et LLC ne sont jamais signalés, dans aucune langue, avec ou sans « s ».
- Le mot que vous êtes en train de taper n'est pas signalé tant que vous n'avez pas quitté ce mot, ce qui évite le clignotement d'un mot à moitié saisi.
- Les traits ne s'affichent qu'à l'écran. Ils ne sont jamais imprimés et ne passent jamais dans un PDF.

Pour corriger un mot, faites un **clic droit** dessus. Le menu présente les choix suivants, au-dessus des choix habituels :

- Jusqu'à six **suggestions**, la meilleure en premier, en gras. Cliquez sur l'une d'elles pour la mettre à la place du mot. Il s'agit d'une seule étape : **Ctrl+Z** l'annule.
- **Ignorer tout** laisse ce mot tranquille partout où il apparaît, jusqu'à la fermeture de JeffWrite.
- **Ajouter au dictionnaire** enregistre le mot de façon définitive. Il est placé dans un petit fichier de votre dossier JeffOffice personnel, et toutes les applications JeffOffice l'utilisent. Pour voir ou modifier votre liste de mots, reportez-vous à la section *Vos propres mots*, plus bas.

Si le dictionnaire ne propose aucune suggestion, le menu l'indique.

## Parcourir les fautes d'orthographe

Appuyez sur **F7**, ou cliquez sur **Révision > Orthographe**. Le volet Orthographe s'ouvre à droite. Il affiche la première faute d'orthographe après le point d'insertion, sélectionnée dans votre texte, avec la ligne où elle se trouve et la liste des suggestions.

- **Modifier** remplace le mot par la suggestion choisie. Double-cliquez sur une suggestion pour obtenir le même résultat.
- **Modifier tout** fait de même pour chaque occurrence du mot dans les emplacements parcourus par le volet, listés ci-dessous. Il s'agit d'une seule étape : **Ctrl+Z** annule toutes les modifications, quel que soit leur emplacement. Une fois l'opération terminée, le volet indique le nombre de mots modifiés et le nombre d'emplacements concernés. Le texte, chaque zone de texte, chaque note et chaque en-tête ou pied de page comptent chacun pour un emplacement.
- **Ignorer** passe ce mot sans le modifier. Son trait rouge reste affiché.
- **Ignorer tout** et **Ajouter au dictionnaire** fonctionnent comme dans le menu du clic droit.

Le volet commence à la position du point d'insertion, dans le texte, ou dans la zone de texte, la note de bas de page, l'en-tête ou le pied de page que vous modifiez. Il parcourt d'abord le texte du document, tableaux compris, puis les zones de texte, puis les notes de bas de page, puis les en-têtes et pieds de page. Lorsqu'un mot se trouve dans une zone de texte, une note de bas de page, un en-tête ou un pied de page, JeffWrite l'ouvre pour modification, comme le fait Rechercher, afin que vous voyiez le mot à son emplacement. Un en-tête ou un pied de page qu'aucune page n'affiche est ignoré, et **Modifier tout** ne le modifie pas non plus. À partir du point d'insertion, le volet va jusqu'à la fin de cet emplacement, puis continue dans les autres dans cet ordre, puis reprend depuis le début jusqu'à l'endroit où il a commencé. **Modifier** est une étape annulable, et **Modifier tout** parcourt chaque emplacement visité par le volet et constitue également une seule étape. Un mot figurant dans une note de fin peut être ignoré ou ajouté au dictionnaire, mais il ne peut pas être modifié. Lorsqu'il ne reste plus aucune faute, le volet indique que la vérification orthographique est terminée. Une fois l'orthographe vérifiée, il passe aux problèmes de grammaire et de style, si la vérification grammaticale est activée (voir *Vérifier la grammaire*, plus bas).

## Compter les erreurs dans la barre d'état

La barre d'état indique le nombre d'erreurs d'orthographe et de grammaire du document, par exemple **3 erreurs de vérification**. Cliquez dessus pour sélectionner l'erreur suivante après le point d'insertion. Lorsque vous atteignez la dernière, un clic revient à la première.

JeffWrite effectue le comptage pendant que vous travaillez, progressivement, et ne revérifie que les paragraphes que vous modifiez. Dans un document long, le compteur affiche un signe plus, par exemple **120+ erreurs de vérification**, tant que chaque paragraphe n'a pas été contrôlé. Un mot signalé à la fois par la vérification orthographique et par la vérification grammaticale est compté deux fois. Les erreurs de grammaire sont comptées pour les paragraphes déjà examinés par la vérification grammaticale, si bien que ce nombre peut augmenter à mesure que vous faites défiler le document. Le compteur est masqué lorsque les vérifications orthographique et grammaticale sont toutes deux désactivées.

## Vos propres mots

Cliquez sur **Fichier > Options > Vérification > Dictionnaire personnalisé** pour voir et modifier vos mots. La boîte de dialogue comporte deux listes, chacune avec une zone **Rechercher** pour les filtrer :

- **Mots ajoutés** : les mots que vous avez enregistrés dans JeffWrite. Ils sont acceptés dans toutes les langues.
- **Mots exclus** : les mots que vous voulez voir signalés comme fautes, même si un dictionnaire les accepte, comme un mot que vous tapez toujours mal. Ils sont signalés partout où ils apparaissent, quelle que soit leur casse, et ne sont jamais proposés comme suggestion. Un mot exclu désigne ce mot exact, et non ses autres formes : exclure « maison » ne signale pas « maisons ». Ajouter un mot à votre dictionnaire, depuis le volet, le menu du clic droit ou la boîte de dialogue, le retire de la liste des mots exclus. **Ignorer tout** fonctionne encore sur un mot exclu jusqu'à la fermeture du programme.

Saisissez un mot puis cliquez sur **Ajouter**, ou sélectionnez des mots puis cliquez sur **Supprimer**. Un mot ne peut figurer que dans une liste à la fois : l'ajouter à l'une le retire de l'autre. **Importer** reprend les mots d'un fichier texte brut, un mot par ligne, et **Exporter** écrit la liste dans le même type de fichier (UTF-8). Une ligne contenant un espace ou un caractère de commande, ou dépassant 100 lettres, est omise, et la boîte de dialogue indique combien de lignes ont été omises. Une liste peut atteindre 1 Mo de texte. Rien n'est modifié tant que vous n'avez pas cliqué sur **OK**. Les listes vous appartiennent, sont identiques dans toutes les applications JeffOffice et ne sont jamais enregistrées dans un document.

## Lorsque la correction automatique modifie un mot

Lorsque la correction automatique modifie un mot pendant la frappe, un petit bouton **Options de correction automatique** apparaît sous le point d'insertion. Il propose trois choix. Un lecteur d'écran annonce la présence de ces choix lorsque le bouton apparaît, et la description du bouton indique comment les ouvrir. Vous pouvez aussi appuyer sur **Alt+Maj+F10** pour les ouvrir. Il s'agit de la commande **Menu des options de correction automatique**, qui figure dans la page Clavier des options avec les autres commandes ; vous pouvez lui attribuer une autre touche à cet endroit.

- **Annuler la correction automatique** rétablit le mot tel que vous l'aviez saisi. Il s'agit d'une seule étape : **Rétablir** rétablit la modification.
- Un choix qui arrête la modification. Il dépend de ce qui a changé le mot : **Arrêter le remplacement** retire l'entrée de votre liste de remplacements, **Conserver tel quel** ajoute un mot comportant deux majuscules (« THursday ») à la liste des mots qui conservent leurs majuscules, et **Arrêter la mise en majuscule** désactive partout la majuscule des noms de jours, de la première lettre des phrases ou de la première lettre des cellules de tableau, et non seulement pour ce mot ; JeffWrite l'indique. Il s'agit du même commutateur que dans la boîte de dialogue Correction automatique. Le mot est également rétabli tel que vous l'aviez saisi.
- **Options de correction automatique...** ouvre la boîte de dialogue Correction automatique.

L'onglet **Symboles mathématiques** de la boîte de dialogue Correction automatique contient un commutateur, **Remplacer les noms des symboles mathématiques pendant la frappe**, désactivé par défaut. Lorsqu'il est activé, une barre oblique inverse suivie d'un nom, comme \alpha, devient le symbole (α) dès que vous tapez une espace ou un signe de ponctuation après lui. Le nom doit commencer un mot, si bien qu'un chemin de dossier tel que C:\temp\pi reste tel que vous l'avez saisi. L'onglet liste tous les noms. Lorsqu'un nom a été remplacé, le choix du bouton devient **Arrêter le remplacement des noms mathématiques**, qui désactive le commutateur. JeffDeck et JeffPub les modifient également ; JeffCrunch ne modifie rien dans une cellule.

Le bouton disparaît lorsque vous tapez, modifiez le texte ou déplacez le point d'insertion.

## Activer ou désactiver les traits rouges

Cliquez sur **Révision > Vérifier l'orthographe pendant la frappe**. Ce même choix se trouve dans **Fichier > Options > Vérification**, et il est commun à toutes les applications JeffOffice. Vous pouvez également choisir d'ignorer les mots en MAJUSCULES et les mots contenant des chiffres, comme 3e. Ces deux options sont ignorées par défaut.

## Vérifier la grammaire pendant la frappe

JeffWrite recherche aussi les fautes de grammaire et de style évidentes dans le texte. Un problème reçoit un trait bleu pointillé. L'orthographe conserve son trait rouge ondulé, et un mot mal orthographié n'est jamais signalé comme problème de grammaire. JeffWrite recherche notamment :

- un mot répété (« the the »)
- « a » et « an » devant un mauvais son (« an test », « a hour »)
- une phrase qui commence par une minuscule après un point
- « its » et « it's », « their » et « there », « your » et « you're », « then » et « than », « lose » et « loose », lorsque les mots voisins rendent la faute évidente
- « could of » pour « could have »
- deux espaces de suite à l'intérieur d'une phrase
- une espace avant une virgule, un point ou un autre signe qui se rattache au mot précédent
- aucune espace après la fin d'une phrase
- « alot », et « everyday » pour « every day » (« almost everyday »)
- des verbes qui ne s'accordent pas avec le sujet dans les cas clairs (« she don't », « they was », « it have », « everyone are », « there is many reasons »)
- une mauvaise forme de verbe (« did went », « has went », « can't hardly », « don't have no »)
- des mots confondus (« who's car », « an affect on », « less people », « irregardless », « can't breath », « weather » pour « whether », « chose » pour « choose », « advise » pour « advice »)
- « between you and I », « Me and him went », « more better » et « these thing »
- une virgule après l'année d'une date (« On June 5, 2020, we left »), une virgule après une ville et un État (« in Austin, Texas, the heat ») et un trait d'union dans un composé placé devant un nom (« a well-known author »)
- un « i » minuscule pour le mot « I », et les jours et les mois en minuscules

JeffWrite préfère manquer une faute plutôt que de signaler un texte correct. Une phrase sans trait bleu n'est donc pas la garantie qu'elle est correcte.

Faites un clic droit sur le problème. Le menu indique ce qui ne va pas, puis propose une ou plusieurs suggestions. Cliquez sur une suggestion pour la placer dans le texte, et appuyez sur Ctrl+Z pour l'annuler. **Ignorer ce problème** supprime le trait à cet endroit précis, jusqu'à la fermeture de JeffWrite. Le même mot ailleurs conserve son trait, et une modification ailleurs dans le paragraphe ne le fait pas réapparaître. **Ignorer tout** supprime le trait de tous les problèmes portant ce message sur ces mots.

La vérification se fait sur votre ordinateur, sans connexion à Internet, en arrière-plan : la frappe n'est jamais interrompue, et un paragraphe est vérifié peu de temps après que vous avez cessé de le modifier. Un paragraphe est vérifié jusqu'à ses 100 000 premiers caractères. Un document dont la vérification dépasse cette limite cesse d'être vérifié pour grammaire, et la barre d'état vous en avertit une fois ; la vérification orthographique continue de fonctionner.

**Langues vérifiées.** L'anglais, le français, l'allemand et l'espagnol, chacun avec ses propres règles. Le français contrôle l'espace insécable avant ; : ! ? et à l'intérieur de « », l'apostrophe dans des mots comme « l'ami » et « qu'il », ainsi que les confusions telles que « a » et « à ». L'allemand contrôle « das » et « dass », « seid » et « seit », « wider » et « wieder », la virgule devant « dass » et « weil », et les majuscules des noms communs. L'espagnol recherche le ¿ ou le ¡ manquant, les accents des mots interrogatifs, « a ver » et « haber », et « del » et « al ». Un paragraphe est vérifié dans la langue qui lui est attribuée ; un texte sans langue est vérifié en anglais. Dans un paragraphe qui comporte plusieurs langues, la première langue vérifiée est utilisée et les autres sont ignorées. Le texte qu'un fichier marque « ne pas vérifier » est également ignoré. La grammaire est vérifiée dans le texte du document, mais pas encore dans les tableaux, les en-têtes, les pieds de page et les notes.

Pour la désactiver, cliquez sur **Révision > Vérifier la grammaire pendant la frappe**. Ce même choix se trouve dans **Fichier > Options > Vérification**. Il s'agit d'un choix unique pour toutes les applications JeffOffice, identique dans toutes les fenêtres. Lorsqu'elle est activée, le volet Orthographe (F7) parcourt les problèmes de grammaire après les fautes d'orthographe.

**Choisir les problèmes recherchés.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les types de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme de verbe, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union superflus ou manquants), **Majuscules**, **Mots confondus** (mots qui se prononcent de la même façon), **Répétitions** et **Style**. L'option **Style** est désactivée tant que vous ne l'activez pas. Elle formule des suggestions, et non des fautes : phrases de plus de 40 mots, voix passive, trois phrases consécutives commençant par le même mot, et formules verbeuses (« in order to », « due to the fact that »). Le choix est conservé lorsque vous cliquez sur **OK** dans les Options. Il s'applique immédiatement aux traits bleus et au volet Orthographe (F7) dans les fenêtres ouvertes de cette application, et aux autres applications JeffOffice au prochain démarrage.

## Langues

JeffWrite vérifie chaque passage de texte dans sa propre langue. Un fichier peut indiquer la langue de son texte, jusqu'à un seul mot, et JeffWrite en tient compte. Un texte qui ne précise rien est vérifié dans la langue du document. Le fichier définit cette langue pour l'ensemble du document, et un nouveau document utilise l'anglais américain.

Un texte dans une langue pour laquelle JeffWrite ne dispose d'aucun dictionnaire n'est pas vérifié. Un texte que le fichier marque « ne pas vérifier l'orthographe » ne l'est pas non plus. Vous ne pouvez pas encore changer la langue d'un texte dans JeffWrite, mais la langue indiquée par le fichier est conservée.

Les 31 dictionnaires comprennent l'anglais (américain, britannique, canadien et australien), l'espagnol (21 variantes régionales, dont le Mexique et l'Espagne), le français, l'allemand, l'italien, le néerlandais et le portugais (Brésil et Portugal).

## Ce qui n'est pas encore vérifié

Les traits rouges s'affichent dans le texte, dans les cellules des tableaux, ainsi que dans un en-tête, un pied de page ou une note de bas de page pendant que vous les modifiez. Ils ne s'affichent pas encore dans les zones de texte, mais le volet Orthographe les parcourt.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
- [Imprimer](print)
