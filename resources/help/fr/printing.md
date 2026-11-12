# Imprimer une composition
<!-- keywords: imprimer, impression, imprimante, copies, plage de pages, livret, affiche, bannière, mosaïque, repères de coupe, fond perdu, séparations, recto verso, aperçu avant impression, niveaux de gris, assembler, assemblée, sélection, dernière page en premier, inverser, zoom -->

L'impression se fait sur la page **Imprimer**. Vous choisissez vos paramètres à gauche et vérifiez un aperçu de chaque page à droite. **Zoom**, au-dessus de l'aperçu, agrandit la page au-delà de la taille qui tient à l'écran ; faites défiler pour voir le reste.

## Imprimer une composition

1. Appuyez sur Ctrl+P, ou cliquez sur **Fichier > Imprimer**. La **barre d'outils rapide** possède aussi un bouton **Imprimer**.
2. À côté du bouton **Imprimer**, réglez **Copies**.
3. Choisissez une **Imprimante**. La liste se termine par **Enregistrer au format PDF**, qui crée un fichier PDF au lieu d'un tirage papier. Avec plusieurs copies, **Assembler** imprime la série complète pour chaque copie (1, 2, 3, puis 1, 2, 3), et lorsqu'il est désactivé, les copies de chaque page sortent ensemble (1, 1, 2, 2, 3, 3). Un fichier PDF n'est créé qu'une seule fois, quel que soit le nombre de copies.
4. Sous **Plage**, choisissez **Toutes les pages**, **Page active**, **Sélection** (seuls les objets sélectionnés, à leur emplacement sur la page active ; un objet appartenant à un groupe s'imprime avec tout son groupe, les objets de la page maître sont exclus, et l'aperçu n'affiche que ce qui sera imprimé ; cette option est grisée lorsque rien n'est sélectionné), ou **Pages**. Pour **Pages**, saisissez les pages dans la zone **Pages** : numéros de page et plages, séparés par des virgules, par exemple 1-3, 5.
5. Choisissez les autres paramètres dont vous avez besoin. Ils sont décrits ci-dessous.
6. Cliquez sur **Imprimer**.

Cliquez sur **Propriétés de l'imprimante…** pour ouvrir la fenêtre d'impression du système, avec les options propres à votre imprimante. Les réglages que vous y faites sont conservés pour le travail, et les copies, l'assemblage et le papier reviennent sur cette page. Pour le recto verso, **Dernière page en premier** conserve chaque recto avec son verso.

L'aperçu à droite comporte des flèches pour passer d'une page à l'autre. Cochez **Afficher les règles** ou **Afficher les numéros de page** pour les voir dans l'aperçu.

## Choisir une disposition

La liste **Disposition** définit la façon dont les pages sont placées sur le papier :

- **Une page par feuille** imprime chaque page séparément. Une page trop grande pour le papier est imprimée en plusieurs morceaux. Une page légèrement trop grande est réduite pour tenir.
- **Plusieurs pages par feuille** place plusieurs pages différentes sur une même feuille.
- **Plusieurs copies par feuille** répète une même page sur toute la feuille. Utilisez cette option pour les cartes de visite et les étiquettes.
- **Livret, pli latéral** et **Livret, pli supérieur** placent deux pages sur chaque feuille, dans le bon ordre, pour qu'une pile pliée se lise dans l'ordre. Le pli latéral place les deux pages côte à côte. Le pli supérieur les place l'une au-dessus de l'autre.
- **Mosaïque (affiches et bannières)** imprime une grande page en plusieurs morceaux sur plusieurs feuilles, avec un léger chevauchement pour faciliter l'alignement des feuilles.

## Autres paramètres

- **Papier** : Lettre US, Légal US, Tabloïd, A4, A3 ou **Identique à la composition**.
- **Dernière page en premier** imprime les feuilles dans l'ordre inverse, pour que la pile soit dans l'ordre à la sortie. Cette option est grisée lorsque vous imprimez recto verso manuellement, car JeffPub gère alors l'ordre lui-même.
- **Faces** : **Imprimer en recto**, ou **Imprimer recto verso** avec retournement sur le grand côté ou sur le petit côté. **Imprimer recto verso manuellement** convient à une imprimante incapable d'imprimer recto verso. JeffPub imprime les pages 1, 3, 5 et ainsi de suite (pour un livret, les faces de feuille), puis un message vous demande de retourner la pile et de la replacer dans l'imprimante. Lorsque vous cliquez sur **OK**, il imprime les autres pages, la dernière en premier. Une composition d'une seule page n'a rien à retourner. L'enregistrement d'un PDF grise ce choix, car un fichier n'a pas de faces. L'aperçu indique alors la feuille et la face sur lesquelles se trouve chaque page.
- **Couleur** : **RVB composite** pour la couleur intégrale, ou **Niveaux de gris composites** pour des nuances de gris.
- **Imprimer tous les enregistrements du publipostage** : disponible lorsque votre composition contient une liste de destinataires. Voir [publipostage](mail-merge).

## Repères d'impression et séparations

Ces options concernent les travaux confiés à un imprimeur professionnel. Demandez à l'imprimeur ce dont il a besoin.

- **Repères de coupe** indiquent où couper. **Repères de fond perdu** indiquent le bord supplémentaire. **Repères de montage** facilitent l'alignement des couleurs. **Barres de densité** et **Barres de couleurs** permettent à l'imprimeur de contrôler l'encre. **Informations sur le travail** imprime le nom, la page et la date.
- **Autoriser les fonds perdus** imprime les objets qui débordent de la page, afin qu'il n'y ait pas de bord blanc après le massicotage. Ce bord supplémentaire s'appelle un **fond perdu**.
- Les repères s'impriment en dehors de la page ; utilisez donc un papier plus grand que votre page.
- **Séparations (plaques CMJN)**, sous **Couleur**, imprime une feuille par encre : **Cyan**, **Magenta**, **Jaune** et **Noir**, ainsi que les couleurs d'accompagnement utilisées dans votre composition (une couleur d'accompagnement est une encre spéciale prémélangée). Cochez ou décochez les plaques souhaitées.

Pour un PDF à remettre à un imprimeur, voir [Fichiers PDF et imprimeurs](pdf).

## Voir aussi

- [Fichiers PDF et imprimeurs](pdf)
- [Taille de page, marges et disposition](page-setup)
- [Publipostage et fusion de catalogue](mail-merge)
- [Vérifier votre conception](design-checker)
