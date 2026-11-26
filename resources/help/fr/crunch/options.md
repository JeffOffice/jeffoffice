# Modifier les paramètres de JeffCrunch
<!-- keywords: options de formules, paramètres de calcul, variation maximale, itération, récupération, récupérer le travail non enregistré, récupération automatique, vérification, modification, saisie semi-automatique, poignée de recopie, glisser-déplacer, décimales, options, paramètres, préférences, statistiques d'utilisation, numéro d'installation, confidentialité, déclaration de confidentialité, JP_NO_TELEMETRY, supprimer les statistiques, mises à jour, à propos, modifier dans la cellule, couleur du quadrillage, zoom par défaut, largeur standard, confidentialité, informations personnelles, personnaliser le clavier, personnaliser le ruban, personnaliser la barre d'outils rapide, barre d'outils rapide, ajouter une commande à la barre d'outils rapide, masquer un onglet, masquer un groupe, ajouter un onglet, ajouter un groupe, renommer un onglet, réinitialiser le ruban, modifier les raccourcis clavier, réattribuer les touches, réinitialiser les touches -->

Vos paramètres se trouvent dans la fenêtre **Options de JeffCrunch**. Cliquez sur **Fichier > Options** pour l'ouvrir. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

## Général

**Afficher la mini barre d'outils à la sélection** est activée par défaut. Elle détermine si la petite barre de mise en forme s'affiche à côté du texte sélectionné et en haut du menu contextuel des cellules (voir [Saisir du texte dans les cellules](edit-cells)). Ce choix est commun aux applications JeffOffice.

## Modification

La page **Modification** modifie le fonctionnement de la saisie dans les cellules.

- **Après validation, déplacer la sélection** indique où va la sélection lorsque vous terminez une saisie : vers le bas, vers la droite, vers le haut, vers la gauche, ou nulle part.
- **Proposer de compléter un texte à partir des cellules situées au-dessus et en dessous** correspond à la saisie semi-automatique. Lorsque vous tapez les premières lettres d'un texte, et qu'un seul texte de cette colonne commence par ces lettres, JeffCrunch affiche la suite, sélectionnée. Appuyez sur **Entrée** pour la conserver. Continuez à taper pour l'ignorer, ou appuyez sur **Retour arrière** pour la supprimer. Elle ne tient compte que du bloc de cellules voisin de celle où vous saisissez, et seulement des textes, pas des nombres.
- **Utiliser la poignée de recopie et glisser les cellules pour les déplacer ou les copier** active le petit carré situé à l'angle de la sélection ainsi que le glisser-déplacer de son contour. Désactivez-le si vous déplacez des cellules par erreur. Copier et Coller fonctionnent toujours.
- **Afficher un zéro dans les cellules dont la valeur est zéro (cette feuille)** active ou désactive l'affichage des zéros pour la feuille que vous consultez. Lorsqu'il est désactivé, une cellule qui contient 0 paraît vide, mais elle contient toujours ce 0 et compte pour 0 dans les calculs. Ce choix est enregistré avec la feuille.
- **Ajouter une virgule décimale aux nombres entiers saisis** correspond à la décimalisation automatique. Si vous réglez **Décimales** sur 2, la saisie de 1234 donne 12,34. Si vous la réglez sur -2, la saisie de 5 donne 500. Un nombre saisi avec une virgule décimale est enregistré tel que vous l'avez tapé.
- **Autoriser la modification directement dans les cellules** est activée par défaut. Lorsque vous la désactivez, une saisie ou la touche F2 se fait dans la barre de formule, et la cellule n'affiche le résultat qu'à la validation.

## Options de formules

La page **Formules** définit la manière dont le classeur ouvert calcule ses formules : **Calcul** (automatique, automatique sauf les tables de données, ou manuel), faut-il calculer les formules avant l'enregistrement du classeur lorsque le calcul est manuel, une formule qui fait référence à elle-même est-elle calculée par itérations, le nombre maximal d'itérations, et la variation qui arrête les itérations. Ces paramètres appartiennent au classeur, et non à vous : ils sont enregistrés dans son fichier et ne sont pas conservés pour les autres classeurs, et ils sont désactivés lorsqu'aucun classeur n'est ouvert. La variation est affichée avec tous ses chiffres, et vous pouvez la saisir sous forme de nombre, par exemple 0,0000001 ou 1e-7. Seul un paramètre que vous modifiez est enregistré : **OK** sans aucune modification n'ajoute aucune étape à Annuler et laisse le classeur tel qu'il était. Une modification constitue une seule étape d'Annuler. Le format de classeur récent et le format .xls ancien conservent tous les paramètres. Une copie au format .ods conserve les paramètres d'itération, mais n'a pas de place pour le calcul manuel : la copie calcule donc les formules au fur et à mesure de leur modification, et le rapport affiché avant l'enregistrement de la copie l'indique. Les mêmes paramètres se trouvent dans l'onglet **Formules**.

## Options d'enregistrement

**Enregistrer les informations de récupération automatique toutes les** définit la fréquence à laquelle JeffCrunch conserve une copie d'un classeur contenant des modifications non enregistrées, de 1 à 120 minutes. La copie est un fichier .xlsx placé dans un dossier que JeffCrunch réserve à cet usage. Elle est créée en arrière-plan, pour que vous puissiez continuer à saisir. Elle est supprimée lorsque vous enregistrez ou fermez le classeur. Si JeffCrunch se ferme brutalement, il vous propose ces copies au prochain démarrage. Vous pouvez aussi cliquer sur **Fichier > Ouvrir > Récupérer le travail non enregistré**. Un classeur protégé par un mot de passe n'est pas copié, car la copie ne serait pas protégée.

## Options de vérification

La page **Vérification** est commune aux applications JeffOffice. Elle détermine si l'orthographe est vérifiée au fur et à mesure de la saisie dans une cellule (désactivez **Vérifier l'orthographe lors de la frappe** pour supprimer les traits rouges ondulés dans toutes les fenêtres ouvertes et dans les autres applications ; la commande **Orthographe** de l'onglet Révision fonctionne toujours), si les mots en majuscules et les mots contenant des chiffres sont ignorés lors de la vérification, si la correction automatique corrige les mots pendant la saisie, si les guillemets droits deviennent des guillemets typographiques, si les tirets, les fractions et les ordinaux sont remplacés, et si une adresse web devient un lien. L'onglet **Symboles mathématiques** des Options de correction automatique concerne JeffWrite, JeffDeck et JeffPub : JeffCrunch ne transforme pas une barre oblique inverse suivie d'un nom en symbole dans une cellule. Voir [Orthographe](notes-links) et [Saisir du texte dans les cellules](edit-cells).

## Options d'affichage

- **Couleur du quadrillage** est la couleur des traits séparant les cellules à l'écran. C'est un choix pour votre écran seulement, il n'est pas enregistré dans le classeur.
- **Zoom d'un nouveau classeur** détermine la taille d'affichage des feuilles d'un nouveau classeur, de 10 % à 400 %.
- **Largeur de colonne standard (cette feuille)** est la largeur de chaque colonne de cette feuille qui n'a pas de largeur propre. Elle est identique à **Largeur par défaut** de l'onglet Accueil.
- **Afficher les onglets de feuille** et **Afficher les barres de défilement** masquent ou affichent la ligne d'onglets de feuille sous les cellules et les barres de défilement à droite et en bas. Ce sont des choix pour votre écran seulement, valables pour tous les classeurs que vous ouvrez.
- **Afficher la note** détermine l'aspect d'une cellule contenant une note : un petit repère sur la cellule, et la note affichée lorsque le pointeur passe dessus, le repère seul, ou rien. Une note que vous avez choisi d'afficher en permanence (**Afficher la note**) reste visible sur la feuille dans tous les cas.

## Personnaliser

L'onglet **Personnaliser** comporte trois pages, **Ruban**, **Barre d'outils rapide** et **Clavier**. La page **Clavier** permet de choisir les raccourcis clavier des commandes de JeffCrunch. Voir « Choisir vos propres raccourcis clavier » dans la rubrique sur le clavier.

### Personnaliser le ruban

La page **Ruban** affiche les onglets du ruban, chacun avec ses groupes.

- **Masquer ou afficher un onglet ou un groupe.** Décochez la case située à côté pour le masquer, et cochez-la pour l'afficher à nouveau. Un groupe masqué ne prend pas de place, et ses commandes restent accessibles par leurs raccourcis clavier. **Accueil** n'a pas de case, car il reste toujours affiché, et il en va de même pour le bouton **Fichier**. Un onglet qui n'apparaît que lorsqu'un élément est sélectionné, comme une image ou un tableau, se trouve en fin de liste et n'a pas de case.
- **Créer votre propre onglet ou groupe.** Saisissez un nom dans la zone **Nom**, puis cliquez sur **Nouvel onglet**. Pour créer un groupe, choisissez un onglet et cliquez sur **Nouveau groupe**. Un groupe personnel peut être placé dans n'importe quel onglet, y compris un onglet qui n'apparaît que lorsqu'un élément est sélectionné.
- **Ajouter des commandes à votre groupe.** Choisissez votre groupe, recherchez une commande dans la liste **Commandes** (saisissez dans la zone **Rechercher des commandes** pour réduire la liste), puis cliquez sur **Ajouter une commande**. Toute commande ayant un nom peut être ajoutée. Une commande ne peut figurer qu'une fois dans un groupe.
- **Renommer.** Choisissez un onglet, un groupe ou une commande que vous avez créé, saisissez le nouveau nom, puis cliquez sur **Renommer**. Une commande renommée fait la même chose, et le nouveau nom n'apparaît que sur son bouton. Pour rendre à une commande son nom d'origine, renommez-la avec un nom vide.
- **Réorganiser.** Choisissez un onglet et cliquez sur **Monter** ou **Descendre**. Vous pouvez déplacer les onglets du ruban ainsi que les vôtres, mais pas les onglets qui n'apparaissent que lorsqu'un élément est sélectionné. Vos groupes et vos commandes se déplacent de la même manière à l'intérieur de leur onglet ou de leur groupe. Les groupes du ruban conservent leur ordre.
- **Supprimer.** Choisissez un onglet, un groupe ou une commande que vous avez créé, puis cliquez sur **Supprimer**.

**Réinitialiser un élément** rétablit l'onglet choisi : il est affiché, ses groupes sont affichés, et les groupes que vous y avez ajoutés disparaissent. Un onglet que vous avez créé est supprimé. **Tout réinitialiser** rétablit l'ensemble du ruban, y compris l'ordre des onglets.

Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres de JeffCrunch ouvertes affichent votre ruban, ainsi que toutes les fenêtres que vous ouvrirez plus tard. JeffCrunch ne conserve que ce qui diffère du ruban habituel : un onglet ou un groupe ajouté par une version ultérieure apparaît donc de lui-même. Vos onglets et vos groupes reçoivent des info-touches (les lettres affichées lorsque vous appuyez sur **Alt**) qui ne se confondent pas avec les autres. Vous pouvez avoir 20 onglets personnels, 30 groupes personnels par onglet et 60 commandes par groupe.

### Personnaliser la barre d'outils rapide

La page **Barre d'outils rapide** concerne la petite barre d'outils située en haut à gauche de la fenêtre. Sa liste de gauche indique ce qui figure actuellement sur la barre, et la liste **Commandes** de droite contient toutes les commandes ayant un nom (saisissez dans la zone **Rechercher des commandes** pour la réduire, comme sur la page **Ruban**).

- **Ajouter une commande.** Choisissez-la dans la liste **Commandes**, puis cliquez sur **Ajouter une commande**. Elle est placée à la fin de la barre. Une commande qui ouvre un menu conserve son menu sur la barre d'outils. Une commande ne peut figurer qu'une fois sur la barre.
- **Clic droit.** Vous pouvez aussi cliquer avec le bouton droit sur une commande du ruban et choisir **Ajouter à la barre d'outils rapide**, ou cliquer avec le bouton droit sur un bouton de la barre et choisir **Supprimer de la barre d'outils rapide**. Le même menu contient **Personnaliser la barre d'outils rapide** et **Personnaliser le ruban**, qui ouvrent cette page.
- **Placer la barre sous le ruban.** Cochez **Afficher la barre d'outils rapide sous le ruban** pour placer la barre sur une ligne sous le ruban, au lieu de la placer en haut à gauche. Les deux menus du clic droit contiennent aussi **Afficher la barre d'outils rapide sous le ruban**. JeffCrunch conserve ce choix.
- **Ajouter un séparateur.** Cliquez sur **Ajouter un séparateur** pour placer un trait fin à la fin de la barre, puis déplacez-le où vous le souhaitez.
- **Réorganiser.** Choisissez une commande ou un séparateur dans la liste de gauche, puis cliquez sur **Monter** ou **Descendre**.
- **Supprimer.** Choisissez-le, puis cliquez sur **Supprimer**.

**Réinitialiser un élément** rétablit la commande choisie : à l'emplacement habituel de JeffCrunch, ou hors de la barre si JeffCrunch ne la place pas habituellement là. **Tout réinitialiser** rétablit la barre d'outils livrée avec JeffCrunch. Sur cette page, Tout réinitialiser ne modifie pas votre ruban, et sur la page **Ruban**, Tout réinitialiser ne modifie pas votre barre d'outils.

Rien ne change tant que vous n'avez pas cliqué sur **OK**. Ensuite, toutes les fenêtres de JeffCrunch ouvertes affichent votre barre d'outils, ainsi que toutes les fenêtres que vous ouvrirez plus tard. La barre peut contenir au total 100 commandes et séparateurs. Lorsque les boutons ne tiennent pas dans un tiers de la largeur de la fenêtre, les autres sont placés derrière un bouton **»** à la fin de la barre, afin que le titre de la fenêtre ne soit jamais repoussé hors de l'écran. JeffCrunch conserve dans votre liste une commande qu'une version ultérieure ne possède plus, et l'affiche à nouveau si elle revient. Les boutons reçoivent des info-touches : les chiffres de 1 à 9, puis deux lettres, dans l'ordre où ils figurent sur la barre. La barre d'outils se trouve toujours en haut à gauche, à côté du bouton **Fichier**.

## Options de confidentialité

- **Supprimer les informations personnelles de ce fichier à l'enregistrement** retire l'auteur, la dernière personne ayant enregistré le fichier, la société, le responsable, ainsi que les noms et les dates des notes et des commentaires, du fichier qui est écrit (un classeur, une copie, et un classeur envoyé par e-mail). Le contenu du fichier reste inchangé. Les propriétés personnalisées du fichier ne sont pas modifiées, et le document ouvert n'est pas modifié. Ce choix est le même dans toutes les applications JeffOffice : modifiez-le dans l'une, et il change dans toutes les autres. Pour effectuer cette opération une seule fois, utilisez **Inspecter le classeur** dans [Informations](info). La même case se trouve sur la page **Informations**.

## Statistiques d'utilisation

**Envoyer des statistiques d'utilisation anonymes** vous permet de partager des informations sur la manière dont JeffCrunch est utilisé. Anonymes signifie que ces informations ne disent rien sur votre identité. Lorsque cette option est activée, JeffCrunch envoie ces informations une fois par jour, ainsi que le jour de la mise à jour : le nom du logiciel, sa version, votre système d'exploitation et votre langue, le nombre de démarrages, la fréquence d'utilisation de chaque commande, et le type de fichier ouvert ou enregistré. Si JeffCrunch se ferme brutalement, il envoie aussi un rapport d'incident au prochain démarrage. JeffCrunch n'envoie jamais vos fichiers, leurs noms, ni leur contenu, ni ce que vous saisissez. Si cette option est désactivée, aucun rapport d'incident n'est envoyé non plus.

Ce choix vaut pour toutes les applications JeffOffice : modifiez-le ici, et il change dans chacune d'elles. JeffCrunch vous pose la question la première fois qu'il démarre, dans une fenêtre intitulée **Aidez à améliorer JeffCrunch**.

- **Déclaration de confidentialité**, à côté de la case à cocher, ouvre notre déclaration de confidentialité à l'adresse jeffoffice.com/privacy. Elle indique ce qui est envoyé, où, et pendant combien de temps nous le conservons. Nous conservons les statistiques pendant 24 mois.
- **Numéro d'installation** est affiché sous la case à cocher. C'est un nombre aléatoire généré pour votre copie de JeffOffice, identique dans toutes les applications JeffOffice. Il ne donne aucune information sur vous. Cliquez sur **Copier** pour le copier. Si les statistiques sont désactivées, ou si aucun numéro n'a encore été généré, la mention **aucun** s'affiche.
- Pour demander la suppression des statistiques que nous détenons déjà, envoyez votre numéro d'installation à info@jeffoffice.com **avant** de désactiver les statistiques. La désactivation efface le numéro de votre ordinateur, et sans lui nous ne pouvons pas savoir quels enregistrements vous concernent.
- Pour désactiver les statistiques sans ouvrir JeffCrunch, définissez la variable d'environnement `JP_NO_TELEMETRY` avec une valeur quelconque avant de le démarrer. Rien n'est alors compté ni envoyé, et JeffCrunch ne pose pas la question du premier démarrage.

## Sécurité

L'onglet **Sécurité** détermine ce que JeffCrunch fait des éléments vers lesquels un fichier peut pointer : fichiers liés, films et sons liés, et fichiers provenant d'Internet. Les macros sont conservées dans le fichier et ne sont jamais exécutées, et un lien qui lancerait un programme est toujours refusé. Voir [Sécurité](security).

## Voir aussi

- [Bienvenue dans JeffCrunch](start)
