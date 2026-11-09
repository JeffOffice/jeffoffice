# Notes, commentaires et liens
<!-- keywords: volet commentaires, nouveau commentaire, répondre, résoudre, rouvrir, fil, commentaire précédent, commentaire suivant, afficher les commentaires, convertir les notes en commentaires, convertir les commentaires en notes, orthographe, vérifier l'orthographe, mot mal orthographié, F7, ignorer tout, remplacer tout, ajouter au dictionnaire, dictionnaire des synonymes, synonymes, liste de mots, note, notes, commentaire, commentaires, survol, marque rouge, coin, nouvelle note, modifier la note, supprimer la note, afficher la note, masquer la note, afficher toutes les notes, lien, liens, lien hypertexte, adresse, page web, e-mail, emplacement dans ce classeur, Ctrl+K, Ctrl+clic, info-bulle, ouvrir le lien, supprimer le lien, conversation, statistiques du classeur, compter, cellules, formules, graphiques, tableaux croisés dynamiques, accessibilité, texte de remplacement, cellules fusionnées, fichiers récents -->

## Notes

Une **note** est un court texte associé à une cellule. Une cellule qui en contient une affiche un petit repère rouge dans son coin supérieur droit. Placez le pointeur sur la cellule : la note apparaît à côté d'elle ; éloignez le pointeur, et elle disparaît.

Pour ajouter une note à une cellule, sélectionnez-la et utilisez **Révision > Nouvelle note** (ou appuyez sur **Maj+F2**). Saisissez le texte, puis cliquez sur **OK**. Sur une cellule qui contient déjà une note, la même commande affiche **Modifier la note**. Si vous supprimez tout le texte, la note disparaît.

- **Supprimer la note** supprime les notes des cellules sélectionnées.
- **Afficher ou masquer la note** garde la note des cellules sélectionnées affichée en permanence sur la feuille, dans une zone à côté de chaque cellule, ou la masque à nouveau.
- **Afficher toutes les notes** fait de même pour toutes les notes de la feuille ; le bouton affiche alors **Masquer toutes les notes**.
- **Note précédente** et **Note suivante** passent d'une cellule contenant une note à la suivante.

Chacune de ces actions peut être annulée en une seule étape. Une note suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque les cellules sont triées. **Effacer tout** supprime les notes et les commentaires des cellules effacées.

Une cellule peut aussi contenir une conversation de **commentaires**, avec des réponses. Voir la partie suivante.

Les notes peuvent être enregistrées dans un classeur .xlsx. Un classeur ouvert à partir d'un ancien type de fichier, ou d'un fichier texte, ne conserve ses notes qu'après l'avoir enregistré au format de classeur .xlsx.

## Commentaires

Un **commentaire** ressemble à une note, mais il lance un **fil** : d'autres personnes peuvent y répondre, et l'une d'elles peut marquer le fil comme **résolu** lorsque la question a trouvé sa réponse. Chaque commentaire indique son auteur et sa date. Une cellule contenant un fil affiche le même repère rouge qu'une cellule contenant une note. Placez le pointeur sur la cellule pour lire l'ensemble du fil, chaque réponse étant placée sous le commentaire auquel elle répond. Un fil résolu commence par le mot **Résolu**.

Dans l'onglet **Révision**, dans le groupe **Commentaires** :

- **Nouveau commentaire** (**Ctrl+Maj+F2**) lance un fil sur la cellule sélectionnée. Saisissez le texte, puis cliquez sur **OK**. La commande fonctionne sur une cellule qui ne contient ni note ni commentaire.
- **Répondre** ajoute votre texte au fil de la cellule sélectionnée.
- **Résoudre le fil** marque le fil comme terminé. Le même bouton affiche alors **Rouvrir le fil**, qui le rouvre.
- **Supprimer le commentaire** supprime les fils des cellules sélectionnées.
- **Commentaire précédent** et **Commentaire suivant** passent d'une cellule contenant un fil à la suivante.
- **Afficher les commentaires** ouvre le volet **Commentaires** à côté de la feuille. Il liste chaque fil de la feuille sous forme de carte : la cellule, chaque commentaire avec son auteur et son heure, et les boutons **Répondre**, **Résoudre** ou **Rouvrir**, et **Supprimer**. **Atteindre** déplace la sélection vers la cellule. Le volet suit chaque modification, ainsi que **Annuler**.
- **Convertir les notes en commentaires** transforme chaque note simple de la feuille en fil d'un seul commentaire. **Convertir les commentaires en notes** transforme chaque fil en note. Une note ne comporte pas de réponses : les commentaires d'un fil deviennent donc une seule note regroupant tous leurs textes, et JeffCrunch vous demande confirmation avant de le faire pour un fil comportant des réponses.

Chacune de ces actions peut être annulée en une seule étape.

Les programmes incapables d'afficher les fils montrent une simple note à leur place. JeffCrunch conserve cette note, avec le texte du fil, à chaque enregistrement. Les fils, les réponses et leurs auteurs restent tels quels lorsque vous enregistrez le classeur puis le rouvrez. Un fil de plus de 500 commentaires est affiché et conservé, mais JeffCrunch n'y ajoute rien et ne le modifie pas.

Les commentaires sont enregistrés dans un classeur .xlsx. Un classeur ouvert à partir d'un ancien type de fichier, ou d'un fichier texte, ne conserve ses commentaires qu'après l'avoir enregistré au format de classeur .xlsx.

## Liens

Un **lien** vous emmène ailleurs lorsque vous le suivez : vers une page web, un fichier, une adresse e-mail, ou un emplacement de ce classeur (une cellule, des cellules ou un nom).

Pour ajouter un lien à une cellule, sélectionnez-la et utilisez **Insertion > Lien** (ou appuyez sur **Ctrl+K**). Choisissez la destination, saisissez l'adresse ou l'emplacement (ou choisissez l'un des fichiers ouverts récemment dans **Ou un fichier récent**), puis, si vous le souhaitez, le texte à afficher dans la cellule et une **info-bulle**. Sur une cellule qui contient déjà un lien, la même commande affiche **Modifier le lien**. **Supprimer le lien** retire les liens des cellules sélectionnées.

Placez le pointeur sur une cellule contenant un lien pour voir sa destination. Pour le suivre, maintenez **Ctrl** enfoncé et cliquez sur la cellule, ou sélectionnez la cellule et utilisez **Ouvrir le lien** dans le menu contextuel (clic droit). Un simple clic se contente de sélectionner la cellule.

JeffCrunch n'ouvre jamais rien de lui-même. Une page web ou une adresse e-mail s'ouvre dans votre programme habituel lorsque vous suivez son lien. Un lien vers un fichier, ou vers un autre type d'adresse, vous demande d'abord confirmation. Un lien qui exécuterait un script n'est jamais ouvert.

Un lien suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque les cellules sont triées ; il est enregistré avec le classeur.

## Statistiques du classeur

**Statistiques du classeur** (onglet **Révision**) compte le contenu du classeur : le nombre de **Feuilles**, de **Cellules avec données** (cellules contenant un nombre, un texte ou une formule), de **Formules**, de **Graphiques** et de **Tableaux croisés dynamiques**. Les feuilles supprimées ne sont pas comptées. Les mêmes chiffres figurent sur la page **Infos** de l'onglet Fichier.

## Orthographe

**Orthographe** (onglet **Révision**, ou **F7**) vérifie les mots de vos cellules de texte, un mot erroné à la fois. Si vous avez sélectionné plusieurs cellules, seules celles-ci sont vérifiées ; sinon, c'est la feuille entière. Les formules, les nombres et les dates ne sont pas examinés.

La fenêtre affiche le mot absent du dictionnaire, une zone nommée **Remplacer par** avec la meilleure suggestion, et une liste d'autres suggestions. Choisissez une suggestion ou saisissez votre propre mot, puis choisissez :

- **Modifier** remplace le mot erroné par le nouveau mot dans cette cellule. Chaque modification peut être annulée en une seule étape.
- **Tout modifier** remplace le même mot dans toutes les cellules vérifiées, en une seule étape annulable.
- **Ignorer** laisse le mot tel quel cette fois. **Ignorer tout** le laisse tel quel partout jusqu'à la fermeture de JeffCrunch.
- **Ajouter** place le mot dans votre propre dictionnaire, où il ne sera plus jamais signalé. Votre dictionnaire est partagé avec les autres applications JeffOffice.

**Fichier > Options > Vérification** détermine si les mots en majuscules et les mots contenant des chiffres sont vérifiés. JeffCrunch vérifie dans la langue d'affichage, si un dictionnaire existe pour elle, et en anglais américain dans le cas contraire.

Pendant la saisie dans une cellule, un mot absent du dictionnaire est souligné d'un trait rouge ondulé. Faites un clic droit sur le mot pour choisir une correction, ou pour sélectionner **Ignorer tout** ou **Ajouter au dictionnaire**. Les cellules de la grille ne sont pas soulignées, et une entrée commençant par un signe égal n'est jamais vérifiée.

## Dictionnaire des synonymes

**Dictionnaire des synonymes** (onglet **Révision**, ou **Maj+F7**) trouve des mots de sens presque identique à un mot d'une cellule. Il recherche le premier mot du texte de la cellule active. Saisissez un autre mot dans **Rechercher** et appuyez sur **Entrée** pour rechercher celui-ci. Choisissez un mot dans la liste, puis cliquez sur **Remplacer**, ou double-cliquez dessus. Le mot de la cellule est remplacé, et une majuscule initiale est conservée. **Rechercher** cherche le mot sélectionné, pour suivre une chaîne de sens. La liste de mots fonctionne en anglais.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** (onglet **Révision**) ouvre une petite fenêtre qui liste les éléments du classeur difficiles à utiliser pour certains lecteurs, ou à restituer pour un lecteur d'écran (un programme qui lit l'écran à voix haute). Chaque ligne commence par **Erreur**, **Avertissement** ou **Conseil**. Cliquez sur une ligne pour aller à la cellule.

Elle recherche :

- Un graphique ou une image sans **texte de remplacement** (le texte qui décrit ce qu'elle montre). C'est une erreur.
- Un tableau sans ligne d'en-tête, donc dont les colonnes n'ont pas de nom. C'est une erreur.
- Des **cellules fusionnées**, difficiles à parcourir au clavier. C'est un avertissement.
- Un texte difficile à lire sur la couleur de fond, comme un texte gris pâle sur fond blanc. JeffCrunch mesure le contraste entre le texte et le remplissage. Un petit texte doit atteindre un rapport d'au moins 4,5 pour 1, et un grand texte (18 points, ou 14 points en gras) un rapport de 3 pour 1. Elle indique la première cellule de chaque combinaison de police et de remplissage. C'est un avertissement.
- Une feuille qui porte encore le nom qui lui a été attribué, comme Feuil1. C'est un conseil.

Cliquez sur **Vérifier à nouveau** après avoir effectué des modifications.
