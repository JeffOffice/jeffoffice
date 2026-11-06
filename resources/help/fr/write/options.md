# Modifier les paramètres de JeffWrite
<!-- keywords: refaire, touche Inser, saisie remplace la sélection, ctrl+clic, suivre le lien, glisser-déplacer, informations personnelles, supprimer les informations personnelles, avancé, unités, mesures, pouces, centimètres, options, paramètres, préférences, enregistrer, format par défaut, dossier par défaut, emplacement de fichier par défaut, récupération, marques de mise en forme, quadrillage, affichage, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffWrite**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Enregistrement

La page **Enregistrement** de la fenêtre Options propose quatre choix.

- **Enregistrer les nouveaux documents sous** choisit le type de fichier que **Enregistrer sous** propose en premier pour un document qui n'a pas encore de fichier : un fichier .docx ou un fichier .odt.
- **Enregistrer les informations de récupération automatique toutes les** indique le nombre de minutes entre les copies de secours que JeffWrite conserve des travaux non enregistrés. Voir [Ouvrir, enregistrer, imprimer et exporter](open-save).
- **Emplacement de fichier par défaut** est le dossier dans lequel s'ouvrent les boîtes **Ouvrir** et **Enregistrer sous**. Cliquez sur **Parcourir** pour le choisir. Laissez ce champ vide pour démarrer dans votre dossier Documents.
- **Supprimer les informations personnelles des propriétés du fichier lors de l'enregistrement** retire votre nom du fichier à chaque enregistrement. Les propriétés du fichier contiennent l'auteur, la personne ayant effectué le dernier enregistrement, la société et le responsable. Le titre et les autres propriétés sont conservés. Les noms sont également retirés du document ouvert, de sorte qu'ils restent absents jusqu'à la fermeture de celui-ci. Pour effectuer cette opération une seule fois, utilisez **Inspecter le document** dans [Informations](info).

## Affichage

La page **Affichage** choisit ce qu'une fenêtre affiche à son ouverture.

- **Afficher les marques de mise en forme** active les marques de paragraphe et les autres signes qui ne s'impriment pas. Chaque fenêtre peut toujours les activer ou les désactiver avec le bouton **¶**.
- **Afficher le quadrillage des tableaux** dessine les bordures discrètes des cellules de tableau qui n'ont pas de traits.

Une modification sur cette page s'applique immédiatement à la fenêtre active, ainsi qu'à chaque fenêtre ouverte ensuite.

## Avancé

**Afficher les mesures en** choisit l'unité de la règle et des boîtes de dialogue : pouces, centimètres, millimètres, points ou picas. Un point vaut 1/72 de pouce. Un pica vaut 12 points.

**Utiliser Ctrl+clic pour suivre un lien** est activé par défaut : maintenez **Ctrl** enfoncée et cliquez sur un lien pour l'ouvrir ; un simple clic place seulement le curseur dans le texte du lien. Désactivez cette option pour suivre un lien par un simple clic. Maintenir **Maj** pendant le clic permet toujours de sélectionner du texte.

**Autoriser le glisser-déplacer du texte** vous permet de sélectionner des mots et de les faire glisser vers un nouvel emplacement. Maintenez **Ctrl** enfoncée au moment de relâcher pour copier les mots au lieu de les déplacer. Désactivez cette option si vous glissez le texte par erreur.

**Remplacer le texte sélectionné lors de la frappe** est activé par défaut : lorsque des mots sont sélectionnés et que vous tapez, les nouvelles lettres prennent leur place. Désactivez cette option pour que les nouvelles lettres soient ajoutées après les mots sélectionnés.

**Utiliser la touche Inser pour activer et désactiver le mode Refrappe** est désactivé par défaut. Lorsqu'il est activé, la touche **Inser** bascule entre deux modes de saisie. Normalement, chaque lettre tapée repousse les lettres suivantes. En mode **Refrappe**, chaque lettre tapée remplace la lettre située après le curseur. Appuyez de nouveau sur **Inser** pour revenir au mode normal.

Ces choix s'appliquent immédiatement à la fenêtre active, ainsi qu'à chaque fenêtre ouverte ensuite.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont JeffWrite est utilisé. Anonyme signifie que ces informations ne disent rien sur votre identité. Lorsque cette option est activée, JeffWrite envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : il s'agit de JeffWrite, de sa version, de votre système d'exploitation et de votre langue, du nombre de démarrages, de la fréquence d'utilisation de chaque commande et du type de fichier ouvert ou enregistré. Si JeffWrite se ferme de façon inattendue, il envoie aussi un rapport de plantage au prochain démarrage. JeffWrite n'envoie jamais vos fichiers, leurs noms ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport de plantage n'est envoyé non plus.

Ce choix concerne l'ensemble des applications JeffOffice : le modifier ici le modifie dans chacune d'elles. JeffWrite vous interroge à ce sujet la première fois qu'il démarre, dans une fenêtre intitulée **Aidez à améliorer JeffWrite**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où cela est transmis et combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** est affiché sous la case à cocher. Il s'agit d'un nombre aléatoire généré pour votre copie de JeffOffice, identique dans chaque application JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, il affiche **aucun**.
- Pour demander la suppression des statistiques que nous avons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur et, sans lui, nous ne pouvons pas savoir quels enregistrements vous appartiennent.
- Pour désactiver les statistiques sans ouvrir JeffWrite, définissez la variable d'environnement `JP_NO_TELEMETRY` avec n'importe quelle valeur avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffWrite ne pose pas la question de premier démarrage.

## Voir aussi

- [Bienvenue dans JeffWrite](start)
