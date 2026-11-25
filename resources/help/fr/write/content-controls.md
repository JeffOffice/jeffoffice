# Créer des formulaires avec des contrôles de contenu
<!-- keywords: contrôle de contenu, contrôles de contenu, formulaire, formulaires, remplir, texte enrichi, contrôle d'image, zone de liste modifiable, section répétitive, ajouter un élément, texte brut, liste déroulante, sélecteur de date, calendrier, case à cocher, titre, balise, espace réservé, verrouiller, suppression impossible, propriétés -->

Un **contrôle de contenu** est une zone de votre texte que quelqu'un remplit : une ligne de texte, des paragraphes mis en forme, une image, un choix dans une liste, une date dans un calendrier, une case à cocher ou une section qui peut être répétée. Utilisez-les pour créer un formulaire que d'autres personnes peuvent remplir sans modifier le reste du document.

## Insérer un contrôle

1. Placez le point d'insertion à l'endroit où le contrôle doit figurer.
2. Choisissez **Insertion > Contrôle de contenu**, puis sélectionnez le type voulu :
   - **Texte enrichi :** une zone pour du texte mis en forme. Si des paragraphes sont sélectionnés, ou si vous êtes dans un paragraphe vide, le contrôle contient des paragraphes entiers, y compris des listes et des tableaux, et la touche Entrée crée un nouveau paragraphe à l'intérieur. Créé dans un paragraphe qui contient déjà du texte, il ne contient que les mots de ce paragraphe, et la touche Entrée y scinde celui-ci en deux contrôles.
   - **Texte brut :** une zone pour du texte. Elle affiche l'espace réservé « Saisissez votre texte » tant que personne n'a tapé quoi que ce soit. Si des mots sont sélectionnés, le contrôle est créé autour d'eux.
   - **Image :** un emplacement pour une seule image. Il affiche « Ajouter une image » tant qu'aucune image n'a été insérée.
   - **Zone de liste modifiable :** une liste de choix qui accepte aussi ce que vous saisissez. Elle affiche « Choisissez ou saisissez un élément » tant qu'un élément n'a pas été choisi ou saisi.
   - **Liste déroulante :** une liste de choix. Elle affiche « Choisissez un élément » tant qu'un élément n'a pas été choisi.
   - **Sélecteur de date :** une date choisie dans un calendrier. Elle affiche « Choisissez une date » tant qu'aucune date n'a été choisie.
   - **Case à cocher :** une petite case que l'on coche ou décoche.
   - **Section répétitive :** le paragraphe où se trouve le point d'insertion (ou les paragraphes sélectionnés) devient le premier élément d'une section qui peut comporter d'autres éléments.

Un contrôle reçoit une légère trame grise pour que vous puissiez voir où il commence et où il se termine. Lorsque le point d'insertion s'y trouve, il est encadré, et son titre apparaît dans un onglet au-dessus de lui. L'insertion d'un contrôle correspond à une seule étape d'annulation.

## Remplir un contrôle

- **Texte enrichi :** cliquez sur l'espace réservé et tapez votre texte. Tout ce que vous pouvez faire dans le texte, comme la mise en gras, les listes et les tableaux, est possible dans un contrôle qui contient des paragraphes.
- **Image :** cliquez sur le contrôle, ou appuyez sur Entrée lorsque le point d'insertion s'y trouve, puis choisissez un fichier image ; vous pouvez aussi glisser un fichier image sur le contrôle. Une image qui remplace une autre prend la taille de la première, en conservant les proportions. Un contrôle d'image mesure initialement 5 cm de côté. **Effacer l'image** (dans le menu du contrôle de contenu) rétablit l'espace réservé.
- **Zone de liste modifiable :** cliquez sur l'espace réservé et tapez, ou double-cliquez sur le contrôle (ou appuyez sur Alt+Flèche bas lorsque le point d'insertion s'y trouve), puis cliquez sur un élément.
- **Section répétitive :** cliquez sur le bouton **+** à la fin de la section, ou choisissez **Ajouter un élément**, pour ajouter un élément après le dernier. Chaque nouvel élément est une copie du dernier, dont tous les contrôles sont de nouveau vides. **Supprimer l'élément** retire l'élément où se trouve le point d'insertion ; une section conserve au moins un élément et en contient au maximum 1 000. Avec le suivi des modifications activé, un élément ajouté est une insertion suivie, et un élément supprimé une suppression suivie, afin qu'un réviseur puisse les voir ; **Refuser toutes les modifications** rétablit tout. Avec le suivi des modifications activé, il est impossible d'ajouter ou de supprimer un élément dans une section qui termine le document : désactivez le suivi des modifications, ou placez un paragraphe après la section.
- **Texte brut :** cliquez sur l'espace réservé et tapez. Ce que vous tapez prend sa place. Appuyez sur Maj+Entrée pour un saut de ligne ; la touche Entrée ne crée pas de nouveau paragraphe dans un contrôle.
- **Liste déroulante :** cliquez sur le contrôle (ou appuyez sur Entrée ou sur Alt+Flèche bas lorsque le point d'insertion s'y trouve), puis cliquez sur un élément. L'élément choisi est coché la prochaine fois.
- **Sélecteur de date :** cliquez sur le contrôle, puis cliquez sur un jour dans le calendrier. **Aujourd'hui** insère la date du jour. **Effacer** rétablit l'espace réservé.
- **Case à cocher :** cliquez dessus, ou appuyez sur la barre d'espace lorsque le point d'insertion s'y trouve, pour la cocher ou la décocher.

Une liste déroulante, un sélecteur de date, une case à cocher et un contrôle d'image ne contiennent que ce qui leur est attribué par un choix : vous ne pouvez pas y taper de texte.

## Modifier les propriétés d'un contrôle

Placez le point d'insertion dans le contrôle, puis choisissez **Insertion > Contrôle de contenu > Propriétés**. Vous pouvez définir :

- **Titre** et **Balise :** le titre apparaît dans l'onglet au-dessus du contrôle. La balise est un nom court que d'autres programmes peuvent utiliser pour retrouver le contrôle.
- **Éléments de la liste** (liste déroulante et zone de liste modifiable) : Ajouter, renommer (double-cliquez sur un élément), Supprimer, Monter et Descendre. Une liste contient jusqu'à 1 000 éléments.
- **Format de date** (sélecteur de date) : choisissez un format ou saisissez le vôtre, par exemple « d MMMM aaaa ». La zone affiche l'aspect de la date du jour dans ce format.
- **Le contrôle ne peut pas être supprimé :** le contrôle reste en place, mais son contenu peut être modifié.
- **Le contenu ne peut pas être modifié :** le contrôle peut être supprimé, mais son contenu ne peut pas être modifié. Sont concernés la saisie, le collage, le glisser-déplacer, Rechercher et remplacer, une correction orthographique, un nouvel aspect du texte, la Reproduction de la mise en forme, les commandes de tableau, ainsi que l'acceptation ou le refus d'une modification suivie à l'intérieur du contrôle. Une modification suivie qui existait avant le verrouillage reste à traiter ultérieurement.
- **Impossible d'ajouter ou de supprimer des éléments** (section répétitive) : Ajouter un élément et Supprimer l'élément ne font rien et indiquent la raison. Placez le point d'insertion dans un élément quelconque pour afficher cette boîte pour l'ensemble de la section.

Choisissez **OK** pour appliquer les modifications en une seule étape d'annulation.

## Bon à savoir

- Un contrôle qui contient des paragraphes entiers conserve son texte séparé des paragraphes qui l'entourent. S'il ne peut pas être supprimé, ou si son contenu ne peut pas être modifié, les touches Retour arrière et Suppr à son bord ne font rien, car elles feraient entrer des mots dans le contrôle ou en sortir. S'il n'est pas verrouillé, la fusion d'un paragraphe à travers son bord place le paragraphe fusionné du côté où se trouve le point d'insertion, et le contrôle conserve ses autres paragraphes ; un contrôle ne comportant qu'un paragraphe n'est jamais vidé, de sorte que le paragraphe fusionné reste en place.
- Un verrouillage appliqué à un contrôle qui englobe des lignes ou des cellules de tableau entières est conservé dans le fichier, mais JeffWrite ne le gère pas. Il en va de même pour un contrôle qui commence par un tableau.
- Un document protégé pour le remplissage de formulaires est entièrement en lecture seule : aucun contrôle ne peut y être rempli, et la touche Tab ne permet pas de passer d'un contrôle à l'autre.
- Un document peut contenir jusqu'à 10 000 contrôles, et des contrôles peuvent être imbriqués sur 8 niveaux au maximum. Une section répétitive et ses éléments comptent comme des contrôles.
- Les contrôles présents dans des fichiers provenant d'autres programmes sont affichés et conservés tels quels. Un type que JeffWrite ne sait pas créer (comme une galerie de blocs de construction, une équation ou un groupe) est conservé à sa place ; vous pouvez modifier son titre, sa balise et ses verrouillages, mais pas son contenu.
- Les contrôles sont enregistrés dans les fichiers `.docx`. Dans les autres types de fichiers, le texte est conservé, mais le contrôle ne l'est pas. **Vérifier la compatibilité** l'indique et précise les types sans équivalent : sections répétitives, contrôles d'image et zones de liste modifiables.
