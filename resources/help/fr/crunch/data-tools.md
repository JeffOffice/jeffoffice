# Outils de données

<!-- keywords: formulaire, formulaire de données, saisie de données, enregistrement, enregistrements, nouvel enregistrement, rechercher suivant, rechercher précédent, critères, gestionnaire de scénarios, scénarios, analyse de scénarios, si, hypothèse, synthèse des scénarios, données, onglet données, convertir du texte en colonnes, scinder, séparateur, virgule, tabulation, largeur fixe, supprimer les doublons, valeur cible, sous-total, totaux, filtre avancé, conditions, validation des données, entourer les données non valides, ovale rouge, effacer les entourages de validation, consolider, combiner des plages, autoriser, liste, liste déroulante, message de saisie, alerte d'erreur, lignes répétées, unique, fichier texte, csv, à partir d'un texte, colonnes, délimiteur, guillemets, modifier les liens, actualiser les valeurs, changer la source, liens, externe, source, rompre le lien, vérifier l'état, ouvrir la source -->

L'onglet **Données** regroupe les outils qui travaillent sur une liste de valeurs : obtenir du texte depuis un fichier, mettre une liste en ordre, répartir du texte en colonnes et supprimer les lignes répétées.

## À partir de texte/CSV

**À partir de texte/CSV** ouvre un fichier texte (.csv, .tsv ou .txt). C'est la même chose que **Ouvrir un fichier texte** dans l'onglet **Fichier**. JeffCrunch demande comment le fichier est réparti en colonnes et quel type de texte il contient, puis l'affiche dans un nouveau classeur.

## Modifier les liens

Si un classeur contient des formules qui lisent un autre classeur (comme ='[Prix.xlsx]Feuil1'!A1), **Modifier les liens**, dans le groupe **Obtenir des données**, affiche la liste de ces autres classeurs, appelés sources. Chaque ligne indique où se trouve la source et son état. Tant que le classeur n'a pas été rouvert avec ses sources, il affiche les nombres que son fichier a enregistrés à partir d'elles.

- **Vérifier l'état** recherche chaque source sur cet ordinateur et indique **Trouvée**, ou qu'elle n'a pas été trouvée. Une source située sur le web n'est pas téléchargée ; son état indique qu'elle n'a pas été vérifiée.
- **Actualiser les valeurs** lit la source choisie sur cet ordinateur et donne à chaque cellule qui n'est qu'une référence à l'une de ses cellules la valeur que cette cellule a maintenant. Une cellule qui fait davantage avec la valeur, comme `=[1]Prix!A1*2`, conserve le nombre enregistré dans son fichier, et le message sous la liste indique combien de cellules ont été mises à jour et combien ne l'ont pas été. Actualiser les valeurs est une seule étape : **Annuler** rétablit les anciennes valeurs. Les nouvelles valeurs sont aussi écrites dans la partie du fichier réservée au lien lors de l'enregistrement, de sorte que le classeur les affiche à sa prochaine ouverture, même si la source est absente.
- **Changer la source** permet de choisir un autre fichier pour le lien sélectionné, par exemple lorsque le classeur a été déplacé. Les formules restent telles quelles. Le nouvel emplacement est écrit dans le fichier lors de l'enregistrement, et **Annuler** annule la modification.
- **Ouvrir la source** ouvre la source choisie dans une nouvelle fenêtre, lorsqu'elle peut être trouvée.
- **Rompre le lien** remplace chaque cellule qui lit la source choisie par la valeur qu'elle affiche maintenant, et tout nom qui désignait un emplacement de cette source par **#REF!**. JeffCrunch demande d'abord confirmation. Rompre le lien est une seule étape : **Annuler** rétablit les formules, les noms et les liens, jusqu'à l'enregistrement du classeur. Lorsqu'il ne reste plus aucun lien, le fichier est enregistré sans les parties qui les contenaient.

## Formulaire

**Formulaire** (dans le groupe **Outils de données** et dans la zone de recherche) affiche une ligne d'une liste à la fois, sous forme de petit formulaire. Une liste est un tableau, ou une plage de cellules avec une ligne de noms en haut et une ligne de données sous chaque nom. Cliquez d'abord dans une cellule quelconque de la liste.

- Chaque zone correspond à une colonne, étiquetée avec le nom de la ligne du haut. Saisissez dans une zone pour modifier la valeur. Une zone grisée contient une formule : le formulaire affiche le résultat, et vous modifiez la formule dans la feuille.
- **Rechercher suivant** et **Rechercher précédent** passent à la ligne suivante et à la ligne précédente. Passer à une autre ligne, ou appuyer sur **Entrée**, enregistre ce que vous avez saisi dans cette ligne.
- **Nouveau** affiche un formulaire vide. Remplissez-le, puis appuyez de nouveau sur **Nouveau**, sur **Rechercher suivant** ou sur **Fermer**. La ligne est ajoutée sous la dernière ligne de la liste. Un tableau s'agrandit d'une ligne. Si une cellule située sous la liste n'est pas vide, le formulaire le signale et n'ajoute rien.
- **Supprimer** retire la ligne affichée, après confirmation. Les lignes situées dessous remontent. Un tableau conserve toujours au moins une ligne de données. Une ligne masquée, ou qu'un filtre masque, n'est pas affichée dans le formulaire, n'est pas comptée et n'est jamais supprimée par lui. Si la liste est trop longue pour être parcourue entièrement, le formulaire indique qu'il n'affiche que les premières lignes.
- **Restaurer** annule ce que vous avez saisi dans cette ligne, avant de la quitter.
- **Critères** permet de rechercher des lignes. Saisissez une condition dans une ou plusieurs zones, puis appuyez sur **Rechercher suivant** ou **Rechercher précédent**. Une condition est une valeur, comme **Ann** (le texte correspond au début du mot, et **?** et ***** représentent des caractères), ou une valeur précédée d'un signe : **>100**, **<=5**, **<>0**, **=Oslo**. Une ligne doit satisfaire à toutes les conditions saisies. Appuyez sur **Formulaire** pour revenir, ou sur **Effacer** pour recommencer. Tant que des critères sont définis, **Rechercher suivant** et **Rechercher précédent** ignorent les lignes qui ne les satisfont pas.
- **Fermer** enregistre la ligne et ferme le formulaire.

Chaque ligne que vous modifiez, ajoutez ou supprimez constitue une étape : **Annuler** dans la feuille la rétablit d'un seul coup.

## Trier et filtrer

Le groupe **Trier et filtrer** contient les mêmes commandes que le menu **Trier et filtrer** de l'onglet **Accueil**. Voir [Trier et filtrer](sort-filter).

## Convertir du texte en colonnes

Utilisez cet outil lorsqu'une colonne contient plusieurs éléments de texte que vous voulez placer dans des colonnes distinctes, comme « Dupont, Anne, 31 ».

1. Sélectionnez les cellules d'**une seule** colonne.
2. Choisissez **Convertir du texte en colonnes** dans l'onglet **Données**.
3. Choisissez comment scinder le texte :
   - **Scinder à un séparateur** : cochez les signes qui séparent les éléments : **Tabulation**, **Point-virgule**, **Virgule**, **Espace** ou **Autre**, où vous saisissez votre propre signe. **Considérer les séparateurs consécutifs comme un seul** évite qu'deux virgules à la suite créent un élément vide. **Délimiteur de texte** indique le signe qui entoure un élément à ne pas scinder, comme « Lee, Bo ». À l'intérieur des guillemets, deux signes à la suite représentent un seul.
   - **Scinder à des positions fixes** : saisissez les positions où commence une nouvelle colonne, en comptant les caractères à partir de 1. Par exemple, 5, 12 démarre une nouvelle colonne au cinquième et au douzième caractère.
4. Choisissez le type de chaque élément : **Standard** interprète les nombres et les dates comme si vous les aviez saisis, et **Texte** conserve chaque élément tel quel, de sorte que « 007 » reste « 007 ».
5. Dans **Placer les éléments à partir de**, saisissez la première cellule de destination des éléments. Elle correspond d'abord à la première cellule sélectionnée.
6. La zone sous les choix affiche les premières lignes scindées selon vos choix. Cliquez sur **OK**.

Les éléments sont placés dans les cellules situées à droite de la première cellule, et remplacent ce qui s'y trouvait. Convertir du texte en colonnes est une seule étape, que vous pouvez annuler.

## Supprimer les doublons

Utilisez cet outil pour ne conserver que le premier de chaque groupe de lignes répétées.

1. Sélectionnez les cellules de la liste, ou une cellule de celle-ci. JeffCrunch prend tout le bloc de données autour d'une cellule unique.
2. Choisissez **Supprimer les doublons**.
3. Cochez **Mes données ont une ligne d'en-tête** si la première ligne contient des noms. Cette ligne reste en place et n'est pas comparée.
4. Cochez les colonnes qui doivent être identiques pour que deux lignes soient considérées comme des doublons. Deux lignes ne sont des doublons que si toutes les colonnes cochées sont identiques. Les majuscules et les minuscules sont considérées comme identiques, et une formule compte pour la valeur qu'elle donne.
5. Cliquez sur **OK**. La boîte de dialogue indique combien de lignes ont été supprimées et combien restent.

Les lignes situées sous une ligne supprimée remontent à l'intérieur des cellules sélectionnées, avec leur mise en forme et leurs formules. Les cellules situées sous la sélection ne bougent pas. Les lignes vides laissées au bas de la sélection sont effacées. Supprimer les doublons est une seule étape, que vous pouvez annuler.

## Valeur cible

Utilisez cet outil lorsque vous connaissez le résultat voulu, mais pas le nombre qui le donne. Par exemple, une cellule de paiement contient une formule qui utilise un taux d'intérêt, et vous voulez que le paiement soit de 500. La valeur cible essaie des nombres dans la cellule du taux jusqu'à ce que le paiement soit de 500.

1. Choisissez **Valeur cible** dans l'onglet **Données**.
2. Dans **Cellule contenant la formule**, saisissez la cellule qui contient la formule, par exemple B3. Elle correspond d'abord à la cellule sélectionnée.
3. Dans **Valeur souhaitée**, saisissez le résultat que vous voulez que la formule donne.
4. Dans **Cellule à modifier**, saisissez la cellule que la valeur cible peut modifier, par exemple B1. Elle doit contenir un nombre, et non une formule.
5. Cliquez sur **OK**.

Si la valeur cible trouve un nombre, celui-ci reste dans la cellule à modifier, et la barre d'état indique sa valeur. Si elle ne peut pas en trouver, rien ne change et un message vous le signale. Essayez une autre valeur de départ dans la cellule à modifier, et vérifiez que la formule utilise bien cette cellule. Valeur cible est une seule étape, que vous pouvez annuler.

## Gestionnaire de scénarios

Un scénario est un ensemble nommé de valeurs pour certaines cellules. Utilisez les scénarios pour faire des simulations « et si » sur une même feuille. Par exemple, conservez un scénario « Ventes faibles » et un scénario « Ventes fortes » pour les mêmes cellules de saisie, et affichez celui que vous voulez examiner.

1. Choisissez **Gestionnaire de scénarios** dans l'onglet **Données**.
2. Cliquez sur **Ajouter**. Saisissez un nom, par exemple Ventes fortes. Dans **Cellules modifiables**, saisissez les cellules que le scénario définit, par exemple B1:B3 ou D5. Un scénario peut modifier jusqu'à 32 cellules. Les cellules que vous avez sélectionnées sont renseignées automatiquement. Chaque cellule commence avec la valeur qu'elle contient actuellement. Saisissez dans le tableau la valeur que ce scénario doit utiliser, puis cliquez sur **OK**.
3. Choisissez un scénario dans la liste et cliquez sur **Afficher** pour placer ses valeurs dans les cellules. Un double-clic sur un nom produit le même effet. Cliquez sur **Modifier** pour changer son nom, son commentaire ou ses valeurs. Cliquez sur **Supprimer** pour le retirer.

Afficher, Ajouter, Modifier et Supprimer sont chacun une étape que vous pouvez annuler.

**Synthèse** place tous les scénarios côte à côte sur une nouvelle feuille nommée Synthèse des scénarios. Elle demande les cellules de résultat, c'est-à-dire les cellules que vous voulez comparer, comme le total en B10. Pour chaque scénario, la synthèse affiche les valeurs des cellules modifiables et le résultat des cellules de résultat, calculé avec les valeurs de ce scénario. Une colonne nommée Valeurs actuelles indique l'état des cellules au moment où vous avez créé la synthèse. Vos propres cellules restent inchangées.

Les scénarios sont enregistrés dans le classeur. Un classeur créé par un autre programme conserve ses scénarios, que vous retrouvez ici. Une copie enregistrée au format .xls ancien ou au format .ods ne les conserve pas, et l'enregistrement vous en avertit.

## Sous-total

Utilisez cet outil pour placer une ligne de totaux sous chaque groupe de lignes ayant la même valeur dans une colonne, ainsi qu'un total général en bas. La liste doit avoir une ligne d'en-tête et doit être triée selon la colonne de regroupement, afin que les valeurs identiques se suivent.

1. Sélectionnez une cellule de la liste.
2. Choisissez **Sous-total** dans le groupe **Plan** de l'onglet **Données**.
3. Dans **À chaque modification de**, choisissez la colonne qui regroupe les lignes, par exemple Région.
4. Dans **Utiliser la fonction**, choisissez le calcul à effectuer : **Somme**, **Nb**, **Moyenne**, **Max**, **Min**, et d'autres.
5. Dans **Ajouter un sous-total à**, cochez chaque colonne à totaliser.
6. Laissez **Remplacer les sous-totaux existants** coché pour remplacer les sous-totaux déjà présents. Cliquez sur **OK**.

Chaque ligne de total affiche « Total Est » (ou la valeur à laquelle elle correspond) et contient une formule SOUS.TOTAL, qui ignore donc les autres lignes de total. Les lignes de chaque groupe sont regroupées : vous pouvez utiliser les boutons 1, 2 et 3 à côté des lignes pour n'afficher que les totaux. Pour les supprimer, rouvrez **Sous-total** et cliquez sur **Supprimer tout**. Ajouter ou supprimer des sous-totaux est une étape que vous pouvez annuler.

## Filtre avancé

Les boutons de filtre sélectionnent les lignes une colonne à la fois. **Avancé** (dans le groupe **Trier et filtrer**) vous permet d'écrire les conditions dans la feuille, aussi longues que nécessaire.

1. À côté de la liste, copiez les en-têtes des colonnes à tester dans une ligne de cellules vides.
2. Sous chaque en-tête, écrivez une condition. Les conditions placées sur la **même ligne** doivent toutes être vraies (c'est le « et »). Les conditions placées sur des **lignes différentes** sont des solutions de rechange : une ligne de la liste doit satisfaire à une seule de ces lignes (c'est le « ou »).
3. Choisissez **Avancé**. Vérifiez que **Plage de la liste** correspond à la liste avec sa ligne d'en-tête, et saisissez les cellules de vos en-têtes et conditions dans **Plage des conditions**.
4. Choisissez **Masquer les autres lignes de la liste**, ou **Copier les lignes à un autre emplacement** et saisissez la première cellule de destination, par exemple H1. Cochez **Ignorer les lignes en double** pour ne copier qu'une occurrence de chaque ligne.
5. Cliquez sur **OK**.

Comment écrire une condition :

- Un mot, comme **An**, sélectionne les textes qui commencent par celui-ci. Les majuscules et les minuscules sont considérées comme identiques. **?** représente un caractère quelconque et ***** une suite quelconque de caractères.
- Un nombre sélectionne les cellules qui lui sont égales.
- **=Ann** sélectionne le texte qui est exactement Ann. **=** seul sélectionne les cellules vides.
- **>**, **>=**, **<**, **<=** et **<>** (différent de) placés devant un nombre ou un mot effectuent la comparaison, comme **>25**. **<>** seul sélectionne les cellules qui ne sont pas vides.

Une ligne de conditions vide sélectionne toutes les lignes. Pour afficher de nouveau les lignes masquées, choisissez **Effacer**. Une copie est créée immédiatement et ne change pas lorsque la liste change. Filtre avancé est une étape que vous pouvez annuler.

## Validation des données

Utilisez cet outil pour définir ce qui peut être saisi dans certaines cellules, afin qu'une saisie incorrecte soit bloquée ou contestée.

1. Sélectionnez les cellules.
2. Choisissez **Validation des données** dans le groupe **Outils de données** de l'onglet **Données**.
3. Dans l'onglet **Paramètres**, choisissez le type de valeur à **Autoriser** : **Toute valeur** (aucune règle), **Nombre entier**, **Décimal**, **Liste**, **Date**, **Heure**, **Longueur du texte** ou **Personnalisé**.
4. Pour un nombre, une date, une heure ou une longueur de texte, choisissez **Données** (**compris entre**, **non compris entre**, **égal à**, etc.) et saisissez les limites. Une limite peut être une valeur ou une formule commençant par =.
5. Pour une **Liste**, saisissez les choix séparés par des virgules, comme Rouge,Vert,Bleu, ou saisissez une plage de cellules après =, comme =$A$1:$A$5. La cellule affiche alors une flèche donnant accès aux choix. Désactivez **Liste déroulante dans la cellule** pour masquer la flèche.
6. Pour **Personnalisé**, saisissez une formule commençant par =. La saisie est acceptée lorsque la formule est vraie.
7. **Ignorer les blancs** laisse passer une cellule vide.

Dans l'onglet **Message de saisie**, cochez la case et saisissez un titre et un message. Ils s'affichent à côté de la cellule chaque fois qu'elle est sélectionnée.

Dans l'onglet **Alerte d'erreur**, choisissez ce qui se passe pour une saisie non autorisée. **Arrêter** la refuse. **Avertissement** demande s'il faut la conserver. **Information** signale la saisie à la personne et la conserve. Saisissez un titre et un message pour l'alerte.

**Entourer les données non valides** (à côté de **Validation des données**) place un ovale rouge autour de chaque cellule de la feuille dont la valeur n'est pas autorisée par sa règle, par exemple un nombre présent avant l'application de la règle, ou le résultat d'une formule. Un message indique si toutes les cellules sont correctes. Une cellule que vous corrigez perd son ovale. **Effacer les entourages de validation** retire tous les ovales.

**Effacer tout** supprime les règles des cellules sélectionnées. La nouvelle règle remplace toute règle que les cellules sélectionnées possédaient ; les autres cellules conservent les leurs. La modification des règles est une étape que vous pouvez annuler, et les règles sont enregistrées dans le fichier.

## Consolider

Utilisez cet outil pour combiner en un seul tableau des plages semblables, comme le même tableau présent sur plusieurs feuilles.

1. Choisissez **Consolider** dans le groupe **Outils de données** de l'onglet **Données**.
2. Choisissez la **Fonction** à appliquer, comme **Somme** ou **Moyenne**.
3. Saisissez une plage dans la zone et cliquez sur **Ajouter**. Utilisez **Feuil2!A1:C10** pour une plage située sur une autre feuille. Ajoutez chaque plage à combiner. Sélectionnez une plage dans la liste et cliquez sur **Supprimer** pour la retirer.
4. Si les plages ont des noms dans leur première ligne, cochez **La ligne du haut contient les noms des colonnes**. Si elles ont des noms dans leur première colonne, cochez **La colonne de gauche contient les noms des lignes**.
5. Dans **Placer le résultat à**, saisissez la première cellule du résultat.
6. Cochez **Créer des liens vers les données source** si chaque résultat doit être une formule qui lit ses cellules source, et donc changer avec elles. Sans cette case, les résultats sont des nombres simples.
7. Cliquez sur **OK**.

Sans noms cochés, la cellule située au même emplacement dans chaque plage est combinée (la première cellule de chaque plage, puis la deuxième, et ainsi de suite). Avec les noms cochés, les cellules portant le même nom sont combinées, quel que soit leur emplacement dans chaque plage, et un nom qui n'existe que dans certaines plages reçoit tout de même une ligne ou une colonne. Consolider est une étape que vous pouvez annuler.
