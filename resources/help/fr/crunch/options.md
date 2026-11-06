# Modifier les paramètres de JeffCrunch
<!-- keywords: modification, saisie automatique, poigne de recopie, glisser-déplacer, nombre de décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos, modifier dans la cellule, couleur du quadrillage, zoom par défaut, largeur standard, confidentialité, informations personnelles -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Modification

La page **Modification** modifie le fonctionnement de la saisie dans les cellules.

- **Déplacer la sélection après validation** indique où va la sélection lorsque vous terminez une saisie : vers le bas, vers la droite, vers le haut, vers la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules au-dessus et en dessous** s'appelle la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par elles, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne tient compte que du bloc de cellules voisin de celle où vous saisissez, et uniquement des textes, pas des nombres.
- **Utiliser la poignée de recopie et faire glisser les cellules pour les déplacer ou les copier** affiche le petit carré à l'angle de la sélection et permet de faire glisser le bord de la sélection. Désactivez cette option si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Afficher un zéro dans les cellules dont la valeur est zéro (cette feuille)** active ou désactive l'affichage des zéros pour la feuille que vous consultez. Lorsque cette option est désactivée, une cellule contenant 0 paraît vide, mais elle conserve le 0 et compte comme 0 dans les calculs. Ce choix est enregistré avec la feuille.
- **Placer une virgule décimale dans les nombres entiers saisis** s'appelle la décimalisation automatique. Si vous indiquez 2 pour **Décimales**, la saisie de 1234 donne 12,34. Si vous indiquez -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule est enregistré tel que vous l'avez tapé.

- **Autoriser la modification directement dans les cellules** est activé par défaut. Lorsque vous le désactivez, une saisie ou la touche F2 se fait dans la barre de formule, et la cellule n'affiche le résultat qu'à la fin de la saisie.

## Options d'affichage

- **Couleur du quadrillage** est la couleur des traits entre les cellules à l'écran. Ce choix ne concerne que votre écran et n'est pas enregistré dans le classeur.
- **Zoom d'un nouveau classeur** indique la taille d'affichage des feuilles d'un nouveau classeur, de 10 % à 400 %.
- **Largeur de colonne standard (cette feuille)** est la largeur de toutes les colonnes de cette feuille qui n'ont pas de largeur propre. Elle correspond à **Largeur par défaut** de l'onglet Accueil.

## Options de confidentialité

- **Supprimer mon nom et celui de mon entreprise des propriétés d'un classeur à chaque enregistrement** efface l'auteur, la dernière personne ayant enregistré, la société et le responsable des propriétés du classeur lors de l'enregistrement. Les propriétés sont les informations qu'un fichier conserve sur lui-même.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont JeffCrunch est utilisé. Anonyme signifie que ces informations ne disent rien sur votre identité. Lorsque cette option est activée, JeffCrunch envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffCrunch, sa version, votre système d'exploitation et votre langue, le nombre de fois où il a été démarré, la fréquence d'utilisation de chaque commande, et le type de fichier que vous avez ouvert ou enregistré. Si JeffCrunch se ferme de façon inattendue, il envoie aussi un rapport d'incident au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport d'incident n'est envoyé.

Ce choix vaut pour toutes les applications JeffOffice : si vous le modifiez ici, il change dans chacune d'elles. JeffCrunch vous pose la question au premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique quelles données sont envoyées, où elles vont et combien de temps nous les conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** s'affiche sous la case à cocher. Il s'agit d'un nombre aléatoire généré pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne contient aucune information sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, il affiche **aucun**.
- Pour faire supprimer les statistiques que nous avons déjà reçues, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans lui nous ne pouvons pas savoir quels enregistrements sont les vôtres.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffCrunch ne pose pas la question au premier démarrage.

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
