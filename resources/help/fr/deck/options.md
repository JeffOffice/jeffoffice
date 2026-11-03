# Modifier les paramètres de JeffDeck
<!-- keywords: options, paramètres, préférences, réglages, thème, mode sombre, langue, nom d'utilisateur, initiales, règle, aligner, orthographe, correction automatique, guillemets, mises à jour, bêta, statistiques d'utilisation, télémétrie, à propos, enregistrer, incorporer les polices, polices -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffDeck**. Pour l'ouvrir, cliquez sur **Fichier > Options**. Elle comporte quatre onglets. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

- **Nom d'utilisateur** et **Initiales** apparaissent sur les commentaires que vous rédigez. Voir [Commentaires](comments).
- **Thème** peut prendre la valeur **Utiliser le paramètre système**, **Clair** ou **Sombre**. La modification s'applique immédiatement.
- **Langue d'affichage** est la langue de l'interface de JeffDeck. **Utiliser la langue du système** suit le réglage de votre ordinateur. D'autres langues apparaissent dans cette liste lorsque des traductions sont disponibles. Une nouvelle langue s'applique au prochain démarrage de JeffDeck.
- **Rechercher les mises à jour au démarrage de JeffDeck** permet à JeffDeck de rechercher une version plus récente à chaque démarrage. S'il en existe une, une fenêtre **Mise à jour disponible** présente les nouveautés. Choisissez **Mettre à jour maintenant**, **Plus tard** ou **Ignorer cette version**. Sous Windows, **Mettre à jour maintenant** télécharge le programme d'installation. Celui-ci installe la nouvelle version de toutes les applications JeffOffice et vous demande de fermer celles qui sont ouvertes. Sur les autres ordinateurs, le bouton s'intitule **Ouvrir la page de téléchargement**. Il ouvre la page où vous pouvez obtenir la nouvelle version. Si JeffPub est également ouvert, une seule des deux applications vous signale une nouvelle version chaque jour.
- **Obtenir les versions bêta (préversions destinées aux tests)** permet à JeffDeck de vous proposer également les versions bêta. Elles sortent avant les versions normales et peuvent présenter quelques imperfections.
- **Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur l'utilisation de JeffDeck. Anonyme signifie que ces informations ne révèlent rien sur votre identité. Lorsque cette option est activée, JeffDeck envoie ces informations une fois par jour, ainsi que le jour d'une mise à jour : le fait qu'il s'agit de JeffDeck, sa version, votre système d'exploitation et votre langue, le nombre de démarrages, la fréquence d'utilisation de chaque commande et le type de fichier ouvert ou enregistré (par exemple, .pptx). Si JeffDeck se ferme de manière inattendue, il envoie également un rapport de plantage au démarrage suivant. Ce rapport indique à quel endroit du programme l'arrêt s'est produit, afin que le problème puisse être identifié et corrigé. JeffDeck n'envoie jamais vos fichiers, leurs noms ni leur contenu, ni rien de ce que vous avez tapé. Si cette option est désactivée, aucun rapport de plantage n'est envoyé non plus. JeffDeck vous pose la question lors de son premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffDeck**. Il ne la pose pas si vous avez déjà fait votre choix dans JeffPub ou lors de l'installation.
- **Déclaration de confidentialité**, à côté de cette case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle précise ce qui est envoyé, où ces données sont transmises et combien de temps nous les conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** figure sous la case à cocher. Il s'agit d'un numéro aléatoire généré pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne révèle rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, la mention **aucun** s'affiche.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. Leur désactivation efface le numéro de votre ordinateur, et sans lui, nous ne pouvons pas identifier les enregistrements qui vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffDeck, définissez la variable d'environnement `JP_NO_TELEMETRY` sur une valeur quelconque avant de le démarrer. Rien n'est alors comptabilisé ni envoyé, et JeffDeck ne pose pas la question du premier démarrage.

**Nom d'utilisateur**, **Initiales**, **Thème** et **Langue d'affichage**, ainsi que les choix relatifs aux mises à jour et aux statistiques, sont communs à toutes les applications JeffOffice : si vous en modifiez un ici, il est modifié dans chacune d'elles.

## Vérification

Ces cases à cocher contrôlent l'aide à l'orthographe et à la frappe. Voir [Taper et mettre en forme du texte](text).

- **Vérifier l'orthographe lors de la frappe**
- **Ignorer les mots en MAJUSCULES**
- **Ignorer les mots qui contiennent des chiffres**
- **Remplacer le texte lors de la frappe (correction automatique)** corrige les fautes de frappe courantes.
- **Remplacer les guillemets droits par des guillemets typographiques** transforme " et ' en guillemets typographiques.
- **Mise en forme automatique lors de la frappe (tirets, fractions, ordinaux, listes automatiques)**

## Affichage

- **Afficher la règle** affiche les règles le long des bords de la diapositive. Cette option équivaut à **Affichage > Afficher > Règle** et s'applique à toutes les fenêtres, actuelles comme futures.
- **Aligner les objets sur les autres objets** aligne un objet que vous faites glisser sur les autres objets de la diapositive.

## Enregistrement

- **Incorporer les polices dans le fichier** intègre au fichier, lors de l'enregistrement, les polices utilisées par vos diapositives, afin qu'elles s'affichent de la même manière sur un ordinateur qui ne dispose pas de ces polices. Cette option est activée tant que vous ne la désactivez pas. Elle n'intègre que les polices fournies avec JeffDeck et augmente la taille du fichier. La rubrique Ouvrir et enregistrer des présentations donne plus de détails, dans la section « Polices dans le fichier ».

## Voir aussi

- [Onglet Affichage](tab-view)
- [Récupérer le travail non enregistré](recover)
- [L'écran Fichier](file-screen)
