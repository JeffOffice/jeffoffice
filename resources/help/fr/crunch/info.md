# Info : propriétés, mots de passe et Inspecter

<!-- keywords: info, propriétés, titre, auteur, objet, mots clés, catégories, commentaires, société, responsable, propriétés personnalisées, marquer comme final, final, lecture seule, mot de passe, chiffrer, protéger, inspecter le classeur, informations personnelles, feuilles masquées, notes, liens externes, xml personnalisé, afficher dans le dossier, copier le chemin, dossier, chemin, historique des versions, versions, revenir -->

Cliquez sur **Fichier > Info** pour voir les informations sur votre classeur et le protéger avant de le partager. Appuyez sur **Échap** pour revenir à votre classeur.

## Emplacement du fichier

En haut, la page indique où le fichier est enregistré. Cliquez sur **Afficher dans le dossier** pour ouvrir ce dossier. Cliquez sur **Copier le chemin** pour copier le chemin, afin de le coller dans un courriel ou une conversation.

## Propriétés

Les propriétés sont des informations sur le classeur, comme son titre et son auteur. La page contient une zone pour chacune : **Titre**, **Auteur**, **Objet**, **Mots clés**, **Catégories**, **Commentaires**, **Société** et **Responsable**. Saisissez du texte dans une zone, puis appuyez sur **Tab** ou **Entrée**. JeffCrunch conserve la modification et l'inscrit dans le fichier la prochaine fois que vous enregistrez le classeur. La page indique aussi la date de création, d'enregistrement et d'impression du classeur.

Cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés, par exemple **Projet** ou **Client**. Cliquez sur **Ajouter**, saisissez un nom, choisissez le type de valeur (texte, nombre entier, nombre, oui ou non, ou date), puis saisissez la valeur. JeffCrunch les enregistre telles que vous les avez saisies.

## Marquer comme final

Cliquez sur **Marquer comme final** pour indiquer aux lecteurs que le classeur est terminé. JeffCrunch enregistre le classeur, puis le place en lecture seule. Lorsqu'on l'ouvre, la barre de titre indique qu'il est en lecture seule. Il s'agit d'un moyen discret de dire « merci de ne pas modifier ceci ». Ce n'est pas un verrou, car chacun peut choisir de nouveau **Marqué comme final** pour le modifier.

## Toujours ouvrir en lecture seule

Cliquez sur **Toujours ouvrir en lecture seule** pour que le classeur s'ouvre chaque fois en lecture seule, afin que personne ne le modifie par erreur. Ce choix est enregistré avec le fichier lors de l'enregistrement. À la prochaine ouverture du fichier, la barre d'état indique qu'il est ouvert en lecture seule. Pour le modifier, ouvrez **Fichier > Info** et cliquez sur **S'ouvre toujours en lecture seule**. Il suffit d'un clic : il s'agit donc d'un rappel et non d'un verrou. Les autres programmes ne tiennent pas compte de ce choix. Un fichier texte, un fichier .xls et un fichier .ods ne peuvent pas le conserver : enregistrez d'abord le classeur au format .xlsx.

## Chiffrer avec un mot de passe

Cliquez sur **Chiffrer avec un mot de passe** pour demander un mot de passe à chaque ouverture du fichier. C'est la même chose que **Protéger par mot de passe**. Voir [Ouvrir des classeurs protégés par mot de passe](open-password). Un mot de passe protège un fichier **.xlsx** avec une protection renforcée. Un mot de passe **ne peut pas être récupéré** si vous l'oubliez. Les versions conservées et la copie de sauvegarde suivent le mot de passe.

Pour empêcher les modifications d'une feuille ou de sa mise en page, voir [Protéger une feuille ou le classeur](protect).

## Protéger la feuille ou les feuilles

**Protéger la feuille active** vous permet de choisir ce que les utilisateurs peuvent modifier sur la feuille en cours, avec un mot de passe si vous le souhaitez. **Protéger la structure du classeur** empêche d'ajouter, de supprimer, de déplacer, de masquer ou de renommer des feuilles. Ces deux commandes se trouvent aussi dans l'onglet **Révision**.

## Historique des versions

**Historique des versions** conserve des copies de votre classeur sur cet ordinateur. Rien n'est envoyé ailleurs. Une copie s'appelle une version. Conservez-en une avant une modification importante.

- **Conserver une version** enregistre le classeur tel qu'il est maintenant, sous un nom que vous choisissez.
- **Ouvrir une copie** ouvre une version dans une fenêtre distincte, afin de la consulter à côté du classeur.
- **Revenir à cette version** remet une version en place comme classeur. Le classeur tel qu'il est actuellement est d'abord conservé comme version, afin que rien ne soit perdu.
- **Supprimer** supprime une version.

Le classeur doit d'abord être enregistré, car les versions sont rattachées à un fichier. Le retour à une version fonctionne pour un fichier .xlsx.

## Inspecter le classeur

Cliquez sur **Inspecter le classeur** avant d'envoyer un classeur à quelqu'un. Cette commande répertorie les éléments du fichier que vous ne souhaitez peut-être pas partager, ainsi que leur nombre :

- **Notes** sont les notes laissées par des personnes sur des cellules. **Tout supprimer** les retire.
- **Propriétés du document et informations personnelles** sont l'auteur, la personne qui a enregistré le fichier en dernier, la société et le responsable. **Tout supprimer** les efface.
- **Feuilles masquées** sont des feuilles qui n'apparaissent pas sous forme d'onglets. **Tout afficher** les rend visibles, afin de voir leur contenu, et vous pouvez supprimer celles que vous ne voulez pas garder.
- **Lignes et colonnes masquées** sont des lignes ou des colonnes que vous avez masquées manuellement. Les lignes masquées par un filtre ne sont pas comptées. **Tout afficher** les rend visibles, afin de voir leur contenu.
- **En-têtes et pieds de page** sont le texte imprimé en haut et en bas de chaque page. **Tout supprimer** les efface sur toutes les feuilles.
- **Liens externes** sont des liens vers des valeurs situées dans d'autres classeurs. **Tout supprimer** remplace chaque cellule qui en lit un par le nombre ou le texte qu'elle affiche actuellement, et omet les liens lors de l'enregistrement. Un nom qui désignait un emplacement dans un autre classeur devient **#REF!**, une erreur indiquant que l'emplacement n'existe plus. Cette action est irréversible : enregistrez donc une copie d'abord si vous pourriez avoir besoin des liens.
- **Données XML personnalisées** sont des données ajoutées au fichier par d'autres programmes, que vous ne voyez pas dans une feuille. **Tout supprimer** les omet lors de l'enregistrement.

Enregistrez ensuite le classeur pour conserver la copie épurée.

**Supprimer les informations personnelles de ce fichier à l'enregistrement**, sous les cartes, effectue cette opération à chaque enregistrement, pour tout fichier, sans modifier le document ouvert. C'est la même case que dans **Options**, et elle est identique dans chaque application JeffOffice.

## Vérifier la compatibilité pour .csv

Choisissez **.csv** dans **Vérifier la compatibilité** pour voir ce qu'un fichier .csv ne conserverait pas de votre classeur. Un fichier .csv ne contient que les valeurs d'une feuille. La liste indique les autres feuilles, les formules (le fichier contient leurs résultats), les images, graphiques, notes, tableaux croisés dynamiques et autres éléments de votre classeur, les noms définis et les tableaux, ainsi que tout ce qui n'est pas une valeur, comme les formats de nombres, les polices, les couleurs, les bordures, les largeurs de colonnes, les cellules fusionnées, les volets figés, les filtres, la mise en forme conditionnelle, la validation des données, les liens et les paramètres d'impression. Elle mentionne aussi les choix conservés dans les propriétés du fichier, comme Marquer comme final et Toujours ouvrir en lecture seule. Cette vérification n'enregistre rien. Choisissez **Enregistrer au format .xlsx** lors de l'enregistrement pour tout conserver.

## Vérifier la compatibilité

Cliquez sur **Vérifier la compatibilité**, puis choisissez **.xls**, **.ods** ou **.csv** pour voir ce que ce type de fichier ne pourrait pas contenir. La liste indique les éléments du classeur qui seraient omis ou tronqués pour tenir dans le format, comme les graphiques, les notes ou les lignes au-delà de l'ancienne limite, ainsi que les choix conservés dans le fichier lui-même, Marquer comme final et Toujours ouvrir en lecture seule, lorsqu'ils sont activés. Cette vérification n'enregistre rien, et votre classeur reste inchangé.
