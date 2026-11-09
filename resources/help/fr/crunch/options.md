# Modifier les paramètres de JeffCrunch
<!-- keywords: récupération, récupérer le travail non enregistré, récupération automatique, vérification, modification, saisie semi-automatique, poignée de recopie, glisser-déplacer, décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos, modifier directement dans les cellules, couleur du quadrillage, zoom par défaut, largeur standard, confidentialité, informations personnelles -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Modification

La page **Modification** modifie le fonctionnement de la saisie dans les cellules.

- **Déplacer la sélection après validation** indique où va la sélection lorsque vous terminez une saisie : vers le bas, vers la droite, vers le haut, vers la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules situées au-dessus et en dessous** correspond à la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par elles, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne regarde que le bloc de cellules situé à côté de celle où vous saisissez, et uniquement les textes, pas les nombres.
- **Utiliser la poignée de recopie et faire glisser les cellules pour les déplacer ou les copier** affiche le petit carré situé dans le coin de la sélection et permet de faire glisser la bordure de la sélection. Désactivez cette option si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Afficher un zéro dans les cellules dont la valeur est zéro (cette feuille)** affiche ou masque les zéros pour la feuille que vous consultez. Lorsqu'il est masqué, une cellule contenant 0 paraît vide, mais elle contient toujours le 0 et est comptée comme 0. Ce choix est enregistré avec la feuille.
- **Placer une virgule décimale dans les nombres entiers saisis** correspond à la saisie automatique des décimales. Si **Décimales** vaut 2, la saisie de 1234 donne 12,34. Si la valeur est -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule décimale est enregistré tel que vous l'avez tapé.

- **Autoriser la modification directement dans les cellules** est activé par défaut. Lorsque vous le désactivez, une saisie ou la touche F2 se fait dans la barre de formule, et la cellule n'affiche le résultat qu'à la fin de la saisie.

## Options d'enregistrement

**Enregistrer les informations de récupération automatique toutes les** détermine la fréquence à laquelle JeffCrunch conserve une copie d'un classeur contenant des modifications non enregistrées, de 1 à 120 minutes. La copie est un fichier .xlsx placé dans un dossier que JeffCrunch réserve à cet usage ; elle est créée en arrière-plan pour que vous puissiez continuer à saisir. Elle est supprimée lorsque vous enregistrez ou fermez le classeur. Si JeffCrunch se ferme de façon inattendue, il vous propose ces copies au prochain démarrage. Vous pouvez aussi cliquer sur **Fichier > Ouvrir > Récupérer le travail non enregistré**. Un classeur protégé par un mot de passe n'est pas copié, car la copie ne serait pas protégée.

## Options de vérification

La page **Vérification** est commune aux applications JeffOffice. Elle détermine si l'orthographe est vérifiée pendant la frappe dans une cellule (désactivez **Vérifier l'orthographe lors de la frappe** pour supprimer les soulignements rouges ondulés ; **Orthographe** dans l'onglet Révision reste disponible), si les mots en majuscules et les mots contenant des chiffres sont ignorés lors de la vérification, si la correction automatique corrige les mots pendant la frappe, si les guillemets droits deviennent des guillemets typographiques, si les tirets, les fractions et les ordinaux sont modifiés, et si une adresse web devient un lien. Voir [Orthographe](notes-links) et [Saisir dans les cellules](edit-cells).

## Options d'affichage

- **Couleur du quadrillage** est la couleur des traits situés entre les cellules sur votre écran. Ce choix ne concerne que votre écran et n'est pas enregistré dans le classeur.
- **Zoom d'un nouveau classeur** indique la taille d'affichage des feuilles d'un nouveau classeur, de 10 % à 400 %.
- **Largeur de colonne standard (cette feuille)** est la largeur de chaque colonne de cette feuille qui n'a pas de largeur propre. Elle correspond à **Largeur par défaut** de l'onglet Accueil.

## Options de confidentialité

- **Supprimer mon nom et celui de mon entreprise des propriétés d'un classeur à chaque enregistrement** efface l'auteur, la personne ayant effectué le dernier enregistrement, la société et le responsable des propriétés du classeur lorsque vous enregistrez. Les propriétés sont les informations que le fichier conserve sur lui-même.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur l'utilisation de JeffCrunch. Anonymes signifie que ces informations ne disent rien sur votre identité. Lorsque cette option est activée, JeffCrunch envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffCrunch, sa version, votre système d'exploitation et votre langue, le nombre de démarrages, la fréquence d'utilisation de chaque commande, et le type de fichier ouvert ou enregistré. Si JeffCrunch se ferme de façon inattendue, il envoie également un rapport de plantage au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms, ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport de plantage n'est envoyé.

Ce choix vaut pour toutes les applications JeffOffice : le modifier ici le modifie dans chacune d'elles. JeffCrunch vous le demande au premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où, et pendant combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** s'affiche sous la case à cocher. Il s'agit d'un nombre aléatoire créé pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien de vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été créé, il affiche **aucun**.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur et, sans lui, nous ne pouvons pas savoir quels enregistrements vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le lancer. Rien n'est alors compté ni envoyé, et JeffCrunch ne pose pas la question du premier démarrage.

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
