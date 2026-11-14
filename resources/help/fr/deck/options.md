# Modifier les paramètres de JeffDeck

<!-- keywords: options, paramètres, préférences, thème, mode sombre, langue, nom d'utilisateur, initiales, règle, aligner, orthographe, correction automatique, guillemets, mises à jour, bêta, statistiques d'utilisation, à propos, enregistrer, incorporer les polices, polices, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, rétablir le ruban, modifier les raccourcis clavier, réaffecter les touches, rétablir les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffDeck**. Cliquez sur **Fichier > Options** pour l'ouvrir. Elle comporte cinq onglets. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

- **Nom d'utilisateur** et **Initiales** apparaissent sur les commentaires que vous rédigez. Voir [Commentaires](comments).
- **Thème** prend la valeur **Utiliser le paramètre système**, **Clair** ou **Sombre**. Le changement est visible immédiatement.
- **Langue d'affichage** est la langue des textes de JeffDeck. **Utiliser la langue du système** suit la langue de votre ordinateur. D'autres langues apparaissent dans cette liste lorsque des traductions sont disponibles. Une nouvelle langue prend effet à la prochaine ouverture de JeffDeck.
- **Rechercher les mises à jour au démarrage de JeffDeck** demande à JeffDeck de chercher une version plus récente à chaque démarrage. Si une version est disponible, la fenêtre **Mise à jour disponible** présente les nouveautés. Choisissez **Mettre à jour maintenant**, **Plus tard** ou **Ignorer cette version**. Sous Windows, **Mettre à jour maintenant** télécharge le programme d'installation. Il installe la nouvelle version de toutes les applications JeffOffice et vous demande de fermer celles qui sont ouvertes. Sur les autres ordinateurs, le bouton s'intitule **Ouvrir la page de téléchargement** ; il ouvre la page où vous pouvez obtenir la nouvelle version. Si JeffPub est également ouvert, un seul des deux vous signale chaque jour une nouvelle version.
- **Obtenir les versions bêta (préversions destinées aux tests)** permet à JeffDeck de vous proposer aussi les versions bêta. Celles-ci sortent avant les versions habituelles et peuvent présenter des imperfections.
- **Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont vous utilisez JeffDeck. Anonyme signifie que ces informations ne disent rien sur votre identité. Lorsque cette option est activée, JeffDeck envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffDeck, sa version, votre système d'exploitation et votre langue, le nombre de fois où il a été lancé, la fréquence d'utilisation de chaque commande, et le type de fichier que vous avez ouvert ou enregistré (par exemple .pptx). Si JeffDeck se ferme de façon inattendue, il envoie également un rapport d'incident au prochain démarrage. Ce rapport indique à quel endroit du programme il s'est arrêté, afin que le problème puisse être trouvé et corrigé. JeffDeck n'envoie jamais vos fichiers, leurs noms, ni leur contenu, ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport d'incident n'est envoyé non plus. JeffDeck vous interroge à ce sujet lors du premier démarrage, dans une fenêtre intitulée **Aidez à améliorer JeffDeck**. Il ne pose pas la question si vous avez déjà fait un choix dans JeffPub ou dans le programme d'installation.
- **Déclaration de confidentialité**, à côté de cette case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où, et pendant combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** est affiché sous la case à cocher. Il s'agit d'un nombre aléatoire généré pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, la mention **aucun** s'affiche.
- Pour faire supprimer les statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans lui, nous ne pouvons pas savoir quels enregistrements vous appartiennent.
- Pour désactiver les statistiques sans ouvrir JeffDeck, définissez la variable d'environnement `JP_NO_TELEMETRY` avec n'importe quelle valeur avant de lancer le programme. Rien n'est alors compté ni envoyé, et JeffDeck ne pose pas la question du premier démarrage.

**Nom d'utilisateur**, **Initiales**, **Thème** et **Langue d'affichage**, ainsi que les choix concernant les mises à jour et les statistiques, s'appliquent à toutes les applications JeffOffice : une modification ici se répercute dans chacune d'elles.

## Vérification

Ces cases contrôlent l'orthographe et les aides à la saisie. Voir [Saisir et mettre en forme du texte](text).

- **Vérifier l'orthographe lors de la frappe**
- **Vérifier la grammaire au fur et à mesure de la saisie** signale par un pointillé bleu les fautes de grammaire et de style évidentes dans les textes en anglais, en français, en allemand et en espagnol.
- **Paramètres de grammaire** ouvre la liste des types de problèmes à rechercher : grammaire, ponctuation, majuscules, mots confondus, répétitions et style (désactivé tant que vous ne l'activez pas).
- **Ignorer les mots en MAJUSCULES**
- **Ignorer les mots qui contiennent des chiffres**
- **Remplacer le texte lors de la frappe (correction automatique)** corrige les fautes de frappe courantes.
- **Remplacer les guillemets droits par des guillemets typographiques** transforme " et ' en guillemets courbes.
- **Mise en forme automatique lors de la frappe (tirets, fractions, ordinaux, listes automatiques)**
- **Transformer les adresses web en liens au fur et à mesure de la saisie** transforme une adresse web ou e-mail en lien lorsque vous appuyez sur la barre d'espace après elle. Annuler ne retire que le lien.

## Affichage

- **Afficher les règles** affiche les règles le long des bords de la diapositive. Cela revient à **Affichage > Afficher > Règle**, et s'applique à toutes les fenêtres, actuelles et à celles que vous ouvrirez.
- **Aligner sur les objets** fait en sorte qu'un objet que vous déplacez s'aligne sur les autres objets de la diapositive.

## Enregistrement

- **Incorporer les polices dans le fichier** ajoute dans le fichier, à l'enregistrement, les polices utilisées par vos diapositives, afin qu'elles s'affichent de la même façon sur un ordinateur qui ne possède pas ces polices. Cette option est activée tant que vous ne la désactivez pas. Seules les polices fournies avec JeffDeck sont incorporées, et le fichier devient plus volumineux. La rubrique Ouvrir et enregistrer des présentations donne les détails, sous « Polices dans le fichier ».
- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire du fichier écrit (fichier .pptx ou .odp, modèle, et présentation envoyée par e-mail) l'auteur, la personne qui a enregistré le fichier en dernier, la société, le responsable, ainsi que les noms, initiales et heures des commentaires. Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier sont conservées telles quelles, et le document ouvert n'est pas modifié. Ce choix est commun à toutes les applications JeffOffice : une modification ici se répercute dans toutes. La même case figure sur la page **Infos**.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages : **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les touches de raccourci des commandes de JeffDeck. Voir « Choisir vos propres raccourcis clavier » dans la rubrique sur le clavier.

### Personnaliser le ruban

La page **Ruban** répertorie les onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case en regard pour le masquer, et cochez-la pour l'afficher de nouveau. Un groupe masqué ne prend plus de place, et ses commandes restent accessibles par leurs touches de raccourci. **Accueil** n'a pas de case, car il reste toujours affiché, de même que le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve à la fin de la liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Saisissez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut être placé sur n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Ajouter des commandes à votre groupe.** Choisissez votre groupe, recherchez une commande dans la liste **Commandes** (saisissez du texte dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande ayant un nom peut être ajoutée. Une commande ne peut figurer qu'une fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande que vous avez créé, saisissez le nouveau nom, puis cliquez sur **Renommer**. Une commande renommée fait la même chose qu'avant : le nouveau nom n'apparaît que sur son bouton. Pour rétablir le nom d'origine d'une commande, renommez-la avec un nom vide.
- **Réorganiser.** Choisissez un onglet, puis cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets du ruban ainsi que les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même façon à l'intérieur de leur onglet ou de leur groupe. Les groupes d'origine du ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande que vous avez créé, puis cliquez sur **Supprimer**.

**Réinitialiser un** rétablit l'onglet choisi : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés disparaissent. Un onglet personnel est supprimé. **Tout réinitialiser** rétablit l'ensemble du ruban, y compris l'ordre des onglets.

Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffDeck affichent votre ruban, de même que toutes les fenêtres que vous ouvrirez plus tard. JeffDeck ne conserve que ce qui diffère du ruban habituel : un onglet ou un groupe ajouté par une version ultérieure apparaît donc automatiquement. Vos onglets et groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne se confondent pas avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche présente ce qui figure actuellement sur la barre, et la liste **Commandes** de droite contient toutes les commandes ayant un nom (saisissez du texte dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes**, puis cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre. Une commande qui ouvre un menu conserve son menu sur la barre. Une commande ne peut figurer qu'une fois sur la barre.
- **Clic droit.** Vous pouvez aussi faire un clic droit sur une commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou faire un clic droit sur un bouton de la barre et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page.
- **Placer la barre sous le ruban.** Cochez **Afficher la barre d'outils rapide sous le ruban** pour placer la barre sur une ligne sous le ruban, au lieu de la placer en haut à gauche. Les deux menus du clic droit proposent aussi **Afficher la barre d'outils rapide sous le ruban**. JeffDeck mémorise votre choix.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour placer un trait fin à la fin de la barre, puis déplacez-le où vous le souhaitez.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le, puis cliquez sur **Supprimer**.

**Réinitialiser un** rétablit la commande choisie : à l'endroit où JeffDeck la place habituellement, ou hors de la barre si JeffDeck ne l'y place pas en temps normal. **Tout réinitialiser** rétablit la barre d'outils livrée avec JeffDeck. Sur cette page, Tout réinitialiser ne modifie pas votre ruban, et sur la page **Ruban**, Tout réinitialiser ne modifie pas votre barre d'outils.

Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffDeck affichent votre barre d'outils, de même que toutes les fenêtres que vous ouvrirez plus tard. La barre peut contenir 100 commandes et séparateurs au total. Lorsqu'il y a plus de boutons qu'il n'en tient dans le tiers de la largeur de la fenêtre, les autres attendent derrière un bouton **»** placé à la fin de la barre, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffDeck conserve dans votre liste une commande qu'une version ultérieure ne possède plus, et l'affiche de nouveau si elle revient. Les boutons reçoivent des info-touches, les chiffres de 1 à 9 puis deux lettres, dans l'ordre où ils figurent sur la barre. La barre se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Voir aussi

- [Onglet Affichage](tab-view)
- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
