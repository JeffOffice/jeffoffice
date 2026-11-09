# Modifier les paramètres de JeffDeck
<!-- keywords: options, paramètres, préférences, thème, mode sombre, langue, nom d'utilisateur, initiales, règle, aligner, orthographe, correction automatique, guillemets, mises à jour, bêta, statistiques d'utilisation, à propos, enregistrer, incorporer les polices, polices -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffDeck**. Cliquez sur **Fichier > Options** pour l'ouvrir. Elle comporte quatre onglets. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

- **Nom d'utilisateur** et **Initiales** apparaissent sur les commentaires que vous rédigez. Voir [Commentaires](comments).
- **Thème** prend la valeur **Utiliser le paramètre système**, **Clair** ou **Sombre**. Le changement est visible immédiatement.
- **Langue d'affichage** est la langue des textes de JeffDeck lui-même. **Utiliser la langue du système** suit la langue de votre ordinateur. D'autres langues apparaissent dans cette liste lorsque des traductions sont disponibles. Une nouvelle langue prend effet à la prochaine ouverture de JeffDeck.
- **Rechercher les mises à jour au démarrage de JeffDeck** fait en sorte que JeffDeck recherche une version plus récente à chaque démarrage. Si elle existe, une fenêtre **Mise à jour disponible** présente les nouveautés. Choisissez **Mettre à jour maintenant**, **Plus tard** ou **Ignorer cette version**. Sous Windows, **Mettre à jour maintenant** télécharge le programme d'installation. Celui-ci installe la nouvelle version de toutes les applications JeffOffice, et vous demande de fermer celles qui sont ouvertes. Sur les autres ordinateurs, le bouton s'intitule **Ouvrir la page de téléchargement**. Il ouvre la page où vous pouvez obtenir la nouvelle version. Si JeffPub est également ouvert, un seul des deux vous signale une nouvelle version chaque jour.
- **Obtenir les versions bêta (préversions destinées aux tests)** fait en sorte que JeffDeck propose aussi les versions bêta. Celles-ci sortent avant les versions habituelles. Elles peuvent comporter des imperfections.
- **Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont JeffDeck est utilisé. Anonyme signifie que ces informations ne disent rien sur vous. Lorsque cette option est activée, JeffDeck envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffDeck, sa version, votre système d'exploitation et votre langue, le nombre de démarrages, la fréquence d'utilisation de chaque commande, et le type de fichier ouvert ou enregistré (par exemple, .pptx). Si JeffDeck se ferme de façon inattendue, il envoie aussi un rapport d'erreur au prochain démarrage. Ce rapport indique à quel endroit du programme l'arrêt s'est produit, afin que le problème puisse être trouvé et corrigé. JeffDeck n'envoie jamais vos fichiers, leurs noms, ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport d'erreur n'est envoyé non plus. JeffDeck pose cette question la première fois qu'il démarre, dans une fenêtre intitulée **Aidez à améliorer JeffDeck**. Il ne la pose pas si vous avez déjà fait ce choix dans JeffPub ou dans le programme d'installation.
- **Déclaration de confidentialité**, à côté de cette case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où, et pendant combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** est affiché sous la case à cocher. C'est un nombre aléatoire généré pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, la mention **aucun** s'affiche.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. Les désactiver efface le numéro de votre ordinateur, et sans lui, nous ne pouvons pas identifier les enregistrements qui vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffDeck, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors comptabilisé ni envoyé, et JeffDeck ne pose pas la question de la première exécution.

**Nom d'utilisateur**, **Initiales**, **Thème** et **Langue d'affichage**, ainsi que les choix relatifs aux mises à jour et aux statistiques, sont communs à toutes les applications JeffOffice : modifiez-les ici, et la modification s'applique à chacune d'elles.

## Vérification

Ces cases à cocher contrôlent l'orthographe et l'aide à la saisie. Voir [Saisir et mettre en forme le texte](text).

- **Vérifier l'orthographe lors de la frappe**
- **Ignorer les mots en MAJUSCULES**
- **Ignorer les mots qui contiennent des chiffres**
- **Remplacer le texte lors de la frappe (correction automatique)** corrige les fautes de frappe courantes.
- **Remplacer les guillemets droits par des guillemets typographiques** transforme " et ' en guillemets courbes.
- **Mise en forme automatique lors de la frappe (tirets, fractions, ordinaux, listes automatiques)**
- **Transformer les adresses web en liens au fur et à mesure de la saisie** transforme une adresse web ou e-mail en lien lorsque vous appuyez sur la barre d'espace après elle. Annuler ne retire que le lien.

## Affichage

- **Afficher les règles** affiche les règles le long des bords de la diapositive. C'est la même chose que **Affichage > Afficher > Règle**, et cela s'applique à toutes les fenêtres, dès maintenant et à celles que vous ouvrirez ensuite.
- **Aligner les objets sur les autres objets** fait en sorte qu'un objet que vous faites glisser s'aligne sur les autres objets de la diapositive.

## Enregistrement

- **Incorporer les polices dans le fichier** place dans le fichier, à l'enregistrement, les polices utilisées par vos diapositives, afin que celles-ci s'affichent de la même façon sur un ordinateur qui ne possède pas ces polices. Cette option est activée tant que vous ne la désactivez pas. Elle n'inclut que les polices fournies avec JeffDeck, et elle augmente la taille du fichier. La rubrique Ouvrir et enregistrer des présentations donne le détail, sous « Polices dans le fichier ».

## Voir aussi

- [Onglet Affichage](tab-view)
- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
