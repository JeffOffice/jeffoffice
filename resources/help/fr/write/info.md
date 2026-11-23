# Info : propriétés, mots de passe et Inspecter
<!-- keywords: vérifier l'accessibilité, info, informations, propriétés, titre, auteur, objet, mots clés, catégories, commentaires, société, responsable, propriétés personnalisées, docproperty, marquer comme final, final, lecture seule, mot de passe, chiffrer, protéger, mot de passe d'ouverture, inspecter le document, informations personnelles, texte masqué, XML personnalisé, vérifier la compatibilité, historique des versions, versions, conserver une version, revenir, restaurer, comparer les versions, afficher dans le dossier, copier le chemin, dossier, chemin -->

Cliquez sur **Fichier > Info** pour voir les informations sur votre document et le protéger avant de le partager. Appuyez sur **Échap** pour revenir à votre document.

## Emplacement du fichier

En haut, la page indique où le fichier est enregistré. Cliquez sur **Afficher dans le dossier** pour ouvrir ce dossier. Cliquez sur **Copier le chemin** pour copier le chemin, afin de le coller dans un courriel ou une conversation.

## Propriétés

Les propriétés sont des informations sur le document, comme son titre et son auteur. La page contient une zone pour chacune : **Titre**, **Auteur**, **Objet**, **Mots clés**, **Catégories**, **Commentaires**, **Société** et **Responsable**. Saisissez du texte dans une zone, puis appuyez sur **Tab** ou **Entrée**. JeffWrite conserve la modification et l'enregistre à votre prochain enregistrement du document.

Sous les zones, la page indique ce que JeffWrite calcule : les dates de création, d'enregistrement et d'impression, ainsi que le nombre de pages, de mots, de caractères et de paragraphes.

### Vos propriétés personnalisées

Cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés, par exemple **Projet** ou **Client**. Cliquez sur **Ajouter**, saisissez un nom, choisissez le type de valeur (texte, nombre entier, nombre, oui ou non, ou date), puis saisissez la valeur.

Un champ peut afficher une propriété personnalisée. Cliquez sur **Insertion > Champ**, puis choisissez un champ. Remplacez son code par `DOCPROPERTY "Client"`, puis cliquez sur **OK**. Le texte affiche la valeur. Appuyez sur **F9** pour la mettre à jour après avoir modifié la propriété. Voir [Champs](fields).

## Marquer comme final

Cliquez sur **Marquer comme final** pour indiquer aux lecteurs que le document est terminé. JeffWrite enregistre le document puis le place en lecture seule. À l'ouverture, la barre de titre indique qu'il est en lecture seule, et une note précise qu'il a été marqué comme final. C'est une façon discrète de demander « merci de ne pas modifier ce document ». Ce n'est pas un verrou, car chacun peut choisir de nouveau **Marqué comme final** pour le modifier.

## Toujours ouvrir en lecture seule

Cliquez sur **Toujours ouvrir en lecture seule** pour que le document s'ouvre en lecture seule à chaque fois, afin que personne ne le modifie par accident. Ce choix est enregistré avec le fichier. À la prochaine ouverture, la barre d'état indique qu'il est ouvert en lecture seule. Pour le modifier, ouvrez **Fichier > Info** et cliquez sur **S'ouvre toujours en lecture seule**. Il suffit d'un clic : c'est donc un rappel et non un verrou. Les autres programmes ne tiennent pas compte de ce choix, et un fichier .odt ne peut pas le conserver. Enregistrez donc d'abord le document au format .docx. La fonction **Vérifier la compatibilité** le signale pour un fichier .odt.

## Chiffrer avec un mot de passe

Cliquez sur **Chiffrer avec un mot de passe** pour demander un mot de passe à chaque ouverture du fichier. Saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le fichier est brouillé à l'enregistrement, si bien que personne ne peut le lire sans le mot de passe. JeffWrite utilise la protection forte employée par les autres traitements de texte, afin qu'ils puissent eux aussi ouvrir le fichier avec le mot de passe.

- Un mot de passe **ne peut pas être récupéré** si vous l'oubliez. Conservez-le en lieu sûr.
- Les versions conservées et la copie de sauvegarde suivent le mot de passe : elles sont protégées par lui et changent lorsque vous le modifiez ou le supprimez.
- Pour modifier le mot de passe, cliquez de nouveau sur la même carte. Cliquez sur **Supprimer le mot de passe** pour le retirer. La modification est enregistrée au prochain enregistrement.
- Les mots de passe fonctionnent pour les fichiers **.docx** et pour les fichiers **.odt**. Un fichier .odt est protégé selon la norme OpenDocument, si bien que les autres programmes qui suivent cette norme peuvent aussi l'ouvrir avec le mot de passe. Un fichier texte brut ou .rtf ne peut pas contenir de mot de passe : JeffWrite n'enregistre donc pas un document protégé sous ces formats. Il vous en informe au lieu d'enregistrer sans le mot de passe.

Pour ouvrir un fichier protégé, ouvrez-le comme d'habitude. JeffWrite demande le mot de passe. S'il est incorrect, vous pouvez réessayer ou cliquer sur **Annuler**.

JeffWrite ouvre aussi un ancien fichier **.doc** qui demande un mot de passe, et le demande de la même façon. JeffWrite n'enregistre jamais de fichier .doc : **Enregistrer sous** demande donc un nouveau nom, et la copie .docx conserve le même mot de passe.

## Inspecter le document

Cliquez sur **Inspecter le document** avant d'envoyer un document à quelqu'un. La fonction liste ce que le fichier contient et que vous ne souhaitez peut-être pas partager, avec le nombre d'éléments trouvés pour chaque type :

- **Commentaires** : notes laissées par des relecteurs dans la marge.
- **Modifications suivies** : modifications qui n'ont pas été acceptées ou refusées. **Tout accepter** intègre chaque modification au texte.
- **Propriétés du document et informations personnelles** : l'auteur, la personne qui a enregistré le document en dernier, la société et le responsable.
- **Texte masqué** : texte défini comme masqué. Il ne s'affiche pas et ne s'imprime pas, mais il reste dans le fichier.
- **En-têtes et pieds de page** : textes situés en haut et en bas des pages.
- **Données XML personnalisées** : données ajoutées au fichier par d'autres programmes. Vous ne les voyez pas sur la page.
- **Liens externes** : liens vers d'autres fichiers, et non liens web : une image ou un objet uniquement lié (il affiche un fichier qui ne se trouve pas dans le document), et un champ qui tire son texte ou son image d'un autre fichier (INCLUREIMAGE, INCLURETEXTE, LIEN, DDE et DDEAUTO). **Tout supprimer** transforme chaque champ en texte qu'il affiche actuellement, retire chaque image ou objet lié, et supprime le lien du fichier à l'enregistrement. Les autres fichiers ne sont pas modifiés. Les liens figurant dans les en-têtes, les pieds de page, les notes de bas de page et les notes de fin sont également comptés et supprimés. Les liens web sont conservés. Les liens contenus dans les commentaires, les zones de texte et les parties que le programme conserve telles quelles (comme le texte des dessins) sont aussi recherchés. Une image liée qui contient aussi sa propre copie de l'image reste, sous forme d'image sans lien, et le lien est supprimé du fichier à l'enregistrement. La suppression des liens dans les commentaires et les autres parties conservées telles quelles ne peut pas être annulée ; les zones de texte relèvent d'une seule étape d'annulation. Le modèle auquel le document est rattaché (son chemin contient souvent un nom d'utilisateur) est aussi compté comme lien externe : **Tout supprimer** le détache, et le document conserve ses propres styles. Les autres liens vers des fichiers ne sont pas supprimés, et une ligne sous le décompte les nomme : la source de données du publipostage, les cadres, les sous-documents inclus dans d'autres parties, les données externes d'un graphique, les vidéos et sons liés, ainsi que les feuilles de style liées. Cette même ligne compte tout autre lien présent dans un champ, un commentaire, une zone de texte ou la liste des liens du fichier qu'aucun élément du document n'utilise, et l'indique lorsque l'adresse d'une image liée ne figure pas dans le fichier ou lorsqu'une partie est dans un codage que la vérification ne peut pas lire. La ligne indique alors « Non entièrement vérifié » au lieu de « Aucun résultat ». Un seul **Tout supprimer** retire au maximum 100 000 liens dans les parties conservées ; s'il en reste, la ligne indique combien, et vous pouvez relancer **Tout supprimer**.

Cliquez sur **Tout supprimer**, à côté d'un type, pour le retirer. Vous pouvez annuler les modifications du texte avec **Ctrl+Z**. Les liens supprimés des commentaires et des autres parties conservées, ainsi que les données XML personnalisées, ne peuvent pas être annulés. Enregistrez ensuite le document pour conserver cette copie épurée.

**Supprimer les informations personnelles de ce fichier à l'enregistrement**, sous les cartes, l'applique à chaque enregistrement, pour tout fichier, sans modifier le document ouvert. C'est la même case que dans **Options**, et elle est identique dans toutes les applications JeffOffice.

## Vérifier la compatibilité

Cliquez sur **Vérifier la compatibilité**, puis choisissez un type de fichier pour voir ce qu'il ne peut pas contenir. JeffWrite enregistre les fichiers **.docx**, **.odt**, **.rtf** et **.txt**. Un fichier .docx contient tout le document. Les autres types ne peuvent pas contenir certains éléments, et cette liste les indique. Elle mentionne aussi les choix conservés dans le fichier lui-même, Marquer comme final et Toujours ouvrir en lecture seule, lorsqu'ils sont activés, ainsi que le mot de passe pour un type qui ne peut pas en contenir un. Si rien n'est listé, rien ne sera perdu.

Lorsque vous enregistrez au format .odt, JeffWrite liste ce que le fichier ne peut pas contenir **avant** d'écrire quoi que ce soit. Vous pouvez choisir **Enregistrer quand même**, **Enregistrer au format .docx** (le fichier .docx est placé à côté, et rien n'est perdu) ou **Annuler**. Cochez **Ne plus demander pour ce fichier** : JeffWrite cessera alors de poser la question pour ce fichier jusqu'à la fermeture du programme.

## Vérifier l'accessibilité

Cliquez sur **Vérifier l'accessibilité** pour ouvrir la liste des éléments qui peuvent être difficiles à lire ou à utiliser pour certaines personnes, comme une image sans description. C'est la même vérification que **Révision > Vérifier l'accessibilité**. Voir [Compter les mots, définir la langue, vérifier l'accessibilité et rechercher des mots](proofing-tools).

## Historique des versions

Cliquez sur **Historique des versions** pour conserver, sur votre ordinateur, des copies nommées du document. Rien n'est envoyé ailleurs. Une version est une copie complète du document tel qu'il est à ce moment-là. Conservez-en une avant une modification importante, afin de pouvoir revenir en arrière. Le document doit d'abord être enregistré, car les versions sont conservées sous le nom du fichier.

- **Conserver une version** demande un nom, par exemple « Avant la révision », et conserve une copie.
- **Ouvrir une copie** ouvre la version choisie dans une nouvelle fenêtre. La version elle-même ne change pas.
- **Comparer** place le document et la version choisie côte à côte dans un seul document, avec les différences affichées comme modifications suivies. Voir [Suivi des modifications](track-changes).
- **Revenir à cette version** rend le document identique à la version choisie. JeffWrite conserve d'abord le document tel qu'il était, sous forme de nouvelle version, si bien que rien n'est perdu. Enregistrez le document pour conserver la modification.
- **Supprimer** retire la version choisie.

JeffWrite conserve jusqu'à 50 versions d'un fichier et supprime les plus anciennes lorsqu'il y en a davantage.
