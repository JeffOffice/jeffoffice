# Compter les mots, vérifier et rechercher des mots

<!-- keywords: cellules fusionnées, texte du lien, nombre de mots, compter les mots, accessibilité, vérifier l'accessibilité, texte de remplacement, ligne d'en-tête, lecteur d'écran, contraste, dictionnaire des synonymes, synonyme, synonymes, traduire, traduction, libretranslate, statistiques, compter les mots, caractères, paragraphes, pages, sélection, notes de bas de page, notes de fin, langue, langue de vérification, définir la langue, ne pas vérifier l'orthographe, langue de l'orthographe, barre d'état -->

Ces outils se trouvent dans l'onglet **Révision**.

## Compter les mots

1. Cliquez sur **Statistiques** dans le groupe **Vérification**.
2. La boîte affiche le nombre de pages, de mots, de caractères sans espaces, de caractères avec espaces et de paragraphes.

Si vous sélectionnez d'abord du texte, la boîte ne compte que ce texte. La barre d'état, en bas de la fenêtre, l'affiche aussi, sous la forme « 12 sur 340 mots ».

Cochez **Inclure les notes de bas de page et de fin** pour les compter également. Cochez **Inclure les zones de texte** pour compter les mots des zones de texte et des formes (le décompte porte sur tout le document, et non sur une sélection). Une zone de texte placée dans une autre zone de texte est également comptée, de même qu'une zone de texte située dans une note de bas de page ou de fin, si l'option **Inclure les notes de bas de page et de fin** est cochée. Les zones de texte situées dans les en-têtes et pieds de page ne sont pas comptées. Le texte masqué n'est jamais compté, pas plus que le texte qu'une modification suivie a supprimé.

## Définir la langue du texte

JeffWrite vérifie l'orthographe avec le dictionnaire de la langue dans laquelle est rédigé le texte. Si vous marquez une citation en français, ses mots sont vérifiés en français.

1. Sélectionnez le texte.
2. Dans le groupe **Langue**, cliquez sur **Définir la langue de vérification**. Vous pouvez aussi cliquer sur le nom de la langue dans la barre d'état.
3. Choisissez une langue, puis cliquez sur **OK**.

La langue est enregistrée dans le fichier, afin que les autres programmes la reconnaissent également. La barre d'état indique la langue du texte où se trouve le curseur.

## Ne pas vérifier certains textes

Certains textes ne doivent pas être vérifiés : un code, un nom ou une ligne dans une langue inventée. Sélectionnez-le, puis cliquez sur **Ne pas vérifier l'orthographe** dans le groupe **Langue**. Les soulignements rouges disparaissent. Cliquez de nouveau sur ce bouton pour que le texte soit à nouveau vérifié.

## Rechercher un mot dans le dictionnaire des synonymes

Le dictionnaire des synonymes répertorie les mots de sens presque identique.

1. Cliquez sur un mot, ou sélectionnez-le.
2. Cliquez sur **Dictionnaire des synonymes** dans le groupe **Vérification**, ou appuyez sur **Maj+F7**.
3. Choisissez un mot dans la liste, puis cliquez sur **Remplacer**. Pour rechercher l'un des mots proposés, cliquez sur **Rechercher**. Vous pouvez aussi saisir un autre mot dans la zone de texte et appuyer sur **Entrée**.

La liste provient d'un fichier présent sur votre ordinateur. Aucune connexion Internet n'est utilisée.

## Traduire un texte

Sélectionnez du texte, puis cliquez sur **Traduire** dans le groupe **Langue**. Le volet Traduction, à droite, traduit le texte sur votre ordinateur et peut remplacer le texte ou ajouter la traduction sous celui-ci. Voir [Traduire un texte](translate).

## Vérifier l'accessibilité

Certaines personnes lisent à l'aide d'un lecteur d'écran, qui énonce le document à voix haute. D'autres ont du mal à voir un texte pâle. La vérification de l'accessibilité repère ce qui rend un document difficile à lire pour elles.

1. Cliquez sur **Vérifier l'accessibilité** dans le groupe **Vérification**.
2. Une liste s'ouvre à droite. Chaque ligne indique le problème. Cliquez sur une ligne pour atteindre l'emplacement concerné.
3. Corrigez le problème, puis cliquez sur **Vérifier à nouveau**.

Elle recherche les points suivants :

- **Une image sans texte de remplacement.** Le texte de remplacement est une courte description de l'image. Pour l'ajouter, faites un clic droit sur l'image, puis choisissez son texte de remplacement.
- **Un tableau sans ligne d'en-tête.** La ligne d'en-tête contient les noms des colonnes.
- **Une cellule de tableau vide.**
- **Un titre qui saute un niveau**, par exemple un titre 3 placé directement après un titre 1.
- **Un texte difficile à lire**, parce que sa couleur est trop proche de la couleur de l'arrière-plan.
- **Une image flottante sur la page** au lieu d'être alignée sur le texte.
- **Un tableau comportant des cellules fusionnées.** Une cellule fusionnée est une cellule unique qui couvre deux colonnes ou lignes ou plus. Un lecteur d'écran peut perdre le fil dans un tel tableau. Ce point est signalé une fois pour chaque tableau.
- **Un lien dont le texte ne dit rien de sa destination**, comme « cliquez ici » ou « en savoir plus », ou une adresse web affichée telle quelle. Une personne qui écoute la liste des liens d'une page a besoin de mots qui désignent la page visée.

Vous pouvez aussi lancer la vérification depuis **Fichier > Informations** : cliquez sur **Vérifier l'accessibilité**.
