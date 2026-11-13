# Onglet Lecture
<!-- keywords: onglet lecture, ruban, film, vidéo, son, audio, lire, lecture, pause, aperçu, couper, découper, sous-titres, légendes, insérer des légendes, supprimer les légendes, webvtt, vtt, volume, muet, démarrer, dans la séquence de clics, automatiquement, au clic, lecture en plein écran, masquer pendant l'absence de lecture, lire en boucle jusqu'à l'arrêt, rembobiner après la lecture, outils multimédias -->

L'onglet **Lecture** définit la manière dont un film ou un son est lu, et permet d'ajouter des légendes à un film. Il n'apparaît que lorsqu'un seul film ou son est sélectionné, et il disparaît dès que vous sélectionnez autre chose.

- **Aperçu** contient **Lire**. Il lit le film ou le son sur la diapositive pour que vous puissiez le voir et l'entendre. Pendant la lecture, le bouton affiche **Pause**.
- **Modification** contient **Couper**. Il ouvre une boîte de dialogue avec **Heure de début** et **Heure de fin**, en secondes. Le film ou le son n'est lu que de l'heure de début à l'heure de fin. Le reste du fichier reste dans la présentation. Pour lire de nouveau jusqu'à la fin du fichier, réglez l'heure de fin sur la durée totale.
  - **Insérer des légendes** ajoute des légendes à un film à partir d'un fichier WebVTT (un fichier texte dont le nom se termine par `.vtt`). Il demande le fichier, puis la langue des légendes, sous forme de code court tel que `en`, `es` ou `pt-BR`. Les légendes accompagnent un film incorporé à la présentation ; pour un film seulement lié depuis l'extérieur, le bouton est grisé. Un film conserve un jeu de légendes par langue : insérer un second fichier pour la même langue remplace le premier. Un fichier qui n'est pas au format WebVTT, ne contient aucune légende ou est trop volumineux (maximum 4 Mo, 20 000 légendes et 2 000 caractères par légende) est refusé avec un message, et rien ne change.
  - **Supprimer les légendes** retire les légendes du film. Si le film comporte des légendes dans plusieurs langues, il demande laquelle supprimer, ou toutes.
- **Options du média** indique comment le film est lu dans le diaporama :
  - **Volume** prend la valeur **Faible**, **Moyen**, **Fort** ou **Muet**.
  - **Démarrer** indique ce qui le lance. **Dans la séquence de clics** le lit au clic, avec les animations de la diapositive. **Automatiquement** le lit de lui-même quand son tour arrive. **Au clic** le lit lorsque vous cliquez sur le film ou le son.
  - **Lecture en plein écran** fait occuper tout l'écran au film pendant sa lecture. Cette option ne fonctionne que pour les films.
  - **Masquer pendant l'absence de lecture** masque l'image jusqu'à ce que le film ou le son soit lu, puis de nouveau après son arrêt. L'icône de haut-parleur d'un son est masquée en permanence.
  - **Lire en boucle jusqu'à l'arrêt** relance la lecture à chaque fin, jusqu'à ce qu'elle soit arrêtée ou que la diapositive change.
  - **Rembobiner après la lecture** ramène un film à sa première image à la fin. Sans cette option, le film reste sur sa dernière image.

Les légendes s'affichent en bas du film, en texte blanc sur une bande sombre, pendant la lecture du film dans le diaporama et dans l'éditeur. Elles sont synchronisées sur le film, et leur taille suit celle du film. Si un film comporte plusieurs langues, la première s'affiche. Les légendes sont enregistrées avec la présentation, dans le fichier `.pptx`, et réapparaissent à la prochaine ouverture. Un fichier de légendes dans un autre format (non WebVTT) est conservé tel quel et réenregistré sans modification, mais il ne s'affiche pas. Une fois qu'un film a des légendes, **Vérifier l'accessibilité** ne le signale plus.

Chaque modification constitue une seule étape. **Ctrl+Z** l'annule.

Un son ou un film qui se lit sur plusieurs diapositives, ce que certains fichiers définissent, continue de se lire pendant le nombre de diapositives indiqué dans le fichier. JeffDeck le lit ainsi dans le diaporama, et conserve ce réglage à l'enregistrement.

Voir [Objets, images, tableaux et texte artistique](objects) et [Lancer le diaporama et utiliser le mode Présentateur](show).

## Voir aussi

- [Objets, images, tableaux et texte artistique](objects)
- [Onglet Insertion](tab-insert)
- [Lancer le diaporama et utiliser le mode Présentateur](show)
