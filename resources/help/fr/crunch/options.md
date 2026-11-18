# Modifier les paramètres de JeffCrunch
<!-- keywords: récupération, récupérer le travail non enregistré, récupération automatique, vérification, modification, saisie semi-automatique, poignée de recopie, glisser-déplacer, décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos, modifier dans la cellule, couleur du quadrillage, zoom par défaut, largeur standard, informations personnelles, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, rétablir le ruban, modifier les raccourcis clavier, réattribuer des touches, rétablir les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

**Afficher la mini barre d'outils à la sélection** est activée par défaut. Elle détermine si la petite barre de mise en forme s'affiche à côté du texte sélectionné et en haut du menu contextuel des cellules (voir [Saisir du texte dans les cellules](edit-cells)). Ce choix est partagé par les applications JeffOffice.

## Modification

La page **Modification** modifie le fonctionnement de la saisie dans les cellules.

- **Après validation, déplacer la sélection** indique où va la sélection lorsque vous terminez une saisie : vers le bas, la droite, le haut, la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules au-dessus et en dessous** correspond à la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par ces lettres, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne tient compte que du bloc de cellules voisin de celle où vous saisissez, et uniquement des textes, pas des nombres.
- **Utiliser la poignée de recopie et faire glisser les cellules pour les déplacer ou les copier** active le petit carré situé à l'angle de la sélection ainsi que le glissement de la bordure de la sélection. Désactivez cette option si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Afficher un zéro dans les cellules qui ont une valeur nulle (cette feuille)** affiche ou masque les zéros pour la feuille que vous consultez. Lorsque cette option est désactivée, une cellule contenant 0 paraît vide, mais elle contient toujours le 0 et est comptée comme 0. Ce choix est enregistré avec la feuille.
- **Placer une virgule décimale dans les nombres entiers saisis** correspond à la saisie automatique de décimales. Si **Décimales** est égal à 2, la saisie de 1234 donne 12,34. S'il est égal à -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule décimale est enregistré tel qu'il est saisi.

- **Autoriser la modification directement dans les cellules** est activée par défaut. Lorsque vous la désactivez, une saisie ou la touche F2 se fait dans la barre de formule, et la cellule n'affiche le résultat qu'à la fin de la saisie.

## Options d'enregistrement

**Enregistrer les informations de récupération automatique toutes les** détermine la fréquence à laquelle JeffCrunch conserve une copie d'un classeur contenant des modifications non enregistrées, de 1 à 120 minutes. La copie est un fichier .xlsx placé dans un dossier que JeffCrunch réserve à cet usage. Elle est créée en arrière-plan, pour que vous puissiez continuer à taper. Elle est supprimée lorsque vous enregistrez ou fermez le classeur. Si JeffCrunch se ferme de façon inattendue, il vous propose ces copies au prochain démarrage. Vous pouvez aussi cliquer sur **Fichier > Ouvrir > Récupérer le travail non enregistré**. Un classeur protégé par un mot de passe n'est pas copié, car la copie ne serait pas protégée.

## Options de vérification

La page **Vérification** est partagée par les applications JeffOffice. Elle détermine si l'orthographe est vérifiée au fur et à mesure de la saisie dans une cellule (désactivez **Vérifier l'orthographe lors de la frappe** pour supprimer les traits rouges ondulés dans toutes les fenêtres ouvertes et dans les autres applications ; **Orthographe** dans l'onglet Révision fonctionne toujours), si les mots en majuscules et les mots contenant des chiffres sont ignorés lors de la vérification, si la correction automatique corrige les mots pendant la saisie, si les guillemets droits deviennent typographiques, si les tirets, les fractions et les ordinaux sont modifiés, et si une adresse web devient un lien. Voir [Orthographe](notes-links) et [Saisir du texte dans les cellules](edit-cells).

## Options d'affichage

- **Couleur du quadrillage** est la couleur des traits entre les cellules à l'écran. Ce choix ne concerne que votre écran et n'est pas enregistré dans le classeur.
- **Zoom d'un nouveau classeur** détermine la taille d'affichage des feuilles d'un nouveau classeur, de 10 % à 400 %.
- **Largeur de colonne standard (cette feuille)** est la largeur de chaque colonne de cette feuille qui n'a pas de largeur propre. Elle correspond à **Largeur par défaut** dans l'onglet Accueil.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages, **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les raccourcis clavier des commandes de JeffCrunch. Voir « Choisir vos propres raccourcis clavier » dans la rubrique consacrée au clavier.

### Personnaliser le ruban

La page **Ruban** affiche les onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case à côté pour le masquer, et cochez-la pour le réafficher. Un groupe masqué ne prend pas de place, et ses commandes restent accessibles par leurs raccourcis clavier. **Accueil** n'a pas de case, car il reste toujours affiché, et il en va de même pour le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve à la fin de la liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Saisissez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut figurer sur n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Placer des commandes dans votre groupe.** Choisissez votre groupe, recherchez une commande dans la liste **Commandes** (saisissez dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande ayant un nom peut être ajoutée. Une commande ne peut figurer qu'une fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande personnelle, saisissez le nouveau nom, puis cliquez sur **Renommer**. Une commande renommée fait la même chose, et le nouveau nom est seulement celui de son bouton. Pour rétablir le nom d'origine d'une commande, renommez-la avec un nom vide.
- **Réorganiser.** Choisissez un onglet et cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets du ruban comme les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même façon à l'intérieur de leur onglet ou de leur groupe. Les groupes du ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande personnelle, puis cliquez sur **Supprimer**.

**Rétablir un** remet l'onglet choisi dans son état d'origine : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés disparaissent. Un onglet personnel est supprimé. **Tout rétablir** remet l'ensemble du ruban dans son état d'origine, y compris l'ordre des onglets.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch affichent votre ruban, ainsi que toutes les fenêtres que vous ouvrirez plus tard. JeffCrunch ne conserve que ce qui diffère du ruban habituel, de sorte qu'un onglet ou un groupe ajouté par une version ultérieure apparaît automatiquement. Vos onglets et groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne se confondent pas avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche affiche ce qui se trouve actuellement sur la barre d'outils, et la liste **Commandes** de droite contient toutes les commandes ayant un nom (saisissez dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes**, puis cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre d'outils. Une commande qui ouvre un menu conserve son menu sur la barre d'outils. Une commande ne peut figurer qu'une fois sur la barre d'outils.
- **Clic droit.** Vous pouvez aussi faire un clic droit sur une commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou faire un clic droit sur un bouton de la barre d'outils et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page.
- **Placer la barre sous le ruban.** Cochez **Afficher la barre d'outils rapide sous le ruban** pour placer la barre d'outils sur une ligne sous le ruban, au lieu de la placer en haut à gauche. Les deux menus du clic droit proposent aussi **Afficher la barre d'outils rapide sous le ruban**. JeffCrunch mémorise ce choix.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour placer un trait fin à la fin, puis déplacez-le où vous le souhaitez.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le, puis cliquez sur **Supprimer**.

**Rétablir un** remet la commande choisie à son emplacement habituel, ou la retire de la barre d'outils si JeffCrunch ne la place pas habituellement dans celle-ci. **Tout rétablir** remet la barre d'outils livrée avec JeffCrunch. Tout rétablir sur cette page ne modifie pas votre ruban, et Tout rétablir sur la page **Ruban** ne modifie pas votre barre d'outils.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffCrunch affichent votre barre d'outils, ainsi que toutes les fenêtres que vous ouvrirez plus tard. La barre d'outils peut comporter 100 commandes et séparateurs au total. Lorsqu'il y a plus de boutons qu'il n'en tient dans un tiers de la largeur de la fenêtre, les autres sont placés derrière un bouton **»** à la fin de la barre d'outils, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffCrunch conserve dans votre liste une commande qu'une version ultérieure ne propose plus, et l'affiche de nouveau si elle revient. Les boutons reçoivent des info-touches, les chiffres de 1 à 9 puis deux lettres, dans l'ordre où ils figurent sur la barre d'outils. La barre d'outils se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Options de confidentialité

- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire l'auteur, la personne ayant enregistré le fichier en dernier, la société, le responsable, ainsi que les noms et les dates des notes et commentaires, du fichier écrit (un classeur, une copie et un classeur envoyé par e-mail). Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier ne sont pas modifiées, et le document ouvert n'est pas modifié. Ce choix est identique dans toutes les applications JeffOffice : le modifier dans l'une le modifie dans toutes les autres. Pour effectuer cette opération une seule fois, utilisez **Inspecter le classeur** dans [Informations](info). La même case se trouve sur la page **Informations**.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des données sur l'utilisation de JeffCrunch. Anonymes signifie que ces données ne disent rien sur votre identité. Lorsque cette option est activée, JeffCrunch envoie ces données une fois par jour, ainsi que le jour de la mise à jour : le fait qu'il s'agit de JeffCrunch, sa version, votre système d'exploitation et votre langue, le nombre de démarrages, la fréquence d'utilisation de chaque commande, et le type de fichier que vous avez ouvert ou enregistré. Si JeffCrunch se ferme de façon inattendue, il envoie aussi un rapport de plantage au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms ou leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport de plantage n'est envoyé non plus.

Ce choix concerne toutes les applications JeffOffice : le modifier ici le modifie dans chacune d'elles. JeffCrunch vous pose la question dès le premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où cela est transmis et combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** est affiché sous la case à cocher. C'est un nombre aléatoire créé pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été créé, il affiche **aucun**.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans ce numéro, nous ne pouvons pas savoir quels enregistrements vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors comptabilisé ni envoyé, et JeffCrunch ne pose pas la question du premier démarrage.

## Sécurité

L'onglet **Sécurité** détermine ce que JeffCrunch fait des éléments vers lesquels un fichier peut pointer : fichiers liés, films et sons liés, et fichiers provenant d'Internet. Les macros sont conservées dans le fichier et ne sont jamais exécutées, et un lien qui lancerait un programme est toujours refusé. Voir [Sécurité](security).

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
