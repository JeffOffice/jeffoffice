# Utiliser les styles
<!-- keywords: style de liste, enregistrer un style de liste, style de liste à plusieurs niveaux, styles de liste, gérer les styles, style verrouillé, verrouiller un style, limiter la mise en forme, restreindre la mise en forme, exporter des styles, fichier modèle, dotx, masquer un style, styles recommandés, styles, style, volet Styles, titre, normal, créer un style, nouveau style, modifier un style, changer de style, basé sur, style suivant, mise à jour automatique, galerie, style de caractère, style de paragraphe, ctrl+alt+maj+s -->

Un **style** est un ensemble d'apparences enregistrées : une police, une taille, un espacement, et plus encore. Lorsque vous appliquez un style à un paragraphe, celui-ci prend toutes ses apparences en une seule fois. Si vous modifiez le style, chaque paragraphe qui le possède change également. C'est pourquoi les styles permettent de garder les longs documents bien ordonnés.

JeffWrite propose deux types de styles :

- Un **style de paragraphe** définit l'apparence de paragraphes entiers. **Titre 1** et **Normal** sont des styles de paragraphe.
- Un **style de caractère** définit l'apparence des mots que vous sélectionnez. **Forte** est un style de caractère.

## Appliquer un style à votre texte

1. Cliquez dans un paragraphe, ou sélectionnez les mots à modifier.
2. Cliquez sur un style dans la galerie **Styles** de l'onglet **Accueil**. Vous pouvez aussi appuyer sur **Ctrl+Maj+N** pour Normal, ou sur **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** pour Titre 1, 2 ou 3.

Un style de paragraphe s'applique à chaque paragraphe que vous touchez. Un style de caractère s'applique aux mots que vous avez sélectionnés.

## Ouvrir le volet Styles

Cliquez sur la petite flèche dans le coin du groupe **Styles**, ou appuyez sur **Ctrl+Alt+Maj+S**. Le volet **Styles** s'ouvre à droite. Il affiche les styles de votre document, chacun dans sa propre apparence.

- Le style situé au niveau de votre curseur apparaît en gras, avec une flèche devant son nom.
- Cliquez sur un style pour l'appliquer à votre sélection.
- Utilisez la liste sous les styles pour choisir ceux à afficher :
  - **Utilisés** affiche uniquement les styles employés dans votre texte.
  - **Tous les styles** affiche tous les styles, y compris ceux que votre document ne contient pas encore. Un style est ajouté au document la première fois que vous l'utilisez.
  - **Recommandés** affiche les styles destinés à être vus. Un style masqué jusqu'à son utilisation apparaît ici dès que vous l'employez.
- Cliquez sur **Gérer…** sous la liste pour choisir, pour chaque style, s'il est **Recommandé** (affiché dans la galerie), s'il est **Masqué** (exclu de toutes les listes) et son **Ordre dans les listes**. Un numéro plus petit apparaît en premier. Cliquez sur **OK** : toutes les modifications sont appliquées en une seule étape, et une seule commande **Annuler** les rétablit.
- Dans **Gérer…**, cliquez sur **Supprimer** pour retirer un style que vous avez créé. Les paragraphes qui l'utilisaient reprennent le style Normal, et les styles qui étaient basés dessus sont désormais basés sur ce dont il dépendait. Les styles fournis avec JeffWrite ne peuvent pas être supprimés. La suppression constitue une seule étape : **Annuler** rétablit le style.
- Dans **Gérer…**, cliquez sur **Importer depuis un fichier…** pour copier des styles d'un autre document. Choisissez le fichier, cochez les styles souhaités, puis cliquez sur **OK**. Un style qui en nécessite un autre l'emporte avec lui. Les styles que vous possédez déjà sont conservés, sauf si vous cochez **Remplacer les styles portant le même nom**. L'importation s'annule en une seule étape.
- Dans **Gérer…**, cliquez sur **Exporter vers un fichier…** pour enregistrer certains de vos styles dans un fichier modèle (un fichier **.dotx**). Cochez les styles souhaités, cliquez sur **OK**, puis choisissez où l'enregistrer. Un autre document peut les récupérer depuis ce fichier avec **Importer depuis un fichier…**. Un style qui en nécessite un autre l'emporte avec lui.
- Cochez **Verrouillé** pour verrouiller un style. Cochez ensuite **Limiter la mise en forme aux styles non verrouillés** pour activer la limite. Tant qu'elle est active, personne ne peut mettre en forme le texte à la main, avec **Gras** ou **Centrer** par exemple, ni utiliser un style verrouillé. Les autres styles restent disponibles. La barre d'état indique lorsqu'une commande est refusée. Décochez la case pour désactiver la limite. Si le document est protégé par un mot de passe, arrêtez d'abord cette protection, dans **Révision > Restreindre la modification**. La limite et les verrouillages sont enregistrés dans le fichier.
- Sous la liste, la zone **Trier** classe les styles. **Alphabétique** les range par nom. **Recommandé** place en premier les styles les plus utiles. **Par type** affiche les styles de paragraphe, puis les styles liés, puis les styles de caractère. **Basé sur** regroupe chaque style avec le style sur lequel il est construit.

## Créer un style à partir de votre texte

1. Mettez en forme un paragraphe comme vous le souhaitez. Par exemple, mettez-le en gras, centrez-le et définissez sa taille.
2. Cliquez sur **Nouveau style** en bas du volet, ou cliquez sur **Créer un style** dans l'onglet **Accueil**.
3. Saisissez un **Nom**. Un nom peut comporter jusqu'à 255 lettres, et il ne peut pas être identique au nom d'un autre style.
4. Vérifiez les autres options, puis cliquez sur **OK**.

Le **Type de style** peut être **Paragraphe**, **Caractère** ou **Lié**. Un style lié est un style de paragraphe accompagné d'un style de caractère de même apparence (portant le nom du style de paragraphe, auquel on ajoute « Car »). Ainsi, l'apparence peut être appliquée à un paragraphe entier ou seulement à quelques mots. Lorsque vous modifiez plus tard le style de paragraphe, son style de caractère adopte également la nouvelle apparence.

Un style **Tableau** gère les bordures, la ligne d'en-tête et les lignes alternées d'un tableau, et un style **Liste** gère les numéros ou les puces d'une liste. Pour ces deux types, saisissez le **Nom** et cliquez sur **OK** : la boîte de dialogue correspondante s'ouvre avec le nom déjà saisi. Une liste créée de cette façon est toujours enregistrée comme style de liste. Un style de tableau ou de liste n'est pas conservé lorsque vous enregistrez le document au format .odt (styles de tableau) ou .rtf (les deux) ; la commande Vérifier la compatibilité (Fichier > Informations) vous en avertit.

**Disponible dans** permet de choisir **Ce document uniquement** ou **Nouveaux documents basés sur ce modèle**. Avec le second choix, le style est également enregistré dans le modèle dont partent les nouveaux documents.

Le nouveau style est enregistré avec votre document, et votre paragraphe l'adopte. Une seule commande **Annuler** (Ctrl+Z) le supprime.

## Modifier un style

1. Cliquez dans un paragraphe qui possède le style, ou cliquez sur le style dans le volet.
2. Cliquez sur **Modifier** en bas du volet, ou cliquez sur **Modifier le style** dans l'onglet **Accueil**.
3. Modifiez les options souhaitées, puis cliquez sur **OK**.

La boîte de dialogue propose les options suivantes :

- **Nom** est le nom du style. Un style fourni avec JeffWrite conserve son nom.
- **Basé sur** désigne le style dont celui-ci part. Le nouveau style ressemble à ce style, sauf pour ce que vous modifiez. Si vous modifiez plus tard le style de base, ce style suit la modification.
- **Style du paragraphe suivant** désigne le style qu'obtient le paragraphe suivant lorsque vous appuyez sur **Entrée** à la fin de celui-ci. Un titre est généralement suivi du style Normal.
- **Police**, **taille**, **Gras**, **Italique**, **Souligné** et **couleur** définissent l'apparence des mots.
- **Alignement**, **retrait**, **espacement** et **interligne** définissent l'apparence des paragraphes.
- **Ajouter à la galerie de styles** place le style dans la galerie de l'onglet **Accueil**.
- **Mise à jour automatique** fait suivre au style votre mise en forme. Lorsque vous modifiez un paragraphe qui possède ce style, le style change pour correspondre, et tous les autres paragraphes qui le possèdent suivent la modification.

Lorsque vous modifiez un style, les paragraphes qui le possèdent changent. Si vous avez donné à un paragraphe une apparence propre, comme un mot en rouge, celle-ci est conservée.

Si vous cliquez sur **OK** sans rien modifier, le document reste inchangé.

## Voir d'où vient l'apparence de votre texte

Cliquez sur **Inspecteur de styles** en bas du volet Styles. Le volet suit votre curseur, dans le texte, un en-tête, un pied de page ou une note, et affiche trois éléments :

- Le **style de paragraphe** du paragraphe. Si le paragraphe se trouve dans un tableau, le style du tableau est indiqué sous son nom. Sous ce nom, « Plus » énumère ce que le paragraphe ajoute à son style, comme son alignement, ses retraits, son espacement, ses lignes, ses taquets de tabulation, ses bordures, sa trame, son saut de page avant, son niveau de plan, sa numérotation ou ses puces, et son sens du texte. **Effacer la mise en forme du paragraphe** supprime ces éléments, afin que le paragraphe corresponde à son style. Ce que le style d'un tableau donne à ses paragraphes n'est pas affiché, car cela ne fait pas partie du paragraphe lui-même.
- Le **style de caractère** du texte, s'il en a un. **Effacer le style de caractère** le supprime.
- La **mise en forme du texte** propre aux lettres, comme la police, la taille, le gras, une couleur, le texte masqué ou une bordure. **Effacer la mise en forme des caractères** la supprime, afin que les lettres correspondent à leurs styles. La langue du texte et son style de caractère sont conservés.

Sélectionnez d'abord le texte pour n'effacer que celui-ci. Sans sélection, le volet décrit le caractère situé avant le curseur, et **Effacer le style de caractère** et **Effacer la mise en forme des caractères** agissent sur toute la suite de caractères autour du curseur qui ont la même apparence que ce caractère. Si vous sélectionnez plusieurs paragraphes ou plusieurs caractères différents, le volet affiche « Plusieurs styles dans la sélection » ou « Plusieurs apparences dans la sélection », énumère ce que contient la sélection, et les boutons agissent sur l'ensemble.

Chaque bouton constitue une seule étape : **Annuler** (Ctrl+Z) rétablit l'apparence précédente, et le curseur reste à sa place. Modifier un style ne fait jamais apparaître une mise en forme propre aux paragraphes : un paragraphe qui suit son style n'affiche donc rien sous « Plus ». Maj+F1 ouvre toujours Afficher la mise en forme, qui montre l'apparence elle-même.

## Mettre un style à jour d'après votre texte

Mettez un paragraphe en forme comme vous le souhaitez, puis cliquez sur **Mettre à jour le style pour correspondre à la sélection** dans l'onglet **Accueil**. Le style de ce paragraphe reprend les modifications que vous avez faites, et chaque paragraphe qui possède ce style les suit.

## Enregistrer une liste comme style de liste

Un **style de liste** est une liste numérotée ou à puces portant un nom. Il conserve les neuf niveaux : le numéro ou la puce de chaque niveau, son texte, sa valeur de départ et la position du texte.

1. Cliquez sur **Liste à plusieurs niveaux** dans l'onglet **Accueil**, puis sur **Définir une nouvelle liste à plusieurs niveaux…**.
2. Réglez les niveaux comme vous le souhaitez.
3. Cochez **Enregistrer comme style de liste** et saisissez un nom. Si le nom est vide ou déjà utilisé, la boîte de dialogue l'indique et reste ouverte.
4. Cliquez sur **OK**. Les paragraphes sélectionnés prennent le style de liste.

Le style de liste apparaît à deux endroits : sous **Styles de liste** dans la galerie **Liste à plusieurs niveaux** (les 100 premiers), et dans le volet **Styles** (tous). Sélectionnez des paragraphes, puis cliquez sur son nom pour leur appliquer le style de liste.

- **Le modifier.** Cliquez sur **Liste à plusieurs niveaux**, puis sur **Modifier le style de liste**, puis sur son nom ; vous pouvez aussi choisir le style de liste dans le volet **Styles** et cliquer sur **Modifier…**. Chaque paragraphe qui possède ce style de liste change avec lui, dans le texte comme dans les en-têtes, les pieds de page et les notes. Dans le texte, la modification constitue une seule étape : une seule commande **Annuler** la rétablit.
- **Le supprimer.** Cliquez sur **Liste à plusieurs niveaux**, puis sur **Supprimer le style de liste**, puis sur son nom. Les paragraphes conservent leurs numéros et leurs puces exactement tels qu'ils apparaissent. Ils ne suivent simplement plus de style.
- **L'enregistrer dans un fichier.** Un style de liste est conservé dans un fichier `.docx` sous forme de style de numérotation, et dans un fichier `.odt` sous forme de style de liste. Il redevient un style de liste lorsque vous rouvrez le fichier. Un fichier peut contenir jusqu'à 1 000 styles de liste ; les autres sont utilisés comme listes simples.

Un niveau peut toujours être lié à un style de paragraphe, comme dans toute liste à plusieurs niveaux. Le style de paragraphe est alors numéroté par le style de liste, et **Modifier le style de liste** affiche ce lien pour que vous puissiez le modifier.

Si la zone n'a aucune liste à appliquer au style (par exemple, si le texte sélectionné est verrouillé), aucun style de liste n'est enregistré, et un message l'indique.

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Copier une apparence avec Reproduire la mise en forme](format-painter)
- [Utiliser JeffWrite au clavier](keyboard)
