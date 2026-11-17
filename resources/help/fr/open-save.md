# Enregistrer et ouvrir des compositions
<!-- keywords: ouvrir, enregistrer, enregistrer sous, enregistrer une copie, fermer, fichiers récents, épinglé, types de fichiers, jpub, renommer, sauvegarde, propriétés, infos, marquer comme final, inspecter, historique des versions, nouveau nom, copier, Ctrl+S, lecture seule, ouvrir en lecture seule, ouvrir une copie, nouvelle fenêtre, mes modèles, enregistrer comme modèle, réparer, ouvrir et réparer, endommagé, vérifier la compatibilité, enregistrer quand même, ne plus demander -->

JeffPub enregistre votre travail dans des fichiers .jpub. Il peut aussi ouvrir et enregistrer des fichiers .pub. Voir [Travailler avec des fichiers .pub](pub-files).

## Enregistrer

Appuyez sur Ctrl+S, cliquez sur le bouton **Enregistrer** dans la barre d'outils rapide, ou cliquez sur **Fichier > Enregistrer**. La première fois, JeffPub vous demande un nom et un emplacement. Ensuite, Enregistrer remplace le fichier par votre dernière version du travail.

Une étoile (*) après le nom dans la barre de titre signifie que certaines modifications ne sont pas encore enregistrées. Enregistrez souvent. Si JeffPub s'arrête brusquement, consultez [Récupérer un travail non enregistré](recover).

Pour conserver la dernière copie enregistrée d'un fichier, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffPub copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi du nom d'origine, dans le même dossier. Si la copie ne peut pas être créée, l'application demande s'il faut enregistrer sans elle. La réponse par défaut est Non, sauf si vous répondez Oui, et le fichier reste alors inchangé. Vous trouvez cette option dans **Fichier > Options**, sous l'onglet **Général**. Toutes les applications JeffOffice la proposent.

## Enregistrer sous un nouveau nom

Cliquez sur **Fichier > Enregistrer sous**, ou appuyez sur F12. Saisissez un nom, choisissez **Type de fichier**, puis cliquez sur **Enregistrer**. Les choix sont les suivants :

- **Composition JeffPub (*.jpub)** est le choix habituel.
- **Fichiers de composition .pub (*.pub)** crée un fichier .pub.
- **PDF (*.pdf)** crée un PDF et laisse votre composition inchangée.
- **Modèle JeffPub (*.jpub)** ajoute la composition à **Mes modèles**.

Après un enregistrement en .jpub ou .pub, la barre de titre affiche le nouveau nom, et vous continuez à travailler dans le nouveau fichier.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**, puis choisissez un nom. JeffPub écrit la composition dans ce fichier, et vous continuez à travailler dans celui que vous aviez ouvert. Son nom et ses modifications non enregistrées restent inchangés.

## Ouvrir

1. Appuyez sur Ctrl+O, ou cliquez sur **Fichier > Ouvrir**.
2. Cliquez sur **Parcourir** pour choisir un fichier. Vous pouvez aussi cliquer sur un nom sous **Épinglé** ou **Compositions récentes**.

Vous pouvez également faire glisser un fichier .jpub ou .pub sur la page. Si la composition en cours contient des modifications non enregistrées, JeffPub vous le demande d'abord.

Faites un clic droit sur un nom de la liste pour **Épingler à la liste** (il reste en haut) ou pour **Supprimer de la liste**. Un nom en gris signifie que JeffPub ne trouve plus ce fichier. Pour changer le nombre de noms récents affichés, voir [Modifier les paramètres de JeffPub](options).

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des récents.

- **Ouvrir en lecture seule** ouvre la composition uniquement pour la lire. Vous pouvez regarder et faire défiler, mais rien ne peut être modifié. La barre de titre affiche « Lecture seule ». Si vous cliquez sur **Enregistrer**, JeffPub demande un nouveau nom, de sorte que le fichier ouvert n'est pas modifié.
- **Ouvrir une copie** ouvre la composition sans fichier propre. **Enregistrer** demande où la placer.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte.
- **Ouvrir et réparer** sert pour un fichier .jpub endommagé, par exemple un fichier tronqué lors d'une copie. JeffPub lit toutes les parties du fichier qu'il peut lire, et laisse de côté celles qu'il ne peut pas lire. La composition s'ouvre comme une copie nommée « Réparée » suivi du nom d'origine, et un message indique ce qui a été omis. Le fichier endommagé n'est pas modifié.

## Fermer

Cliquez sur **Fichier > Fermer**, ou appuyez sur Ctrl+F4. JeffPub demande s'il faut enregistrer les modifications, puis affiche une page vierge.

## Infos : propriétés, Marquer comme final, Inspecter et versions

**Fichier > Infos** indique où se trouve le fichier, avec **Afficher dans le dossier** et **Copier le chemin**. Elle présente des informations sur votre composition : pages, taille de page, images, mots et dates. Saisissez dans les zones pour définir le **Titre**, l'**Auteur**, l'**Objet**, les **Mots clés**, les **Catégories**, les **Commentaires**, la **Société** et le **Responsable**, puis cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés. La modification est enregistrée avec la composition, et **Ctrl+Z** l'annule. La page donne aussi accès à vos informations professionnelles, aux **Paramètres d'impression commerciale** (voir [Fichiers PDF et imprimeurs](pdf)) et au [Vérificateur de conception](design-checker).

- **Vérifier la compatibilité** liste ce qu'un fichier .pub ne peut pas contenir de votre composition, sans rien enregistrer : objets masqués, certains styles de liste et tout autre élément que l'écrivain .pub omet. Lorsque vous enregistrez en fichier .pub, JeffPub affiche la même liste **avant** d'écrire quoi que ce soit. Choisissez **Enregistrer quand même**, **Enregistrer en .jpub** (le fichier .jpub est placé à côté, et rien n'est perdu), ou **Annuler**. Cochez **Ne plus demander pour ce fichier** pour que JeffPub arrête de poser la question pour ce fichier jusqu'à la fermeture du programme.
- **Toujours ouvrir en lecture seule** fait s'ouvrir la composition en lecture seule à chaque fois. Ce choix est enregistré avec un fichier .jpub, et la barre d'état l'indique lorsqu'une personne l'ouvre. Pour la modifier, choisissez **S'ouvre toujours en lecture seule** sur la même page. Il s'agit d'un rappel et non d'un verrou. Un fichier .pub ne peut pas contenir ce réglage (l'écrivain .pub ne conserve aucune propriété de ce type, et JeffPub n'en lit pas dans un fichier .pub) : enregistrez donc d'abord en fichier .jpub. Vérifier la compatibilité le signale pour un fichier .pub.
- **Marquer comme final** enregistre la composition, puis la réserve à la lecture seule. Lorsqu'une personne l'ouvre, la barre de titre indique qu'elle est en lecture seule, et une note précise qu'elle a été marquée comme finale. Ce n'est pas un verrou : choisissez de nouveau **Marqué comme final** pour la modifier. Un fichier .pub ne peut pas conserver ce marquage : enregistrez donc d'abord en fichier .jpub.
- **Inspecter la composition** liste ce que le fichier contient et que vous ne souhaitez peut-être pas partager, puis le supprime : les commentaires et les noms de leurs auteurs, l'auteur et les autres noms dans les propriétés, les informations professionnelles, la liste de fusion et les objets placés dans la zone de travail à côté des pages.
- **Supprimer les informations personnelles de ce fichier à l'enregistrement**, sous les cartes, retire à chaque enregistrement l'auteur, la personne ayant enregistré le fichier en dernier, la société et le responsable, pour un fichier .jpub comme pour un fichier .pub. C'est la même case que dans **Options**, et elle est identique dans toutes les applications JeffOffice.
- **Historique des versions** conserve sur votre ordinateur des copies nommées de la composition. Enregistrez d'abord la composition. Cliquez sur **Conserver une version…**, puis choisissez plus tard une version pour **Ouvrir une copie**, **Revenir à cette version** (la composition telle qu'elle est actuellement est d'abord conservée comme version), ou **Supprimer**.

Un fichier .jpub ne comporte pas de mot de passe, si bien que JeffPub n'a pas de carte pour cela. JeffPub ne dispose pas encore de vérification d'accessibilité : la page ne comporte donc pas de carte à ce sujet.

## Si Windows empêche JeffPub d'enregistrer

Le Centre de sécurité Windows peut empêcher des programmes d'enregistrer dans des dossiers tels que Documents. JeffPub affiche alors un message avec les étapes pour l'autoriser. Vous pouvez aussi enregistrer dans un autre dossier.

## Voir aussi

- [Récupérer un travail non enregistré](recover)
- [Travailler avec des fichiers .pub](pub-files)
- [Créer une nouvelle composition](create)
- [Modifier les paramètres de JeffPub](options)
