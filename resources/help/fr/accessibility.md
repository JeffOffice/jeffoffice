# Lecteurs d'écran, contraste élevé et texte de remplacement
<!-- keywords: accessibilité, lecteur d'écran, narrateur, aveugle, malvoyant, contraste élevé, mode sombre, thème sombre, contour de focus, texte de remplacement, texte alternatif, description d'image, zoom, agrandir, clavier uniquement -->

JeffPub est conçu pour que les personnes qui utilisent un lecteur d'écran, le clavier ou des couleurs à contraste élevé puissent l'utiliser elles aussi.

## Lecteurs d'écran

Un **lecteur d'écran** est un programme qui lit à voix haute le contenu de l'écran. JeffPub fonctionne avec des lecteurs d'écran tels que Narrateur sous Windows. Chaque commande a un nom et une description. Cela concerne les onglets, boutons, galeries et boutons de couleur du ruban, la liste des pages, la page, les volets latéraux et la barre d'état.

Les objets de la page sont également lus. Sur la page, appuyez sur **Tab** pour sélectionner l'objet suivant : le lecteur d'écran indique son type et son contenu, par exemple « Zone de texte : Soldes de printemps » ou « Image : une grange rouge », puis sa position et sa taille. Une image sans texte de remplacement est annoncée comme « Image, sans texte de remplacement », afin que vous sachiez lesquelles doivent encore en recevoir un. Appuyez sur **Entrée** pour saisir du texte dans une zone de texte ; le lecteur d'écran peut alors lire son texte, la ligne ou le mot où se trouve le curseur, ainsi que ce que vous sélectionnez.

- La liste des pages annonce chaque page sous la forme « Page 1 », « Page 2 », et ainsi de suite. Si vous avez donné un nom à une page, celui-ci suit, par exemple « Page 3, Couverture ».
- La zone de la page s'appelle « Page ». Sa description indique les touches à utiliser : Tab sélectionne l'objet suivant, les touches fléchées le déplacent, Entrée permet de modifier son texte et Supprimer le retire.
- Les onglets sont annoncés comme des onglets du ruban. Ceux qui apparaissent lorsque vous sélectionnez un élément indiquent aussi le groupe auquel ils appartiennent, par exemple Outils image.

## Utiliser le clavier

Vous pouvez accéder au ruban, à la liste des pages et aux objets de la page uniquement avec le clavier. Appuyez sur **F6** pour passer du ruban à la liste des pages, puis à la page. Appuyez sur **Alt** pour afficher les info-touches. Un contour coloré indique la commande qui a le focus. Voir [Utiliser JeffPub avec le clavier](keyboard).

## Modifier les couleurs

- **Clair ou sombre** : cliquez sur **Fichier > Options**, puis, dans l'onglet **Général**, définissez **Thème** sur **Clair**, **Sombre** ou **Utiliser le paramètre système**. Le changement est immédiat.
- **Contraste élevé** : sous Windows, lorsque vous activez un thème à contraste élevé, JeffPub le suit. Il utilise les couleurs choisies par Windows et s'adapte lorsque vous activez ou désactivez ce thème.

## Rendre les images plus faciles à comprendre

Le **texte de remplacement** (texte alternatif) est une courte description d'une image ou d'un autre objet. Les lecteurs d'écran la lisent à voix haute. JeffPub la reprend également dans les pages web et les livres électroniques que vous enregistrez.

1. Faites un clic droit sur l'image ou l'objet.
2. Cliquez sur **Format de l'objet…**.
3. Cliquez sur l'onglet **Texte de remplacement**.
4. Saisissez une description dans la zone **Texte de remplacement**.
5. Cliquez sur **OK**.

Décrivez en quelques mots ce que montre l'image, par exemple « Deux enfants plantant un arbre ». Les icônes et codes-barres que vous insérez reçoivent automatiquement un texte de remplacement.

## Vérifier la page

Cliquez sur **Révision > Vérifier l'accessibilité** pour lister ce qui peut être difficile à utiliser pour certaines personnes. Outre les images et les formes sans texte de remplacement, la vérification détecte les quatre autres problèmes suivants.

- **Texte difficile à lire.** Le texte doit présenter un contraste suffisant avec ce qui se trouve derrière lui : le remplissage de sa zone de texte, ou la couleur de la page lorsque la zone n'en a pas. La vérification calcule le contraste sous forme de rapport. Un rapport de 4,5 pour 1 suffit pour un texte courant, et de 3 pour 1 pour un grand texte (18 points ou plus, ou 14 points ou plus en gras). Par exemple, le gris #777777 sur blanc donne 4,48 pour 1, ce qui est juste insuffisant, tandis que #767676 donne 4,54 pour 1. Choisissez une couleur de texte plus foncée ou un arrière-plan plus clair. Lorsque le texte se trouve sur une image, la vérification ne peut pas le déterminer ; elle ajoute donc un conseil vous invitant à contrôler le contraste à l'œil.
- **Objets lus dans un ordre différent.** Un lecteur d'écran et un PDF balisé lisent les objets d'une page de l'arrière vers l'avant de la pile. Si cet ordre ne va pas de haut en bas, puis de gauche à droite, la vérification signale sous forme de conseils les objets mal placés. Cliquez sur **Ordre de lecture** pour afficher les objets dans l'ordre où ils sont lus. Sélectionnez un objet, puis cliquez sur **Lire plus tôt** ou **Lire plus tard**, ou faites-le glisser. Chaque déplacement correspond à une étape d'annulation et modifie également l'objet situé au premier plan.
- **Tableau sans ligne d'en-tête.** Un lecteur d'écran utilise la première ligne d'un tableau pour nommer ses colonnes. Un tableau dont la première ligne n'est pas marquée comme ligne d'en-tête est signalé comme avertissement. Cliquez sur le problème, puis sur **Utiliser la première ligne comme en-tête** pour activer la ligne d'en-tête. Cette action peut être annulée.
- **Tableau avec cellules fusionnées.** Un lecteur d'écran peut perdre sa position dans un tableau lorsque des cellules sont fusionnées ; un tableau contenant une cellule fusionnée est donc signalé comme avertissement. Gardez chaque cellule du tableau entière : séparez les cellules fusionnées, ou utilisez deux tableaux.
- **Lien qui n'indique pas sa destination.** Un lecteur d'écran peut énumérer les liens d'une page sous forme de liste ; le texte de chaque lien doit donc indiquer la page qu'il ouvre. Un lien dont le texte est sa propre adresse, se limite à « ici », « cliquez ici », « lire la suite » ou une expression équivalente, ou est vide, est signalé comme avertissement. Modifiez le texte, par exemple « Notre rapport annuel » au lieu de « cliquez ici ». Les 50 premiers sont listés un à un, et les autres sont comptés sur une seule ligne.

## Agrandir la page

Faites un zoom avant pour lire un petit texte. Utilisez les commandes de **zoom** de la barre d'état, maintenez Ctrl enfoncée tout en faisant tourner la molette de la souris, ou choisissez une taille dans la zone **Zoom** de l'onglet **Affichage**. Le curseur de zoom va d'environ 10 % à 1600 %.

## Voir aussi

- [Utiliser JeffPub avec le clavier](keyboard)
- [Ajouter et ajuster des images](pictures)
- [Modifier les paramètres de JeffPub](options)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
