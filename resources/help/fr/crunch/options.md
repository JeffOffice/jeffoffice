# Modifier les paramètres de JeffCrunch
<!-- keywords: récupération, récupérer le travail non enregistré, récupération automatique, vérification, modification, saisie semi-automatique, poignée de recopie, glisser-déplacer, décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos, modifier directement dans la cellule, couleur du quadrillage, zoom par défaut, largeur standard, informations personnelles, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, rétablir le ruban, modifier les raccourcis clavier, réattribuer des touches, réinitialiser les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Modification

La page **Modification** modifie le fonctionnement de la saisie dans les cellules.

- **Après validation, déplacer la sélection** indique où va la sélection lorsque vous terminez une saisie : vers le bas, la droite, le haut, la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules au-dessus et en dessous** s'appelle la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par elles, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne regarde que le bloc de cellules situé à côté de celle où vous tapez, et seulement les textes, pas les nombres.
- **Utiliser la poignée de recopie et glisser les cellules pour les déplacer ou les copier** affiche le petit carré à l'angle de la sélection et permet de faire glisser le contour de la sélection. Désactivez-la si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Afficher un zéro dans les cellules dont la valeur est nulle (cette feuille)** affiche ou masque les zéros dans la feuille que vous consultez. Lorsque cette option est désactivée, une cellule contenant 0 paraît vide, mais elle contient toujours le 0 et est comptée comme 0. Ce choix est enregistré avec la feuille.
- **Ajouter une virgule décimale aux nombres entiers saisis** s'appelle les décimales automatiques. Si **Décimales** vaut 2, la saisie de 1234 donne 12,34. Si elle vaut -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule décimale est enregistré tel que vous l'avez tapé.

- **Autoriser la modification directement dans les cellules** est activée par défaut. Lorsque vous la désactivez, une saisie ou la touche F2 se fait dans la barre de formule, et la cellule n'affiche le résultat qu'à la validation.

## Options d'enregistrement

**Enregistrer les informations de récupération automatique toutes les** définit à quelle fréquence JeffCrunch conserve une copie d'un classeur contenant des modifications non enregistrées, de 1 à 120 minutes. La copie est un fichier .xlsx placé dans un dossier que JeffCrunch réserve à cet usage, créé en arrière-plan pour que vous puissiez continuer à taper. Elle est supprimée lorsque vous enregistrez ou fermez le classeur. Si JeffCrunch se ferme de façon inattendue, il propose les copies au prochain démarrage. Vous pouvez aussi cliquer sur **Fichier > Ouvrir > Récupérer le travail non enregistré**. Un classeur protégé par un mot de passe n'est pas copié, car la copie ne serait pas protégée.

## Options de vérification

La page **Vérification** est commune aux applications JeffOffice. Elle détermine si l'orthographe est vérifiée pendant la frappe dans une cellule (désactivez **Vérifier l'orthographe lors de la frappe** pour supprimer les traits rouges ondulés dans toutes les fenêtres ouvertes et dans les autres applications ; **Orthographe** dans l'onglet Révision fonctionne toujours), si les mots en majuscules et les mots contenant des chiffres sont ignorés lors de la vérification, si la correction automatique corrige les mots pendant la frappe, si les guillemets droits deviennent typographiques, si les tirets, les fractions et les ordinaux sont remplacés, et si une adresse web devient un lien. Voir [Orthographe](notes-links) et [Saisir dans les cellules](edit-cells).

## Options d'affichage

- **Couleur du quadrillage** est la couleur des traits qui séparent les cellules à l'écran. Ce choix ne concerne que votre écran et n'est pas enregistré dans le classeur.
- **Zoom d'un nouveau classeur** indique la taille d'affichage des feuilles d'un nouveau classeur, de 10 % à 400 %.
- **Largeur de colonne standard (cette feuille)** est la largeur de chaque colonne de cette feuille qui n'a pas de largeur propre. C'est la même chose que **Largeur par défaut** dans l'onglet Accueil.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages, **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les touches de raccourci des commandes de JeffCrunch. Voir « Choisir vos propres touches de raccourci » dans la rubrique sur le clavier.

### Personnaliser le ruban

La page **Ruban** affiche la liste des onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case à côté pour le masquer, et cochez-la pour l'afficher à nouveau. Un groupe masqué ne prend pas de place, et ses commandes restent accessibles par leurs touches de raccourci. **Accueil** n'a pas de case, car il reste toujours affiché, de même que le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve à la fin de la liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Tapez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut être placé sur n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Ajouter des commandes à votre groupe.** Choisissez votre groupe, recherchez une commande dans la liste **Commandes** (tapez dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande ayant un nom peut être ajoutée. Une commande ne peut figurer qu'une fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande personnel, tapez le nouveau nom, puis cliquez sur **Renommer**. Une commande renommée fait la même chose, et le nouveau nom n'apparaît que sur son bouton. Pour renommer une commande avec un nom vide, lui rendre son nom d'origine.
- **Réorganiser.** Choisissez un onglet et cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets du ruban comme les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même manière à l'intérieur de leur onglet ou de leur groupe. Les groupes du ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande personnel, puis cliquez sur **Supprimer**.

**Réinitialiser un élément** rétablit l'onglet choisi : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés disparaissent. Un onglet personnel est supprimé. **Tout réinitialiser** rétablit l'ensemble du ruban, y compris l'ordre des onglets.

Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch affichent votre ruban, de même que toutes les fenêtres ouvertes plus tard. JeffCrunch ne conserve que ce qui diffère du ruban habituel : un onglet ou un groupe ajouté par une version ultérieure apparaît donc de lui-même. Vos onglets et vos groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne se confondent pas avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche montre ce qui figure actuellement sur la barre, et la liste **Commandes** de droite comprend toutes les commandes ayant un nom (tapez dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes** et cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre. Une commande qui ouvre un menu garde son menu sur la barre. Une commande ne peut figurer qu'une fois sur la barre.
- **Clic droit.** Vous pouvez aussi faire un clic droit sur une commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou faire un clic droit sur un bouton de la barre et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour placer un trait fin à la fin de la barre, puis déplacez-le où vous le souhaitez.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le et cliquez sur **Supprimer**.

**Réinitialiser un élément** rétablit la commande choisie : à l'emplacement habituel de JeffCrunch, ou hors de la barre si JeffCrunch ne la place pas habituellement là. **Tout réinitialiser** rétablit la barre d'outils livrée avec JeffCrunch. Sur cette page, Tout réinitialiser ne modifie pas votre ruban, et sur la page **Ruban**, Tout réinitialiser ne modifie pas votre barre d'outils.

Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch affichent votre barre d'outils, de même que toutes les fenêtres ouvertes plus tard. La barre peut comporter 100 commandes et séparateurs au total. Lorsque les boutons ne tiennent pas dans un tiers de la largeur de la fenêtre, les autres sont placés derrière un bouton **»** à la fin de la barre, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffCrunch conserve dans votre liste une commande qu'une version ultérieure ne propose plus, et la réaffiche si elle revient. Les boutons reçoivent des info-touches, les chiffres de 1 à 9 puis deux lettres, dans l'ordre où ils figurent sur la barre. La barre se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Options de confidentialité

- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire l'auteur, la personne ayant enregistré le fichier en dernier, la société, le responsable, ainsi que les noms et les heures des notes et des commentaires, du fichier écrit (un classeur, une copie, et un classeur envoyé par courrier électronique). Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier sont conservées telles quelles, et le document ouvert n'est pas modifié. Ce choix est identique dans toutes les applications JeffOffice : le modifier dans l'une le modifie dans toutes les autres. Pour ne le faire qu'une fois, utilisez **Inspecter le classeur** dans [Informations](info). La même case se trouve sur la page **Informations**.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** permet de partager des informations sur la façon dont JeffCrunch est utilisé. Anonymes signifie que ces informations ne disent rien de votre identité. Lorsque cette option est activée, JeffCrunch envoie ces informations une fois par jour, et le jour de la mise à jour : qu'il s'agit de JeffCrunch, sa version, votre système d'exploitation et votre langue, le nombre de fois où il a été démarré, la fréquence d'utilisation de chaque commande, et le type de fichier ouvert ou enregistré. Si JeffCrunch se ferme de façon inattendue, il envoie aussi un rapport de plantage au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms, ni rien de leur contenu, ni rien de ce que vous avez tapé. Si cette option est désactivée, aucun rapport de plantage n'est envoyé non plus.

Ce choix s'applique à toutes les applications JeffOffice : le modifier ici le modifie dans chacune d'elles. JeffCrunch vous pose la question au premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où, et pendant combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** s'affiche sous la case à cocher. C'est un nombre aléatoire créé pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien de vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été créé, la mention affichée est **aucun**.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans lui nous ne pouvons pas savoir quels enregistrements vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffCrunch ne pose pas la question du premier démarrage.

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
