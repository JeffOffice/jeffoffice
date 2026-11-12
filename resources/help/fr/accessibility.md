# Lecteurs d'écran, contraste élevé et texte de remplacement
<!-- keywords: accessibilité, lecteur d'écran, narrateur, non-voyant, aveugle, malvoyant, basse vision, contraste élevé, mode sombre, thème sombre, contour de focus, texte de remplacement, texte alternatif, description d'image, zoom, agrandir, loupe, clavier uniquement -->

JeffPub est conçu pour que les personnes qui utilisent un lecteur d'écran, le clavier ou des couleurs à contraste élevé puissent aussi s'en servir.

## Lecteurs d'écran

Un **lecteur d'écran** est un programme qui lit l'écran à voix haute. JeffPub fonctionne avec des lecteurs d'écran comme le Narrateur de Windows. Chaque contrôle possède un nom et une description. Cela comprend les onglets, boutons, galeries et boutons de couleur du ruban, la liste Pages, la page, les volets et la barre d'état.

Les objets de la page sont également lus. Sur la page, appuyez sur **Tab** pour sélectionner l'objet suivant : le lecteur d'écran indique son type et son contenu, par exemple « Zone de texte : Soldes de printemps » ou « Image : une grange rouge », puis sa position et sa taille. Une image sans texte de remplacement est lue « Image, sans texte de remplacement » : vous savez ainsi lesquelles doivent encore en recevoir un. Appuyez sur **Entrée** pour taper dans une zone de texte ; le lecteur d'écran peut alors lire son texte, la ligne ou le mot où se trouve le curseur, et ce que vous sélectionnez.

- La liste Pages lit chaque page sous la forme « Page 1 », « Page 2 », etc. Si vous avez donné un nom à une page, ce nom suit, par exemple « Page 3, Couverture ».
- La zone de la page s'appelle « Page ». Sa description indique les touches à utiliser : Tab sélectionne l'objet suivant, les touches de direction le déplacent, Entrée modifie son texte et Suppr le supprime.
- Les onglets sont lus comme des onglets du ruban. Les onglets qui apparaissent lorsque vous sélectionnez un élément indiquent aussi leur groupe, par exemple Outils Image.

## Utiliser le clavier

Vous pouvez atteindre le ruban, la liste Pages et les objets de la page uniquement au clavier. Appuyez sur F6 pour passer du ruban à la liste Pages et à la page. Appuyez sur Alt pour afficher les Info-touches. Un contour coloré indique le contrôle qui a le focus. Voir [Utiliser JeffPub au clavier](keyboard).

## Modifier les couleurs

- **Sombre ou clair** : cliquez sur **Fichier > Options** puis, sous l'onglet **Général**, définissez **Thème** sur **Clair**, **Sombre** ou **Utiliser le paramètre système**. La modification est immédiate.
- **Contraste élevé** : sous Windows, lorsque vous activez un thème à contraste élevé, JeffPub le suit. Il utilise les couleurs choisies par Windows et change dès que vous activez ou désactivez le thème.

## Rendre les images plus compréhensibles

Le **texte de remplacement** est une courte description d'une image ou d'un autre objet. Les lecteurs d'écran le lisent à voix haute. JeffPub l'intègre aussi dans les pages web et les livres électroniques que vous enregistrez.

1. Cliquez avec le bouton droit sur l'image ou l'objet.
2. Cliquez sur **Format de l'objet…**.
3. Cliquez sur l'onglet **Texte de remplacement**.
4. Tapez une description dans la zone **Texte de remplacement**.
5. Cliquez sur **OK**.

Décrivez ce que montre l'image en quelques mots, par exemple « Deux enfants plantant un arbre ». Les icônes et les codes-barres que vous insérez reçoivent automatiquement un texte de remplacement.

## Vérifier la page

Cliquez sur **Révision > Vérifier l'accessibilité** pour lister ce qui peut être difficile à utiliser pour certaines personnes. En plus des images et des formes sans texte de remplacement, le vérificateur trouve trois autres choses.

- **Du texte difficile à lire.** Le texte doit avoir assez de *contraste* avec ce qui se trouve derrière lui : le remplissage de son cadre, ou la couleur de la page si le cadre n'en a pas. Le vérificateur calcule le contraste sous forme de rapport. Un rapport de 4,5 pour 1 suffit pour le texte ordinaire, et 3 pour 1 pour le grand texte (18 points ou plus, ou 14 points ou plus en gras). Par exemple, le gris #777777 sur du blanc donne 4,48 pour 1, un peu trop peu, et #767676 donne 4,54 pour 1. Choisissez une couleur de texte plus foncée ou un fond plus clair. Quand le texte se trouve sur une image, le vérificateur ne peut pas juger : il ajoute alors un conseil pour vérifier le contraste à l'œil.
- **Des objets lus dans un autre ordre.** Un lecteur d'écran et un PDF balisé lisent les objets d'une page de l'arrière vers l'avant de la pile. Si cet ordre n'est pas de haut en bas, puis de gauche à droite, le vérificateur liste en conseils les objets qui ne sont pas à leur place. Cliquez sur **Ordre de lecture** pour voir les objets dans l'ordre où ils sont lus. Sélectionnez-en un et cliquez sur **Lire plus tôt** ou **Lire plus tard**, ou faites-le glisser. Chaque déplacement est une étape d'Annuler, et il change aussi l'objet qui est devant.
- **Un tableau sans ligne d'en-tête.** Un lecteur d'écran se sert de la première ligne d'un tableau pour nommer ses colonnes. Un tableau dont la première ligne n'est pas marquée comme ligne d'en-tête est signalé par un avertissement. Cliquez sur le problème, puis sur **Utiliser la première ligne comme en-tête** pour activer la ligne d'en-tête. C'est une étape d'Annuler.

## Agrandir la page

Effectuez un zoom avant pour lire les petits textes. Utilisez les commandes de **Zoom** de la barre d'état, maintenez la touche Ctrl enfoncée en tournant la molette de la souris, ou choisissez une taille dans la zone **Zoom** de l'onglet **Affichage**. Le curseur de zoom va d'environ 10 % à 1 600 %.

## Voir aussi

- [Utiliser JeffPub au clavier](keyboard)
- [Ajouter et ajuster des images](pictures)
- [Modifier les paramètres de JeffPub](options)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
