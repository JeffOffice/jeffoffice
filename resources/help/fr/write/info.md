# Info : propriétés, mots de passe et Inspection
<!-- keywords: vérifier l'accessibilité, info, informations, propriétés, titre, auteur, objet, mots clés, catégories, commentaires, société, responsable, propriétés personnalisées, docproperty, marquer comme final, final, lecture seule, mot de passe, chiffrer, protéger, mot de passe d'ouverture, inspecter le document, informations personnelles, texte masqué, XML personnalisé, vérifier la compatibilité, convertir, mode de compatibilité, historique des versions, versions, conserver une version, revenir, restaurer, comparer les versions, afficher dans le dossier, copier le chemin, dossier, chemin -->

Cliquez sur **Fichier > Informations** pour afficher les caractéristiques de votre document et le protéger avant de le partager. Appuyez sur **Échap** pour revenir à votre document.

## Emplacement du fichier

En haut de la page, l'emplacement d'enregistrement du fichier est indiqué. Cliquez sur **Afficher dans le dossier** pour ouvrir ce dossier. Cliquez sur **Copier le chemin** pour copier le chemin, afin de le coller dans un e-mail ou une conversation.

## Propriétés

Les propriétés sont des informations sur le document, comme son titre et son auteur. La page contient une zone pour chacune d'elles : **Titre**, **Auteur**, **Objet**, **Mots clés**, **Catégories**, **Commentaires**, **Société** et **Responsable**. Saisissez du texte dans une zone, puis appuyez sur **Tab** ou **Entrée**. JeffWrite conserve la modification et l'enregistre la prochaine fois que vous enregistrez le document.

Sous les zones, la page affiche les informations calculées par JeffWrite : dates de création, d'enregistrement et d'impression du document, ainsi que son nombre de pages, de mots, de caractères et de paragraphes.

### Vos propres propriétés

Cliquez sur **Propriétés personnalisées** pour créer vos propres propriétés, comme **Projet** ou **Client**. Cliquez sur **Ajouter**, saisissez un nom, choisissez le type de valeur (texte, nombre entier, nombre, oui ou non, ou date), puis saisissez la valeur.

Un champ peut afficher une propriété personnalisée. Cliquez sur **Insertion > Champ**, puis choisissez un champ. Remplacez son code par `DOCPROPERTY "Client"`, puis cliquez sur **OK**. Le texte affiche la valeur. Appuyez sur **F9** pour la mettre à jour après avoir modifié la propriété. Voir [Champs](fields).

## Marquer comme final

Cliquez sur **Marquer comme final** pour indiquer aux lecteurs que le document est terminé. JeffWrite enregistre le document, puis le place en lecture seule. Lorsqu'une personne l'ouvre, la barre de titre indique qu'il est en lecture seule, et une note précise qu'il a été marqué comme final. C'est un moyen discret de dire « merci de ne pas modifier ce document ». Ce n'est pas un verrou, car n'importe qui peut choisir de nouveau **Marquer comme final** pour le modifier.

## Toujours ouvrir en lecture seule

Cliquez sur **Toujours s'ouvrir en lecture seule** pour que le document s'ouvre en lecture seule à chaque fois, afin que personne ne le modifie par erreur. Ce choix est enregistré avec le fichier lors de l'enregistrement. La prochaine fois que quelqu'un ouvrira le fichier, la barre d'état indiquera qu'il est ouvert en lecture seule. Pour le modifier, ouvrez **Fichier > Informations** et cliquez sur **S'ouvre toujours en lecture seule**. Il suffit d'un clic : il s'agit donc d'un rappel et non d'un verrou. Les autres programmes ne tiennent pas compte de ce choix, et un fichier .odt ne peut pas le conserver. Enregistrez donc d'abord le document au format .docx. **Vérifier la compatibilité** le signale pour un fichier .odt.

## Chiffrer avec un mot de passe

Cliquez sur **Chiffrer avec un mot de passe** pour demander un mot de passe à chaque ouverture du fichier. Saisissez le mot de passe deux fois, puis cliquez sur **Protéger**. Le fichier est brouillé lors de l'enregistrement, de sorte que personne ne peut le lire sans le mot de passe. JeffWrite utilise la protection robuste employée par les autres traitements de texte, afin que ceux-ci puissent également ouvrir le fichier avec le mot de passe.

- Un mot de passe **ne peut pas être récupéré** si vous l'oubliez. Conservez-le en lieu sûr.
- Les versions conservées et la copie de sauvegarde suivent le mot de passe : elles sont protégées par lui, et changent lorsque vous le modifiez ou le supprimez.
- Pour modifier le mot de passe, cliquez de nouveau sur la même carte. Cliquez sur **Supprimer le mot de passe** pour le retirer. La modification est enregistrée lors du prochain enregistrement.
- Les mots de passe fonctionnent pour les fichiers **.docx** et pour les fichiers **.odt**. Un fichier .odt est protégé selon la norme OpenDocument, de sorte que les autres programmes qui respectent cette norme peuvent également l'ouvrir avec le mot de passe. Un fichier texte brut ou .rtf ne peut pas contenir de mot de passe : JeffWrite n'enregistre donc pas un document protégé dans ces formats. Il vous en informe au lieu de l'enregistrer sans mot de passe.

Pour ouvrir un fichier protégé, ouvrez-le comme d'habitude. JeffWrite demande le mot de passe. S'il n'est pas correct, vous pouvez réessayer ou cliquer sur **Annuler**.

JeffWrite ouvre également un ancien fichier **.doc** protégé par un mot de passe, et le demande de la même manière. JeffWrite n'enregistre jamais de fichier .doc : **Enregistrer sous** demande donc un nouveau nom, et la copie .docx conserve le même mot de passe.

## Inspecter le document

Cliquez sur **Inspecter le document** avant d'envoyer un document à quelqu'un. Cette commande liste les éléments contenus dans le fichier que vous ne souhaitez peut-être pas partager, avec le nombre trouvé pour chacun :

- **Commentaires** : notes laissées par des personnes dans la marge.
- **Modifications suivies** : modifications qui n'ont pas été acceptées ni refusées. **Accepter tout** conserve chaque modification dans le texte.
- **Propriétés du document et informations personnelles** : l'auteur, la personne qui a enregistré le document en dernier, la société et le responsable.
- **Texte masqué** : texte défini comme masqué. Il ne s'affiche pas et n'est pas imprimé, mais il se trouve dans le fichier.
- **En-têtes et pieds de page** : texte situé en haut et en bas des pages.
- **Données XML personnalisées** : données ajoutées au fichier par d'autres programmes. Vous ne les voyez pas sur la page.
- **Liens externes** : liens vers d'autres fichiers, et non liens web : une image ou un objet uniquement lié (il affiche un fichier qui ne se trouve pas dans le document), et un champ qui tire son texte ou son image d'un autre fichier (INCLUREIMAGE, INCLURETEXTE, LIEN, DDE et DDEAUTO). **Tout supprimer** remplace chaque champ par le texte qu'il affiche actuellement, retire chaque image ou objet lié, et supprime le lien du fichier lors de l'enregistrement. Les autres fichiers ne sont pas modifiés. Les liens figurant dans les en-têtes, les pieds de page, les notes de bas de page et les notes de fin sont également comptés et supprimés. Les liens web sont conservés. Les liens contenus dans les commentaires, les zones de texte et les parties que le programme conserve telles quelles (comme le texte des dessins) sont aussi recherchés. Une image liée qui contient également sa propre copie de l'image est conservée, sous forme d'image sans lien, et le lien est supprimé du fichier lors de l'enregistrement. La suppression des liens contenus dans les commentaires et les autres parties conservées telles quelles ne peut pas être annulée ; les zones de texte sont une étape de Annuler. Le modèle auquel le document est rattaché (son chemin contient souvent un nom d'utilisateur) compte également comme lien externe : **Tout supprimer** le détache, et le document conserve ses propres styles. Les autres liens vers des fichiers ne sont pas supprimés, et une ligne sous le décompte les nomme : la source de données du publipostage, les cadres, les sous-documents inclus dans d'autres parties, les données externes d'un graphique, les vidéos et sons liés, et les feuilles de style liées. Cette même ligne compte tout autre lien présent dans un champ, un commentaire, une zone de texte ou la liste des liens du fichier qu'aucun élément du document n'utilise, et indique lorsque l'adresse d'une image liée ne figure pas dans le fichier ou lorsqu'une partie est dans un codage que la vérification ne peut pas lire. La ligne indique alors « Non entièrement vérifié » au lieu de « Aucun résultat trouvé ». Une seule action **Tout supprimer** retire au maximum 100 000 liens dans les parties conservées ; s'il en reste, la ligne indique combien, et vous pouvez relancer **Tout supprimer**.
- **Images d'origine conservées sous les effets d'image** : images à partir desquelles un effet, comme Flou ou Mosaïque, a été créé. Le fichier conserve l'original à côté de l'image finale, et toute personne qui reçoit le fichier peut le retirer. **Supprimer les originaux** ne conserve que l'image finale, et **Rétablir l'image** ne peut alors plus restaurer l'original.
- **Parties rognées des images** : parties que vous avez rognées et qui ne sont plus visibles. Elles restent dans l'image, et toute personne qui reçoit le fichier peut les voir. **Supprimer les parties rognées** les efface, et la page reste exactement la même. Une image avec un effet devient d'abord son image finale.

Cliquez sur **Tout supprimer** à côté d'une catégorie pour la retirer. Vous pouvez annuler les modifications du texte avec **Ctrl+Z**. Les liens retirés des commentaires et des autres parties conservées, ainsi que les données XML personnalisées, ne peuvent pas être annulés. Enregistrez ensuite le document pour conserver la copie nettoyée.

**Supprimer les informations personnelles de ce fichier à l'enregistrement**, sous les cartes, effectue cette opération à chaque enregistrement, pour tout fichier, sans modifier le document ouvert. C'est la même case que dans **Options**, et elle est identique dans toutes les applications JeffOffice.

## Convertir

Un fichier .docx qui indique avoir été créé selon les règles d'un ancien programme affiche **[Mode de compatibilité]** après son nom dans la barre de titre. Cela signifie que le document est mis en page selon les règles de cet ancien programme. Un nouveau document, un fichier qui ne l'indique pas, ainsi qu'un fichier .odt, .rtf, .doc ou texte n'affichent rien. Ouvrez **Fichier > Informations** et cliquez sur **Convertir** pour utiliser les règles les plus récentes. Le programme demande d'abord confirmation, et l'opération ne peut pas être annulée avec Ctrl+Z. Les pages prennent immédiatement l'aspect qu'elles auront lorsque vous enregistrerez le fichier et le rouvrirez. La conversion modifie les éléments suivants :

- **Tableaux.** Dans l'ancienne méthode, le retrait d'un tableau est mesuré jusqu'au texte de sa première cellule, de sorte que le bord gauche du tableau se trouve un peu à gauche de celui-ci. Dans la méthode la plus récente, le retrait est mesuré jusqu'au bord du tableau. Les tableaux qui ont un retrait se décalent donc vers la droite de la largeur de la marge de cellule.
- **Sauts de page.** Dans l'ancienne méthode, un paragraphe qui se termine par un saut de page permet au paragraphe suivant de commencer la nouvelle page. Dans la méthode la plus récente, la fin de ce paragraphe passe à la nouvelle page et laisse une ligne vide en haut de celle-ci.
- **Hauteurs de ligne.** Un fichier créé selon la plus ancienne méthode peut demander des lignes d'un nombre entier de points. Après la conversion, les lignes ont leur hauteur naturelle, de sorte que les lignes et les pages peuvent se couper à d'autres endroits.

La prochaine fois que vous enregistrerez le fichier, il indiquera qu'il est en mode le plus récent, et la barre de titre n'affichera plus l'étiquette. Les choix que le fichier définit un à un, comme la coupure de mots ou les pages en vis-à-vis, restent inchangés. Un document déjà en mode le plus récent n'affiche pas le bouton **Convertir**.

## Vérifier la compatibilité

Cliquez sur **Vérifier la compatibilité**, puis choisissez un type de fichier pour voir ce qu'il ne peut pas contenir. JeffWrite enregistre les fichiers **.docx**, **.odt**, **.rtf** et **.txt**. Un fichier .docx contient tout le contenu de votre document. Les autres types ne peuvent pas contenir certains éléments, et cette liste les indique. Elle mentionne également les choix conservés dans le fichier lui-même, Marquer comme final et Toujours ouvrir en lecture seule, lorsqu'ils sont activés, ainsi que le mot de passe pour un type qui ne peut pas en contenir. Si rien n'est listé, rien n'est perdu.

Lorsque vous enregistrez au format .odt, JeffWrite affiche ce que le fichier ne peut pas contenir **avant** d'écrire quoi que ce soit. Vous pouvez choisir **Enregistrer quand même**, **Enregistrer au format .docx** (le fichier .docx est placé à côté, et rien n'est perdu) ou **Annuler**. Cochez **Ne plus demander pour ce fichier** pour que JeffWrite cesse de poser la question pour ce fichier jusqu'à la fermeture du programme.

## Vérifier l'accessibilité

Cliquez sur **Vérifier l'accessibilité** pour ouvrir la liste des éléments qui peuvent être difficiles à lire ou à utiliser pour certaines personnes, comme une image sans description. C'est la même vérification que **Révision > Vérifier l'accessibilité**. Voir [Compter les mots, définir la langue, vérifier l'accessibilité et rechercher des mots](proofing-tools).

## Historique des versions

Cliquez sur **Historique des versions** pour conserver, sur votre ordinateur, des copies nommées du document. Rien n'est envoyé ailleurs. Une version est une copie complète du document tel qu'il est à ce moment-là. Conservez-en une avant une modification importante, afin de pouvoir revenir à cet état. Le document doit d'abord être enregistré, car les versions sont conservées sous le nom du fichier.

- **Conserver une version** demande un nom, par exemple « Avant la modification », et conserve une copie.
- **Ouvrir une copie** ouvre la version choisie dans une nouvelle fenêtre. La version elle-même ne change pas.
- **Comparer** place le document et la version choisie côte à côte dans un seul document, avec les différences affichées sous forme de modifications suivies. Voir [Suivi des modifications](track-changes).
- **Revenir à cette version** rend le document identique à la version choisie. JeffWrite conserve d'abord le document tel qu'il était, sous forme d'une nouvelle version, de sorte que rien n'est perdu. Enregistrez le document pour conserver la modification.
- **Supprimer** retire la version choisie.

JeffWrite conserve jusqu'à 50 versions d'un fichier et supprime la plus ancienne lorsqu'il y en a davantage.
