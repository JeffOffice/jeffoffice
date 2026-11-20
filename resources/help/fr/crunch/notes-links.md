# Notes, commentaires et liens
<!-- keywords: déplacer note, redimensionner note, taille note, zone de note, rétablir taille note, clavier, langue, langue de vérification, définir la langue, ne pas vérifier l'orthographe, traduire, traduction, libretranslate, volet commentaires, nouveau commentaire, répondre, résoudre, rouvrir, fil, commentaire précédent, commentaire suivant, afficher les commentaires, convertir les notes en commentaires, convertir les commentaires en notes, orthographe, vérification orthographique, mal orthographié, F7, ignorer tout, remplacer tout, ajouter au dictionnaire, dictionnaire des synonymes, synonymes, liste de mots, note, notes, commentaire, commentaires, survol, marque rouge, coin, nouvelle note, modifier la note, supprimer la note, afficher la note, masquer la note, afficher toutes les notes, lien, liens, lien hypertexte, adresse, page web, e-mail, emplacement dans ce classeur, Ctrl+K, Ctrl+clic, info-bulle, ouvrir le lien, supprimer le lien, conversation, statistiques du classeur, compter, cellules, formules, graphiques, tableaux croisés dynamiques, accessibilité, texte de remplacement, cellules fusionnées, fichiers récents -->

## Notes

Une **note** est un court texte associé à une cellule. Une cellule qui en contient une affiche un petit repère rouge dans son coin supérieur droit. Placez le pointeur sur la cellule : la note apparaît à côté d'elle ; éloignez le pointeur, elle disparaît.

Pour ajouter une note à une cellule, sélectionnez-la puis utilisez **Révision > Nouvelle note** (ou appuyez sur **Maj+F2**). Saisissez le texte puis cliquez sur **OK**. Sur une cellule qui contient déjà une note, la même commande s'intitule **Modifier la note**. Si vous effacez le texte, la note disparaît.

- **Supprimer la note** supprime les notes des cellules sélectionnées.
- **Afficher ou masquer la note** affiche en permanence la note des cellules sélectionnées sur la feuille, dans une zone à côté de chaque cellule, ou la masque à nouveau.
- **Afficher toutes les notes** fait de même pour toutes les notes de la feuille ; le bouton s'intitule alors **Masquer toutes les notes**.
- **Note précédente** et **Note suivante** passent d'une cellule contenant une note à la suivante.
- **Déplacer ou redimensionner la note** déplace ou redimensionne, avec le clavier, la zone d'une note affichée. Sélectionnez la cellule, puis utilisez la commande (**Révision > Notes**, ou clic droit sur la cellule). Les **touches fléchées** déplacent alors la zone de 10 pixels ; **Maj** combinée à une flèche la rend plus large, plus étroite, plus haute ou plus basse de 10 pixels ; **Ctrl** (**Option** sur Mac) réduit le pas à 1 pixel. Les combinaisons avec Alt ou Méta (ou Commande sur Mac) n'ont alors aucun effet, et toute autre commande, un changement de feuille, un Annuler ou un Rétablir met fin à l'opération. **Entrée** conserve la zone à sa nouvelle place, sous forme d'une seule étape que vous pouvez annuler ; **Échap** abandonne la modification. Un lecteur d'écran annonce la position de la zone après chaque touche, en pixels à 100 % de zoom (distance horizontale et verticale depuis le coin supérieur gauche de la cellule, ainsi que la largeur et la hauteur de la zone), et ne l'annonce qu'au relâchement d'une touche maintenue, et non à chaque répétition ; il annonce également quand la zone est conservée ou abandonnée. Une zone mesure au maximum 640 pixels de large et 480 de haut, et commence à moins de 1 000 pixels de sa cellule, comme lorsque vous la faites glisser. Faire glisser la zone ou ses huit poignées avec le pointeur produit le même résultat.
- **Rétablir la taille de la note** remet la zone de la note à sa taille par défaut : à côté de la cellule, aussi grande que l'exige son texte.

Chacune de ces actions constitue une seule étape que vous pouvez annuler. Une note suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque des cellules sont triées. **Effacer tout** supprime les notes et les commentaires des cellules effacées.

Une cellule peut aussi contenir une conversation de **commentaires**, avec des réponses. Voir la partie suivante.

Les notes peuvent être enregistrées dans un classeur .xlsx, un classeur .ods et un classeur .xls. Un classeur ouvert à partir d'un fichier .ods ou .xls affiche ses notes, et vous pouvez les ajouter, les modifier, les déplacer et les supprimer comme dans un classeur .xlsx. La date d'une note .ods n'est pas conservée lors de l'enregistrement par JeffCrunch. Une conversation de commentaires enregistrée dans un fichier .ods ou .xls devient une simple note contenant son texte, et reste une simple note lorsque ce fichier est ouvert. Un fichier .xls enregistre la distance d'une zone par rapport à sa cellule selon une taille fixe ; une zone située loin dans une cellule large peut donc se déplacer légèrement. Un classeur ouvert à partir d'un fichier texte ne reprend pas les notes.

## Commentaires

Un **commentaire** ressemble à une note, mais il démarre un **fil** : d'autres personnes peuvent y répondre, et l'une d'elles peut marquer le fil comme **résolu** une fois la question réglée. Chaque commentaire indique son auteur et sa date. Une cellule contenant un fil affiche le même repère rouge qu'une cellule contenant une note. Placez le pointeur sur la cellule pour lire le fil entier, chaque réponse étant placée sous le commentaire auquel elle répond. Un fil résolu commence par le mot **Résolu**.

Dans l'onglet **Révision**, dans le groupe **Commentaires** :

- **Nouveau commentaire** (**Ctrl+Maj+F2**) démarre un fil sur la cellule sélectionnée. Saisissez le texte puis cliquez sur **OK**. La commande fonctionne sur une cellule qui ne contient ni note ni commentaire.
- **Répondre** ajoute votre texte au fil de la cellule sélectionnée.
- **Résoudre le fil** marque le fil comme terminé. Le même bouton s'intitule alors **Rouvrir le fil**, qui le rouvre.
- **Supprimer le commentaire** supprime les fils des cellules sélectionnées.
- **Commentaire précédent** et **Commentaire suivant** passent d'une cellule contenant un fil à la suivante.
- **Afficher les commentaires** ouvre le volet **Commentaires** à côté de la feuille. Il liste chaque fil de la feuille sous forme de carte : la cellule, chaque commentaire avec son auteur et son heure, ainsi que les boutons **Répondre**, **Résoudre** ou **Rouvrir**, et **Supprimer**. **Atteindre** se rend à la cellule. Le volet suit chaque modification, y compris Annuler.
- **Convertir les notes en commentaires** transforme chaque note simple de la feuille en fil d'un seul commentaire. **Convertir les commentaires en notes** transforme chaque fil en note. Une note n'ayant pas de réponses, les commentaires d'un fil deviennent une seule note contenant tous leurs textes ; JeffCrunch demande confirmation avant de le faire pour un fil comportant des réponses.

Chacune de ces actions constitue une seule étape que vous pouvez annuler.

Les programmes incapables d'afficher des fils montrent à la place une simple note. JeffCrunch conserve cette note, avec le texte du fil, à chaque enregistrement. Les fils, les réponses et leurs auteurs restent tels quels lorsque vous enregistrez le classeur puis le rouvrez. Un fil de plus de 500 commentaires est affiché et conservé, mais JeffCrunch ne peut ni y ajouter de commentaire ni le modifier.

Les commentaires sont enregistrés dans un classeur .xlsx. Un classeur ouvert à partir d'un type de fichier plus ancien, ou d'un fichier texte, ne conserve ses commentaires qu'après l'avoir enregistré au format de classeur .xlsx.

## Liens

Un **lien** vous envoie quelque part lorsque vous le suivez : vers une page web, un fichier, une adresse e-mail ou un emplacement de ce classeur (une cellule, des cellules ou un nom).

Pour ajouter un lien à une cellule, sélectionnez-la puis utilisez **Insertion > Lien** (ou appuyez sur **Ctrl+K**). Choisissez la destination, saisissez l'adresse ou l'emplacement (ou choisissez l'un des fichiers ouverts récemment dans **Ou un fichier récent**), puis, si vous le souhaitez, le texte à afficher dans la cellule et une **info-bulle**. Sur une cellule qui contient déjà un lien, la même commande s'intitule **Modifier le lien**. **Supprimer le lien** retire les liens des cellules sélectionnées.

Placez le pointeur sur une cellule contenant un lien pour voir sa destination. Pour le suivre, maintenez **Ctrl** et cliquez sur la cellule, ou sélectionnez la cellule et utilisez **Ouvrir le lien** dans le menu contextuel (clic droit). Un simple clic se contente de sélectionner la cellule.

JeffCrunch n'ouvre jamais rien de lui-même. Une page web ou une adresse e-mail s'ouvre dans votre programme habituel lorsque vous suivez son lien. Un lien vers un fichier, ou vers un autre type d'adresse, vous demande d'abord confirmation. Un lien qui exécuterait un script n'est jamais ouvert.

Un lien suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque des cellules sont triées ; il est enregistré avec le classeur.

## Statistiques du classeur

**Statistiques du classeur** (onglet **Révision**) compte le contenu du classeur : le nombre de **Feuilles**, de **Cellules avec données** (cellules contenant un nombre, un texte ou une formule), de **Formules**, de **Graphiques** et de **Tableaux croisés dynamiques**. Les feuilles supprimées ne sont pas comptées. Les mêmes chiffres figurent dans la page **Infos** de l'onglet Fichier.

## Orthographe

**Orthographe** (onglet **Révision**, ou **F7**) vérifie les mots de vos cellules de texte, un mot erroné à la fois. Si vous avez sélectionné plusieurs cellules, seules celles-ci sont vérifiées ; sinon, c'est toute la feuille. Les formules, les nombres et les dates ne sont pas examinés.

La fenêtre affiche le mot absent du dictionnaire, une zone **Remplacer par** contenant la meilleure suggestion, et la liste des autres suggestions. Choisissez une suggestion ou saisissez votre propre mot, puis choisissez :

- **Modifier** remplace le mot erroné par le nouveau mot dans cette cellule. Chaque modification constitue une étape que vous pouvez annuler.
- **Tout modifier** remplace ce même mot par le nouveau dans toutes les cellules vérifiées, en une seule étape que vous pouvez annuler.
- **Ignorer** laisse le mot tel quel cette fois-ci. **Tout ignorer** le laisse tel quel partout jusqu'à la fermeture de JeffCrunch.
- **Ajouter** place le mot dans votre dictionnaire personnel, afin qu'il ne soit plus jamais signalé. Votre dictionnaire est partagé avec les autres applications JeffOffice.

**Fichier > Options > Vérification** détermine si les mots en majuscules et les mots contenant des chiffres sont vérifiés. JeffCrunch vérifie dans la langue de son affichage, si un dictionnaire existe pour cette langue, et en anglais américain dans le cas contraire.

Pendant la saisie dans une cellule, un mot absent du dictionnaire est souligné d'un trait rouge ondulé. Faites un clic droit sur le mot pour choisir une correction, ou pour sélectionner **Tout ignorer** ou **Ajouter au dictionnaire**. Les cellules de la grille ne sont pas soulignées, et une entrée commençant par le signe égal n'est jamais vérifiée. Pour afficher, modifier, importer ou exporter votre liste de mots ajoutés, et pour établir la liste des mots toujours signalés comme mal orthographiés, cliquez sur **Fichier > Options > Vérification > Dictionnaire personnalisé**. Vos listes sont les mêmes dans toutes les applications JeffOffice.

## Définir la langue de certaines cellules

La vérification orthographique examine chaque cellule dans sa propre langue. Pour la modifier, sélectionnez les cellules puis cliquez sur **Définir la langue de vérification** dans le groupe **Langue** de l'onglet **Révision**. Choisissez la langue dans la liste, puis cliquez sur **OK**. Une cellule à laquelle vous n'avez pas attribué de langue est vérifiée dans la langue définie dans **Fichier > Options**. Une feuille de calcul OpenDocument (.ods) conserve la langue de chaque cellule. Un classeur .xlsx n'a pas d'emplacement prévu pour cela : la langue n'est donc conservée que pendant que le classeur est ouvert.

## Ne pas vérifier certaines cellules

Certains textes ne doivent pas être vérifiés : un code, un nom ou une ligne dans une langue inventée. Sélectionnez les cellules puis cliquez sur **Ne pas vérifier l'orthographe** dans le groupe **Langue**. **Orthographe** et les traits rouges ondulés de la cellule ignorent alors ces cellules. Cliquez à nouveau dessus pour vérifier de nouveau les cellules. Un fichier .ods conserve cette marque, tandis qu'un classeur .xlsx ne la conserve que pendant que le classeur est ouvert.

## Traduire du texte

1. Sélectionnez les cellules, ou sélectionnez une partie des mots que vous saisissez dans une cellule. Le texte des cellules sélectionnées est envoyé, une ligne après l'autre.
2. Cliquez sur **Traduire** dans le groupe **Langue**.
3. Choisissez la langue de traduction.
4. Cliquez sur **OK**.

Votre navigateur web ouvre alors un service de traduction nommé LibreTranslate, avec votre texte. Rien n'est envoyé tant que vous ne cliquez pas sur **OK**. Vous pouvez modifier l'adresse du service dans la zone prévue, par exemple pour utiliser un service que vous hébergez vous-même. Seules les adresses web sécurisées (https) sont utilisées, ainsi que les adresses http de cet ordinateur. Au maximum 5 000 lettres sont envoyées.

## Dictionnaire des synonymes

**Dictionnaire des synonymes** (onglet **Révision**, ou **Maj+F7**) recherche des mots de sens presque identique à un mot de la cellule. Il examine le premier mot du texte de la cellule active. Saisissez un autre mot dans **Rechercher** puis appuyez sur **Entrée** pour rechercher celui-ci. Choisissez un mot dans la liste puis cliquez sur **Remplacer**, ou double-cliquez dessus. Le mot de la cellule est remplacé, et une première lettre en majuscule le reste. **Rechercher** cherche le mot que vous avez choisi, afin de suivre une chaîne de sens. La liste de mots fonctionne en anglais.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** (onglet **Révision**) ouvre une petite fenêtre qui liste les éléments du classeur difficiles à utiliser pour certains lecteurs, ou à restituer par un lecteur d'écran (un programme qui lit l'écran à voix haute). Chaque ligne commence par **Erreur**, **Avertissement** ou **Conseil**. Cliquez sur une ligne pour atteindre la cellule.

Elle recherche :

- Un graphique ou une image sans **texte de remplacement** (les mots qui décrivent ce qu'elle montre). C'est une erreur.
- Un tableau sans ligne d'en-tête, de sorte que ses colonnes n'ont pas de noms. C'est une erreur.
- Des **cellules fusionnées**, difficiles à parcourir au clavier. C'est un avertissement.
- Un texte difficile à lire sur la couleur de fond, comme un texte gris pâle sur fond blanc. JeffCrunch mesure le contraste entre le texte et son remplissage. Un petit texte nécessite un rapport d'au moins 4,5 pour 1, et un grand texte (18 points, ou 14 points en gras) un rapport de 3 pour 1. Elle nomme la première cellule de chaque combinaison de police et de remplissage. C'est un avertissement.
- Une feuille qui porte encore le nom qui lui a été donné par défaut, comme Feuil1. C'est un conseil.

Cliquez sur **Vérifier à nouveau** après avoir effectué des modifications.
