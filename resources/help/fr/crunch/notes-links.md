# Notes, commentaires et liens
<!-- keywords: langue, langue de vérification, définir la langue, ne pas vérifier l'orthographe, traduire, traduction, libretranslate, volet commentaires, nouveau commentaire, répondre, résoudre, rouvrir, fil, commentaire précédent, commentaire suivant, afficher les commentaires, convertir les notes en commentaires, convertir les commentaires en notes, orthographe, vérifier l'orthographe, faute, F7, ignorer tout, remplacer tout, ajouter au dictionnaire, dictionnaire des synonymes, synonymes, liste de mots, note, notes, commentaire, commentaires, survol, marque rouge, coin, nouvelle note, modifier la note, supprimer la note, afficher la note, masquer la note, afficher toutes les notes, lien, liens, lien hypertexte, adresse, page web, e-mail, courriel, emplacement dans ce classeur, Ctrl+K, Ctrl+clic, info-bulle, ouvrir le lien, supprimer le lien, conversation, statistiques du classeur, compter, cellules, formules, graphiques, tableaux croisés dynamiques, accessibilité, texte de remplacement, cellules fusionnées, fichiers récents -->

## Notes

Une **note** est quelques mots associés à une cellule. Une cellule qui en comporte une affiche un petit repère rouge dans son coin supérieur droit. Placez le pointeur sur la cellule : la note apparaît à côté d'elle ; éloignez le pointeur, et elle disparaît.

Pour ajouter une note à une cellule, sélectionnez-la puis utilisez **Révision > Nouvelle note** (ou appuyez sur **Maj+F2**). Saisissez le texte et cliquez sur **OK**. Sur une cellule qui contient déjà une note, la même commande s'intitule **Modifier la note**. Si vous effacez le texte, la note est supprimée.

- **Supprimer la note** supprime les notes des cellules sélectionnées.
- **Afficher ou masquer la note** garde en permanence la note des cellules sélectionnées affichée sur la feuille, dans une zone à côté de chaque cellule, ou la masque à nouveau.
- **Afficher toutes les notes** fait de même pour toutes les notes de la feuille ; le même bouton devient alors **Masquer toutes les notes**.
- **Note précédente** et **Note suivante** passent d'une cellule comportant une note à la suivante.

Chacune de ces actions peut être annulée en une seule étape. Une note suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, ainsi que lorsque les cellules sont triées. **Effacer tout** supprime les notes et les commentaires des cellules effacées.

Une cellule peut aussi porter une conversation de **commentaires**, avec des réponses. Voir la partie suivante.

Les notes peuvent être enregistrées dans un classeur .xlsx. Un classeur ouvert à partir d'un ancien format de fichier ou d'un fichier texte ne conserve ses notes qu'après l'avoir enregistré au format de classeur .xlsx.

## Commentaires

Un **commentaire** ressemble à une note, mais il lance un **fil** : d'autres personnes peuvent y répondre, et l'une d'elles peut marquer le fil comme **résolu** lorsque la question a trouvé sa réponse. Chaque commentaire indique son auteur et sa date. Une cellule dotée d'un fil porte le même repère rouge qu'une cellule dotée d'une note. Placez le pointeur sur la cellule pour lire l'ensemble du fil, chaque réponse étant placée sous le commentaire auquel elle répond. Un fil résolu commence par le mot **Résolu**.

Sous l'onglet **Révision**, dans le groupe **Commentaires** :

- **Nouveau commentaire** (**Ctrl+Maj+F2**) lance un fil sur la cellule sélectionnée. Saisissez le texte et cliquez sur **OK**. La commande fonctionne sur une cellule qui ne contient ni note ni commentaire.
- **Répondre** ajoute votre texte au fil de la cellule sélectionnée.
- **Résoudre le fil** marque le fil comme terminé. Le même bouton devient alors **Rouvrir le fil**, qui le rouvre.
- **Supprimer le commentaire** supprime les fils des cellules sélectionnées.
- **Commentaire précédent** et **Commentaire suivant** passent d'une cellule comportant un fil à la suivante.
- **Afficher les commentaires** ouvre le volet **Commentaires** à côté de la feuille. Il liste chaque fil de la feuille sous forme de carte : la cellule, chaque commentaire avec son auteur et son heure, et les boutons **Répondre**, **Résoudre** ou **Rouvrir**, ainsi que **Supprimer**. **Atteindre** affiche la cellule. Le volet suit chaque modification, y compris l'annulation.
- **Convertir les notes en commentaires** transforme chaque note simple de la feuille en fil d'un seul commentaire. **Convertir les commentaires en notes** retransforme chaque fil en note. Une note n'a pas de réponses : les commentaires d'un fil deviennent donc une seule note contenant tous leurs textes, et JeffCrunch vous demande confirmation avant de le faire pour un fil comportant des réponses.

Chacune de ces actions peut être annulée en une seule étape.

Les programmes qui ne savent pas afficher les fils montrent à la place une note simple. JeffCrunch conserve cette note, avec le texte du fil, à chaque enregistrement. Les fils, les réponses et leurs auteurs restent tels quels lorsque vous enregistrez le classeur puis le rouvrez. Un fil de plus de 500 commentaires est affiché et conservé, mais JeffCrunch n'y ajoute rien et ne le modifie pas.

Les commentaires sont enregistrés dans un classeur .xlsx. Un classeur ouvert à partir d'un ancien format de fichier ou d'un fichier texte ne conserve ses commentaires qu'après l'avoir enregistré au format de classeur .xlsx.

## Liens

Un **lien** vous envoie quelque part lorsque vous le suivez : vers une page web, vers un fichier, vers une adresse e-mail, ou vers un emplacement de ce classeur (une cellule, des cellules ou un nom).

Pour ajouter un lien à une cellule, sélectionnez-la puis utilisez **Insertion > Lien** (ou appuyez sur **Ctrl+K**). Choisissez la destination, saisissez l'adresse ou l'emplacement (ou choisissez l'un des fichiers ouverts récemment dans **Ou un fichier récent**) et, si vous le souhaitez, le texte à afficher dans la cellule ainsi qu'une **info-bulle**. Sur une cellule qui contient déjà un lien, la même commande s'intitule **Modifier le lien**. **Supprimer le lien** retire les liens des cellules sélectionnées.

Placez le pointeur sur une cellule comportant un lien pour voir sa destination. Pour le suivre, maintenez **Ctrl** enfoncée et cliquez sur la cellule, ou sélectionnez la cellule et utilisez **Ouvrir le lien** dans le menu contextuel (clic droit). Un simple clic ne fait que sélectionner la cellule.

JeffCrunch n'ouvre jamais rien de lui-même. Une page web ou une adresse e-mail s'ouvre dans votre programme habituel lorsque vous suivez son lien. Un lien vers un fichier, ou vers un autre type d'adresse, vous demande d'abord confirmation. Un lien qui exécuterait un script n'est jamais ouvert.

Un lien suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, ainsi que lorsque les cellules sont triées, et il est enregistré avec le classeur.

## Statistiques du classeur

**Statistiques du classeur** (onglet **Révision**) compte ce que contient le classeur : le nombre de **Feuilles**, le nombre de **Cellules avec données** (cellules contenant un nombre, un texte ou une formule), le nombre de **Formules**, le nombre de **Graphiques** et le nombre de **Tableaux croisés dynamiques**. Les feuilles supprimées ne sont pas comptées. Les mêmes chiffres figurent dans la page **Infos** de l'onglet Fichier.

## Orthographe

**Orthographe** (onglet **Révision**, ou **F7**) vérifie les mots de vos cellules de texte, un mot erroné à la fois. Si vous avez sélectionné plusieurs cellules, seules celles-ci sont vérifiées ; sinon, c'est toute la feuille. Les formules, les nombres et les dates ne sont pas vérifiés.

La fenêtre affiche le mot absent du dictionnaire, une zone nommée **Remplacer par** contenant la meilleure suggestion, ainsi qu'une liste d'autres suggestions. Choisissez une suggestion ou saisissez votre propre mot, puis sélectionnez :

- **Modifier** remplace le mot erroné par le nouveau mot dans cette cellule. Chaque modification peut être annulée en une étape.
- **Modifier tout** remplace le même mot par le nouveau dans toutes les cellules vérifiées, en une seule étape annulable.
- **Ignorer** laisse le mot tel quel cette fois-ci. **Ignorer tout** le laisse tel quel partout jusqu'à la fermeture de JeffCrunch.
- **Ajouter** place le mot dans votre dictionnaire personnel, où il ne sera plus signalé. Votre dictionnaire est partagé avec les autres applications JeffOffice.

**Fichier > Options > Vérification** détermine si les mots en majuscules et les mots contenant des chiffres sont vérifiés. JeffCrunch vérifie dans la langue de son affichage si un dictionnaire existe pour celle-ci, et en anglais américain sinon.

Pendant que vous saisissez dans une cellule, un mot absent du dictionnaire est souligné d'un trait rouge ondulé. Faites un clic droit sur le mot pour choisir une correction, ou pour sélectionner **Ignorer tout** ou **Ajouter au dictionnaire**. Les cellules de la grille ne sont pas soulignées, et une entrée commençant par un signe égal n'est jamais vérifiée.

## Définir la langue de certaines cellules

L'orthographe vérifie chaque cellule dans sa propre langue. Pour la modifier, sélectionnez les cellules puis cliquez sur **Définir la langue de vérification** dans le groupe **Langue** de l'onglet **Révision**. Choisissez la langue dans la liste, puis cliquez sur **OK**. Une cellule à laquelle vous n'avez pas attribué de langue est vérifiée dans la langue définie dans **Fichier > Options**. Une feuille de calcul OpenDocument (.ods) conserve la langue de chaque cellule. Un classeur .xlsx n'a pas d'emplacement prévu pour cela : la langue n'est donc conservée que pendant que le classeur est ouvert.

## Ne pas vérifier certaines cellules

Certains textes ne doivent pas être vérifiés : un code, un nom, ou une ligne dans une langue inventée. Sélectionnez les cellules puis cliquez sur **Ne pas vérifier l'orthographe** dans le groupe **Langue**. **Orthographe** et les traits rouges ondulés de la cellule les ignorent. Cliquez à nouveau sur le bouton pour vérifier de nouveau les cellules. Un fichier .ods conserve cette marque, et un classeur .xlsx ne la conserve que pendant que le classeur est ouvert.

## Traduire du texte

1. Sélectionnez les cellules, ou sélectionnez une partie des mots que vous saisissez dans une cellule. Le texte des cellules sélectionnées est envoyé, une ligne de cellules par ligne de texte.
2. Cliquez sur **Traduire** dans le groupe **Langue**.
3. Choisissez la langue de traduction.
4. Cliquez sur **OK**.

Votre navigateur web ouvre alors un service de traduction appelé LibreTranslate, avec votre texte. Rien n'est envoyé tant que vous ne cliquez pas sur **OK**. Vous pouvez modifier l'adresse du service dans la zone prévue à cet effet, par exemple pour utiliser un service que vous hébergez vous-même. Seules les adresses web sécurisées (https) sont utilisées, ainsi que les adresses http de cet ordinateur. 5 000 caractères au maximum sont envoyés.

## Dictionnaire des synonymes

**Dictionnaire des synonymes** (onglet **Révision**, ou **Maj+F7**) recherche des mots de sens presque identique à un mot de la cellule. Il recherche le premier mot du texte de la cellule active. Saisissez un autre mot dans **Rechercher** et appuyez sur **Entrée** pour rechercher celui-ci à la place. Choisissez un mot dans la liste et cliquez sur **Remplacer**, ou double-cliquez dessus. Le mot de la cellule est remplacé, et une majuscule initiale est conservée. **Rechercher** cherche le mot que vous avez choisi, pour suivre une chaîne de sens. La liste de mots fonctionne en anglais.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** (onglet **Révision**) ouvre une petite fenêtre qui liste les éléments du classeur difficiles à utiliser pour certains lecteurs, ou difficiles à restituer pour un lecteur d'écran (un programme qui lit l'écran à voix haute). Chaque ligne commence par **Erreur**, **Avertissement** ou **Conseil**. Cliquez sur une ligne pour aller à la cellule.

Elle recherche :

- Un graphique ou une image sans **texte de remplacement** (les mots qui décrivent ce qu'elle montre). C'est une erreur.
- Un tableau sans ligne d'en-tête, de sorte que ses colonnes n'ont pas de noms. C'est une erreur.
- Les **cellules fusionnées**, difficiles à parcourir au clavier. C'est un avertissement.
- Un texte difficile à lire sur la couleur de fond, comme un texte gris clair sur fond blanc. JeffCrunch mesure le contraste du texte par rapport au remplissage. Un petit texte doit avoir un rapport d'au moins 4,5 pour 1, et un grand texte (18 points, ou 14 points en gras) un rapport de 3 pour 1. Elle indique la première cellule de chaque combinaison de police et de remplissage. C'est un avertissement.
- Une feuille qui porte encore le nom qui lui a été donné à l'origine, comme Feuil1. C'est un conseil.

Cliquez sur **Vérifier à nouveau** après avoir apporté des modifications.
