# Récupérer le travail non enregistré
<!-- keywords: récupérer, récupération, récupération automatique, enregistrement automatique, sauvegarde automatique, plantage, blocage, travail perdu, non enregistré, coupure de courant, panne de courant, restaurer, fermé par erreur, sauvegarde, copie de secours -->

Si JeffPub se ferme de manière inattendue ou en cas de coupure de courant, votre travail le plus récent est peut-être encore en sécurité. Pendant que vous travaillez, JeffPub conserve une copie de secours de chaque composition qui comporte des modifications non enregistrées. C'est ce qu'on appelle la **Récupération automatique**.

## Fonctionnement de la Récupération automatique

- Toutes les quelques minutes, JeffPub enregistre une copie de secours de chaque composition qui comporte des modifications non enregistrées. Par défaut, toutes les 10 minutes.
- JeffPub conserve ces copies dans un dossier de Récupération automatique, situé dans le dossier où il stocke ses propres données. Vous n'avez jamais besoin d'y accéder vous-même.
- La copie de secours ne remplace jamais votre véritable fichier.
- La copie disparaît lorsque vous enregistrez la composition, la fermez ou quittez JeffPub normalement.

La Récupération automatique ne remplace pas l'enregistrement. Si JeffPub s'arrête, vous pouvez perdre le travail des dernières minutes. Appuyez souvent sur Ctrl+S.

## Récupérer votre travail

Au démarrage suivant de JeffPub après un plantage, la fenêtre **Récupérer le travail non enregistré** s'ouvre automatiquement. Chaque ligne indique le nom de la composition, l'emplacement du fichier (ou « jamais enregistré ») et l'heure de création de la copie.

1. Laissez cochée chaque composition que vous voulez récupérer. Décochez les autres.
2. Cliquez sur **Ouvrir**.
3. Vérifiez le travail, puis appuyez sur Ctrl+S.

La composition récupérée affiche « (Récupéré) » dans la barre de titre jusqu'à ce que vous l'enregistriez. Ctrl+S vous demande un nom, en partant du dossier du fichier d'origine, avec le nom d'origine. Choisissez le nom d'origine pour remplacer le fichier, ou un nouveau nom pour conserver les deux.

Les images liées à des fichiers sont retrouvées comme si la composition était ouverte dans le dossier du fichier d'origine : celles qui se trouvent dans ce dossier, ou dans un de ses sous-dossiers, se chargent normalement. Une image liée à un fichier situé ailleurs affiche son aperçu enregistré et porte la mention **Non mise à jour** ; sélectionnez-la et cliquez sur **Mettre à jour le lien** dans le [Gestionnaire de graphiques](graphics-manager). Une composition jamais enregistrée n'a pas de dossier : toutes ses images liées attendent donc **Mettre à jour le lien**.

Les autres boutons de la fenêtre sont les suivants :

- **Supprimer** efface définitivement les copies cochées, après vous avoir demandé confirmation.
- **Pas maintenant** ferme la fenêtre et conserve les copies. JeffPub vous les propose de nouveau au démarrage suivant.

JeffPub ne propose que les copies d'une session qui s'est terminée de manière inattendue. Il ne propose pas les copies d'une fenêtre JeffPub encore ouverte.

## Retrouver plus tard le travail récupéré

Si vous avez fermé la fenêtre et voulez récupérer vos copies, cliquez sur **Fichier > Ouvrir**, puis sur **Récupérer le travail non enregistré**. S'il n'y a rien à récupérer, JeffPub vous l'indique.

## Modifier la fréquence d'enregistrement

Cliquez sur **Fichier > Options**, puis sur l'onglet **Enregistrement**. Réglez **Enregistrer les informations de Récupération automatique toutes les** sur une valeur comprise entre 1 et 120 minutes. Plus la valeur est petite, moins vous risquez de perdre de travail. Le nouveau délai prend pleinement effet au prochain démarrage de JeffPub.

## Voir aussi

- [Ouvrir et enregistrer des compositions](open-save)
- [Modifier les paramètres de JeffPub](options)
- [Obtenir de l'aide et signaler des problèmes](support)
