# Styles de cellule

<!-- keywords: modifier le style de cellule, changer un style, fusionner les styles, importer des styles, style de cellule, styles, style nommé, Satisfaisant, Insatisfaisant, Neutre, Titre, Total, Accent, Entrée, Sortie, Calcul, Cellule de contrôle, Texte explicatif, Note, Texte d'avertissement, Cellule liée, Milliers, Monétaire, Pourcentage, Normal, nouveau style, nom, supprimer le style, retirer le style -->

Un **style de cellule** est un ensemble nommé de présentations : une police, un remplissage, une bordure, un format de nombre et la façon dont le texte est disposé. Si vous appliquez le style « Titre 1 » à une cellule, elle prend tous ces éléments en une fois. Les styles assurent la cohérence d'un classeur, car toutes les cellules qui ont un même style se présentent de la même manière.

## Appliquer un style

1. Sélectionnez les cellules.
2. Cliquez sur **Accueil > Styles de cellule**, puis cliquez sur un style. Le menu affiche un petit aperçu de chacun d'eux.

Le menu comporte les groupes suivants :

- **Satisfaisant, insatisfaisant et neutre** : Normal, Bon, Mauvais et Neutre.
- **Données et modèle** : Calcul, Cellule de contrôle, Texte explicatif, Entrée, Cellule liée, Note, Sortie et Texte d'avertissement.
- **Titres et en-têtes** : Titre, Titre 1 à 4, et Total.
- **Styles de cellule avec thème** : quatre nuances de chacune des six couleurs d'accent du thème du classeur.
- **Format de nombre** : Milliers, Milliers [0], Monétaire, Monétaire [0] et Pourcentage.
- **Personnalisé** : les styles que le classeur possède déjà sous d'autres noms.

Un style que le classeur possède déjà est utilisé tel quel, avec l'aspect qu'il a dans ce classeur. Un style qui n'existe pas dans le classeur est créé la première fois que vous l'utilisez, à partir de la première police du classeur, puis il fait partie du fichier au même titre que les autres.

## Créer un style personnalisé

1. Mettez en forme une cellule comme vous le souhaitez : police, remplissage, bordure, format de nombre, etc.
2. Laissez le curseur sur cette cellule. Sélectionnez les cellules qui doivent également recevoir ce style.
3. Cliquez sur **Accueil > Styles de cellule > Nouveau style de cellule**, saisissez un nom, puis cliquez sur **OK**.

Le style reprend l'aspect de la cellule où se trouve le curseur, et les cellules sélectionnées reçoivent ce style. Il apparaît désormais dans le groupe **Personnalisé** du menu et il est enregistré dans le classeur. Chaque style doit avoir un nom unique. Un nom déjà utilisé (sans tenir compte des majuscules et des minuscules) est refusé, de même qu'un nom vide.

## Modifier un style

Cliquez sur **Accueil > Styles de cellule > Modifier le style de cellule**, puis choisissez un style du classeur (un style proposé dans la galerie fait partie du classeur dès qu'une cellule l'a utilisé). La boîte de dialogue Format de cellule s'ouvre, en partant de l'aspect propre au style. Modifiez ce que vous souhaitez, puis cliquez sur **OK** : le style prend la modification, ainsi que toutes les cellules qui l'utilisent, en une seule étape que vous pouvez annuler. Seuls les éléments que la cellule reprend du style sont modifiés : une cellule qui possède sa propre mise en forme la conserve, donc une cellule dotée de son propre remplissage garde ce remplissage lorsque celui du style change. Les colonnes et lignes entières mises en forme avec le style prennent également la modification. Sur une feuille protégée contre la mise en forme des cellules, si une cellule utilise le style, la modification est refusée et rien n'est changé. Vous ne pouvez pas modifier Normal de cette manière. Le style est enregistré dans le classeur avec son nouvel aspect, et une cellule à laquelle il sera appliqué ultérieurement en héritera.

## Fusionner des styles d'un autre classeur

Ouvrez le classeur qui contient les styles, puis, dans le classeur qui doit les recevoir, cliquez sur **Accueil > Styles de cellule > Fusionner les styles** et choisissez le classeur. Chaque style qu'il contient et que ce classeur n'a pas est ajouté. Lorsqu'un style porte le même nom ici (sans tenir compte des majuscules et des minuscules) mais un aspect différent, il vous est demandé une seule fois s'il faut les remplacer : **Remplacer** leur donne l'aspect de l'autre classeur, comme le fait Modifier (les cellules qui les utilisent ne changent que pour ce qu'elles reprennent du style), et **Conserver** laisse vos propres styles. Une couleur qui est une couleur du thème est importée sous la forme de la couleur qu'elle affiche dans l'autre classeur, sauf si les deux classeurs ont le même thème. Les styles d'apparence identique sont ignorés. L'opération est une seule étape que vous pouvez annuler : Annuler retire chaque style ajouté et rétablit chaque aspect modifié. Les styles de l'autre classeur sont seulement lus, et ce classeur n'est pas modifié. Au maximum 1 000 styles sont importés à la fois (au-delà, l'opération est refusée et rien n'est fusionné) ; si l'autre classeur est occupé, un message vous invite à réessayer ; Normal reste inchangé, et un style que vous avez supprimé ici reste supprimé.

## Supprimer un style

Cliquez sur **Accueil > Styles de cellule > Supprimer le style de cellule**, puis choisissez le style. Toutes les cellules qui l'utilisent reviennent à Normal, la présentation simple, et le style disparaît du menu. L'opération est une seule étape que vous pouvez annuler ; l'annulation rétablit le style et l'aspect des cellules. Vous ne pouvez pas supprimer Normal. Le fichier du classeur conserve le nom du style, marqué comme masqué, afin que les autres programmes ne l'affichent pas non plus.

## Remarques

- L'application d'un style est une étape que vous pouvez annuler. Elle remplace la police, le remplissage, la bordure, le format de nombre, l'alignement et la protection de la cellule par ceux du style.
- Une cellule conserve son style. Si vous modifiez ensuite la mise en forme de la cellule, par exemple en la mettant en gras, seule cette cellule change, et elle continue de suivre le reste du style.
- JeffCrunch ne propose pas encore de commande permettant de modifier la définition d'un style ni d'importer des styles depuis un autre classeur. Un classeur qui possède ses propres styles les affiche sous **Personnalisé** et dans leur groupe.
