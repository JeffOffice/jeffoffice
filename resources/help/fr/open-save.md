# Enregistrer et ouvrir des compositions
<!-- keywords: ouvrir, enregistrer, enregistrer sous, enregistrer une copie, fermer, fichiers récents, épinglé, types de fichiers, jpub, renommer, sauvegarde, propriétés, informations, marquer comme final, inspecter, historique des versions, nouveau nom, copie, Ctrl+S, lecture seule, ouvrir en lecture seule, ouvrir comme copie, nouvelle fenêtre, mes modèles, enregistrer comme modèle, réparer, ouvrir et réparer, endommagé, vérifier la compatibilité, enregistrer quand même, ne plus demander -->

JeffPub enregistre votre travail dans des fichiers .jpub. Il ouvre et enregistre aussi les fichiers .pub. Voir [Travailler avec les fichiers .pub](pub-files).

## Enregistrer

Appuyez sur Ctrl+S, cliquez sur le bouton **Enregistrer** dans la barre d'outils rapide, ou cliquez sur **Fichier > Enregistrer**. La première fois, JeffPub vous demande un nom et un emplacement. Ensuite, Enregistrer remplace le fichier par la version la plus récente de votre travail.

Une étoile (*) après le nom dans la barre de titre indique que des modifications ne sont pas encore enregistrées. Enregistrez souvent. Si JeffPub s'arrête brusquement, voir [Récupérer un travail non enregistré](recover).

Pour conserver la dernière copie enregistrée d'un fichier, activez **Toujours créer une copie de sauvegarde**. À chaque enregistrement, JeffPub copie d'abord l'ancien fichier dans un fichier nommé « Sauvegarde de » suivi du nom du fichier, dans le même dossier. Si la copie est impossible, le logiciel demande s'il faut enregistrer sans elle. La réponse par défaut est Non, sauf si vous répondez Oui, et le fichier reste alors tel qu'il était. Vous trouvez cette option dans **Fichier > Options**, sous l'onglet **Général**. Toutes les applications JeffOffice la proposent.

## Enregistrer sous un nouveau nom

Cliquez sur **Fichier > Enregistrer sous**, ou appuyez sur F12. Saisissez un nom, choisissez **Type de fichier**, puis cliquez sur **Enregistrer**. Les choix sont les suivants :

- **Composition JeffPub (*.jpub)** est le choix habituel.
- **Fichiers de composition .pub (*.pub)** crée un fichier .pub.
- **PDF (*.pdf)** crée un PDF et laisse votre composition telle quelle.
- **Modèle JeffPub (*.jpub)** ajoute la composition à **Mes modèles**.

Après un enregistrement en .jpub ou en .pub, la barre de titre affiche le nouveau nom, et vous continuez à travailler dans le nouveau fichier.

## Enregistrer une copie

Cliquez sur **Fichier > Enregistrer une copie**, puis choisissez un nom. JeffPub écrit la composition dans ce fichier, et vous continuez à travailler dans celui que vous aviez ouvert. Son nom et ses modifications non enregistrées restent inchangés.

## Ouvrir

1. Appuyez sur Ctrl+O, ou cliquez sur **Fichier > Ouvrir**.
2. Cliquez sur **Parcourir** pour choisir un fichier. Vous pouvez aussi cliquer sur un nom sous **Épinglé** ou **Compositions récentes**.

Vous pouvez également faire glisser un fichier .jpub ou .pub sur la page. Si votre composition actuelle contient des modifications non enregistrées, JeffPub vous demande d'abord ce qu'il faut en faire.

Faites un clic droit sur un nom de la liste pour **Épingler à la liste** (il reste en haut) ou **Retirer de la liste**. Un nom en gris signifie que JeffPub ne trouve plus ce fichier. Pour modifier le nombre de noms récents affichés, voir [Modifier les paramètres de JeffPub](options).

## Autres façons d'ouvrir un fichier

Sur la page **Ouvrir**, cliquez sur **Autres façons d'ouvrir**. Vous pouvez aussi faire un clic droit sur un nom de la liste des fichiers récents.

- **Ouvrir en lecture seule** ouvre la composition pour la lecture seulement. Vous pouvez la consulter et faire défiler la page, mais vous ne pouvez rien modifier. La barre de titre indique « Lecture seule ». Si vous cliquez sur **Enregistrer**, JeffPub demande un nouveau nom, de sorte que le fichier ouvert reste inchangé.
- **Ouvrir une copie** ouvre la composition sans fichier propre. **Enregistrer** demande où la placer.
- **Ouvrir dans une nouvelle fenêtre** ouvre le fichier dans une fenêtre distincte.
- **Ouvrir et réparer** sert pour un fichier .jpub endommagé, par exemple un fichier tronqué lors d'une copie. JeffPub lit toutes les parties du fichier qu'il peut lire et laisse de côté celles qu'il ne peut pas lire. La composition s'ouvre comme une copie nommée « Réparée » suivi du nom, et un message indique ce qui a été omis. Le fichier endommagé n'est pas modifié.

## Fermer

Cliquez sur **Fichier > Fermer**, ou appuyez sur Ctrl+F4. JeffPub demande s'il faut enregistrer les modifications, puis affiche une page vierge.

## Informations : propriétés, Marquer comme final, Inspecter et versions

**Fichier > Informations** affiche l'emplacement du fichier, avec **Afficher dans le dossier** et **Copier le chemin**. Cette page présente des données sur votre composition : pages, taille de page, images, mots et dates. Saisissez dans les zones pour définir le **Titre**, l'**Auteur**, l'**Objet**, les **Mots clés**, les **Catégories**, les **Commentaires**, la **Société** et le **Responsable**, puis cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés. La modification est enregistrée avec la composition, et **Ctrl+Z** l'annule. La page ouvre aussi vos informations professionnelles, les **Paramètres d'impression commerciale** (voir [Fichiers PDF et imprimeurs](pdf)) et le [Vérificateur de conception](design-checker).

- **Vérifier la compatibilité** liste ce qu'un fichier .pub ne peut pas conserver de votre composition, sans rien enregistrer : objets masqués, certains styles de liste, et tout autre élément que l'écriture au format .pub omet. Lorsque vous enregistrez en fichier .pub, JeffPub affiche la même liste **avant** d'écrire quoi que ce soit. Choisissez **Enregistrer quand même**, **Enregistrer en .jpub** (le fichier .jpub est placé à côté, et rien n'est perdu), ou **Annuler**. Cochez **Ne plus demander pour ce fichier** : JeffPub cesse alors de poser la question pour ce fichier jusqu'à la fermeture du programme.
- **Marquer comme final** enregistre la composition, puis la réserve à la lecture seule. Lorsqu'on l'ouvre, la barre de titre indique qu'elle est en lecture seule, et une note précise qu'elle a été marquée comme finale. Ce n'est pas un verrou : choisissez de nouveau **Marquée comme finale** pour la modifier. Un fichier .pub ne peut pas conserver ce marquage, donc enregistrez d'abord en fichier .jpub.
- **Inspecter la composition** liste ce que le fichier contient et que vous ne souhaitez peut-être pas partager, puis le supprime : les commentaires et les noms de leurs auteurs, l'auteur et les autres noms dans les propriétés, les informations professionnelles, la liste de publipostage et les objets placés dans la zone de travail à côté des pages.
- **Supprimer les informations personnelles de ce fichier à l'enregistrement**, sous les cartes, retire à chaque enregistrement l'auteur, la personne ayant effectué le dernier enregistrement, la société et le responsable, pour un fichier .jpub ou .pub. C'est la même case que dans **Options**, et elle est la même dans toutes les applications JeffOffice.
- **Historique des versions** conserve sur votre ordinateur des copies nommées de la composition. Enregistrez d'abord la composition. Cliquez sur **Conserver une version…**, puis choisissez ensuite une version pour **Ouvrir une copie**, **Revenir à cette version** (la composition telle qu'elle est actuellement est d'abord conservée comme version) ou **Supprimer**.

Un fichier .jpub ne prévoit pas de champ pour un mot de passe, donc JeffPub n'a pas de carte de mot de passe. JeffPub ne dispose pas encore de vérification d'accessibilité, la page ne comporte donc pas de carte correspondante.

## Si Windows empêche JeffPub d'enregistrer

Le Centre de sécurité Windows peut empêcher des programmes d'enregistrer dans des dossiers comme Documents. JeffPub affiche alors un message avec les étapes pour l'autoriser. Vous pouvez aussi enregistrer dans un autre dossier.

## Voir aussi

- [Récupérer un travail non enregistré](recover)
- [Travailler avec les fichiers .pub](pub-files)
- [Créer une nouvelle composition](create)
- [Modifier les paramètres de JeffPub](options)
