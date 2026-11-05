# Imprimer

<!-- keywords: pages impaires, pages paires, inverse, dernière page en premier, recto verso, sélection, section, p2s3, zoom, plusieurs pages, recto verso manuel, duplex, retourner la pile, imprimer, imprimante, impression, copies, assemblées, assemblage, plage de pages, pages, page active, aperçu, papier, orientation, paysage, portrait, pdf, enregistrer en pdf, ctrl+p, feuille, section, retourner la pile, recto verso -->

Appuyez sur **Ctrl+P**, ou cliquez sur **Fichier > Imprimer**, pour ouvrir la page Imprimer. Les options se trouvent à gauche. À droite, un aperçu de chaque page telle qu'elle sera imprimée. Utilisez les flèches sous l'aperçu pour afficher les autres pages. **Afficher** au-dessus permet d'en afficher une, deux ou quatre à la fois. **Zoom** agrandit les pages au-delà de la taille qui tient dans la fenêtre. Faites défiler pour voir le reste.

## Choisir le mode d'impression

- **Imprimante** liste les imprimantes connues de votre ordinateur. **Enregistrer en PDF** crée un fichier PDF à la place d'un tirage papier. JeffWrite demande un nom de fichier lorsque vous cliquez sur **Imprimer**.
- **Copies** indique le nombre d'exemplaires à imprimer.
- **Assemblées** a de l'intérêt lorsque vous imprimez plusieurs exemplaires. Activée, les pages sortent dans l'ordre 1, 2, 3, puis de nouveau 1, 2, 3. Désactivée, elles sortent dans l'ordre 1, 1, 2, 2, 3, 3.
- **Toutes les pages**, **Page active**, **Sélection** ou **Pages** détermine ce qui est imprimé. **Sélection** n'imprime que ce que vous avez sélectionné dans le document, avec le même papier, les mêmes styles, les mêmes en-têtes et pieds de page. Cette option est grisée lorsque rien n'est sélectionné.
- Pour **Pages**, vous pouvez aussi désigner une page d'une section. Tapez **p2s3** pour la deuxième page de la section 3, ou **s3** pour toute la section 3. Une section est une partie du document mise en forme à part dans l'onglet **Disposition**. Vous pouvez mélanger les écritures : 1-2, s3, p1s4-p2s4.
- Pour **Pages**, tapez les numéros de page et les plages, séparés par des virgules, par exemple 1-3, 5. Une plage peut aller à l'envers (5-3). Une plage ouverte va jusqu'à la dernière page (8-) ou part de la première (-2).
- La liste sous les numéros de page peut limiter l'impression aux **Pages impaires uniquement** ou aux **Pages paires uniquement**. Elle compte les pages dans l'ordre, à partir de la première page du document : la première page est la page 1 (impaire), la deuxième la page 2 (paire), et ainsi de suite. Elle n'utilise pas les numéros imprimés sur les pages : un document dont la numérotation repart, ou commence à 5, voit tout de même sa première page comptée comme impaire. La limite s'applique aux pages retenues par la plage.
- **Dernière page en premier** imprime les pages dans l'ordre inverse, si bien que la pile sort dans l'ordre. Combinée aux choix pages impaires et paires, cette option permet d'imprimer les deux faces d'une feuille à la main : imprimez les pages impaires, retournez la pile, puis imprimez les pages paires avec **Dernière page en premier** activée ou désactivée, selon le comportement de votre imprimante.
- **Recto verso manuel** est destiné à une imprimante incapable d'imprimer des deux côtés. JeffWrite imprime d'abord les pages impaires. Un message vous demande alors de retourner la pile et de la replacer dans l'imprimante. Lorsque vous cliquez sur **OK**, il imprime les pages paires, de la dernière à la première, afin qu'elles tombent au verso des pages impaires. Cette option n'est pas utilisée lors d'une impression vers un fichier PDF, car un fichier n'a pas de faces.
- **Papier** indique sur quel papier est imprimée la page de l'aperçu.

Les pages sont imprimées dans l'ordre où vous les tapez, une seule fois chacune. Un numéro qui ne correspond à aucune page du document est ignoré. Si aucun des numéros saisis ne correspond à une page, la zone affiche un contour rouge et le bouton **Imprimer** reste inactif.

Cliquez sur **Imprimer** pour envoyer les pages. La page Imprimer se ferme lorsque l'imprimante les a reçues.

## Les numéros de plage désignent des emplacements, pas les numéros imprimés

Les numéros que vous tapez pour **Pages** sont des emplacements dans le document : 1 est la première page, 2 la deuxième, et ainsi de suite, quel que soit le numéro affiché sur chaque page. Si une section recommence sa numérotation, par exemple à la page 1 d'un nouveau chapitre, ou commence à un autre numéro, le numéro affiché par une page peut différer de son emplacement. Tapez l'emplacement. La ligne sous l'aperçu, telle que **Page 5 sur 12**, indique l'emplacement de la page que vous regardez, afin que vous puissiez retrouver le numéro à saisir.

## Le papier provient du document

Chaque section d'un document a son propre format de papier et son propre sens, définis dans l'onglet **Disposition**. JeffWrite imprime chaque page sur le papier de sa section et tourne une page en paysage. Si les pages choisies utilisent plusieurs formats de papier, la ligne **Papier** le signale. Chargez le papier dont chaque page a besoin.

## Papier et PDF identiques

JeffWrite dessine une page de la même façon pour l'imprimante et pour un fichier PDF : mêmes traits, mêmes sauts, mêmes polices. Les soulignements rouges de l'orthographe et les marques de paragraphe ne sont jamais imprimés.

Pour créer un PDF sans passer par la page Imprimer, voir [Ouvrir, enregistrer, imprimer et exporter](open-save).

## Voir aussi

- [Vérifier l'orthographe](spelling)
- [Ouvrir, enregistrer, imprimer et exporter](open-save)
