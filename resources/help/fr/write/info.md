# Info : propriétés, mots de passe et Inspection
<!-- keywords: info, propriétés, titre, auteur, objet, mots clés, catégories, commentaires, société, responsable, propriétés personnalisées, docproperty, marquer comme final, final, lecture seule, mot de passe, chiffrer, protéger, mot de passe d'ouverture, inspecter le document, informations personnelles, texte masqué, xml personnalisé, vérifier la compatibilité, afficher dans le dossier, copier le chemin, dossier, chemin -->

Cliquez sur **Fichier > Info** pour afficher les informations sur votre document et le protéger avant de le partager. Appuyez sur **Échap** pour revenir à votre document.

## Emplacement du fichier

En haut de la page, l'emplacement d'enregistrement du fichier est indiqué. Cliquez sur **Afficher dans le dossier** pour ouvrir ce dossier. Cliquez sur **Copier le chemin** pour copier le chemin, afin de le coller dans un courriel ou une conversation.

## Propriétés

Les propriétés sont des informations sur le document, comme son titre et son auteur. La page contient une zone pour chacune d'elles : **Titre**, **Auteur**, **Objet**, **Mots clés**, **Catégories**, **Commentaires**, **Société** et **Responsable**. Saisissez du texte dans une zone, puis appuyez sur **Tab** ou **Entrée**. JeffWrite conserve la modification et l'enregistre la prochaine fois que vous enregistrez le document.

Sous ces zones, la page indique les informations calculées par JeffWrite : dates de création, d'enregistrement et d'impression du document, ainsi que le nombre de pages, de mots, de caractères et de paragraphes.

### Vos propres propriétés

Cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés, comme **Projet** ou **Client**. Cliquez sur **Ajouter**, saisissez un nom, choisissez le type de valeur (texte, nombre entier, nombre décimal, oui ou non, ou date), puis saisissez la valeur.

Un champ peut afficher une propriété personnalisée. Cliquez sur **Insertion > Champ**, puis choisissez un champ. Remplacez son code par `DOCPROPERTY "Client"`, puis cliquez sur **OK**. Le texte affiche la valeur. Appuyez sur **F9** pour la mettre à jour après avoir modifié la propriété. Voir [Champs](fields).

## Marquer comme final

Cliquez sur **Marquer comme final** pour indiquer aux lecteurs que le document est terminé. JeffWrite enregistre le document, puis le place en lecture seule. Lorsqu'une personne l'ouvre, la barre de titre indique qu'il est en lecture seule, et une note précise qu'il a été marqué comme final. Il s'agit d'un moyen simple de dire « merci de ne pas modifier ce document ». Ce n'est pas un verrou : n'importe qui peut choisir de nouveau **Marqué comme final** pour le modifier.

## Chiffrer avec un mot de passe

Cliquez sur **Chiffrer avec un mot de passe** pour demander un mot de passe à chaque ouverture du fichier. Saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le fichier est brouillé lors de l'enregistrement, afin que personne ne puisse le lire sans le mot de passe. JeffWrite utilise le type de protection robuste employé par les autres traitements de texte, de sorte que ceux-ci peuvent également ouvrir le fichier avec le mot de passe.

- Un mot de passe **ne peut pas être retrouvé** si vous l'oubliez. Conservez-le en lieu sûr.
- Pour modifier le mot de passe, cliquez de nouveau sur la même carte. Cliquez sur **Supprimer le mot de passe** pour le retirer. La modification est enregistrée la prochaine fois que vous enregistrez le document.
- Les mots de passe fonctionnent avec les fichiers **.docx**. Un fichier **.odt** ne peut pas encore contenir de mot de passe : JeffWrite n'enregistre donc pas un document protégé au format .odt. Il vous en avertit au lieu de l'enregistrer sans mot de passe.

Pour ouvrir un fichier protégé, ouvrez-le comme d'habitude. JeffWrite demande le mot de passe. S'il est incorrect, vous pouvez réessayer ou cliquer sur **Annuler**.

## Inspecter le document

Cliquez sur **Inspecter le document** avant d'envoyer un document à quelqu'un. Cette commande liste les éléments contenus dans le fichier que vous ne souhaitez peut-être pas partager, ainsi que le nombre de chaque élément trouvé :

- **Commentaires** : notes laissées par des personnes dans la marge.
- **Modifications suivies** : modifications qui n'ont pas été acceptées ni refusées. **Tout accepter** intègre chaque modification au texte.
- **Propriétés du document et informations personnelles** : l'auteur, la personne qui a enregistré le document en dernier, la société et le responsable.
- **Texte masqué** : texte configuré pour être masqué. Il ne s'affiche pas et n'est pas imprimé, mais il se trouve dans le fichier.
- **En-têtes et pieds de page** : le texte situé en haut et en bas des pages.
- **Données XML personnalisées** : données ajoutées au fichier par d'autres programmes. Elles ne sont pas visibles sur la page.

Cliquez sur **Tout supprimer** à côté d'un type d'élément pour le retirer. Vous pouvez annuler les modifications apportées au texte avec **Ctrl+Z**. Enregistrez ensuite le document pour conserver la copie nettoyée.

## Vérifier la compatibilité

Cliquez sur **Vérifier la compatibilité** pour voir ce qu'un format de fichier plus ancien ne peut pas contenir. JeffWrite enregistre les fichiers **.docx** et **.odt**. Un fichier .docx contient tout le contenu de votre document. Un fichier .odt ne peut pas contenir certains éléments, et cette liste indique lesquels. Si rien n'est listé, rien n'est perdu.
