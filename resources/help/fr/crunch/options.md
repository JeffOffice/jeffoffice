# Modifier les paramètres de JeffCrunch
<!-- keywords: modification, saisie automatique, poignée de recopie, glisser-déplacer, décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Modification

La page **Modification** change la façon dont la saisie fonctionne dans les cellules.

- **Déplacer la sélection après validation** indique où va la sélection lorsque vous terminez une saisie : vers le bas, vers la droite, vers le haut, vers la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules au-dessus et en dessous** s'appelle la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par ces lettres, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne tient compte que du bloc de cellules voisin de celle où vous tapez, et seulement des textes, pas des nombres.
- **Utiliser la poignée de recopie et faire glisser les cellules pour les déplacer ou les copier** active le petit carré situé à l'angle de la sélection et le glisser-déplacer de la bordure de la sélection. Désactivez-la si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Insérer une virgule décimale dans les nombres entiers saisis** s'appelle les décimales automatiques. Si vous réglez **Décimales** sur 2, la saisie de 1234 donne 12,34. Si vous le réglez sur -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule est enregistré tel qu'il a été saisi.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont JeffCrunch est utilisé. Anonymes signifie que ces informations ne disent rien sur vous. Lorsque cette option est activée, JeffCrunch envoie ces informations une fois par jour, ainsi que le jour où vous effectuez une mise à jour : il s'agit de JeffCrunch, de sa version, de votre système d'exploitation et de votre langue, du nombre de fois où il a été démarré, de la fréquence d'utilisation de chaque commande, et du type de fichier que vous avez ouvert ou enregistré. Si JeffCrunch se ferme brutalement, il envoie également un rapport de plantage au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport de plantage n'est envoyé non plus.

Ce choix vaut pour toutes les applications JeffOffice : si vous le modifiez ici, il est modifié dans chacune d'elles. JeffCrunch vous pose la question au premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où cela va et combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** s'affiche sous la case à cocher. Il s'agit d'un numéro aléatoire attribué à votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été créé, il affiche **aucun**.
- Pour faire supprimer les statistiques que nous avons déjà reçues, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans lui nous ne pouvons pas savoir quelles données vous appartiennent.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffCrunch ne pose pas la question du premier démarrage.

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
