# Notes et liens
<!-- keywords: thesaurus, synonymes, liste de mots, note, notes, commentaire, commentaires, survol, marque rouge, coin, nouvelle note, modifier la note, supprimer la note, afficher la note, masquer la note, afficher toutes les notes, lien, liens, lien hypertexte, adresse, page web, e-mail, emplacement dans ce classeur, Ctrl+K, Ctrl+clic, info-bulle, ouvrir le lien, supprimer le lien, conversation, statistiques du classeur, compter, cellules, formules, graphiques, tableaux croisés dynamiques, accessibilité, texte de remplacement, cellules fusionnées, fichiers récents -->

## Notes

Une **note** est un court texte associé à une cellule. Une cellule qui en contient une affiche un petit repère rouge dans son coin supérieur droit. Placez le pointeur sur la cellule, et la note apparaît à côté ; éloignez le pointeur, et elle disparaît.

Pour ajouter une note à une cellule, sélectionnez-la et utilisez **Révision > Nouvelle note** (ou appuyez sur **Maj+F2**). Saisissez le texte, puis cliquez sur **OK**. Sur une cellule qui contient déjà une note, la même commande s'intitule **Modifier la note**. Si vous effacez le texte, la note disparaît.

- **Supprimer la note** supprime les notes des cellules sélectionnées.
- **Afficher ou masquer la note** garde la note des cellules sélectionnées affichée en permanence sur la feuille, dans une bulle à côté de chaque cellule, ou la masque à nouveau.
- **Afficher toutes les notes** fait de même pour toutes les notes de la feuille ; le bouton devient alors **Masquer toutes les notes**.
- **Note précédente** et **Note suivante** vous permettent de passer d'une cellule contenant une note à la suivante.

Chacune de ces actions peut être annulée en une seule étape. Une note suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque les cellules sont triées. **Effacer tout** supprime les notes des cellules qu'il efface.

Un classeur peut contenir une conversation de commentaires, avec des réponses. JeffCrunch affiche l'ensemble de la conversation sous forme de note et la conserve à l'enregistrement, mais ne la modifie pas : la note est en lecture seule.

Les notes peuvent être enregistrées dans un classeur .xlsx. Un classeur ouvert à partir d'un ancien type de fichier, ou d'un fichier texte, ne conserve ses notes qu'après avoir été enregistré au format de classeur .xlsx.

## Liens

Un **lien** vous envoie quelque part lorsque vous le suivez : vers une page web, vers un fichier, vers une adresse e-mail ou vers un emplacement de ce classeur (une cellule, des cellules ou un nom).

Pour ajouter un lien à une cellule, sélectionnez-la et utilisez **Insertion > Lien** (ou appuyez sur **Ctrl+K**). Choisissez la destination, saisissez l'adresse ou l'emplacement (ou sélectionnez l'un des fichiers ouverts récemment dans **Ou un fichier récent**), puis, si vous le souhaitez, le texte que la cellule doit afficher et une **info-bulle**. Sur une cellule qui contient déjà un lien, la même commande s'intitule **Modifier le lien**. **Supprimer le lien** supprime les liens des cellules sélectionnées.

Placez le pointeur sur une cellule contenant un lien pour voir où il mène. Pour le suivre, maintenez la touche **Ctrl** enfoncée et cliquez sur la cellule, ou sélectionnez la cellule et utilisez **Ouvrir le lien** dans le menu contextuel. Un simple clic se contente de sélectionner la cellule.

JeffCrunch n'ouvre jamais rien de lui-même. Une page web ou une adresse e-mail s'ouvre dans votre programme habituel lorsque vous suivez son lien. Un lien vers un fichier, ou vers un autre type d'adresse, vous demande d'abord confirmation. Un lien qui exécuterait un script n'est jamais ouvert.

Un lien suit sa cellule lorsque des lignes ou des colonnes sont insérées ou supprimées, et lorsque les cellules sont triées ; il est enregistré avec le classeur.

## Statistiques du classeur

**Statistiques du classeur** (onglet **Révision**) compte ce que contient le classeur : le nombre de **Feuilles**, le nombre de **Cellules avec données** (cellules contenant un nombre, un texte ou une formule), le nombre de **Formules**, le nombre de **Graphiques** et le nombre de **Tableaux croisés dynamiques**. Les feuilles supprimées ne sont pas comptées. Les mêmes chiffres figurent dans la page **Informations** de l'onglet Fichier.

## Dictionnaire des synonymes

**Dictionnaire des synonymes** (onglet **Révision**, ou **Maj+F7**) recherche des mots de sens presque identique à un mot figurant dans une cellule. Il examine le premier mot du texte de la cellule active. Saisissez un autre mot dans **Rechercher** et appuyez sur **Entrée** pour rechercher celui-ci à la place. Sélectionnez un mot dans la liste, puis cliquez sur **Remplacer**, ou double-cliquez dessus. Le mot de la cellule est remplacé, et une majuscule initiale est conservée. **Rechercher** recherche le mot que vous avez choisi, afin de suivre une chaîne de sens. La liste de mots fonctionne en anglais.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** (onglet **Révision**) ouvre une petite fenêtre qui liste les éléments du classeur difficiles à utiliser pour certains lecteurs, ou pour un lecteur d'écran (un programme qui lit l'écran à voix haute). Chaque ligne commence par **Erreur**, **Avertissement** ou **Conseil**. Cliquez sur une ligne pour atteindre la cellule concernée.

Il recherche :

- Un graphique ou une image sans **texte de remplacement** (les mots qui décrivent ce qu'elle montre). C'est une erreur.
- Un tableau sans ligne d'en-tête, donc dont les colonnes n'ont pas de nom. C'est une erreur.
- Des **cellules fusionnées**, difficiles à parcourir au clavier. C'est un avertissement.
- Un texte difficile à lire sur la couleur de fond, comme un texte gris clair sur fond blanc. JeffCrunch mesure le contraste du texte par rapport à son remplissage. Un petit texte doit présenter un rapport d'au moins 4,5 pour 1, et un grand texte (18 points, ou 14 points en gras) un rapport de 3 pour 1. Il indique la première cellule de chaque combinaison de police et de remplissage. C'est un avertissement.
- Une feuille qui conserve le nom qui lui a été attribué à l'origine, comme Feuil1. C'est un conseil.

Cliquez sur **Vérifier à nouveau** après avoir apporté des modifications.
