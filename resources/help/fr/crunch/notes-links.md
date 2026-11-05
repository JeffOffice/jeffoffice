# Notes et liens
<!-- keywords: note, notes, commentaire, commentaires, survol, marque rouge, coin, nouvelle note, modifier la note, supprimer la note, afficher la note, masquer la note, afficher toutes les notes, lien, liens, lien hypertexte, adresse, page web, e-mail, emplacement dans ce classeur, Ctrl+K, Ctrl+clic, info-bulle, ouvrir le lien, supprimer le lien, conversation, statistiques du classeur, compter, cellules, formules, graphiques, tableaux croisés dynamiques, accessibilité, texte de remplacement, cellules fusionnées -->

## Notes

Une **note** est quelques mots rattachés à une cellule. Une cellule qui en contient une affiche une petite marque rouge dans son coin supérieur droit. Placez le pointeur sur la cellule : la note apparaît à côté. Éloignez le pointeur, et elle disparaît.

Pour ajouter une note à une cellule, sélectionnez-la et utilisez **Révision > Nouvelle note** (ou appuyez sur **Maj+F2**). Saisissez le texte, puis cliquez sur **OK**. Sur une cellule qui contient déjà une note, la même commande s'intitule **Modifier la note**. Si vous effacez le texte, la note disparaît.

- **Supprimer la note** supprime les notes des cellules sélectionnées.
- **Afficher ou masquer la note** garde en permanence la note des cellules sélectionnées sur la feuille, dans une zone à côté de chaque cellule, ou la replie.
- **Afficher toutes les notes** fait de même pour toutes les notes de la feuille ; le même bouton devient alors **Masquer toutes les notes**.
- **Note précédente** et **Note suivante** passent d'une cellule avec note à la suivante.

Chacune de ces actions peut être annulée en une seule étape. Une note suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque des cellules sont triées. **Effacer tout** supprime les notes des cellules effacées.

Un classeur peut contenir une conversation de commentaires, avec des réponses. JeffCrunch affiche la conversation entière comme une note et la conserve à l'enregistrement, mais ne la modifie pas : la note est en lecture seule.

Les notes peuvent être enregistrées dans un classeur .xlsx. Un classeur ouvert à partir d'un ancien type de fichier, ou d'un fichier texte, ne conserve ses notes qu'après l'avoir enregistré au format de classeur .xlsx.

## Liens

Un **lien** vous envoie quelque part lorsque vous le suivez : vers une page web, vers un fichier, vers une adresse e-mail, ou vers un emplacement de ce classeur (une cellule, des cellules ou un nom).

Pour ajouter un lien à une cellule, sélectionnez-la et utilisez **Insertion > Lien** (ou appuyez sur **Ctrl+K**). Choisissez la destination, saisissez l'adresse ou l'emplacement et, si vous le souhaitez, le texte que la cellule doit afficher ainsi qu'une **info-bulle**. Sur une cellule qui contient déjà un lien, la même commande s'intitule **Modifier le lien**. **Supprimer le lien** retire les liens des cellules sélectionnées.

Placez le pointeur sur une cellule avec un lien pour voir sa destination. Pour le suivre, maintenez **Ctrl** enfoncé et cliquez sur la cellule, ou sélectionnez la cellule et utilisez **Ouvrir le lien** dans le menu contextuel (clic droit). Un simple clic se contente de sélectionner la cellule.

JeffCrunch n'ouvre jamais rien de lui-même. Une page web ou une adresse e-mail s'ouvre dans votre programme habituel lorsque vous suivez son lien. Un lien vers un fichier, ou vers un autre type d'adresse, vous demande d'abord une confirmation. Un lien qui exécuterait un script n'est jamais ouvert.

Un lien suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque des cellules sont triées ; il est enregistré avec le classeur.

## Statistiques du classeur

**Statistiques du classeur** (onglet **Révision**) compte le contenu du classeur : le nombre de **Feuilles**, de **Cellules avec données** (cellules contenant un nombre, un texte ou une formule), de **Formules**, de **Graphiques** et de **Tableaux croisés dynamiques**. Les feuilles supprimées ne sont pas comptées. Les mêmes chiffres figurent dans la page **Informations** de l'onglet Fichier.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** (onglet **Révision**) ouvre une petite fenêtre qui liste les éléments du classeur difficiles à utiliser pour certains lecteurs, ou à restituer pour un lecteur d'écran (programme qui lit l'écran à voix haute). Chaque ligne commence par **Erreur**, **Avertissement** ou **Conseil**. Cliquez sur une ligne pour atteindre la cellule.

Elle recherche :

- Un graphique ou une image sans **texte de remplacement** (les mots qui décrivent ce qu'elle montre). C'est une erreur.
- Un tableau sans ligne d'en-tête, de sorte que ses colonnes n'ont pas de noms. C'est une erreur.
- Des **cellules fusionnées**, difficiles à parcourir au clavier. C'est un avertissement.
- Une feuille qui porte toujours le nom qui lui a été attribué, comme Feuil1. C'est un conseil.

Cliquez sur **Vérifier à nouveau** après avoir apporté des modifications.
