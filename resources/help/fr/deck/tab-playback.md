# Onglet Lecture

<!-- keywords: onglet lecture, ruban, film, vidéo, son, audio, lire, lecture, pause, aperçu, couper, découper, sous-titres, légendes, insérer des légendes, supprimer les légendes, sous-titrage, webvtt, vtt, volume, muet, démarrer, dans la séquence de clics, automatiquement, au clic, lire en plein écran, masquer pendant la lecture non active, boucle jusqu'à l'arrêt, rembobiner après la lecture, lire sur toutes les diapositives, texte de remplacement, aligner, hauteur, largeur, taille, outils multimédias -->

L'onglet **Lecture** définit la manière dont un film ou un son est lu, et ajoute des légendes à un film. Il n'apparaît que lorsqu'un seul film ou son est sélectionné, et disparaît dès que vous sélectionnez autre chose.

- Le groupe **Aperçu** contient **Lire**. Il lit le film ou le son sur la diapositive pour que vous puissiez le voir et l'entendre. Pendant la lecture, le bouton affiche **Pause**.
- Le groupe **Édition** contient **Couper**. Il ouvre une boîte avec une **Heure de début** et une **Heure de fin**, en secondes. Le film ou le son n'est lu que entre l'heure de début et l'heure de fin. Le reste du fichier reste dans la présentation. Pour lire à nouveau jusqu'à la fin du fichier, réglez l'heure de fin sur la durée totale.
  - **Insérer des légendes** ajoute des légendes à un film à partir d'un fichier WebVTT (un fichier texte qui se termine par `.vtt`). Il demande d'abord le fichier, puis la langue des légendes, sous forme de code court tel que `en`, `es` ou `pt-BR`. Les légendes s'appliquent à un film inséré dans la présentation ; pour un film seulement lié depuis l'extérieur, le bouton est grisé. Un film conserve un seul jeu de légendes par langue : insérer un second fichier pour la même langue remplace le premier. Un fichier qui n'est pas au format WebVTT, qui ne contient aucune légende, ou qui est trop volumineux (maximum 4 Mo, 20 000 légendes et 2 000 caractères par légende) est refusé avec un message, et rien ne change.
  - **Supprimer les légendes** retire les légendes du film. Si le film possède des légendes dans plusieurs langues, il demande lesquelles supprimer, ou toutes.
- Le groupe **Options du média** indique comment le média se lit dans le diaporama :
  - **Volume** est **Faible**, **Moyen**, **Fort** ou **Muet**.
  - **Démarrer** indique ce qui le lance. **Dans la séquence de clics** le lance à un clic, avec les animations de la diapositive. **Automatiquement** le lance seul quand son tour arrive. **Au clic** le lance lorsque vous cliquez sur le film ou le son.
  - **Lire en plein écran** fait occuper tout l'écran au film pendant sa lecture. Cette option ne fonctionne que pour les films.
  - **Masquer pendant la lecture non active** cache l'image tant que le film ou le son n'est pas lu, puis de nouveau après son arrêt. L'icône du haut-parleur d'un son est masquée en permanence.
  - **Boucle jusqu'à l'arrêt** relance la lecture à chaque fin, jusqu'à son arrêt ou jusqu'au changement de diapositive.
  - **Rembobiner après la lecture** ramène un film à sa première image à la fin. Sans cette option, le film reste sur sa dernière image.
  - **Lire sur toutes les diapositives** (pour un son ; un film n'est dessiné que sur sa propre diapositive) maintient la lecture du son dans le diaporama lorsque la diapositive change, jusqu'à sa fin ou son arrêt, jusqu'à la fin du diaporama. Un fichier provenant d'un autre programme peut indiquer sur combien de diapositives le son est lu : ce nombre reste tant que la case est cochée, et le son s'arrête après ce nombre de diapositives, en comptant à partir de la diapositive où il a commencé (revenir en arrière entre elles n'en consomme aucune). Décochée, le son s'arrête avec sa diapositive ; recochée, il se poursuit jusqu'à la fin du diaporama. Avec **Boucle jusqu'à l'arrêt**, il repart à chaque fin. Ctrl+Z annule la modification, et un fichier .pptx la conserve.

Les légendes s'affichent au bas du film, en texte blanc sur un bandeau sombre, pendant la lecture du film dans le diaporama et dans l'éditeur. Elles sont synchronisées sur le film, et leur taille suit celle du film. Si un film comporte plusieurs langues, la première s'affiche. Les légendes sont enregistrées avec la présentation dans le fichier `.pptx`, et elles sont donc retrouvées à la prochaine ouverture. Un fichier de légendes dans un autre format (non WebVTT) est conservé tel quel et enregistré à nouveau sans modification, mais il ne s'affiche pas. Dès qu'un film possède des légendes, **Vérifier l'accessibilité** ne le signale plus.

Les groupes **Accessibilité**, **Organiser** et **Taille** sont ceux de l'onglet Format de l'image : **Texte de remplacement**, **Aligner**, ainsi que les zones **Hauteur** et **Largeur**, pour le film ou le son sélectionné.

Chaque modification constitue une étape. **Ctrl+Z** l'annule.

Un son ou un film qui se lit sur plusieurs diapositives, ce que certains fichiers définissent, continue de se lire pendant autant de diapositives que le fichier l'indique. JeffDeck le lit ainsi dans le diaporama, et conserve ce réglage à l'enregistrement.

Voir [Formes, images, tableaux et Texte artistique](objects) et [Lancer le diaporama et utiliser le mode Présentateur](show).

## Voir aussi

- [Formes, images, tableaux et Texte artistique](objects)
- [Onglet Insertion](tab-insert)
- [Lancer le diaporama et utiliser le mode Présentateur](show)
