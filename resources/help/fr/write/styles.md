# Utiliser les styles
<!-- keywords: gérer les styles, style verrouillé, verrouiller un style, limiter la mise en forme, restreindre la mise en forme, exporter les styles, fichier modèle, dotx, masquer un style, styles recommandés, styles, style, volet Styles, titre, normal, créer un style, nouveau style, modifier un style, changer de style, basé sur, style suivant, mise à jour automatique, galerie, style de caractère, style de paragraphe, ctrl+alt+maj+s -->

Un **style** est un ensemble d'attributs enregistrés : une police, une taille, un espacement, et plus encore. Lorsque vous appliquez un style à un paragraphe, celui-ci prend tous ces attributs d'un coup. Lorsque vous modifiez le style, chaque paragraphe qui l'utilise change aussi. C'est pourquoi les styles permettent de garder les longs documents bien ordonnés.

JeffWrite propose deux types de styles :

- Un **style de paragraphe** définit l'apparence de paragraphes entiers. **Titre 1** et **Normal** sont des styles de paragraphe.
- Un **style de caractère** définit l'apparence des mots que vous sélectionnez. **Forte** est un style de caractère.

## Appliquer un style à votre texte

1. Cliquez dans un paragraphe, ou sélectionnez les mots à modifier.
2. Cliquez sur un style dans la galerie **Styles** de l'onglet **Accueil**. Vous pouvez aussi appuyer sur **Ctrl+Maj+N** pour Normal, ou sur **Ctrl+Alt+1**, **Ctrl+Alt+2** ou **Ctrl+Alt+3** pour Titre 1, 2 ou 3.

Un style de paragraphe s'applique à chaque paragraphe que vous touchez. Un style de caractère s'applique aux mots que vous avez sélectionnés.

## Ouvrir le volet Styles

Cliquez sur la petite flèche dans le coin du groupe **Styles**, ou appuyez sur **Ctrl+Alt+Maj+S**. Le volet **Styles** s'ouvre à droite. Il affiche les styles de votre document, chacun dans son propre aspect.

- Le style situé à l'emplacement du curseur est en gras, avec une flèche devant son nom.
- Cliquez sur un style pour l'appliquer à votre sélection.
- Utilisez la liste sous les styles pour choisir ceux à afficher :
  - **Utilisés** affiche uniquement les styles employés dans votre texte.
  - **Tous les styles** affiche tous les styles, y compris ceux que votre document ne contient pas encore. Un style est ajouté au document la première fois que vous l'utilisez.
  - **Recommandés** affiche les styles destinés à être visibles. Un style masqué jusqu'à son utilisation apparaît ici dès que vous l'utilisez.
- Cliquez sur **Gérer…** sous la liste pour choisir, pour chaque style, s'il est **Recommandé** (affiché dans la galerie), s'il est **Masqué** (exclu de toutes les listes) et son **Ordre dans les listes**. Un numéro plus petit apparaît en premier. Cliquez sur **OK** : toutes les modifications sont appliquées en une seule étape, donc un seul **Annuler** les annule toutes.
- Dans **Gérer…**, cliquez sur **Supprimer** pour retirer un style que vous avez créé. Les paragraphes qui l'utilisaient reviennent à Normal, et les styles qui étaient basés sur lui se basent sur ce dont il dépendait. Les styles fournis avec JeffWrite ne peuvent pas être supprimés. La suppression est une seule étape : **Annuler** rétablit le style.
- Dans **Gérer…**, cliquez sur **Importer depuis un fichier…** pour copier des styles d'un autre document. Choisissez le fichier, cochez les styles souhaités, puis cliquez sur **OK**. Un style qui en nécessite un autre l'emporte avec lui. Les styles que vous possédez déjà sont conservés, sauf si vous cochez **Remplacer les styles portant le même nom**. L'importation s'annule en une seule étape.
- Dans **Gérer…**, cliquez sur **Exporter vers un fichier…** pour enregistrer certains de vos styles dans un fichier modèle (un fichier **.dotx**). Cochez les styles souhaités, cliquez sur **OK**, puis choisissez l'emplacement d'enregistrement. Un autre document peut récupérer ces styles depuis ce fichier avec **Importer depuis un fichier…**. Un style qui en nécessite un autre l'emporte avec lui.
- Cochez **Verrouillé** pour verrouiller un style. Cochez ensuite **Limiter la mise en forme aux styles non verrouillés** pour activer la limitation. Tant qu'elle est active, personne ne peut mettre le texte en forme manuellement, par exemple avec **Gras** ou **Centrer**, ni utiliser un style verrouillé. Les autres styles restent disponibles. La barre d'état indique quand une commande est refusée. Décochez la case pour désactiver la limitation. Si le document est protégé par un mot de passe, arrêtez d'abord cette protection dans **Révision > Restreindre la modification**. La limitation et les verrouillages sont enregistrés dans le fichier.
- Sous la liste, la zone **Trier** range les styles. **Alphabétique** trie par nom. **Recommandé** place en premier les styles les plus utiles. **Par type** affiche les styles de paragraphe, puis les styles liés, puis les styles de caractère. **Basé sur** regroupe chaque style avec le style sur lequel il est construit.

## Créer un style à partir de votre texte

1. Mettez en forme un paragraphe comme vous le souhaitez. Par exemple, mettez-le en gras, centrez-le et définissez sa taille.
2. Cliquez sur **Nouveau style** en bas du volet, ou cliquez sur **Créer un style** dans l'onglet **Accueil**.
3. Saisissez un **Nom**. Un nom peut comporter jusqu'à 255 caractères, et il ne peut pas être identique au nom d'un autre style.
4. Vérifiez les autres options, puis cliquez sur **OK**.

Le nouveau style est enregistré avec votre document, et votre paragraphe l'adopte. Un seul **Annuler** (Ctrl+Z) supprime le style.

## Modifier un style

1. Cliquez dans un paragraphe qui possède ce style, ou cliquez sur le style dans le volet.
2. Cliquez sur **Modifier** en bas du volet, ou cliquez sur **Modifier le style** dans l'onglet **Accueil**.
3. Modifiez ce que vous souhaitez, puis cliquez sur **OK**.

La boîte de dialogue propose les options suivantes :

- **Nom** est le nom du style. Un style fourni avec JeffWrite conserve son nom.
- **Basé sur** est le style à partir duquel celui-ci est construit. Le nouveau style ressemble à ce style, sauf pour ce que vous modifiez. Si vous modifiez plus tard le style de base, celui-ci suit la modification.
- **Style du paragraphe suivant** est le style que reçoit le paragraphe suivant lorsque vous appuyez sur **Entrée** à la fin de celui-ci. Un titre est généralement suivi de Normal.
- **Police**, **taille**, **Gras**, **Italique**, **Souligné** et **couleur** définissent l'apparence des mots.
- **Alignement**, **retraits**, **espacement** et **interligne** définissent la disposition des paragraphes.
- **Ajouter à la galerie de styles** place le style dans la galerie de l'onglet **Accueil**.
- **Mise à jour automatique** fait suivre au style votre mise en forme. Lorsque vous modifiez un paragraphe qui possède ce style, le style change en conséquence, et tous les autres paragraphes qui l'utilisent suivent.

Lorsque vous modifiez un style, les paragraphes qui l'utilisent changent. Si vous avez donné à un paragraphe un aspect qui lui est propre, comme un mot en rouge, celui-ci le conserve.

Si vous cliquez sur **OK** sans rien modifier, le document n'est pas changé.

## Mettre à jour un style à partir de votre texte

Mettez en forme un paragraphe comme vous le souhaitez. Cliquez ensuite sur **Mettre à jour le style pour correspondre à la sélection** dans l'onglet **Accueil**. Le style de ce paragraphe reprend les modifications que vous avez faites, et chaque paragraphe qui utilise ce style les suit.

## Voir aussi

- [Modifier l'apparence du texte](font)
- [Copier une mise en forme avec Reproduire la mise en forme](format-painter)
- [Utiliser JeffWrite au clavier](keyboard)
