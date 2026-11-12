# Compter les mots, vérifier et rechercher des mots

<!-- keywords: cellules fusionnées, texte du lien, nombre de mots, compter les mots, accessibilité, vérifier l'accessibilité, texte de remplacement, ligne d'en-tête, lecteur d'écran, contraste, dictionnaire des synonymes, synonyme, synonymes, traduire, traduction, libretranslate, statistiques, mots, caractères, paragraphes, pages, sélection, notes de bas de page, notes de fin, langue, langue de vérification, définir la langue, ne pas vérifier l'orthographe, langue de l'orthographe, barre d'état -->

Ces outils se trouvent sous l'onglet **Révision**.

## Compter les mots

1. Cliquez sur **Statistiques** dans le groupe **Vérification**.
2. La boîte affiche les pages, les mots, les caractères sans espaces, les caractères avec espaces et les paragraphes.

Si vous sélectionnez d'abord du texte, la boîte ne compte que ce texte. La barre d'état, en bas de la fenêtre, l'affiche aussi, sous la forme « 12 sur 340 mots ».

Cochez **Inclure les notes de bas de page et de fin** pour les compter également. Le texte masqué n'est jamais compté. Le texte supprimé par une modification suivie ne l'est pas non plus.

## Définir la langue du texte

JeffWrite vérifie l'orthographe avec le dictionnaire de la langue du texte. Si vous indiquez qu'une citation en français est en français, ses mots sont vérifiés en français.

1. Sélectionnez le texte.
2. Dans le groupe **Langue**, cliquez sur **Définir la langue de vérification**. Vous pouvez aussi cliquer sur le nom de la langue dans la barre d'état.
3. Choisissez une langue, puis cliquez sur **OK**.

La langue est enregistrée dans le fichier, de sorte que les autres programmes la connaissent aussi. La barre d'état indique la langue du texte où se trouve le curseur.

## Ne pas vérifier certains textes

Certains textes ne doivent pas être vérifiés : un code, un nom ou une ligne dans une langue inventée. Sélectionnez-les, puis cliquez sur **Ne pas vérifier l'orthographe** dans le groupe **Langue**. Les soulignements rouges disparaissent. Cliquez à nouveau dessus pour vérifier de nouveau le texte.

## Rechercher un mot dans le dictionnaire des synonymes

Un dictionnaire des synonymes répertorie les mots de sens presque identique.

1. Cliquez sur un mot, ou sélectionnez-le.
2. Cliquez sur **Dictionnaire des synonymes** dans le groupe **Vérification**, ou appuyez sur **Maj+F7**.
3. Choisissez un mot dans la liste, puis cliquez sur **Remplacer**. Pour rechercher l'un de ces mots à la place, cliquez sur **Rechercher**. Vous pouvez aussi saisir un autre mot dans la zone et appuyer sur **Entrée**.

La liste provient d'un fichier de votre ordinateur. Aucune connexion internet n'est utilisée.

## Traduire un texte

1. Sélectionnez le texte. Si rien n'est sélectionné, c'est le document entier qui est utilisé.
2. Cliquez sur **Traduire** dans le groupe **Langue**.
3. Choisissez la langue de destination.
4. Cliquez sur **OK**.

Votre navigateur web ouvre alors un service de traduction nommé LibreTranslate, avec votre texte. Rien n'est envoyé tant que vous ne cliquez pas sur **OK**. Vous pouvez modifier l'adresse du service dans la zone, par exemple pour utiliser un service que vous hébergez vous-même. Seules les adresses web (http ou https) sont utilisées. Au maximum 5 000 lettres sont envoyées.

## Vérifier l'accessibilité

Certaines personnes lisent avec un lecteur d'écran, qui lit le document à voix haute. D'autres ont du mal à voir un texte pâle. La vérification de l'accessibilité repère ce qui rend un document difficile à lire pour elles.

1. Cliquez sur **Vérifier l'accessibilité** dans le groupe **Vérification**.
2. Une liste s'ouvre à droite. Chaque ligne indique le problème. Cliquez sur une ligne pour atteindre l'endroit concerné.
3. Corrigez le problème, puis cliquez sur **Vérifier à nouveau**.

La vérification recherche les points suivants :

- **Une image sans texte de remplacement.** Le texte de remplacement est une courte description de l'image. Pour l'ajouter, cliquez avec le bouton droit sur l'image et choisissez son texte de remplacement.
- **Un tableau sans ligne d'en-tête.** La ligne d'en-tête contient les noms des colonnes.
- **Une cellule de tableau vide.**
- **Un titre qui saute un niveau**, par exemple un titre 3 placé directement après un titre 1.
- **Un texte difficile à lire**, parce que sa couleur est trop proche de celle de l'arrière-plan.
- **Une image flottante sur la page** au lieu d'être alignée sur le texte.
- **Un tableau comportant des cellules fusionnées.** Une cellule fusionnée est une seule cellule qui couvre deux colonnes ou lignes ou plus. Un lecteur d'écran peut perdre le fil dans un tel tableau. Ce point n'est signalé qu'une fois pour chaque tableau.
- **Un lien dont le texte n'indique pas la destination**, comme « cliquez ici » ou « en savoir plus », ou une adresse web affichée telle quelle. Une personne qui écoute la liste des liens d'une page a besoin de mots qui désignent la page.

Vous pouvez aussi lancer la vérification depuis **Fichier > Infos** : cliquez sur **Vérifier l'accessibilité**.
