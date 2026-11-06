# Info : propriétés, mots de passe et Inspecter

<!-- keywords: vérifier l'accessibilité, info, informations, propriétés, titre, auteur, objet, mots clés, catégories, commentaires, société, responsable, propriétés personnalisées, docproperty, marquer comme final, final, lecture seule, mot de passe, chiffrer, protéger, mot de passe d'ouverture, inspecter le document, informations personnelles, texte masqué, xml personnalisé, vérifier la compatibilité, historique des versions, versions, conserver une version, revenir, restaurer, comparer les versions, afficher dans le dossier, copier le chemin, dossier, chemin -->

Cliquez sur **Fichier > Info** pour voir les informations sur votre document et le protéger avant de le partager. Appuyez sur **Échap** pour revenir à votre document.

## Emplacement du fichier

En haut de la page, l'emplacement d'enregistrement du fichier s'affiche. Cliquez sur **Afficher dans le dossier** pour ouvrir ce dossier. Cliquez sur **Copier le chemin** pour copier le chemin, afin de le coller dans un e-mail ou une conversation.

## Propriétés

Les propriétés sont des informations sur le document, comme son titre et son auteur. La page contient une zone pour chacune d'elles : **Titre**, **Auteur**, **Objet**, **Mots clés**, **Catégories**, **Commentaires**, **Société** et **Responsable**. Saisissez du texte dans une zone, puis appuyez sur **Tab** ou **Entrée**. JeffWrite conserve la modification et l'enregistre la prochaine fois que vous enregistrez le document.

Sous ces zones, la page indique ce que JeffWrite calcule : les dates de création, d'enregistrement et d'impression du document, ainsi que son nombre de pages, de mots, de caractères et de paragraphes.

### Vos propres propriétés

Cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés, comme **Projet** ou **Client**. Cliquez sur **Ajouter**, saisissez un nom, choisissez le type de valeur (texte, nombre entier, nombre, oui ou non, ou date), puis saisissez la valeur.

Un champ peut afficher une propriété personnalisée. Cliquez sur **Insertion > Champ**, puis choisissez un champ. Remplacez son code par `DOCPROPERTY "Client"`, puis cliquez sur **OK**. Le texte affiche la valeur. Appuyez sur **F9** pour la mettre à jour après avoir modifié la propriété. Voir [Champs](fields).

## Marquer comme final

Cliquez sur **Marquer comme final** pour indiquer aux lecteurs que le document est terminé. JeffWrite enregistre le document, puis le place en lecture seule. Lorsque quelqu'un l'ouvre, la barre de titre indique qu'il est en lecture seule, et une note précise qu'il a été marqué comme final. C'est une façon discrète de dire « merci de ne pas modifier ceci ». Ce n'est pas un verrou, car n'importe qui peut choisir de nouveau **Marqué comme final** pour le modifier.

## Chiffrer avec un mot de passe

Cliquez sur **Chiffrer avec un mot de passe** pour demander un mot de passe à chaque ouverture du fichier. Saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le fichier est brouillé lors de l'enregistrement, de sorte que personne ne peut le lire sans le mot de passe. JeffWrite utilise le type de protection robuste qu'utilisent les autres traitements de texte, afin que ceux-ci puissent aussi ouvrir le fichier avec le mot de passe.

- Un mot de passe **ne peut pas être retrouvé** si vous l'oubliez. Conservez-le en lieu sûr.
- Pour modifier le mot de passe, cliquez de nouveau sur la même carte. Cliquez sur **Supprimer le mot de passe** pour le retirer. La modification est enregistrée lors du prochain enregistrement.
- Les mots de passe fonctionnent pour les fichiers **.docx**. Un fichier **.odt** ne peut pas encore contenir de mot de passe ; JeffWrite n'enregistre donc pas un document protégé au format .odt. Il vous le signale au lieu de l'enregistrer sans mot de passe.

Pour ouvrir un fichier protégé, ouvrez-le comme d'habitude. JeffWrite demande le mot de passe. S'il est incorrect, vous pouvez réessayer ou cliquer sur **Annuler**.

## Inspecter le document

Cliquez sur **Inspecter le document** avant d'envoyer un document à quelqu'un. Cette fonction liste ce que le fichier contient et que vous ne voulez peut-être pas partager, ainsi que le nombre d'éléments trouvés pour chaque type :

- **Commentaires** : notes laissées par des personnes dans la marge.
- **Modifications suivies** : modifications qui n'ont pas été acceptées ni refusées. **Tout accepter** conserve chaque modification dans le texte.
- **Propriétés du document et informations personnelles** : l'auteur, la personne qui a enregistré le document en dernier, la société et le responsable.
- **Texte masqué** : texte défini comme masqué. Il ne s'affiche pas et n'est pas imprimé, mais il se trouve dans le fichier.
- **En-têtes et pieds de page** : texte situé en haut et en bas des pages.
- **Données XML personnalisées** : données ajoutées au fichier par d'autres programmes. Vous ne les voyez pas sur la page.

Cliquez sur **Tout supprimer** à côté d'un type pour le retirer. Vous pouvez annuler les modifications apportées au texte avec **Ctrl+Z**. Enregistrez ensuite le document pour conserver la copie nettoyée.

## Vérifier la compatibilité

Cliquez sur **Vérifier la compatibilité** pour voir ce qu'un format de fichier plus ancien ne peut pas contenir. JeffWrite enregistre les fichiers **.docx** et **.odt**. Un fichier .docx conserve tout le contenu de votre document. Un fichier .odt ne peut pas contenir certains éléments, et cette liste les indique. Si rien n'apparaît, rien n'est perdu.

Lorsque vous enregistrez au format .odt, JeffWrite affiche la liste de ce que le fichier ne peut pas contenir **avant** d'écrire quoi que ce soit. Vous pouvez choisir **Enregistrer quand même en .odt**, **Enregistrer plutôt en .docx** (le fichier .docx est placé à côté, et rien n'est perdu) ou **Annuler**.

## Vérifier l'accessibilité

Cliquez sur **Vérifier l'accessibilité** pour ouvrir la liste des éléments qui peuvent être difficiles à lire ou à utiliser pour certaines personnes, comme une image sans description. C'est la même vérification que **Révision > Vérifier l'accessibilité**. Voir [Compter les mots, définir la langue, vérifier l'accessibilité et rechercher des mots](proofing-tools).

## Historique des versions

Cliquez sur **Historique des versions** pour conserver des copies nommées du document sur votre ordinateur. Rien n'est envoyé ailleurs. Une version est une copie complète du document tel qu'il est à ce moment. Conservez-en une avant une modification importante, afin de pouvoir y revenir. Le document doit être enregistré au préalable, car les versions sont conservées sous le nom du fichier.

- **Conserver une version** demande un nom, par exemple « Avant la modification », et conserve une copie.
- **Ouvrir une copie** ouvre la version choisie dans une nouvelle fenêtre. La version elle-même ne change pas.
- **Comparer** affiche le document et la version choisie côte à côte, dans un seul document, avec les différences présentées sous forme de modifications suivies. Voir [Suivi des modifications](track-changes).
- **Revenir à celle-ci** rend le document identique à la version choisie. JeffWrite conserve d'abord le document dans son état actuel comme nouvelle version, afin que rien ne soit perdu. Enregistrez le document pour conserver la modification.
- **Supprimer** retire la version choisie.

JeffWrite conserve jusqu'à 50 versions d'un fichier et supprime les plus anciennes lorsqu'il y en a davantage.
