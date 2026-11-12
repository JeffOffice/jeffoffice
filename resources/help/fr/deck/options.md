# Modifier les options de JeffDeck
<!-- keywords: options, paramètres, préférences, thème, mode sombre, langue, nom d'utilisateur, initiales, règle, aimantation, orthographe, correction automatique, guillemets, mises à jour, bêta, statistiques d'utilisation, à propos, enregistrer, incorporer les polices, polices, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, rétablir le ruban, modifier les raccourcis clavier, réattribuer des touches, rétablir les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffDeck**. Cliquez sur **Fichier > Options** pour l'ouvrir. Elle comporte cinq onglets. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

- **Nom d'utilisateur** et **Initiales** apparaissent sur les commentaires que vous rédigez. Voir [Commentaires](comments).
- **Thème** prend la valeur **Utiliser le paramètre système**, **Clair** ou **Sombre**. La modification est visible immédiatement.
- **Langue d'affichage** correspond à la langue des textes de JeffDeck. **Utiliser la langue du système** suit celle de votre ordinateur. Les autres langues apparaissent dans cette liste lorsque des traductions sont disponibles. Une nouvelle langue prend effet au prochain démarrage de JeffDeck.
- **Rechercher les mises à jour au démarrage de JeffDeck** fait en sorte que JeffDeck recherche une version plus récente à chaque démarrage. Si une version plus récente existe, la fenêtre **Mise à jour disponible** présente les nouveautés. Choisissez **Mettre à jour maintenant**, **Plus tard** ou **Ignorer cette version**. Sous Windows, **Mettre à jour maintenant** télécharge le programme d'installation. Il installe la nouvelle version de toutes les applications JeffOffice et vous demande de fermer celles qui sont ouvertes. Sur les autres ordinateurs, le bouton s'intitule **Ouvrir la page de téléchargement**. Il ouvre la page où vous pouvez obtenir la nouvelle version. Si JeffPub est également ouvert, un seul des deux vous signale une nouvelle version chaque jour.
- **Obtenir les versions bêta (préversions destinées aux tests)** permet à JeffDeck de proposer aussi les versions bêta. Elles sortent avant les versions habituelles et peuvent comporter des imperfections.
- **Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont JeffDeck est utilisé. Anonyme signifie que ces informations ne disent rien sur vous. Lorsque cette option est activée, JeffDeck envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffDeck, sa version, votre système d'exploitation et votre langue, le nombre de fois où il a été démarré, la fréquence d'utilisation de chaque commande, et le type de fichier que vous avez ouvert ou enregistré (par exemple, .pptx). Si JeffDeck se ferme de façon inattendue, il envoie aussi un rapport d'erreur au prochain démarrage. Ce rapport indique à quel endroit du programme il s'est arrêté, afin que le problème puisse être trouvé et corrigé. JeffDeck n'envoie jamais vos fichiers, leurs noms, ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport d'erreur n'est envoyé non plus. JeffDeck pose la question lors du premier démarrage, dans la fenêtre **Aider à améliorer JeffDeck**. Il ne la pose pas si vous avez déjà fait votre choix dans JeffPub ou pendant l'installation.
- **Déclaration de confidentialité**, à côté de cette case, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où cela va et combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** s'affiche sous la case à cocher. Il s'agit d'un nombre aléatoire généré pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, il affiche **aucun**.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. Les désactiver efface le numéro de votre ordinateur, et sans lui, nous ne pouvons pas savoir quelles données vous appartiennent.
- Pour désactiver les statistiques sans ouvrir JeffDeck, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors comptabilisé ni envoyé, et JeffDeck ne pose pas la question du premier démarrage.

**Nom d'utilisateur**, **Initiales**, **Thème** et **Langue d'affichage**, ainsi que les choix concernant les mises à jour et les statistiques, s'appliquent à toutes les applications JeffOffice : une modification ici se répercute dans chacune d'elles.

## Vérification

Ces cases contrôlent l'orthographe et les aides à la saisie. Voir [Saisir et mettre en forme du texte](text).

- **Vérifier l'orthographe lors de la frappe**
- **Ignorer les mots en MAJUSCULES**
- **Ignorer les mots qui contiennent des chiffres**
- **Remplacer le texte lors de la frappe (correction automatique)** corrige les fautes de frappe courantes.
- **Remplacer les guillemets droits par des guillemets typographiques** transforme " et ' en guillemets courbes.
- **Mise en forme automatique lors de la frappe (tirets, fractions, ordinaux, listes automatiques)**
- **Transformer les adresses web en liens au fur et à mesure de la saisie** transforme une adresse web ou e-mail en lien lorsque vous appuyez sur la barre d'espace après elle. Annuler ne supprime que le lien.

## Affichage

- **Afficher les règles** affiche les règles le long des bords de la diapositive. C'est la même chose que **Affichage > Afficher > Règle**, et cela s'applique à toutes les fenêtres, actuelles et à celles que vous ouvrirez.
- **Aligner les objets sur les autres objets** fait en sorte qu'un objet que vous déplacez s'aligne sur les autres objets de la diapositive.

## Enregistrement

- **Incorporer les polices dans le fichier** place dans le fichier, à l'enregistrement, les polices utilisées par vos diapositives, afin qu'elles s'affichent de la même façon sur un ordinateur qui ne les possède pas. Cette option est activée tant que vous ne la désactivez pas. Elle n'incorpore que les polices fournies avec JeffDeck, et elle augmente la taille du fichier. La rubrique Ouvrir et enregistrer des présentations donne les détails, sous « Polices dans le fichier ».
- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire, du fichier écrit (un fichier .pptx ou .odp, un modèle et une présentation envoyée par e-mail), l'auteur, la personne ayant enregistré le fichier en dernier, la société, le responsable, ainsi que les noms, initiales et heures des commentaires. Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier sont conservées telles quelles, et le document ouvert n'est pas modifié. C'est le même choix dans toutes les applications JeffOffice : le modifier dans l'une le modifie dans toutes. La même case figure sur la page **Infos**.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages, **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les raccourcis clavier des commandes de JeffDeck. Voir « Choisir vos propres raccourcis clavier » dans la rubrique sur le clavier.

### Personnaliser le ruban

La page **Ruban** répertorie les onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case à côté pour le masquer, et cochez-la pour le rafficher de nouveau. Un groupe masqué ne prend pas de place, et ses commandes restent accessibles par leurs raccourcis clavier. **Accueil** n'a pas de case, car il reste toujours affiché, et il en va de même pour le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve à la fin de la liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Saisissez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut figurer sur n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Placer des commandes dans votre groupe.** Choisissez votre groupe, repérez une commande dans la liste **Commandes** (saisissez du texte dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande qui a un nom peut être ajoutée. Une commande ne peut figurer qu'une fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande personnel, saisissez le nouveau nom, puis cliquez sur **Renommer**. Une commande renommée fait la même chose qu'auparavant, et le nouveau nom n'est que le texte de son bouton. Pour renommer une commande avec un nom vide, lui redonner son nom d'origine.
- **Réorganiser.** Choisissez un onglet et cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets propres au ruban ainsi que les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même façon à l'intérieur de leur onglet ou de leur groupe. Les groupes propres au ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande personnel, puis cliquez sur **Supprimer**.

**Réinitialiser une** remet l'onglet choisi dans son état initial : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés disparaissent. Un onglet personnel est supprimé. **Tout réinitialiser** remet l'ensemble du ruban dans son état initial, y compris l'ordre des onglets.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres JeffDeck ouvertes affichent votre ruban, ainsi que toutes les fenêtres que vous ouvrirez plus tard. JeffDeck ne conserve que ce qui diffère du ruban habituel : un onglet ou un groupe ajouté par une version ultérieure apparaît donc automatiquement. Vos onglets et groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne se confondent pas avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche indique ce qui figure actuellement sur la barre, et la liste **Commandes**, à droite, contient toutes les commandes qui ont un nom (saisissez du texte dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes**, puis cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre. Une commande qui ouvre un menu conserve son menu sur la barre. Une commande ne peut figurer qu'une fois sur la barre.
- **Clic droit.** Vous pouvez aussi cliquer avec le bouton droit sur une commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou cliquer avec le bouton droit sur un bouton de la barre et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour placer un trait fin à la fin, puis déplacez-le où vous le souhaitez.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le, puis cliquez sur **Supprimer**.

**Réinitialiser une** remet la commande choisie à sa place habituelle, ou la retire de la barre lorsque JeffDeck ne la place pas habituellement là. **Tout réinitialiser** rétablit la barre d'outils livrée avec JeffDeck. Le bouton Tout réinitialiser de cette page ne modifie pas votre ruban, et celui de la page **Ruban** ne modifie pas votre barre d'outils.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres JeffDeck ouvertes affichent votre barre d'outils, ainsi que toutes les fenêtres que vous ouvrirez plus tard. La barre peut compter 100 commandes et séparateurs au total. Lorsque les boutons sont plus nombreux que ce qui tient dans le tiers de la largeur de la fenêtre, les autres attendent derrière un bouton **»** à l'extrémité de la barre, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffDeck conserve dans votre liste une commande qu'une version ultérieure ne possède plus, et l'affiche de nouveau si elle revient. Les boutons reçoivent des info-touches, les chiffres de 1 à 9 puis deux lettres, dans l'ordre où ils se trouvent sur la barre. La barre d'outils se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Voir aussi

- [Onglet Affichage](tab-view)
- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
