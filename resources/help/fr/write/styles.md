# Utiliser les styles
<!-- keywords: style de liste, enregistrer un style de liste, style de liste à plusieurs niveaux, styles de liste, gérer les styles, style verrouillé, verrouiller un style, limiter la mise en forme, restreindre la mise en forme, exporter des styles, fichier modèle, dotx, masquer un style, styles recommandés, styles, style, volet Styles, titre, normal, créer un style, nouveau style, modifier un style, changer de style, basé sur, style suivant, mise à jour automatique, galerie, style de caractère, style de paragraphe, ctrl+alt+maj+s -->

Un **style** est un ensemble de présentations enregistrées : une police, une taille, un espacement, et plus encore. Quand vous appliquez un style à un paragraphe, celui-ci prend toute sa mise en forme d'un coup. Quand vous modifiez le style, chaque paragraphe qui l'utilise change aussi. C'est pour cela que les styles gardent les longs documents bien ordonnés.

JeffWrite propose deux types de styles :

- Un **style de paragraphe** définit la mise en forme de paragraphes entiers. **Titre 1** et **Normal** sont des styles de paragraphe.
- Un **style de caractère** définit la mise en forme des mots que vous sélectionnez. **Forte** est un style de caractère.

## Appliquer un style à votre texte

1. Cliquez dans un paragraphe, ou sélectionnez les mots à modifier.
2. Cliquez sur un style dans la galerie **Styles** de l'onglet **Accueil**. Vous pouvez aussi appuyer sur **Ctrl+Maj+N** pour **Normal**, ou sur **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** pour **Titre 1**, **2** ou **3**.

Un style de paragraphe s'applique à chaque paragraphe que vous touchez. Un style de caractère s'applique aux mots que vous avez sélectionnés.

## Ouvrir le volet Styles

Cliquez sur la petite flèche à l'angle du groupe **Styles**, ou appuyez sur **Ctrl+Alt+Maj+S**. Le volet **Styles** s'ouvre à droite. Il liste les styles de votre document, chacun dans sa propre présentation.

- Le style situé à votre curseur apparaît en gras, avec une flèche devant son nom.
- Cliquez sur un style pour l'appliquer à votre sélection.
- Utilisez la liste sous les styles pour choisir ceux à afficher :
  - **Utilisés** affiche uniquement les styles employés dans votre texte.
  - **Tous les styles** affiche chaque style, y compris ceux que votre document ne contient pas encore. Un style est ajouté au document la première fois que vous l'utilisez.
  - **Recommandés** affiche les styles destinés à être vus. Un style masqué tant qu'il n'est pas utilisé apparaît ici dès que vous l'utilisez.
- Cliquez sur **Gérer…** sous la liste pour choisir, pour chaque style, s'il est **Recommandé** (affiché dans la galerie), s'il est **Masqué** (exclu de toutes les listes), et son **Ordre dans les listes**. Un numéro plus petit apparaît en premier. Cliquez sur **OK** : toutes les modifications sont appliquées en une seule fois, si bien qu'un seul **Annuler** les rétablit.
- Dans **Gérer…**, cliquez sur **Supprimer** pour retirer un style que vous avez créé. Les paragraphes qui l'utilisaient reviennent à Normal, et les styles qui étaient basés sur lui sont désormais basés sur ce dont il était lui-même basé. Les styles fournis avec JeffWrite ne peuvent pas être supprimés. La suppression se fait en une seule étape : **Annuler** rétablit le style.
- Dans **Gérer…**, cliquez sur **Importer depuis un fichier…** pour copier des styles d'un autre document. Choisissez le fichier, cochez les styles voulus, puis cliquez sur **OK**. Un style qui en nécessite un autre entraîne aussi celui-ci. Les styles que vous avez déjà sont conservés, sauf si vous cochez **Remplacer les styles portant le même nom**. L'importation s'annule en une seule étape.
- Dans **Gérer…**, cliquez sur **Exporter vers un fichier…** pour enregistrer certains de vos styles dans un fichier modèle (un fichier **.dotx**). Cochez les styles voulus, cliquez sur **OK**, puis choisissez où enregistrer le fichier. Un autre document peut les récupérer depuis ce fichier avec **Importer depuis un fichier…**. Un style qui en nécessite un autre entraîne aussi celui-ci.
- Cochez **Verrouillé** pour verrouiller un style. Cochez ensuite **Limiter la mise en forme aux styles non verrouillés** pour activer la limitation. Lorsqu'elle est active, personne ne peut mettre le texte en forme manuellement, par exemple avec **Gras** ou **Centrer**, et personne ne peut utiliser un style verrouillé. Les autres styles restent disponibles. La barre d'état l'indique lorsqu'une commande est refusée. Décochez la case pour désactiver la limitation. Si le document est protégé par un mot de passe, arrêtez d'abord cette protection, dans **Révision > Restreindre la modification**. La limitation et les verrouillages sont enregistrés dans le fichier.
- Sous la liste, la zone **Trier** classe les styles. **Alphabétique** trie par nom. **Recommandé** place en premier les styles les plus utiles. **Par type** affiche les styles de paragraphe, puis les styles liés, puis les styles de caractère. **Basé sur** regroupe chaque style avec le style sur lequel il est construit.

## Créer un style à partir de votre texte

1. Mettez un paragraphe en forme comme vous le souhaitez. Par exemple, mettez-le en gras, centrez-le et réglez sa taille.
2. Cliquez sur **Nouveau style** en bas du volet, ou sur **Créer un style…** dans l'onglet **Accueil**.
3. Saisissez un **Nom**. Un nom peut comporter jusqu'à 255 lettres, et il ne peut pas être le nom d'un autre style.
4. Vérifiez les autres options, puis cliquez sur **OK**.

Le nouveau style est enregistré avec votre document, et votre paragraphe l'adopte. Un seul **Annuler** (Ctrl+Z) supprime le style.

## Modifier un style

1. Cliquez dans un paragraphe qui a le style, ou cliquez sur le style dans le volet.
2. Cliquez sur **Modifier** en bas du volet, ou sur **Modifier le style** dans l'onglet **Accueil**.
3. Modifiez ce que vous voulez, puis cliquez sur **OK**.

La boîte de dialogue propose les options suivantes :

- **Nom** est le nom du style. Un style fourni avec JeffWrite garde son nom.
- **Basé sur** est le style dont celui-ci part. Le nouveau style ressemble à ce style, sauf pour ce que vous modifiez. Si vous modifiez plus tard le style de base, celui-ci le suit.
- **Style du paragraphe suivant** est le style que reçoit le paragraphe suivant lorsque vous appuyez sur **Entrée** à la fin de celui-ci. Un titre est généralement suivi de Normal.
- **Police**, **taille**, **Gras**, **Italique**, **Soulignement** et **couleur** déterminent l'apparence des mots.
- **Alignement**, **retrait**, **espacement** et **interligne** déterminent l'apparence des paragraphes.
- **Ajouter à la galerie de styles** place le style dans la galerie de l'onglet **Accueil**.
- **Mettre à jour automatiquement** fait suivre votre mise en forme au style. Lorsque vous modifiez un paragraphe qui a ce style, le style change pour correspondre, et tous les autres paragraphes qui l'ont suivent la modification.

Lorsque vous modifiez un style, les paragraphes qui l'utilisent changent. Si vous avez donné à un paragraphe une présentation propre, comme un mot en rouge, il conserve cette présentation.

Si vous cliquez sur **OK** sans rien modifier, le document reste inchangé.

## Mettre un style à jour à partir de votre texte

Mettez un paragraphe en forme comme vous le souhaitez. Cliquez ensuite sur **Mettre à jour le style pour correspondre à la sélection** dans l'onglet **Accueil**. Le style de ce paragraphe reprend les modifications que vous avez faites, et chaque paragraphe qui a ce style suit.

## Enregistrer une liste comme style de liste

Un **style de liste** est une liste numérotée ou à puces qui porte un nom. Il conserve les neuf niveaux : le numéro ou la puce de chaque niveau, son texte, sa position de départ et l'emplacement du texte.

1. Cliquez sur **Liste à plusieurs niveaux** dans l'onglet **Accueil**, puis sur **Définir une nouvelle liste à plusieurs niveaux…**.
2. Réglez les niveaux comme vous le souhaitez.
3. Cochez **Enregistrer comme style de liste** et saisissez un nom. Si le nom est vide ou déjà utilisé, la boîte le signale et reste ouverte.
4. Cliquez sur **OK**. Les paragraphes sélectionnés prennent le style de liste.

Le style de liste apparaît à deux endroits : sous **Styles de liste**, dans la galerie **Liste à plusieurs niveaux** (les 100 premiers), et dans le volet **Styles** (tous). Sélectionnez des paragraphes, puis cliquez sur son nom pour leur appliquer le style de liste.

- **Le modifier.** Cliquez sur **Liste à plusieurs niveaux**, puis sur **Modifier le style de liste**, puis sur le nom ; vous pouvez aussi choisir le style de liste dans le volet **Styles** et cliquer sur **Modifier…**. Chaque paragraphe qui a le style de liste change avec lui, dans le texte comme dans les en-têtes, les pieds de page et les notes. Dans le texte, la modification se fait en une seule étape : un seul **Annuler** la rétablit.
- **Le supprimer.** Cliquez sur **Liste à plusieurs niveaux**, puis sur **Supprimer le style de liste**, puis sur le nom. Les paragraphes conservent exactement leurs numéros et leurs puces. Ils ne suivent simplement plus de style.
- **L'enregistrer dans un fichier.** Un style de liste est conservé dans un fichier `.docx` sous forme de style de numérotation, et dans un fichier `.odt` sous forme de style de liste. Il est restauré comme style de liste lorsque vous rouvrez le fichier. Un fichier peut contenir jusqu'à 1 000 styles de liste ; les autres sont utilisés comme listes simples.

Un niveau peut aussi être lié à un style de paragraphe, comme dans toute liste à plusieurs niveaux. Le paragraphe est alors numéroté par le style de liste, et **Modifier le style de liste** affiche le lien pour que vous puissiez le changer.

Si la boîte n'a aucune liste à appliquer au style (par exemple, si le texte sélectionné est verrouillé), aucun style de liste n'est enregistré, et un message le signale.

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Copier une mise en forme avec Copier la mise en forme](format-painter)
- [Utiliser JeffWrite au clavier](keyboard)
