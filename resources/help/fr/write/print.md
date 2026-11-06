# Imprimer
<!-- keywords: pages par feuille, plusieurs pages sur une feuille, mise à l'échelle au format papier, réduire, ajuster au papier, pages impaires, pages paires, inverser, dernière page en premier, recto verso, sélection, section, p2s3, zoom, plusieurs pages, recto verso manuel, duplex, retourner la pile, imprimer, imprimante, impression, copies, assembler, assemblées, plage de pages, pages, page active, aperçu, papier, orientation, paysage, portrait, pdf, enregistrer en pdf, ctrl+p, feuille, section, imprimer, format, impression -->

Appuyez sur **Ctrl+P**, ou cliquez sur **Fichier > Imprimer**, pour ouvrir la page Imprimer. Les options se trouvent à gauche. À droite, une image de chaque page telle qu'elle sera imprimée. Utilisez les flèches sous l'image pour afficher les autres pages. **Afficher** au-dessus permet d'en afficher une, deux ou quatre à la fois. **Zoom** agrandit les pages au-delà de la taille qui tient à l'écran. Faites défiler pour voir le reste.

## Choisir la façon d'imprimer

- **Imprimante** liste les imprimantes que votre ordinateur connaît. **Enregistrer en PDF** crée un fichier PDF à la place d'un tirage sur papier. JeffWrite demande un nom lorsque vous cliquez sur **Imprimer**.
- **Copies** indique le nombre d'exemplaires à imprimer.
- **Assemblées** n'a d'effet que si vous imprimez plusieurs copies. Activée, l'impression donne les pages dans l'ordre 1, 2, 3, puis de nouveau 1, 2, 3. Désactivée, elle donne 1, 1, 2, 2, 3, 3.
- **Toutes les pages**, **Page active**, **Sélection** ou **Pages** détermine ce qui est imprimé. **Sélection** n'imprime que ce que vous avez sélectionné dans le document, avec le même papier, les mêmes styles, en-têtes et pieds de page. Elle est grisée lorsque rien n'est sélectionné.
- Pour **Pages**, vous pouvez aussi désigner une page d'une section. Tapez **p2s3** pour la deuxième page de la section 3, ou **s3** pour toute la section 3. Une section est une partie du document mise en forme à part dans l'onglet **Disposition**. Vous pouvez les combiner : 1-2, s3, p1s4-p2s4.
- Pour **Pages**, tapez les numéros de page et les plages, séparés par des virgules, par exemple 1-3, 5. Une plage peut aller dans l'autre sens (5-3). Une plage sans fin va jusqu'à la dernière page (8-) ou commence à la première (-2).
- La liste sous les numéros de page peut limiter l'impression aux **Pages impaires uniquement** ou aux **Pages paires uniquement**. Elle compte les pages dans l'ordre, à partir de la première page du document : la première page est la page 1 (impaire), la deuxième est la page 2 (paire), et ainsi de suite. Elle n'utilise pas les numéros imprimés sur les pages : un document dont la numérotation repart, ou commence à 5, compte tout de même sa première page comme impaire. La limite s'applique aux pages retenues par la plage.
- **Dernière page en premier** imprime les pages dans l'ordre inverse, pour que la pile sorte dans le bon ordre. Combinée aux choix pages impaires et paires, cette option permet d'imprimer les deux faces d'une feuille à la main : imprimez les pages impaires, retournez la pile, puis imprimez les pages paires avec **Dernière page en premier** activée ou désactivée, selon le comportement de votre imprimante.
- La liste **Pages par feuille** place 2, 4, 6, 9 ou 16 pages sur chaque feuille. JeffWrite réduit chaque page pour qu'elle tienne dans son emplacement et trace un fin trait autour d'elle. Les pages sont disposées en lignes, à partir du coin supérieur gauche. Une feuille de 2 ou 6 pages est tournée à l'horizontale, pour que les pages ne soient pas compressées. La petite image de la page ne le montre pas : elle affiche chaque page entière.
- La liste **Mise à l'échelle au format** fait tenir chaque page dans un format de papier : Letter, Legal, A4, A5 ou A3. La page garde ses proportions et se place au centre de la feuille. Utilisez cette option lorsque le papier chargé dans l'imprimante n'est pas le format prévu pour le document.
- **Recto verso manuel** est destiné à une imprimante incapable d'imprimer des deux côtés. JeffWrite imprime d'abord les pages impaires. Un message vous demande alors de retourner la pile et de la replacer dans l'imprimante. Lorsque vous cliquez sur **OK**, il imprime les pages paires, de la dernière à la première, pour qu'elles tombent au dos des pages impaires. Cette option n'est pas utilisée lors d'un enregistrement en PDF, car un fichier n'a pas de faces.
- **Papier** indique le papier sur lequel la page affichée dans l'aperçu sera imprimée.

Les pages s'impriment dans l'ordre où vous les saisissez, une seule fois chacune. Un numéro qui ne correspond à aucune page du document est ignoré. Si aucun des numéros saisis ne correspond à une page, le champ affiche un contour rouge et le bouton **Imprimer** reste inactif.

Cliquez sur **Imprimer** pour envoyer les pages. La page Imprimer se ferme lorsque l'imprimante les a reçues.

## Les numéros de plage désignent des emplacements, non les numéros imprimés

Les numéros saisis pour **Pages** sont des emplacements dans le document : 1 est la première page, 2 la deuxième, et ainsi de suite, quel que soit le numéro affiché sur chaque page. Si une section recommence sa numérotation, par exemple à la page 1 d'un nouveau chapitre, ou commence à un autre numéro, le numéro affiché peut différer de l'emplacement. Saisissez l'emplacement. La ligne sous l'aperçu, par exemple **Page 5 sur 12**, indique l'emplacement de la page que vous regardez, ce qui vous permet de trouver le numéro à saisir.

## Le papier provient du document

Chaque section d'un document a son propre format de papier et son sens d'orientation, définis dans l'onglet **Disposition**. JeffWrite imprime chaque page sur le papier de sa section, et tourne une page en paysage à l'horizontale. Si les pages choisies utilisent plusieurs types de papier, la ligne **Papier** l'indique. Chargez le papier dont chaque page a besoin.

## Papier et PDF identiques

JeffWrite dessine une page de la même façon pour l'imprimante et pour un fichier PDF : mêmes traits, mêmes sauts de ligne et mêmes polices. Les traits rouges de vérification orthographique et les marques de paragraphe ne sont jamais imprimés.

Pour créer un PDF sans passer par la page Imprimer, voir [Ouvrir, enregistrer, imprimer et exporter](open-save).

## Voir aussi

- [Vérifier l'orthographe](spelling)
- [Ouvrir, enregistrer, imprimer et exporter](open-save)
