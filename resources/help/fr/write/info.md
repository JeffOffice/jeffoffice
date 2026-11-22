# Infos : propriétés, mots de passe et Inspecter

<!-- keywords: vérifier l'accessibilité, infos, informations, propriétés, titre, auteur, objet, mots clés, catégories, commentaires, société, responsable, propriétés personnalisées, docproperty, marquer comme final, final, lecture seule, mot de passe, chiffrer, protéger, mot de passe d'ouverture, inspecter le document, informations personnelles, texte masqué, xml personnalisé, vérifier la compatibilité, historique des versions, versions, conserver une version, revenir, restaurer, comparer les versions, afficher dans le dossier, copier le chemin, dossier, chemin -->

Cliquez sur **Fichier > Infos** pour voir les informations sur votre document, et le protéger avant de le partager. Appuyez sur **Échap** pour revenir à votre document.

## Emplacement du fichier

En haut de la page, l'emplacement d'enregistrement du fichier est affiché. Cliquez sur **Afficher dans le dossier** pour ouvrir ce dossier. Cliquez sur **Copier le chemin** pour copier le chemin, afin de le coller dans un e-mail ou une conversation.

## Propriétés

Les propriétés sont des informations sur le document, comme son titre et son auteur. La page contient une zone pour chacune d'elles : **Titre**, **Auteur**, **Objet**, **Mots clés**, **Catégories**, **Commentaires**, **Société** et **Responsable**. Saisissez du texte dans une zone, puis appuyez sur **Tab** ou **Entrée**. JeffWrite conserve la modification et l'enregistre à la prochaine sauvegarde du document.

Sous les zones, la page affiche les informations calculées par JeffWrite : dates de création, d'enregistrement et d'impression, ainsi que le nombre de pages, de mots, de caractères et de paragraphes.

### Vos propres propriétés

Cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés, par exemple **Projet** ou **Client**. Cliquez sur **Ajouter**, saisissez un nom, choisissez le type de valeur (texte, nombre entier, nombre, oui ou non, ou date), puis saisissez la valeur.

Un champ peut afficher une propriété personnalisée. Cliquez sur **Insertion > Champ**, puis choisissez un champ. Remplacez son code par `DOCPROPERTY "Client"`, puis cliquez sur **OK**. Le texte affiche la valeur. Appuyez sur **F9** pour la mettre à jour après avoir modifié la propriété. Voir [Champs](fields).

## Marquer comme final

Cliquez sur **Marquer comme final** pour indiquer aux lecteurs que le document est terminé. JeffWrite enregistre le document puis le place en lecture seule. À l'ouverture, la barre de titre indique qu'il est en lecture seule, et une note précise qu'il a été marqué comme final. C'est une manière douce de dire « merci de ne pas modifier ce document ». Ce n'est pas un verrou, car n'importe qui peut choisir de nouveau **Marqué comme final** pour le modifier.

## Toujours ouvrir en lecture seule

Cliquez sur **Toujours ouvrir en lecture seule** pour que le document s'ouvre à chaque fois en lecture seule, afin que personne ne le modifie par erreur. Ce choix est enregistré avec le fichier lors de la sauvegarde. À la prochaine ouverture du fichier, la barre d'état indique qu'il est ouvert en lecture seule. Pour le modifier, ouvrez **Fichier > Infos** et cliquez sur **S'ouvre toujours en lecture seule**. Il suffit d'un clic : il s'agit donc d'un rappel, et non d'un verrou. Les autres programmes ne tiennent pas compte de ce choix, et un fichier .odt ne peut pas le conserver : enregistrez d'abord le document au format .docx. Vérifier la compatibilité le signale pour un fichier .odt.

## Chiffrer avec un mot de passe

Cliquez sur **Chiffrer avec un mot de passe** pour demander un mot de passe à chaque ouverture du fichier. Saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le fichier est brouillé lors de l'enregistrement, de sorte que personne ne peut le lire sans le mot de passe. JeffWrite utilise le type de protection robuste qu'utilisent les autres traitements de texte, de sorte que ceux-ci peuvent aussi ouvrir le fichier avec le mot de passe.

- Un mot de passe **ne peut pas être retrouvé** si vous l'oubliez. Conservez-le en lieu sûr.
- Les versions conservées et la copie de sauvegarde suivent le mot de passe : elles sont protégées par lui, et changent lorsque vous le modifiez ou le supprimez.
- Pour modifier le mot de passe, cliquez de nouveau sur la même carte. Cliquez sur **Supprimer le mot de passe** pour le retirer. La modification est enregistrée à la prochaine sauvegarde.
- Les mots de passe fonctionnent pour les fichiers **.docx** et pour les fichiers **.odt**. Un fichier .odt est protégé selon la norme OpenDocument, de sorte que les autres programmes qui suivent cette norme peuvent aussi l'ouvrir avec le mot de passe. Un fichier texte brut ou .rtf ne peut pas contenir de mot de passe : JeffWrite n'enregistre donc pas un document protégé sous ces formats. Il vous en informe au lieu d'enregistrer sans le mot de passe.

Pour ouvrir un fichier protégé, ouvrez-le comme d'habitude. JeffWrite demande le mot de passe. Si celui-ci est incorrect, vous pouvez réessayer ou cliquer sur **Annuler**.

JeffWrite ouvre aussi un ancien fichier **.doc** qui demande un mot de passe, et le demande de la même manière. JeffWrite n'enregistre jamais de fichier .doc : **Enregistrer sous** demande donc un nouveau nom, et la copie .docx conserve le même mot de passe.

## Inspecter le document

Cliquez sur **Inspecter le document** avant d'envoyer un document à quelqu'un. Cette commande liste les éléments contenus dans le fichier que vous ne souhaitez peut-être pas partager, et indique le nombre trouvé pour chaque type :

- **Commentaires** : notes laissées par des personnes dans la marge.
- **Modifications suivies** : modifications qui n'ont pas été acceptées ni refusées. **Tout accepter** intègre chaque modification au texte.
- **Propriétés du document et informations personnelles** : l'auteur, la personne qui a enregistré le document en dernier, la société et le responsable.
- **Texte masqué** : texte défini comme masqué. Il ne s'affiche pas et n'est pas imprimé, mais il se trouve dans le fichier.
- **En-têtes et pieds de page** : textes situés en haut et en bas des pages.
- **Données XML personnalisées** : données ajoutées au fichier par d'autres programmes. Elles ne sont pas visibles sur la page.
- **Liens externes** : liens vers d'autres fichiers, et non liens web : une image ou un objet uniquement lié (il affiche un fichier qui ne se trouve pas dans le document), ainsi qu'un champ qui tire son texte ou son image d'un autre fichier (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE et DDEAUTO). **Tout supprimer** transforme chaque champ en texte qu'il affiche actuellement, retire chaque image ou objet lié, et laisse le lien hors du fichier à l'enregistrement. Les autres fichiers ne sont pas modifiés. Les liens figurant dans les en-têtes, les pieds de page et les notes de bas de page ne sont pas comptés, et les liens web sont conservés.

Cliquez sur **Tout supprimer** à côté d'un type pour l'éliminer. Vous pouvez annuler les modifications du texte avec **Ctrl+Z**. Enregistrez ensuite le document pour conserver la copie nettoyée.

**Supprimer les informations personnelles de ce fichier à l'enregistrement**, sous les cartes, le fait à chaque enregistrement, pour tout fichier, sans modifier le document ouvert. C'est la même case que dans **Options**, et elle est identique dans toutes les applications JeffOffice.

## Vérifier la compatibilité

Cliquez sur **Vérifier la compatibilité**, puis choisissez un type de fichier pour voir ce qu'il ne peut pas contenir. JeffWrite enregistre les fichiers **.docx**, **.odt**, **.rtf** et **.txt**. Un fichier .docx contient tout le contenu de votre document. Les autres types ne peuvent pas contenir certains éléments, et cette liste indique lesquels. Elle nomme aussi les choix conservés dans le fichier lui-même, Marquer comme final et Toujours ouvrir en lecture seule, lorsqu'ils sont activés, ainsi que le mot de passe pour un type de fichier qui ne peut pas en contenir. Si rien n'est listé, rien ne sera perdu.

Lorsque vous enregistrez un fichier .odt, JeffWrite liste ce que le fichier ne peut pas contenir **avant** d'écrire quoi que ce soit. Vous pouvez choisir **Enregistrer quand même**, **Enregistrer en .docx** (le fichier .docx est placé à côté, et rien n'est perdu) ou **Annuler**. Cochez **Ne plus demander pour ce fichier**, et JeffWrite cesse de poser la question pour ce fichier jusqu'à la fermeture du programme.

## Vérifier l'accessibilité

Cliquez sur **Vérifier l'accessibilité** pour ouvrir la liste des éléments qui peuvent être difficiles à lire ou à utiliser pour certaines personnes, comme une image sans description. C'est la même vérification que **Révision > Vérifier l'accessibilité**. Voir [Compter les mots, définir la langue, vérifier l'accessibilité et rechercher des mots](proofing-tools).

## Historique des versions

Cliquez sur **Historique des versions** pour conserver des copies nommées du document sur votre ordinateur. Rien n'est envoyé ailleurs. Une version est une copie complète du document tel qu'il est à ce moment-là. Conservez une version avant une modification importante, afin de pouvoir y revenir. Le document doit être enregistré au préalable, car les versions sont conservées sous le nom du fichier.

- **Conserver une version** demande un nom, par exemple « Avant la modification », et conserve une copie.
- **Ouvrir une copie** ouvre la version choisie dans une nouvelle fenêtre. La version elle-même ne change pas.
- **Comparer** place le document et la version choisie côte à côte dans un seul document, les différences étant affichées sous forme de modifications suivies. Voir [Suivi des modifications](track-changes).
- **Revenir à cette version** rend le document identique à la version choisie. JeffWrite conserve d'abord le document tel qu'il était, sous forme d'une nouvelle version, afin que rien ne soit perdu. Enregistrez le document pour conserver la modification.
- **Supprimer** retire la version choisie.

JeffWrite conserve jusqu'à 50 versions d'un fichier et supprime les plus anciennes lorsqu'il y en a davantage.
