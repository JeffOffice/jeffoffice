# Modifier les paramètres de JeffCrunch
<!-- keywords: récupération, récupérer le travail non enregistré, récupération automatique, vérification, modification, saisie semi-automatique, complétion automatique, poignée de recopie, glisser-déplacer, nombre de décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos, modifier directement dans la cellule, couleur du quadrillage, zoom par défaut, largeur standard, confidentialité, informations personnelles, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, rétablir le ruban, modifier les raccourcis clavier, réattribuer des touches, rétablir les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Modification

La page **Modification** change le fonctionnement de la saisie dans les cellules.

- **Après validation, déplacer la sélection** indique où va la sélection lorsque vous terminez une saisie : vers le bas, vers la droite, vers le haut, vers la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules situées au-dessus et en dessous** s'appelle la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par elles, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne tient compte que du bloc de cellules à côté de celle où vous tapez, et uniquement des textes, pas des nombres.
- **Utiliser la poignée de recopie et faire glisser les cellules pour les déplacer ou les copier** active le petit carré situé à l'angle de la sélection et le glissement de la bordure de la sélection. Désactivez-le si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Afficher un zéro dans les cellules dont la valeur est zéro (cette feuille)** affiche ou masque les zéros dans la feuille que vous consultez. Lorsqu'il est désactivé, une cellule contenant 0 paraît vide, mais elle contient toujours le 0 et est comptée comme 0. Ce choix est enregistré avec la feuille.
- **Ajouter une virgule décimale aux nombres entiers saisis** s'appelle la saisie automatique des décimales. Si vous indiquez **Décimales** à 2, la saisie de 1234 donne 12,34. Si vous indiquez -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule décimale est enregistré tel quel.
- **Autoriser la modification directe dans les cellules** est activé par défaut. Lorsque vous le désactivez, une saisie ou la touche F2 passe dans la barre de formule, et la cellule n'affiche le résultat qu'à la fin de la saisie.

## Options d'enregistrement

**Enregistrer les informations de récupération automatique toutes les** définit à quelle fréquence JeffCrunch conserve une copie d'un classeur comportant des modifications non enregistrées, de 1 à 120 minutes. La copie est un fichier .xlsx placé dans un dossier que JeffCrunch réserve à cet usage ; elle est créée en arrière-plan pour que vous puissiez continuer à taper. Elle est supprimée lorsque vous enregistrez ou fermez le classeur. Si JeffCrunch se ferme brusquement, il vous propose ces copies au prochain démarrage. Vous pouvez aussi cliquer sur **Fichier > Ouvrir > Récupérer le travail non enregistré**. Un classeur protégé par un mot de passe n'est pas copié, car la copie ne serait pas protégée.

## Options de vérification

La page **Vérification** est commune aux applications JeffOffice. Elle détermine si l'orthographe est vérifiée pendant la saisie dans une cellule (désactivez **Vérifier l'orthographe lors de la frappe** pour supprimer les traits rouges ondulés dans toutes les fenêtres ouvertes et dans les autres applications ; **Orthographe** dans l'onglet Révision fonctionne toujours), si les mots en majuscules et les mots contenant des chiffres sont ignorés lors de la vérification, si la correction automatique corrige les mots pendant la frappe, si les guillemets droits deviennent des guillemets typographiques, si les tirets, les fractions et les indicateurs ordinaux sont modifiés, et si une adresse web devient un lien. Voir [Orthographe](notes-links) et [Saisir dans les cellules](edit-cells).

## Options d'affichage

- **Couleur du quadrillage** est la couleur des lignes entre les cellules à l'écran. Ce choix ne concerne que votre écran et n'est pas enregistré dans le classeur.
- **Zoom d'un nouveau classeur** définit la taille d'affichage des feuilles d'un nouveau classeur, de 10 % à 400 %.
- **Largeur de colonne standard (cette feuille)** est la largeur de chaque colonne de cette feuille qui n'a pas de largeur propre. Elle correspond à **Largeur par défaut** de l'onglet Accueil.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages, **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les touches de raccourci des commandes de JeffCrunch. Voir « Choisir vos propres touches de raccourci » dans la rubrique sur le clavier.

### Personnaliser le ruban

La page **Ruban** affiche les onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case à côté pour le masquer, et cochez-la pour l'afficher de nouveau. Un groupe masqué ne prend pas de place, et ses commandes restent accessibles par leurs touches de raccourci. **Accueil** n'a pas de case, car il reste toujours affiché, de même que le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve à la fin de la liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Saisissez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut figurer dans n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Ajouter des commandes à votre groupe.** Choisissez votre groupe, recherchez une commande dans la liste **Commandes** (saisissez dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande ayant un nom peut être ajoutée. Une commande ne peut figurer qu'une fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande personnel, saisissez le nouveau nom, puis cliquez sur **Renommer**. Une commande renommée fait la même chose ; le nouveau nom n'apparaît que sur son bouton. Pour rétablir le nom d'origine d'une commande, renommez-la avec un nom vide.
- **Réorganiser.** Choisissez un onglet, puis cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets du ruban comme les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même façon dans leur onglet ou leur groupe. Les groupes du ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande personnel, puis cliquez sur **Supprimer**.

**Rétablir un élément** remet en place l'onglet choisi : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés disparaissent. Un onglet personnel est supprimé. **Tout rétablir** remet en place l'ensemble du ruban, y compris l'ordre des onglets.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch affichent votre ruban, de même que toutes les fenêtres que vous ouvrirez plus tard. JeffCrunch ne conserve que ce qui diffère du ruban habituel ; ainsi, un onglet ou un groupe ajouté par une version ultérieure apparaît automatiquement. Vos onglets et vos groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne se confondent pas avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche indique ce qui figure actuellement sur la barre, et la liste **Commandes** à droite contient toutes les commandes ayant un nom (saisissez dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes**, puis cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre. Une commande qui ouvre un menu conserve son menu sur la barre. Une commande ne peut figurer qu'une fois sur la barre.
- **Clic droit.** Vous pouvez aussi faire un clic droit sur une commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou faire un clic droit sur un bouton de la barre et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page.
- **Placer la barre sous le ruban.** Cochez **Afficher la barre d'outils rapide sous le ruban** pour la placer sur une ligne sous le ruban, au lieu du coin supérieur gauche. Les deux menus du clic droit contiennent aussi **Afficher la barre d'outils rapide sous le ruban**. JeffCrunch mémorise ce choix.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour ajouter un trait fin à la fin de la barre, puis déplacez-le à l'endroit voulu.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le, puis cliquez sur **Supprimer**.

**Rétablir un élément** remet en place la commande choisie : à l'endroit où JeffCrunch la place habituellement, ou hors de la barre si JeffCrunch ne l'y place pas habituellement. **Tout rétablir** remet en place la barre d'outils livrée avec JeffCrunch. Tout rétablir sur cette page ne modifie pas votre ruban, et tout rétablir sur la page **Ruban** ne modifie pas votre barre d'outils.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch affichent votre barre d'outils, de même que toutes les fenêtres que vous ouvrirez plus tard. La barre peut comporter au total 100 commandes et séparateurs. Lorsque les boutons ne tiennent pas dans un tiers de la largeur de la fenêtre, les autres sont placés derrière un bouton **»** à la fin de la barre, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffCrunch conserve dans votre liste une commande qu'une version ultérieure ne contient plus, et l'affiche de nouveau si elle revient. Les boutons reçoivent des info-touches : les chiffres de 1 à 9, puis deux lettres, dans l'ordre où ils figurent sur la barre. La barre d'outils se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Options de confidentialité

- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire l'auteur, la personne ayant enregistré le fichier en dernier, la société, le responsable, ainsi que les noms et les dates des notes et des commentaires, dans le fichier écrit (un classeur, une copie, et un classeur envoyé par e-mail). Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier sont conservées telles quelles, et le document ouvert n'est pas modifié. Ce choix est identique dans toutes les applications JeffOffice : le modifier dans l'une le modifie dans toutes les autres. Pour ne le faire qu'une fois, utilisez **Inspecter le classeur** dans [Informations](info). La même case se trouve sur la page **Informations**.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des données sur l'utilisation de JeffCrunch. Anonyme signifie que ces données ne disent rien sur votre identité. Lorsque cette option est activée, JeffCrunch envoie ces données une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffCrunch, sa version, votre système d'exploitation et votre langue, le nombre de fois où il a été démarré, la fréquence d'utilisation de chaque commande, et le type de fichier ouvert ou enregistré. Si JeffCrunch se ferme brusquement, il envoie également un rapport d'erreur au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport d'erreur n'est envoyé non plus.

Le choix concerne toutes les applications JeffOffice : le modifier ici le modifie dans chacune d'elles. JeffCrunch vous pose la question au premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où, et pendant combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** est affiché sous la case à cocher. Il s'agit d'un nombre aléatoire créé pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne contient aucune information sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été créé, il indique **aucun**.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur ; sans lui, nous ne pouvons pas savoir quels enregistrements vous appartiennent.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffCrunch ne pose pas la question du premier démarrage.

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
