# Notes et liens
<!-- keywords: note, notes, commentaire, commentaires, survol, marque rouge, coin, nouvelle note, modifier la note, supprimer la note, afficher la note, masquer la note, afficher toutes les notes, lien, liens, lien hypertexte, fichiers récents, adresse, page web, e-mail, emplacement dans ce classeur, Ctrl+K, Ctrl+clic, info-bulle, ouvrir le lien, supprimer le lien, conversation, statistiques du classeur, nombre, cellules, formules, graphiques, tableaux croisés dynamiques, accessibilité, texte de remplacement, cellules fusionnées -->

## Notes

Une **note** est un court texte associé à une cellule. Une cellule qui en contient une affiche un petit repère rouge dans son coin supérieur droit. Placez le pointeur sur la cellule : la note apparaît à côté d'elle ; éloignez le pointeur, et elle disparaît.

Pour ajouter une note à une cellule, sélectionnez-la, puis utilisez **Révision > Nouvelle note** (ou appuyez sur **Maj+F2**). Saisissez le texte, puis cliquez sur **OK**. Sur une cellule qui contient déjà une note, la même commande s'intitule **Modifier la note**. Si vous effacez le texte, la note est supprimée.

- **Supprimer la note** supprime les notes des cellules sélectionnées.
- **Afficher ou masquer la note** garde la note des cellules sélectionnées affichée en permanence sur la feuille, dans une bulle à côté de chaque cellule, ou la remet à sa place.
- **Afficher toutes les notes** fait de même pour toutes les notes de la feuille ; le bouton s'intitule alors **Masquer toutes les notes**.
- **Note précédente** et **Note suivante** vous font passer d'une cellule contenant une note à la suivante.

Chacune de ces actions peut être annulée en une seule étape. Une note suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque des cellules sont triées. **Effacer tout** supprime les notes des cellules qu'il efface.

Un classeur peut contenir une conversation de commentaires, avec des réponses. JeffCrunch affiche l'ensemble de la conversation comme une note et la conserve à l'enregistrement, mais ne la modifie pas : la note est en lecture seule.

Les notes peuvent être enregistrées dans un classeur .xlsx. Un classeur ouvert à partir d'un type de fichier plus ancien, ou d'un fichier texte, ne conserve ses notes qu'après l'avoir enregistré au format de classeur .xlsx.

## Liens

Un **lien** vous envoie vers un autre emplacement lorsque vous le suivez : une page web, un fichier, une adresse e-mail ou un emplacement dans ce classeur (une cellule, des cellules ou un nom).

Pour ajouter un lien à une cellule, sélectionnez-la, puis utilisez **Insertion > Lien** (ou appuyez sur **Ctrl+K**). Choisissez la destination, saisissez l'adresse ou l'emplacement (ou sélectionnez l'un des fichiers ouverts récemment dans **Ou un fichier récent**), puis, si vous le souhaitez, le texte que la cellule doit afficher et une **info-bulle**. Sur une cellule qui contient déjà un lien, la même commande s'intitule **Modifier le lien**. **Supprimer le lien** supprime les liens des cellules sélectionnées.

Placez le pointeur sur une cellule contenant un lien pour voir où il mène. Pour le suivre, maintenez la touche **Ctrl** enfoncée et cliquez sur la cellule, ou sélectionnez la cellule et utilisez **Ouvrir le lien** dans le menu contextuel (clic droit). Un simple clic ne fait que sélectionner la cellule.

JeffCrunch n'ouvre jamais rien de lui-même. Une page web ou une adresse e-mail s'ouvre dans votre programme habituel lorsque vous suivez son lien. Un lien vers un fichier, ou vers un autre type d'adresse, vous demande d'abord confirmation. Un lien qui exécuterait un script n'est jamais ouvert.

Un lien suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque des cellules sont triées ; il est enregistré avec le classeur.

## Statistiques du classeur

**Statistiques du classeur** (onglet **Révision**) compte ce que contient le classeur : le nombre de **Feuilles**, de **Cellules avec données** (cellules contenant un nombre, un texte ou une formule), de **Formules**, de **Graphiques** et de **Tableaux croisés dynamiques**. Les feuilles supprimées ne sont pas comptées. Les mêmes chiffres figurent sur la page **Informations** de l'onglet **Fichier**.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** (onglet **Révision**) ouvre une petite fenêtre qui liste les éléments du classeur difficiles à utiliser pour certains lecteurs, ou difficiles à restituer pour un lecteur d'écran (programme qui lit l'écran à voix haute). Chaque ligne commence par **Erreur**, **Avertissement** ou **Conseil**. Cliquez sur une ligne pour atteindre la cellule.

Elle recherche :

- Un graphique ou une image sans **texte de remplacement** (le texte qui décrit ce qu'elle montre). Il s'agit d'une erreur.
- Un tableau sans ligne d'en-tête, donc dont les colonnes n'ont pas de nom. Il s'agit d'une erreur.
- Des **cellules fusionnées**, difficiles à parcourir au clavier. Il s'agit d'un avertissement.
- Un texte difficile à lire sur la couleur de fond, comme un texte gris clair sur fond blanc. JeffCrunch mesure le contraste entre le texte et le remplissage. Un petit texte nécessite un rapport d'au moins 4,5 pour 1, et un grand texte (18 points, ou 14 points en gras) un rapport de 3 pour 1. Elle indique la première cellule de chaque combinaison de police et de remplissage. Il s'agit d'un avertissement.
- Une feuille qui porte encore le nom qui lui a été donné automatiquement, comme Feuil1. Il s'agit d'un conseil.

Appuyez sur **Vérifier à nouveau** après avoir effectué des modifications.
