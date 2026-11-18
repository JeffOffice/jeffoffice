# Outils de données
<!-- keywords: prévision, feuille de prévision, valeurs futures, intervalle de confiance, saisonnalité, tendance, lissage, formulaire, saisie de données, enregistrement, enregistrements, nouvel enregistrement, rechercher suivant, rechercher précédent, critères, gestionnaire de scénarios, scénarios, analyse de scénarios, si, hypothèse, synthèse des scénarios, données, onglet données, convertir du texte en colonnes, scinder, séparateur, virgule, tabulation, largeur fixe, supprimer les doublons, valeur cible, sous-total, totaux, filtre avancé, conditions, validation des données, entourer les données non valides, ovale rouge, effacer les entourages de validation, consolider, combiner des plages, autoriser, liste, liste déroulante, message de saisie, alerte d'erreur, lignes répétées, unique, fichier texte, csv, à partir d'un texte, colonnes, délimiteur, guillemets, modifier les liens, mettre à jour les valeurs, changer la source, liens, externe, source, rompre le lien, vérifier l'état, ouvrir la source -->

L'onglet **Données** regroupe les outils qui travaillent sur une liste de valeurs : obtenir du texte à partir d'un fichier, trier une liste, répartir du texte en colonnes et supprimer les lignes répétées.

## À partir de texte/CSV

**À partir de texte/CSV** ouvre un fichier texte (.csv, .tsv ou .txt). Cette commande est identique à **Ouvrir un fichier texte** dans l'onglet **Fichier**. JeffCrunch demande comment le fichier est réparti en colonnes et quel type de texte il contient, puis l'affiche dans un nouveau classeur.

## Modifier les liens

Si un classeur contient des formules qui lisent un autre classeur (par exemple ='[Tarifs.xlsx]Feuil1'!A1), la commande **Modifier les liens**, dans le groupe **Obtenir des données**, affiche la liste de ces autres classeurs, appelés sources. Chaque ligne indique où se trouve la source et son état. Tant que le classeur n'a pas été rouvert avec ses sources, il affiche les nombres que son fichier a enregistrés à partir de celles-ci.

- **Vérifier l'état** recherche chaque source sur cet ordinateur et indique **Trouvée**, ou indique qu'elle n'a pas été trouvée. Une source située sur le web n'est pas récupérée : son état indique qu'elle n'a pas été vérifiée.
- **Mettre à jour les valeurs** lit la source choisie sur cet ordinateur et donne à chaque cellule qui n'est qu'une référence à l'une de ses cellules la valeur que cette cellule a maintenant. Une cellule qui fait davantage avec la valeur, comme `=[1]Tarifs!A1*2`, conserve le nombre enregistré dans son fichier, et le message sous la liste indique combien de cellules ont été mises à jour et combien ne l'ont pas été. La mise à jour des valeurs est une seule étape : **Annuler** rétablit les anciennes valeurs. Les nouvelles valeurs sont aussi écrites dans la partie du fichier réservée au lien lors de l'enregistrement, de sorte que le classeur les affiche à sa prochaine ouverture, même si la source est absente.
- **Changer la source** permet de choisir un autre fichier pour le lien sélectionné, par exemple lorsque le classeur a été déplacé. Les formules restent inchangées. Le nouvel emplacement est écrit dans le fichier lors de l'enregistrement, et **Annuler** annule la modification.
- **Ouvrir la source** ouvre la source choisie dans une nouvelle fenêtre, lorsqu'elle peut être trouvée.
- **Rompre le lien** transforme chaque cellule qui lit la source choisie en la valeur qu'elle affiche actuellement, et un nom qui désignait un emplacement dans cette source en **#REF!**. JeffCrunch demande confirmation avant. La rupture de lien est une seule étape : **Annuler** rétablit les formules, les noms et les liens, jusqu'à l'enregistrement du classeur. Lorsqu'il ne reste plus aucun lien, le fichier est enregistré sans les parties qui les contenaient.

## Formulaire

**Formulaire** (dans le groupe **Outils de données**, et dans la zone de recherche) affiche une ligne d'une liste à la fois, dans un petit formulaire. Une liste est un tableau, ou une plage de cellules avec une ligne de noms en haut et une ligne de données sous chaque nom. Cliquez d'abord dans une cellule quelconque de la liste.

- Chaque zone correspond à une colonne, avec le nom de la ligne du haut comme libellé. Tapez dans une zone pour modifier la valeur. Une zone grisée contient une formule : le formulaire affiche le résultat, et vous modifiez la formule dans la feuille.
- **Rechercher suivant** et **Rechercher précédent** passent à la ligne suivante et à la ligne précédente. Passer à une autre ligne, ou appuyer sur **Entrée**, enregistre ce que vous avez saisi dans cette ligne.
- **Nouveau** affiche un formulaire vide. Remplissez-le, puis appuyez de nouveau sur **Nouveau**, sur **Rechercher suivant** ou sur **Fermer**. La ligne est ajoutée sous la dernière ligne de la liste. Un tableau s'agrandit d'une ligne. Si une cellule située sous la liste n'est pas vide, le formulaire le signale et n'ajoute rien.
- **Supprimer** retire la ligne affichée, après une demande de confirmation. Les lignes situées en dessous remontent. Un tableau conserve toujours au moins une ligne de données. Une ligne masquée, ou qu'un filtre masque, n'est pas affichée dans le formulaire, n'est pas comptée et n'est jamais supprimée par celui-ci. Si la liste est trop longue pour être parcourue en entier, le formulaire indique qu'il n'affiche que les premières lignes.
- **Restaurer** annule ce que vous avez saisi dans cette ligne, avant de la quitter.
- **Critères** permet de rechercher des lignes. Saisissez une condition dans une ou plusieurs zones, puis appuyez sur **Rechercher suivant** ou **Rechercher précédent**. Une condition est une valeur, comme **Ann** (le texte commence par ce mot, et **?** et ***** remplacent des caractères), ou une valeur précédée d'un signe : **>100**, **<=5**, **<>0**, **=Oslo**. Une ligne doit respecter toutes les conditions saisies. Appuyez sur **Formulaire** pour revenir, ou sur **Effacer** pour recommencer. Tant que des critères sont définis, **Rechercher suivant** et **Rechercher précédent** ignorent les lignes qui ne les respectent pas.
- **Fermer** enregistre la ligne et ferme le formulaire.

Chaque ligne que vous modifiez, ajoutez ou supprimez constitue une étape : **Annuler** dans la feuille la rétablit d'un coup.

## Trier et filtrer

Le groupe **Trier et filtrer** contient les mêmes commandes que le menu **Trier et filtrer** de l'onglet Accueil. Voir [Trier et filtrer](sort-filter).

## Convertir du texte en colonnes

Utilisez cette commande lorsqu'une colonne contient plusieurs éléments de texte que vous voulez placer dans des colonnes distinctes, par exemple « Dupont, Anne, 31 ».

1. Sélectionnez les cellules d'**une seule** colonne.
2. Choisissez **Convertir du texte en colonnes** dans l'onglet **Données**.
3. Choisissez la manière de répartir le texte :
   - **Scinder à un séparateur** : cochez les caractères qui séparent les éléments : **Tabulation**, **Point-virgule**, **Virgule**, **Espace** ou **Autre**, où vous saisissez votre propre caractère. **Considérer les séparateurs consécutifs comme un seul** évite qu'deux virgules à la suite ne créent un élément vide. **Texte entre guillemets** indique le caractère qui encadre un élément qu'il ne faut pas scinder, comme « Lee, Bo ». À l'intérieur des guillemets, deux caractères consécutifs ne représentent qu'un seul caractère.
   - **Scinder à des positions fixes** : saisissez les positions où commence une nouvelle colonne, en comptant les caractères à partir de 1. Par exemple, 5, 12 démarre une nouvelle colonne au cinquième et au douzième caractère.
4. Choisissez le type de chaque élément : **Standard** interprète les nombres et les dates comme si vous les aviez saisis, et **Texte** conserve chaque élément tel quel, de sorte que « 007 » reste « 007 ».
5. Dans **Placer les éléments à partir de**, saisissez la première cellule destinée aux éléments. Elle correspond d'abord à la première cellule sélectionnée.
6. La zone située sous les choix affiche les premières lignes réparties selon vos choix. Appuyez sur **OK**.

Les éléments sont placés dans les cellules situées à droite de la première cellule, et remplacent ce qui s'y trouvait. Convertir du texte en colonnes est une seule étape que vous pouvez annuler.

## Supprimer les doublons

Utilisez cette commande pour ne conserver que la première ligne de chaque groupe de lignes qui se répètent.

1. Sélectionnez les cellules de la liste, ou une cellule de celle-ci. JeffCrunch prend le bloc de données entier autour d'une cellule unique.
2. Choisissez **Supprimer les doublons**.
3. Cochez **Mes données ont une ligne d'en-tête** si la première ligne contient des noms. Cette ligne est conservée et n'est pas comparée.
4. Cochez les colonnes qui doivent être identiques pour que deux lignes soient considérées comme des doublons. Deux lignes ne sont doublons que si toutes les colonnes cochées sont identiques. Les majuscules et les minuscules sont considérées comme identiques, et une formule compte pour la valeur qu'elle produit.
5. Appuyez sur **OK**. La boîte de dialogue indique combien de lignes ont été supprimées et combien en restent.

Les lignes situées sous une ligne supprimée remontent à l'intérieur des cellules sélectionnées, avec leur mise en forme et leurs formules. Les cellules situées sous la sélection ne bougent pas. Les lignes vides laissées au bas de la sélection sont effacées. Supprimer les doublons est une seule étape que vous pouvez annuler.

## Valeur cible

Utilisez cette commande lorsque vous connaissez le résultat voulu, mais pas le nombre qui le produit. Par exemple, une cellule de paiement contient une formule qui utilise un taux d'intérêt, et vous voulez que le paiement soit de 500. La valeur cible essaie des nombres dans la cellule du taux jusqu'à ce que le paiement soit égal à 500.

1. Choisissez **Valeur cible** dans l'onglet **Données**.
2. Dans **Cellule avec la formule**, saisissez la cellule qui contient la formule, par exemple B3. Elle correspond d'abord à la cellule sélectionnée.
3. Dans **Valeur souhaitée**, saisissez le résultat que la formule doit donner.
4. Dans **Cellule à modifier**, saisissez la cellule que la valeur cible peut modifier, par exemple B1. Elle doit contenir un nombre, et non une formule.
5. Appuyez sur **OK**.

Si la valeur cible trouve un nombre, celui-ci reste dans la cellule à modifier, et la barre d'état indique sa valeur. Si elle n'en trouve pas, rien ne change et un message vous le signale. Essayez un autre nombre de départ dans la cellule à modifier, et vérifiez que la formule utilise bien cette cellule. Valeur cible est une seule étape que vous pouvez annuler.

## Gestionnaire de scénarios

Un scénario est un ensemble nommé de valeurs pour certaines cellules. Utilisez les scénarios pour tester des hypothèses (« et si ») sur une même feuille. Par exemple, conservez un scénario « Ventes faibles » et un scénario « Ventes fortes » pour les mêmes cellules de saisie, et affichez celui que vous voulez examiner.

1. Choisissez **Gestionnaire de scénarios** dans l'onglet **Données**.
2. Appuyez sur **Ajouter**. Saisissez un nom, par exemple Ventes fortes. Dans **Cellules modifiables**, saisissez les cellules que le scénario définit, par exemple B1:B3 ou D5. Un scénario peut modifier jusqu'à 32 cellules. Les cellules que vous avez sélectionnées sont remplies automatiquement. Chaque cellule commence avec la valeur qu'elle contient actuellement. Saisissez dans le tableau la valeur que ce scénario doit utiliser, puis appuyez sur **OK**.
3. Choisissez un scénario dans la liste et appuyez sur **Afficher** pour placer ses valeurs dans les cellules. Un double-clic sur un nom produit le même effet. Appuyez sur **Modifier** pour changer son nom, son commentaire ou ses valeurs. Appuyez sur **Supprimer** pour le retirer.

Afficher, Ajouter, Modifier et Supprimer sont chacune une étape que vous pouvez annuler.

**Synthèse** place tous les scénarios côte à côte sur une nouvelle feuille nommée Synthèse des scénarios. Elle demande les cellules de résultat, c'est-à-dire les cellules que vous voulez comparer, par exemple le total en B10. Pour chaque scénario, la synthèse affiche les valeurs des cellules modifiables et le résultat des cellules de résultat, calculé avec les valeurs de ce scénario. Une colonne nommée Valeurs actuelles indique l'état des cellules au moment où vous avez créé la synthèse. Vos propres cellules restent inchangées.

Les scénarios sont enregistrés dans le classeur. Un classeur créé par un autre logiciel conserve ses scénarios, que vous retrouvez ici. Une copie enregistrée au format .xls ancien ou au format .ods ne les conserve pas, et l'enregistrement vous le signale.

## Sous-total

Utilisez cette commande pour placer une ligne de totaux sous chaque groupe de lignes ayant la même valeur dans une colonne, ainsi qu'un total général en bas. La liste doit comporter une ligne d'en-tête et doit être triée selon la colonne de regroupement, afin que les valeurs identiques se suivent.

1. Sélectionnez une cellule de la liste.
2. Choisissez **Sous-total** dans le groupe **Plan** de l'onglet **Données**.
3. Dans **À chaque modification de**, choisissez la colonne qui regroupe les lignes, par exemple Région.
4. Dans **Utiliser la fonction**, choisissez le calcul à effectuer : **Somme**, **Nombre**, **Moyenne**, **Max**, **Min**, et d'autres.
5. Dans **Ajouter un sous-total à**, cochez chaque colonne à totaliser.
6. Laissez **Remplacer les sous-totaux existants** coché pour remplacer les sous-totaux déjà présents. Appuyez sur **OK**.

Chaque ligne de total indique « Total Est » (ou la valeur à laquelle elle correspond) et contient une formule SOUS.TOTAL, qui ignore donc les autres lignes de total. Les lignes de chaque groupe sont regroupées : vous pouvez utiliser les boutons 1, 2 et 3 à côté des lignes pour n'afficher que les totaux. Pour supprimer les totaux, ouvrez de nouveau **Sous-total** et appuyez sur **Supprimer tout**. Ajouter ou supprimer des sous-totaux est une seule étape que vous pouvez annuler.

## Filtre avancé

Les boutons de filtre sélectionnent les lignes colonne par colonne. **Avancé** (dans le groupe **Trier et filtrer**) vous permet d'écrire les conditions dans la feuille, aussi longues que nécessaire.

1. À côté de la liste, copiez les en-têtes des colonnes à tester dans une ligne de cellules vides.
2. Sous chaque en-tête, écrivez une condition. Les conditions situées sur la **même ligne** doivent être toutes vraies (c'est le « et »). Les conditions situées sur des **lignes différentes** sont des solutions de rechange : une ligne de la liste doit satisfaire seulement l'une de ces lignes (c'est le « ou »).
3. Choisissez **Avancé**. Vérifiez que **Plage de la liste** correspond à la liste avec sa ligne d'en-tête, et saisissez les cellules de vos en-têtes et de vos conditions dans **Plage des conditions**.
4. Choisissez **Masquer les autres lignes de la liste**, ou **Copier les lignes à un autre emplacement** et saisissez la première cellule de la destination, par exemple H1. Cochez **Ignorer les lignes en double** pour ne copier qu'une seule occurrence de chaque ligne.
5. Appuyez sur **OK**.

Comment écrire une condition :

- Un mot, par exemple **An**, sélectionne les textes qui commencent par ce mot. Les majuscules et les minuscules sont considérées comme identiques. **?** remplace un caractère quelconque et ***** remplace une suite quelconque de caractères.
- Un nombre sélectionne les cellules qui lui sont égales.
- **=Ann** sélectionne le texte exactement égal à Ann. **=** seul sélectionne les cellules vides.
- **>**, **>=**, **<**, **<=** et **<>** (différent de) placés devant un nombre ou un mot permettent la comparaison, par exemple **>25**. **<>** seul sélectionne les cellules non vides.

Une ligne de conditions vide sélectionne toutes les lignes. Pour afficher de nouveau les lignes masquées, choisissez **Effacer**. Une copie est faite immédiatement et ne change pas lorsque la liste change. Filtre avancé est une seule étape que vous pouvez annuler.

## Validation des données

Utilisez cette commande pour définir ce qui peut être saisi dans certaines cellules, afin qu'une saisie erronée soit bloquée ou mise en question.

1. Sélectionnez les cellules.
2. Choisissez **Validation des données** dans le groupe **Outils de données** de l'onglet **Données**.
3. Dans l'onglet **Paramètres**, choisissez ce qu'il faut **Autoriser** : **Toute valeur** (aucune règle), **Nombre entier**, **Décimal**, **Liste**, **Date**, **Heure**, **Longueur du texte** ou **Personnalisé**.
4. Pour un nombre, une date, une heure ou une longueur de texte, choisissez **Données** (**entre**, **non compris entre**, **égal à**, etc.) et saisissez les limites. Une limite peut être une valeur ou une formule qui commence par =.
5. Pour une **Liste**, saisissez les choix séparés par des virgules, par exemple Rouge,Vert,Bleu, ou saisissez une plage de cellules après =, par exemple =$A$1:$A$5. La cellule affiche alors une flèche donnant accès aux choix. Désactivez **Liste déroulante dans la cellule** pour masquer la flèche.
6. Pour **Personnalisé**, saisissez une formule qui commence par =. La saisie est autorisée lorsque la formule est vraie.
7. **Ignorer les blancs** laisse passer une cellule vide.

Dans l'onglet **Message de saisie**, cochez la case, puis saisissez un titre et un message. Ils s'affichent à côté de la cellule chaque fois qu'elle est sélectionnée.

Dans l'onglet **Alerte d'erreur**, choisissez ce qui se produit lors d'une saisie non autorisée. **Arrêt** la refuse. **Avertissement** demande s'il faut la conserver. **Information** avertit la personne et conserve la saisie. Saisissez un titre et un message pour l'alerte.

**Entourer les données non valides** (à côté de **Validation des données**) place un ovale rouge autour de chaque cellule de la feuille dont la valeur n'est pas autorisée par sa règle, par exemple un nombre saisi avant la règle, ou le résultat d'une formule. Un message indique si toutes les cellules sont correctes. Une cellule que vous corrigez perd son ovale. **Effacer les entourages de validation** supprime tous les ovales.

**Effacer tout** supprime les règles des cellules sélectionnées. La nouvelle règle remplace toute règle que les cellules sélectionnées possédaient ; les autres cellules conservent les leurs. La modification des règles est une seule étape que vous pouvez annuler, et les règles sont enregistrées dans le fichier.

## Consolider

Utilisez cette commande pour combiner en un seul tableau des plages semblables, par exemple le même tableau présent dans plusieurs feuilles.

1. Choisissez **Consolider** dans le groupe **Outils de données** de l'onglet **Données**.
2. Choisissez la **Fonction** à appliquer, par exemple **Somme** ou **Moyenne**.
3. Saisissez une plage dans la zone et appuyez sur **Ajouter**. Utilisez **Feuil2!A1:C10** pour une plage située sur une autre feuille. Ajoutez chaque plage à combiner. Sélectionnez une plage dans la liste et appuyez sur **Supprimer** pour la retirer.
4. Si les plages comportent des noms dans leur première ligne, cochez **La ligne du haut contient les noms des colonnes**. Si elles comportent des noms dans leur première colonne, cochez **La colonne de gauche contient les noms des lignes**.
5. Dans **Placer le résultat à**, saisissez la première cellule du résultat.
6. Cochez **Créer des liens vers les données source** si chaque résultat doit être une formule qui lit ses cellules source, et donc se met à jour lorsqu'elles changent. Sans cette case, les résultats sont de simples nombres.
7. Appuyez sur **OK**.

Lorsqu'aucun nom n'est coché, la cellule située au même emplacement dans chaque plage est combinée (la première cellule de chaque plage, puis la deuxième, et ainsi de suite). Lorsque les noms sont cochés, les cellules portant le même nom sont combinées, quel que soit leur emplacement dans chaque plage, et un nom que seules certaines plages possèdent reçoit tout de même une ligne ou une colonne. Consolider est une seule étape que vous pouvez annuler.

## Feuille de prévision

La feuille de prévision crée une nouvelle feuille contenant un tableau et un graphique de valeurs futures, calculés à partir d'une chronologie et d'une série de valeurs. Elle détermine la tendance et la saison récurrente des valeurs, et ajoute une bande dans laquelle les valeurs futures sont susceptibles de rester.

1. Sélectionnez la chronologie et les valeurs, deux colonnes côte à côte (la chronologie d'abord). La chronologie est constituée de dates ou de nombres régulièrement espacés, par exemple chaque mois ou tous les 5 éléments. Une première ligne de noms est ignorée.
2. Choisissez **Feuille de prévision** dans le groupe **Prévision** de l'onglet **Données**.
3. Une image montre la prévision, et se modifie lorsque vous changez les options. Choisissez **Graphique en courbes** ou **Graphique en colonnes**.
4. Réglez les options :
   - **Plage de la chronologie** et **Plage des valeurs** sont les cellules à lire. Saisissez ici d'autres cellules si la sélection était incorrecte.
   - **Début de la prévision** est vide pour commencer après le dernier point. Une date ou un nombre antérieur au dernier point commence la prévision à cet endroit et n'utilise que les points qui le précèdent, ce qui vous permet de comparer la prévision à ce qui s'est réellement produit.
   - **Fin de la prévision** est le dernier moment à prévoir. 10 000 points au maximum sont prévus.
   - **Intervalle de confiance** indique le degré de certitude de la bande, 95 % par défaut.
   - **Saisonnalité** est déterminée à partir des valeurs ; vous pouvez aussi indiquer la longueur de la saison en points, ou choisir aucune. Les valeurs doivent couvrir deux saisons complètes.
   - **Combler les points manquants par** indique la valeur d'un moment sans valeur : une droite entre ses voisins, ou zéro.
   - **Regrouper les doublons à l'aide de** indique comment sont combinées les valeurs d'un même moment : Moyenne, Nombre, NbVal, Max, Médiane, Min ou Somme.
   - **Inclure les statistiques de prévision** ajoute un tableau d'Alpha, Bêta, Gamma, MASE, SMAPE, MAE et RMSE, qui indiquent dans quelle mesure la prévision correspond aux valeurs.
5. Si quelque chose est incorrect, la boîte de dialogue l'explique en termes simples (étapes inégales, trop peu de points, texte dans les valeurs), et **Créer** reste inactif.
6. Appuyez sur **Créer**.

La nouvelle feuille est placée avant celle sur laquelle vous vous trouviez. Son tableau comporte les colonnes Chronologie, Valeurs, Prévision, Borne inférieure de l'intervalle de confiance et Borne supérieure de l'intervalle de confiance. La prévision et les bornes sont des formules qui utilisent `PREVISION.ETS` et `PREVISION.ETS.CONFINT` sur les colonnes Chronologie et Valeurs du tableau lui-même, de sorte qu'elles sont recalculées lorsque vous modifiez une valeur. Le graphique affiche les valeurs, la prévision et la bande. La feuille de prévision est une seule étape que vous pouvez annuler.

Les nombres peuvent différer de quelques pourcents de ceux d'autres tableurs, qui utilisent leur propre méthode d'ajustement du lissage. Une prévision qu'un fichier contient déjà conserve les valeurs enregistrées dans ce fichier, tant que les données qu'elle lit ne sont pas modifiées.

Un fichier comportant plus de 1 000 000 de points n'est pas pris en compte.
