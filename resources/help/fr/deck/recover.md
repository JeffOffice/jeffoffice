# Récupérer le travail non enregistré
<!-- keywords: récupérer, récupération, récupération automatique, enregistrement automatique, sauvegarde automatique, plantage, blocage, travail perdu, non enregistré, coupure de courant, panne, restaurer, figé, fermé par erreur, copie de sauvegarde -->

Si JeffDeck se ferme de façon inattendue ou si le courant est coupé, votre travail le plus récent est peut-être encore en sécurité. Pendant que vous travaillez, JeffDeck conserve une copie de secours de toute présentation comportant des modifications non enregistrées. C'est ce qu'on appelle la **Récupération automatique**.

## Fonctionnement de la Récupération automatique

- Toutes les 10 minutes, JeffDeck enregistre une copie de secours de chaque présentation comportant des modifications non enregistrées.
- JeffDeck conserve ces copies dans un dossier AutoRecover, à l'intérieur du dossier où il stocke ses propres données. Vous n'avez jamais besoin d'y accéder vous-même.
- La copie de secours ne remplace jamais votre véritable fichier.
- La copie disparaît lorsque vous enregistrez la présentation, la fermez ou quittez JeffDeck normalement.

La Récupération automatique ne remplace pas l'enregistrement. Si JeffDeck s'arrête, vous pouvez perdre le travail des dernières minutes. Appuyez souvent sur Ctrl+S.

## Récupérer votre travail

Au prochain démarrage de JeffDeck après un plantage, la fenêtre **Récupérer le travail non enregistré** s'ouvre automatiquement. Chaque ligne indique le nom de la présentation, l'emplacement du fichier (ou « jamais enregistré ») et l'heure à laquelle la copie a été créée.

1. Laissez une coche en regard de chaque présentation à récupérer. Décochez les autres.
2. Cliquez sur **Ouvrir**.
3. Vérifiez le travail, puis appuyez sur Ctrl+S.

Une présentation récupérée s'ouvre comme un travail non enregistré. Ctrl+S vous demande un nom et s'ouvre dans le dossier du fichier d'origine, avec le nom d'origine. Choisissez le nom d'origine pour remplacer le fichier, ou un nouveau nom pour conserver les deux.

Les autres boutons de la fenêtre sont les suivants :

- **Supprimer** supprime définitivement les copies cochées, après vous avoir demandé confirmation.
- **Pas maintenant** ferme la fenêtre et conserve les copies. JeffDeck vous les propose de nouveau au prochain démarrage.

JeffDeck propose uniquement les copies d'une session qui s'est terminée de façon inattendue. Il ne propose pas les copies d'une fenêtre JeffDeck encore ouverte.

## Retrouver plus tard le travail récupéré

Si vous avez fermé la fenêtre et souhaitez récupérer vos copies, cliquez sur **Fichier > Ouvrir**, puis sur **Récupérer le travail non enregistré**. S'il n'y a rien à récupérer, JeffDeck vous l'indique.

## Voir aussi

- [Ouvrir et enregistrer des présentations](open-save)
- [L'écran Fichier](file-screen)
- [Modifier les paramètres de JeffDeck](options)
