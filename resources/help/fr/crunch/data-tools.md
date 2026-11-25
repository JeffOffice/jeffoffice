# Outils de données
<!-- keywords: feuille de prévision, prévision, valeurs futures, intervalle de confiance, saisonnalité, tendance, lissage, formulaire, formulaire de données, saisie de données, enregistrement, enregistrements, nouvel enregistrement, rechercher suivant, rechercher précédent, critères, gestionnaire de scénarios, scénarios, simulation, hypothèse, synthèse des scénarios, données, onglet données, convertir du texte en colonnes, scinder, séparateur, virgule, tabulation, largeur fixe, supprimer les doublons, valeur cible, sous-total, totaux, filtre avancé, conditions, validation des données, entourer les données non valides, ovale rouge, effacer les entourages de validation, consolider, combiner des plages, autoriser, liste, liste déroulante, message de saisie, alerte d'erreur, lignes répétées, unique, fichier texte, csv, à partir de texte, colonnes, délimiteur, guillemets, modifier les liens, actualiser les valeurs, modifier la source, liens, externe, source, rompre le lien, vérifier l'état, ouvrir la source -->

L'onglet **Données** regroupe les outils qui travaillent sur une liste de valeurs : obtenir du texte depuis un fichier, mettre une liste en ordre, découper un texte en colonnes et supprimer les lignes répétées.

## À partir de texte/CSV

**À partir de texte/CSV** ouvre un fichier texte (.csv, .tsv ou .txt). C'est la même chose que **Ouvrir un fichier texte** dans l'onglet Fichier. JeffCrunch demande comment le fichier est découpé en colonnes et quel type de texte il contient, puis l'affiche dans un nouveau classeur.

## À partir d'un fichier

**À partir d'un fichier**, dans le groupe **Obtenir des données**, charge les lignes d'un fichier JSON, d'un fichier XML ou d'un autre classeur sous forme de tableau. Seul un fichier situé sur cet ordinateur peut être chargé. Une adresse web ou un fichier situé sur un autre ordinateur est refusé.

1. Cliquez sur **À partir d'un fichier**, puis sur **Parcourir** et choisissez le fichier. Les premières lignes s'affichent aussitôt, avec le type que JeffCrunch a reconnu pour chaque colonne d'après ces premières lignes : nombre, date, vrai ou faux, ou texte. Tout le fichier est vérifié au chargement, si bien qu'une colonne peut finir en texte. Rien ne change dans la feuille tant que vous n'avez pas cliqué sur **Charger**.
2. Si le fichier contient plusieurs listes, choisissez celle à charger. Dans un fichier JSON, il s'agit d'une liste d'éléments comportant des valeurs nommées, par exemple `$.items`. Dans un fichier XML, il s'agit d'un élément qui se répète, par exemple `/feed/entry`. Dans un classeur, il s'agit d'une feuille, d'un tableau ou d'une plage nommée.
3. Un élément JSON à l'intérieur d'un élément devient une colonne nommée `externe.interne`, jusqu'à quatre niveaux. Une liste de valeurs simples est réunie dans une seule cellule (les 10 000 premières valeurs), ou répartie sur des colonnes (les 16 premières), selon votre choix. En XML, les attributs et les éléments enfants deviennent des colonnes. Deux valeurs différentes ne partagent jamais une colonne : si deux noms seraient identiques, le second reçoit un numéro ajouté ou, en XML, son préfixe, et une note le signale. Un nom répété à l'intérieur d'un même élément JSON conserve sa dernière valeur, et une note le signale. Un nombre trop grand ou trop petit pour une cellule est conservé sous forme de texte, tel qu'il est écrit, et un caractère illisible est affiché sous forme de marque de remplacement ; ces deux cas sont signalés.
4. Cochez **Charger chaque colonne comme texte** pour conserver chaque valeur telle qu'elle est écrite. Une colonne de nombres commençant par un zéro, comme 007, est toujours conservée sous forme de texte.
5. Saisissez la cellule où doit aller le tableau, puis cliquez sur **Charger**. Si le tableau recouvrirait des cellules qui contiennent des données, JeffCrunch demande avant de les remplacer. S'il n'y a pas assez de place, ou si la feuille est protégée, un message le signale et rien ne change.

Le tableau constitue une étape d'**Annuler**. Il mémorise le fichier et vos choix. Pour relire le fichier, cliquez dans une cellule du tableau, puis cliquez sur **Actualiser** dans l'onglet Données. **Actualiser tout** est le grand bouton à côté : il relit tous les tableaux créés par Obtenir des données, puis actualise tous les tableaux croisés dynamiques, le tout en une seule étape d'Annuler. Le tableau s'agrandit ou se réduit selon les lignes du fichier. Les colonnes que vous avez ajoutées à côté du tableau sont conservées : une formule recopiée vers le bas est recopiée dans les nouvelles lignes, et les lignes qui ont disparu du fichier disparaissent aussi. Si le fichier n'existe plus ou ne peut pas être lu, un message le signale et rien ne change. Si les colonnes du fichier ont changé, un message indique lesquelles manquent ou sont nouvelles, et rien ne change. Recréez le tableau avec Obtenir des données, ou rétablissez les colonnes dans le fichier.

Le lien vers le fichier est conservé dans le tableau, dans un classeur `.xlsx` enregistré, dans une partie que les autres programmes ignorent. Le lien contient le chemin complet du fichier sur cet ordinateur : toute personne qui reçoit le classeur peut donc lire ce chemin. Pour le supprimer avant de partager le classeur, utilisez **Inspecter**, puis **Liens vers les fichiers de données**, puis **Supprimer**. Une copie enregistrée dans un autre type de fichier ne conserve pas ce lien, et **Vérifier la compatibilité** le signale. **Inspecter** répertorie les liens sous **Liens vers les fichiers de données** et peut les supprimer. Les tableaux et leurs cellules restent en place.

Les fichiers sont lus avec des limites : un fichier de 64 Mo au plus, 200 000 lignes, 256 colonnes et 32 767 caractères par cellule. Un XML comportant une déclaration de type de document ou des entités est refusé. Tout ce qui est coupé par une limite est signalé.

## Modifier les liens

Si un classeur contient des formules qui lisent un autre classeur (par exemple ='[Prix.xlsx]Feuil1'!A1), **Modifier les liens**, dans le groupe **Obtenir des données**, répertorie ces autres classeurs, appelés sources. Chaque ligne indique où se trouve la source et son état. Tant que le classeur n'a pas été rouvert avec ses sources, il affiche les nombres que son fichier avait enregistrés à partir d'elles.

- **Vérifier l'état** recherche chaque source sur cet ordinateur et indique **Trouvée**, ou qu'elle n'a pas été trouvée. Une source située sur le web n'est pas téléchargée : son état indique qu'elle n'a pas été vérifiée.
- **Actualiser les valeurs** lit la source choisie sur cet ordinateur et donne à chaque cellule qui n'est qu'une référence à l'une de ses cellules la valeur que cette cellule possède maintenant. Une cellule qui fait davantage avec la valeur, comme `=[1]Prix!A1*2`, conserve le nombre enregistré dans son fichier, et le message sous la liste indique combien de cellules ont été mises à jour et combien ne l'ont pas été. **Actualiser les valeurs** constitue une seule étape : **Annuler** rétablit les anciennes valeurs. Les nouvelles valeurs sont aussi écrites dans la partie du fichier propre au lien lors de l'enregistrement, si bien que le classeur les affiche à sa prochaine ouverture, même si la source est absente.
- **Modifier la source** permet de choisir un autre fichier pour le lien sélectionné, par exemple lorsque le classeur a été déplacé. Les formules restent telles quelles. Le nouvel emplacement est écrit dans le fichier lors de l'enregistrement, et **Annuler** annule la modification.
- **Ouvrir la source** ouvre la source choisie dans une nouvelle fenêtre, lorsqu'elle peut être trouvée.
- **Rompre le lien** remplace chaque cellule qui lit la source choisie par la valeur qu'elle affiche maintenant, et un nom qui désignait un emplacement dans cette source devient **#REF!**. JeffCrunch demande confirmation avant. **Rompre le lien** constitue une seule étape : **Annuler** rétablit les formules, les noms et les liens, jusqu'à l'enregistrement du classeur. Lorsqu'il ne reste plus aucun lien, le fichier est enregistré sans les parties qui les contenaient.

## Formulaire

**Formulaire** (dans le groupe **Outils de données**, et dans la zone de recherche) affiche une seule ligne d'une liste à la fois, sous forme de petit formulaire. Une liste est un tableau, ou une plage de cellules avec une ligne de noms en haut et une ligne de données sous chaque nom. Cliquez d'abord dans une cellule quelconque de la liste.

- Chaque zone correspond à une colonne, avec pour étiquette le nom de la ligne du haut. Saisissez dans une zone pour modifier la valeur. Une zone grisée contient une formule : le formulaire affiche le résultat, et vous modifiez la formule dans la feuille.
- **Rechercher suivant** et **Rechercher précédent** passent à la ligne suivante et à la ligne précédente. Passer à une autre ligne, ou appuyer sur **Entrée**, enregistre ce que vous avez saisi dans cette ligne.
- **Nouveau** affiche un formulaire vide. Remplissez-le, puis appuyez de nouveau sur **Nouveau**, sur **Rechercher suivant** ou sur **Fermer**. La ligne est ajoutée sous la dernière ligne de la liste. Un tableau s'agrandit d'une ligne. Si une cellule située sous la liste n'est pas vide, le formulaire le signale et n'ajoute rien.
- **Supprimer** retire la ligne affichée, après une demande de confirmation. Les lignes situées en dessous remontent. Un tableau conserve toujours au moins une ligne de données. Une ligne masquée, ou qu'un filtre masque, n'est pas affichée dans le formulaire, n'est pas comptée et n'est jamais supprimée par lui. Si la liste est trop longue pour être parcourue entièrement, le formulaire indique qu'il n'affiche que les premières lignes.
- **Rétablir** annule ce que vous avez saisi dans cette ligne, avant de la quitter.
- **Critères** permet de rechercher des lignes. Saisissez une condition dans une ou plusieurs zones, puis appuyez sur **Rechercher suivant** ou **Rechercher précédent**. Une condition est une valeur, comme **Ann** (le texte commence par ce mot, et **?** et ***** remplacent des lettres), ou une valeur précédée d'un signe : **>100**, **<=5**, **<>0**, **=Oslo**. Une ligne doit remplir toutes les conditions saisies. Appuyez sur **Formulaire** pour revenir, ou sur **Effacer** pour recommencer. Tant que des critères sont définis, **Rechercher suivant** et **Rechercher précédent** ignorent les lignes qui ne les remplissent pas.
- **Fermer** enregistre la ligne et ferme le formulaire.

Chaque ligne que vous modifiez, ajoutez ou supprimez constitue une étape : **Annuler** dans la feuille la rétablit d'un seul coup.

## Trier et filtrer

Le groupe **Trier et filtrer** contient les mêmes commandes que le menu **Trier et filtrer** de l'onglet Accueil. Voir [Trier et filtrer](sort-filter).

## Convertir du texte en colonnes

Utilisez cette commande lorsqu'une colonne contient plusieurs morceaux de texte que vous voulez placer dans des colonnes séparées, comme « Dupont, Anne, 31 ».

1. Sélectionnez les cellules d'**une seule** colonne.
2. Choisissez **Convertir du texte en colonnes** dans l'onglet **Données**.
3. Choisissez comment découper le texte :
   - **Scinder à un séparateur** : cochez les signes qui séparent les morceaux : **Tabulation**, **Point-virgule**, **Virgule**, **Espace** ou **Autre**, où vous saisissez votre propre signe. **Traiter les séparateurs consécutifs comme un seul** évite que deux virgules à la suite ne créent un élément vide. **Texte entre guillemets** indique le signe qui encadre un élément à ne pas scinder, comme « Lee, Bo ». À l'intérieur des guillemets, deux signes à la suite représentent un seul signe.
   - **Scinder à des positions fixes** : saisissez les positions où commence une nouvelle colonne, en comptant les caractères à partir de 1. Par exemple, 5, 12 commence une nouvelle colonne au cinquième et au douzième caractère.
4. Choisissez le type de chaque élément : **Standard** interprète les nombres et les dates comme si vous les aviez saisis, et **Texte** conserve chaque élément tel quel, si bien que « 007 » reste « 007 ».
   Pour traiter un élément différemment, saisissez son numéro dans **Élément** et choisissez sa lecture : **Identique à ci-dessus**, **Tel que saisi**, **Texte**, une date écrite **mois, jour, année**, **jour, mois, année** ou **année, mois, jour**, ou **Ignorer**, qui ne place cet élément nulle part. Le premier élément est le 1. Un élément de date peut comporter le mois sous forme de nom (**5 mars 2024**, **5 mars 2024**) en français ou dans votre langue, peut être séparé par /, -, un point, une virgule ou des espaces, et peut être suivi d'une heure (**10:30**, **10:30:45** ou **10:30 PM**). Un élément qui n'est pas une date dans l'ordre choisi (un mois 13, un jour qui n'existe pas, une année qui ne comporte pas deux ou quatre chiffres, ou une heure impossible) est conservé comme texte, tel qu'il a été saisi ; il n'est jamais lu dans un autre ordre.
5. Dans **Placer les éléments à partir de**, saisissez la première cellule des éléments. Elle correspond d'abord à la première cellule sélectionnée.
6. La zone située sous les choix affiche les premières lignes découpées selon votre choix. Cliquez sur **OK**.

Les éléments sont placés dans les cellules à droite de la première cellule, et remplacent leur contenu actuel. Convertir du texte en colonnes constitue une seule étape que vous pouvez annuler.

## Supprimer les doublons

Utilisez cette commande pour ne conserver que le premier exemplaire de chaque groupe de lignes répétées.

1. Sélectionnez les cellules de la liste, ou une seule cellule de celle-ci. JeffCrunch prend tout le bloc de données autour d'une cellule unique.
2. Choisissez **Supprimer les doublons**.
3. Cochez **Mes données comportent une ligne d'en-tête** si la première ligne contient des noms. Cette ligne est conservée et n'est pas comparée.
4. Cochez les colonnes qui doivent être identiques pour que deux lignes soient considérées comme répétées. Deux lignes ne sont répétées que si toutes les colonnes cochées sont identiques. Les majuscules et les minuscules sont considérées comme identiques, et une formule compte comme la valeur qu'elle donne.
5. Cliquez sur **OK**. La boîte de dialogue indique combien de lignes ont été supprimées et combien en restent.

Les lignes situées sous une ligne supprimée remontent à l'intérieur des cellules sélectionnées, avec leur mise en forme et leurs formules. Les cellules situées sous la sélection ne bougent pas. Les lignes vides laissées en bas de la sélection sont effacées. Supprimer les doublons constitue une seule étape que vous pouvez annuler.

## Valeur cible

Utilisez cette commande lorsque vous connaissez le résultat voulu, mais pas le nombre qui le produit. Par exemple, une cellule de mensualité contient une formule qui utilise un taux d'intérêt, et vous voulez que la mensualité soit de 500. Valeur cible essaie des nombres dans la cellule du taux jusqu'à ce que la mensualité vaille 500.

1. Choisissez **Valeur cible** dans l'onglet **Données**.
2. Dans **Cellule contenant la formule**, saisissez la cellule qui contient la formule, par exemple B3. Elle correspond d'abord à la cellule sélectionnée.
3. Dans **Valeur à atteindre**, saisissez le résultat que doit donner la formule.
4. Dans **Cellule à modifier**, saisissez la cellule que Valeur cible peut modifier, par exemple B1. Elle doit contenir un nombre, et non une formule.
5. Cliquez sur **OK**.

Si Valeur cible trouve un nombre, celui-ci reste dans la cellule à modifier, et la barre d'état indique sa valeur. Sinon, rien ne change et un message le signale. Essayez un autre nombre de départ dans la cellule à modifier, et vérifiez que la formule utilise bien cette cellule. Valeur cible constitue une seule étape que vous pouvez annuler.

## Gestionnaire de scénarios

Un scénario est un ensemble de valeurs nommé, pour certaines cellules. Les scénarios permettent de tester des hypothèses (« et si… ») sur une seule feuille. Par exemple, gardez un scénario « Ventes faibles » et un scénario « Ventes élevées » pour les mêmes cellules de saisie, et affichez celui que vous voulez examiner.

1. Choisissez **Gestionnaire de scénarios** dans l'onglet **Données**.
2. Cliquez sur **Ajouter**. Saisissez un nom, comme Ventes élevées. Dans **Cellules modifiables**, saisissez les cellules que le scénario définit, par exemple B1:B3 ou D5. Un scénario peut modifier jusqu'à 32 cellules. Les cellules sélectionnées sont saisies automatiquement. Chaque cellule commence avec la valeur qu'elle contient actuellement. Saisissez dans le tableau la valeur que doit utiliser ce scénario, puis cliquez sur **OK**.
3. Choisissez un scénario dans la liste et cliquez sur **Afficher** pour placer ses valeurs dans les cellules. Un double-clic sur un nom produit le même effet. Cliquez sur **Modifier** pour changer son nom, son commentaire ou ses valeurs. Cliquez sur **Supprimer** pour le retirer.

Afficher, Ajouter, Modifier et Supprimer constituent chacun une étape que vous pouvez annuler.

**Synthèse** place tous les scénarios côte à côte sur une nouvelle feuille nommée Synthèse des scénarios. La commande demande les cellules de résultat, c'est-à-dire les cellules que vous voulez comparer, comme le total en B10. Pour chaque scénario, la synthèse affiche les valeurs des cellules modifiables et le résultat des cellules de résultat, calculé avec les valeurs de ce scénario. Une colonne nommée Valeurs actuelles indique l'état des cellules au moment de la création de la synthèse. Vos cellules restent telles qu'elles étaient.

Les scénarios sont enregistrés dans le classeur. Un classeur créé par un autre programme conserve ses scénarios, que vous voyez ici. Une copie enregistrée au format .xls ancien ou au format .ods ne les conserve pas, et l'enregistrement le signale.

## Sous-total

Utilisez cette commande pour placer une ligne de totaux sous chaque groupe de lignes ayant la même valeur dans une colonne, ainsi qu'un total général en bas. La liste doit comporter une ligne d'en-tête, et il est préférable de la trier selon la colonne de regroupement, afin que les valeurs identiques se suivent.

1. Sélectionnez une cellule de la liste.
2. Choisissez **Sous-total** dans le groupe **Plan** de l'onglet **Données**.
3. Dans **À chaque modification de**, choisissez la colonne qui regroupe les lignes, par exemple Région.
4. Dans **Utiliser la fonction**, choisissez le calcul à effectuer : **Somme**, **Nb**, **Moyenne**, **Max**, **Min**, et d'autres.
5. Dans **Ajouter un sous-total à**, cochez chaque colonne à totaliser.
6. Laissez **Remplacer les sous-totaux existants** coché pour remplacer les sous-totaux déjà présents.
7. **Récapitulatif sous les données** reprend le réglage du plan de la feuille. Laissez-le coché pour placer chaque ligne de total sous son groupe, ou décochez-le pour placer chaque ligne de total au-dessus de son groupe, avec le total général en haut. Cliquez sur **OK**.

Chaque ligne de total affiche « Total Est » (ou la valeur à laquelle elle correspond) et contient une formule SOUS.TOTAL, qui ignore donc les autres lignes de total. Les lignes de chaque groupe sont regroupées, si bien que vous pouvez utiliser les boutons 1, 2 et 3 à côté des lignes pour n'afficher que les totaux. Pour supprimer les totaux, ouvrez de nouveau **Sous-total** et cliquez sur **Supprimer tout**. Si les totaux ont été placés de l'autre côté d'une feuille qui comportait déjà ses propres groupes, la barre d'état indique que ces groupes ont maintenant leurs lignes récapitulatives de ce côté ; **Supprimer tout** rétablit alors le côté tel qu'il était avant le premier sous-total, sauf si vous l'avez modifié vous-même depuis. Lorsque les totaux sont au-dessus, les boutons du plan se trouvent sur les lignes de total : un bouton plus ou moins à côté de « Total Est » affiche ou masque les lignes Est situées en dessous. Si vous décochez **Remplacer les sous-totaux existants**, les nouveaux totaux doivent se trouver du même côté que ceux qui existent déjà. Sinon, rien ne change et un message le signale. Ajouter ou supprimer des sous-totaux constitue une seule étape que vous pouvez annuler.

## Filtre avancé

Les boutons de filtre sélectionnent les lignes colonne par colonne. **Avancé** (dans le groupe **Trier et filtrer**) vous permet d'écrire les conditions dans la feuille, aussi longues que nécessaire.

1. À côté de la liste, copiez les en-têtes des colonnes à tester dans une ligne de cellules vides.
2. Sous chaque en-tête, écrivez une condition. Les conditions situées sur la **même ligne** doivent toutes être vraies (c'est un « et »). Les conditions situées sur des **lignes différentes** sont des solutions de remplacement : une ligne de la liste doit remplir seulement une de ces lignes (c'est un « ou »).
3. Choisissez **Avancé**. Vérifiez que **Plage de la liste** correspond à la liste avec sa ligne d'en-tête, et saisissez les cellules de vos en-têtes et conditions dans **Plage des conditions**.
4. Choisissez **Masquer les autres lignes de la liste**, ou **Copier les lignes à un autre emplacement** et saisissez la première cellule de l'emplacement, par exemple H1. Cochez **Ignorer les lignes en double** pour ne copier qu'un exemplaire de chaque ligne.
5. Cliquez sur **OK**.

Comment écrire une condition :

- Un mot, comme **An**, sélectionne le texte qui commence par ce mot. Les majuscules et les minuscules sont considérées comme identiques. **?** remplace un caractère quelconque et ***** une suite quelconque de caractères.
- Un nombre sélectionne les cellules qui lui sont égales.
- **=Ann** sélectionne le texte qui est exactement Ann. **=** seul sélectionne les cellules vides.
- **>**, **>=**, **<**, **<=** et **<>** (différent de) placés devant un nombre ou un mot effectuent la comparaison, comme **>25**. **<>** seul sélectionne les cellules qui ne sont pas vides.

Une ligne de conditions vide sélectionne toutes les lignes. Pour afficher de nouveau les lignes masquées, choisissez **Effacer**. Une copie est faite immédiatement et ne change pas lorsque la liste change. Filtre avancé constitue une seule étape que vous pouvez annuler.

## Validation des données

Utilisez cette commande pour définir ce qui peut être saisi dans certaines cellules, afin qu'une saisie erronée soit bloquée ou contestée.

1. Sélectionnez les cellules.
2. Choisissez **Validation des données** dans le groupe **Outils de données** de l'onglet **Données**.
3. Dans l'onglet **Paramètres**, choisissez ce qu'il faut **Autoriser** : **Toute valeur** (aucune règle), **Nombre entier**, **Décimal**, **Liste**, **Date**, **Heure**, **Longueur du texte**, ou **Personnalisé**.
4. Pour un nombre, une date, une heure ou une longueur de texte, choisissez **Données** (**comprise entre**, **non comprise entre**, **égal à**, etc.) et saisissez les limites. Une limite peut être une valeur ou une formule commençant par =.
5. Pour une **Liste**, saisissez les choix séparés par des virgules, comme Rouge,Vert,Bleu, ou saisissez une plage de cellules après =, comme =$A$1:$A$5. La cellule affiche alors une flèche avec les choix. Désactivez **Liste déroulante dans la cellule** pour masquer la flèche.
6. Pour **Personnalisé**, saisissez une formule commençant par =. La saisie est autorisée lorsque la formule est vraie.
7. **Ignorer les blancs** laisse passer une cellule vide.

Dans l'onglet **Message de saisie**, cochez la case et saisissez un titre et un message. Ils s'affichent à côté de la cellule chaque fois qu'elle est sélectionnée.

Dans l'onglet **Alerte d'erreur**, choisissez ce qui arrive à une saisie non autorisée. **Arrêt** la refuse. **Avertissement** demande si elle doit être conservée. **Information** prévient l'utilisateur et la conserve. Saisissez un titre et un message pour l'alerte.

**Entourer les données non valides** (à côté de **Validation des données**) place un ovale rouge autour de chaque cellule de la feuille dont la valeur n'est pas autorisée par sa règle, par exemple un nombre saisi avant la règle, ou le résultat d'une formule. Un message indique si toutes les cellules sont correctes. Une cellule que vous corrigez perd son ovale. **Effacer les entourages de validation** supprime tous les ovales.

**Effacer tout** supprime les règles des cellules sélectionnées. La nouvelle règle remplace toute règle que les cellules sélectionnées possédaient ; les autres cellules conservent les leurs. La modification des règles constitue une seule étape que vous pouvez annuler, et les règles sont enregistrées dans le fichier.

## Consolider

Utilisez cette commande pour combiner des plages semblables, comme un même tableau présent sur plusieurs feuilles, en un seul tableau.

1. Choisissez **Consolider** dans le groupe **Outils de données** de l'onglet **Données**.
2. Choisissez la **Fonction** à appliquer pour combiner, comme **Somme** ou **Moyenne**.
3. Saisissez une plage dans la zone et cliquez sur **Ajouter**. Utilisez **Feuil2!A1:C10** pour une plage située sur une autre feuille. Ajoutez chaque plage à combiner. Sélectionnez une plage dans la liste et cliquez sur **Supprimer** pour la retirer.
4. Si les plages ont des noms dans leur première ligne, cochez **La ligne du haut contient les noms des colonnes**. S'ils ont des noms dans leur première colonne, cochez **La colonne de gauche contient les noms des lignes**.
5. Dans **Placer le résultat à**, saisissez la première cellule du résultat.
6. Cochez **Créer des liens vers les données source** si chaque résultat doit être une formule qui lit ses cellules source, afin qu'il change lorsque celles-ci changent. Sans cette case, les résultats sont des nombres simples.
7. Cliquez sur **OK**.

Lorsqu'aucun nom n'est coché, la cellule située au même endroit dans chaque plage est combinée (la première cellule de chaque plage, puis la deuxième, et ainsi de suite). Lorsque les noms sont cochés, les cellules portant les mêmes noms sont combinées, où qu'elles se trouvent dans chaque plage, et un nom présent seulement dans certaines plages donne tout de même une ligne ou une colonne. Consolider constitue une seule étape que vous pouvez annuler.

## Feuille de prévision

La feuille de prévision crée une nouvelle feuille contenant un tableau et un graphique de valeurs futures, calculés à partir d'une chronologie et d'une série de valeurs. Elle détermine la tendance et la saison qui se répète dans les valeurs, et ajoute une bande dans laquelle les valeurs futures devraient probablement rester.

1. Sélectionnez la chronologie et les valeurs, deux colonnes côte à côte (la chronologie en premier). La chronologie est constituée de dates ou de nombres espacés régulièrement, par exemple chaque mois ou tous les 5 unités. Une première ligne de noms est ignorée.
2. Choisissez **Feuille de prévision** dans le groupe **Prévision** de l'onglet **Données**.
3. Une image montre la prévision et change lorsque vous modifiez les choix. Choisissez **Graphique en courbes** ou **Graphique en colonnes**.
4. Réglez les choix :
   - **Plage de la chronologie** et **Plage des valeurs** sont les cellules à lire. Saisissez ici d'autres cellules si la sélection était incorrecte.
   - **Début de la prévision** est vide pour commencer après le dernier point. Une date ou un nombre situé avant le dernier point commence à cet endroit et n'utilise que les points qui le précèdent, ce qui permet de comparer la prévision avec ce qui s'est réellement produit.
   - **Fin de la prévision** est le dernier moment à prévoir. 10 000 points au maximum sont prévus.
   - **Intervalle de confiance** indique le degré de certitude de la bande, 95 % par défaut.
   - **Saisonnalité** est déterminée à partir des valeurs, ou vous pouvez fixer la longueur de la saison en points, ou choisir aucune. Les valeurs doivent couvrir deux saisons complètes.
   - **Compléter les points manquants par** indique la valeur attribuée à un instant sans valeur : une droite entre ses voisins, ou zéro.
   - **Regrouper les doublons à l'aide de** indique comment les valeurs situées au même instant sont combinées : Moyenne, Nb, NBVAL, Max, Médiane, Min ou Somme.
   - **Inclure les statistiques de prévision** ajoute un tableau avec Alpha, Bêta, Gamma, MASE, SMAPE, MAE et RMSE, qui indiquent l'adéquation de la prévision aux valeurs.
5. Si quelque chose ne va pas, la boîte de dialogue l'explique en termes simples (étapes inégales, trop peu de points, texte dans les valeurs), et **Créer** reste inactif.
6. Cliquez sur **Créer**.

La nouvelle feuille est placée avant celle où vous vous trouviez. Son tableau comporte les colonnes Chronologie, Valeurs, Prévision, Borne inférieure de l'intervalle de confiance et Borne supérieure de l'intervalle de confiance. La prévision et les bornes sont des formules qui utilisent `PREVISION.ETS` et `PREVISION.ETS.CONFINT` sur les colonnes Chronologie et Valeurs du tableau lui-même, si bien qu'elles sont recalculées lorsque vous modifiez une valeur. Le graphique affiche les valeurs, la prévision et la bande. Feuille de prévision constitue une seule étape que vous pouvez annuler.

Les nombres peuvent différer de quelques pour cent de ceux d'autres tableurs, qui utilisent leur propre méthode d'ajustement du lissage. Une prévision qu'un fichier contient déjà conserve les valeurs enregistrées dans ce fichier, tant que les données qu'elle lit ne sont pas modifiées.

Un fichier comportant plus de 1 000 000 de points n'est pas utilisé.
