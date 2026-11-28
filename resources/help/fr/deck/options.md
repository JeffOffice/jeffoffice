# Modifier les options de JeffDeck
<!-- keywords: options, paramètres, préférences, thème, mode sombre, langue, nom d'utilisateur, initiales, règle, magnétisme, orthographe, correction automatique, guillemets, mises à jour, bêta, statistiques d'utilisation, à propos, enregistrer, incorporer les polices, polices, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, rétablir le ruban, modifier les raccourcis clavier, réattribuer les touches, rétablir les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffDeck**. Cliquez sur **Fichier > Options** pour l'ouvrir. Elle comporte sept onglets. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

- **Nom d'utilisateur** et **Initiales** apparaissent sur les commentaires que vous rédigez. Voir [Commentaires](comments).
- **Thème** peut être **Utiliser le paramètre système**, **Clair** ou **Sombre**. Le changement est visible immédiatement.
- **Langue d'affichage** est la langue des textes de JeffDeck. **Utiliser la langue du système** suit la langue de votre ordinateur. Les autres langues apparaissent dans cette liste lorsque des traductions sont disponibles. Une nouvelle langue prend effet à la prochaine ouverture de JeffDeck.
- **Rechercher les mises à jour au démarrage de JeffDeck** fait en sorte que JeffDeck recherche une version plus récente à chaque démarrage. Si elle existe, la fenêtre **Mise à jour disponible** présente les nouveautés. Choisissez **Mettre à jour maintenant**, **Plus tard** ou **Ignorer cette version**. Sous Windows, **Mettre à jour maintenant** télécharge le programme d'installation. Il installe la nouvelle version de toutes les applications JeffOffice et vous demande de fermer celles qui sont ouvertes. Sur les autres ordinateurs, le bouton s'intitule **Ouvrir la page de téléchargement** ; il ouvre la page où obtenir la nouvelle version. Si JeffPub est également ouvert, un seul des deux vous signale une nouvelle version chaque jour.
- **Obtenir les versions bêta (préversions destinées aux tests)** fait proposer à JeffDeck les versions bêta également. Elles sortent avant les versions habituelles et peuvent présenter des imperfections.
- **Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la façon dont JeffDeck est utilisé. Anonyme signifie que ces informations ne disent rien sur vous. Lorsque cette option est activée, JeffDeck envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : qu'il s'agit de JeffDeck, sa version, votre système d'exploitation et votre langue, le nombre de fois où il a été démarré, la fréquence d'utilisation de chaque commande, et le type de fichier ouvert ou enregistré (par exemple .pptx). Si JeffDeck se ferme de façon inattendue, il envoie aussi un rapport de plantage au démarrage suivant. Ce rapport indique à quel endroit du programme il s'est arrêté, afin que le problème puisse être trouvé et corrigé. JeffDeck n'envoie jamais vos fichiers, leurs noms, leur contenu ni rien de ce que vous avez saisi. Si cette option est désactivée, aucun rapport de plantage n'est envoyé non plus. JeffDeck pose la question la première fois qu'il démarre, dans une fenêtre intitulée **Aider à améliorer JeffDeck**. Il ne la pose pas si vous avez déjà fait un choix dans JeffPub ou lors de l'installation.
- **Déclaration de confidentialité**, à côté de cette case, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où cela va et combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** s'affiche sous la case. C'est un nombre aléatoire créé pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne dit rien sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été créé, il affiche **aucun**.
- Pour faire supprimer les statistiques que nous avons déjà reçues, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans lui nous ne pouvons pas savoir quels enregistrements vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffDeck, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffDeck ne pose pas la question de premier démarrage.

**Nom d'utilisateur**, **Initiales**, **Thème** et **Langue d'affichage**, ainsi que les choix concernant les mises à jour et les statistiques, sont communs à toutes les applications JeffOffice : une modification ici s'applique à chacune d'elles.

## Vérification

Ces cases contrôlent la vérification de l'orthographe et l'aide à la saisie. Voir [Saisir et mettre en forme du texte](text).

- **Vérifier l'orthographe lors de la frappe**
- **Vérifier la grammaire au fur et à mesure de la saisie** signale par un trait pointillé bleu les erreurs de grammaire et de style évidentes dans un texte en anglais, quelle que soit la variante régionale. Les textes dans d'autres langues ne sont vérifiés que pour l'orthographe.
- **Paramètres de grammaire** ouvre la liste des types de problèmes à rechercher : grammaire, ponctuation, majuscules, mots confondus, répétitions et style (désactivé tant que vous ne l'activez pas).
- **Ignorer les mots en MAJUSCULES**
- **Ignorer les mots qui contiennent des chiffres**
- **Remplacer le texte pendant la saisie (correction automatique)** corrige les fautes de frappe courantes. Dans **Options de correction automatique**, l'onglet **Symboles mathématiques** contient un commutateur, **Remplacer les noms des symboles mathématiques pendant la saisie**, désactivé tant que vous ne l'activez pas. Lorsqu'il est activé, une barre oblique inverse suivie d'un nom, comme \alpha, devient le symbole (α) dès que vous tapez une espace ou un signe après lui. Le nom doit commencer un mot : un chemin de dossier tel que C:\temp\pi reste tel que vous l'avez saisi. L'onglet présente tous les noms. JeffDeck, JeffWrite et JeffPub les transforment ; JeffCrunch ne modifie rien dans une cellule.
- **Remplacer les guillemets droits par des guillemets typographiques** transforme " et ' en guillemets courbes.
- **Mise en forme automatique pendant la saisie (tirets, fractions, ordinaux, listes automatiques)**
- **Transformer les adresses web en liens au fur et à mesure de la saisie** fait d'une adresse web ou e-mail un lien lorsque vous appuyez sur la barre d'espace après elle. Annuler ne retire que le lien.

## Affichage

- **Afficher la règle** affiche les règles le long des bords de la diapositive. C'est la même chose que **Affichage > Afficher > Règle**, et cela s'applique à toutes les fenêtres, actuelles et futures.
- **Aligner les objets sur d'autres objets** fait en sorte qu'un objet que vous déplacez s'aligne sur les autres objets de la diapositive.

## Options avancées

- **Utiliser la touche Inser pour activer et désactiver le mode Refrappe** est désactivée au départ. Lorsqu'elle est activée, la touche **Insert** bascule entre deux modes de saisie dans une zone de texte. Normalement, chaque lettre saisie repousse les lettres qui suivent. En mode **Refrappe**, chaque lettre saisie remplace la lettre située après le curseur, jusqu'à la fin du paragraphe. La barre d'état affiche **Insertion** ou **Refrappe** pour indiquer le mode actif. Appuyez de nouveau sur **Insert** pour revenir au mode précédent.

## Enregistrement

- **Incorporer les polices dans le fichier** place dans le fichier, à l'enregistrement, les polices utilisées par vos diapositives, afin qu'elles s'affichent de la même façon sur un ordinateur qui ne possède pas ces polices. Cette option est activée tant que vous ne la désactivez pas. Elle n'incorpore que les polices livrées avec JeffDeck, et elle augmente la taille du fichier. La rubrique Ouvrir et enregistrer des présentations contient les détails, sous « Polices dans le fichier ».
- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire l'auteur, la personne ayant enregistré le fichier en dernier, la société, le responsable, ainsi que les noms, initiales et heures des commentaires, du fichier écrit (fichier .pptx ou .odp, modèle, et présentation envoyée par e-mail). Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier ne sont pas modifiées, et le document ouvert n'est pas touché. C'est le même choix dans toutes les applications JeffOffice : une modification ici s'applique à toutes. La même case figure sur la page **Infos**.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages : **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les raccourcis clavier des commandes de JeffDeck. Voir « Choisir vos propres raccourcis clavier » dans la rubrique sur le clavier.

### Personnaliser le ruban

La page **Ruban** répertorie les onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case à côté pour le masquer, et cochez-la pour le réafficher. Un groupe masqué ne prend pas de place, et ses commandes restent accessibles par leurs raccourcis clavier. **Accueil** n'a pas de case, car il reste toujours affiché, de même que le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve en fin de liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Saisissez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut figurer sur n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Placer des commandes dans votre groupe.** Choisissez votre groupe, recherchez une commande dans la liste **Commandes** (saisissez dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande ayant un nom peut être ajoutée. Une commande ne peut figurer qu'une seule fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande personnel, saisissez le nouveau nom et cliquez sur **Renommer**. Une commande renommée fait toujours la même chose, et le nouveau nom n'est que le texte de son bouton. Pour renommer une commande avec un nom vide, rétablissez son nom d'origine.
- **Réorganiser.** Choisissez un onglet, puis cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets du ruban comme les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même façon à l'intérieur de leur onglet ou de leur groupe. Les groupes du ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande personnel et cliquez sur **Supprimer**.

**Rétablir un élément** remet l'onglet choisi dans son état d'origine : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés sont supprimés. Un onglet personnel est supprimé. **Tout rétablir** remet le ruban entier dans son état d'origine, y compris l'ordre des onglets.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffDeck affichent votre ruban, de même que toutes les fenêtres que vous ouvrirez ensuite. JeffDeck ne conserve que ce qui diffère du ruban habituel : un onglet ou un groupe ajouté par une version ultérieure apparaît donc de lui-même. Vos onglets et vos groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne entrent pas en conflit avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche affiche ce qui figure actuellement sur la barre, et la liste **Commandes** de droite présente toutes les commandes ayant un nom (saisissez dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes** et cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre. Une commande qui ouvre un menu conserve son menu sur la barre. Une commande ne peut figurer qu'une seule fois sur la barre.
- **Clic droit.** Vous pouvez aussi cliquer avec le bouton droit sur n'importe quelle commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou cliquer avec le bouton droit sur un bouton de la barre et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page. Le même menu contient aussi **Afficher la barre d'outils rapide sous le ruban**. JeffDeck mémorise ce choix.
- **Placer la barre sous le ruban.** Cochez **Afficher la barre d'outils rapide sous le ruban** pour placer la barre dans une ligne sous le ruban, au lieu de la laisser en haut à gauche. Les deux menus du clic droit proposent aussi **Afficher la barre d'outils rapide sous le ruban**. JeffDeck mémorise ce choix.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour placer un trait fin à la fin de la barre, puis déplacez-le où vous le souhaitez.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le et cliquez sur **Supprimer**.

**Rétablir un élément** remet la commande choisie à sa place habituelle dans JeffDeck, ou la retire de la barre si JeffDeck ne la place pas habituellement sur celle-ci. **Tout rétablir** remet la barre d'outils livrée avec JeffDeck. Tout rétablir sur cette page ne modifie pas votre ruban, et tout rétablir sur la page **Ruban** ne modifie pas votre barre d'outils.

Rien ne change tant que vous ne cliquez pas sur **OK**. Ensuite, toutes les fenêtres ouvertes de JeffDeck affichent votre barre d'outils, de même que toutes les fenêtres que vous ouvrirez ensuite. La barre peut contenir au total 100 commandes et séparateurs. Lorsqu'il y a plus de boutons qu'il n'en tient dans un tiers de la largeur de la fenêtre, les autres attendent derrière un bouton **»** placé à la fin de la barre, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffDeck conserve dans votre liste une commande qu'une version ultérieure ne possède plus, et la réaffiche si elle revient. Les boutons reçoivent des info-touches, les chiffres de 1 à 9 puis deux lettres, dans l'ordre où ils figurent dans la barre. La barre se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Sécurité

L'onglet **Sécurité** détermine ce que fait JeffDeck à l'égard des éléments vers lesquels un fichier peut pointer : fichiers liés, films et sons liés, et fichiers provenant d'Internet. Les macros sont conservées dans un fichier et ne sont jamais exécutées, et un lien qui lancerait un programme est toujours refusé. Voir [Sécurité](security).

## Voir aussi

- [Onglet Affichage](tab-view)
- [Récupérer un travail non enregistré](recover)
- [L'écran Fichier](file-screen)
