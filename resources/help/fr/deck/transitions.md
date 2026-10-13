# Transitions entre les diapositives
<!-- keywords: transition, transitions, morphose, fondu, pousser, effacer, balayer, fractionner, révéler, couvrir, découvrir, zoom, couper, options d'effet, objets, mots, caractères, correspondance, paire, nom, direction, durée, défilement des diapositives, manuellement, après, automatique, minutage, appliquer partout, aperçu, étoile, enchaînement -->

Une **transition** est l'effet qui s'exécute lorsque le diaporama passe d'une diapositive à la suivante. Une transition appartient à la diapositive vers laquelle elle mène. Le diaporama l'exécute lorsqu'il avance vers cette diapositive.

## Ajouter une transition

1. Cliquez sur la diapositive dans le volet des diapositives.
2. Cliquez sur l'onglet **Transitions**. Dans la galerie **Transition vers cette diapositive**, cliquez sur un effet : **Aucune**, **Morphose**, **Fondu**, **Pousser**, **Effacer**, **Fractionner**, **Révéler**, **Couvrir**, **Découvrir**, **Zoom** ou **Couper**. La miniature de la diapositive dans le volet des diapositives reçoit une petite étoile indiquant qu'elle en possède une.
3. Cliquez sur **Transitions > Aperçu > Aperçu** pour la voir s'exécuter vers la diapositive, dans la zone de la diapositive. Appuyez sur Échap pour l'arrêter.

Choisissez **Aucune** pour supprimer une transition.

## Choisir son déroulement

Cliquez sur **Options d'effet** pour afficher les choix de l'effet :

- **Pousser**, **Effacer**, **Révéler**, **Couvrir** et **Découvrir** ont une direction : **À partir de la droite**, **À partir de la gauche**, **À partir du bas** ou **À partir du haut**.
- **Fractionner** et **Zoom** vont vers l'extérieur (**Sortant**) ou vers l'intérieur (**Entrant**).
- **Morphose** déplace des **Objets**, des **Mots** ou des **Caractères**. Voir ci-dessous.
- **Fondu** et **Couper** n'ont pas de choix. L'effet Couper passe par le noir.

## Morphose

**Morphose** déplace ce que deux diapositives ont en commun. Un **objet** est un élément d'une diapositive : une forme, une image, une zone de texte, un trait ou un tableau. Le diaporama passe d'une diapositive à la suivante. Chaque objet présent sur les deux diapositives glisse de sa position sur la première diapositive vers sa position sur la seconde. En chemin, il s'agrandit ou rétrécit, pivote et change de couleur. Un objet présent uniquement sur la première diapositive disparaît en fondu. Un objet présent uniquement sur la seconde apparaît en fondu.

Pour créer une morphose :

1. Cliquez avec le bouton droit sur une diapositive dans le volet des diapositives et choisissez **Dupliquer la diapositive**. La copie contient les mêmes objets.
2. Sur la copie, déplacez un objet, agrandissez-le, faites-le pivoter ou changez sa couleur. Vous pouvez aussi modifier son texte, ou ajouter et supprimer des objets.
3. Cliquez sur la copie, puis sur **Transitions > Transition vers cette diapositive > Morphose**. Une morphose dure 2 secondes. Vous pouvez modifier cette valeur avec **Durée**.
4. Cliquez sur **Aperçu** pour la regarder, ou lancez le diaporama.

### Quels objets vont ensemble

JeffDeck associe un objet de la première diapositive à un objet de la seconde. Il essaie les méthodes suivantes, dans cet ordre. Chaque objet fait partie d'une paire au plus.

1. **Un nom qui commence par !!.** Deux objets portant le même nom, ce nom commençant par deux points d'exclamation, forment toujours une paire. Il peut s'agir de deux objets quelconques, même un cercle et une étoile.
2. **Le même nom.** Deux objets du même type portant le même nom forment une paire. Une zone de texte et une forme sont de types différents.
3. **Le même texte.** Deux zones de texte contenant les mêmes mots forment une paire, quels que soient leurs noms. Il en va de même pour deux formes contenant les mêmes mots.
4. **La même image.** Deux images de la même illustration forment une paire, même si elles ont un autre nom ou une autre taille.
5. **Le même type de forme.** Deux cercles forment une paire. De même pour deux traits ou deux tableaux.

Lorsque plusieurs objets peuvent former une paire, les plus proches les uns des autres sont associés en premier.

Les noms des objets proviennent du fichier de présentation. Une présentation créée dans un autre programme conserve les noms qu'elle a donnés à ses objets, et JeffDeck les utilise.

### Comment se déplace une paire

- Deux objets de même forme deviennent un seul objet qui se déplace. Sa position, sa taille, sa rotation, sa couleur de remplissage, sa couleur de trait et sa transparence changent progressivement. Le rognage d'une image change aussi.
- Deux objets de formes différentes, comme un cercle et un carré, se déplacent vers les mêmes positions tandis que l'un se fond dans l'autre.
- Si le texte d'une paire n'est pas identique, l'ancien texte se fond dans le nouveau.
- Les objets masqués par une animation sont ignorés. Un objet déplacé par une animation est pris à l'endroit où il se trouve à la fin de celle-ci.

### Ce qui se déplace : Objets, Mots ou Caractères

Cliquez sur **Options d'effet** pour choisir :

- **Objets.** Le texte d'une paire reste dans son objet. L'ancien texte se fond dans le nouveau.
- **Mots.** Un mot présent dans les deux textes se déplace de son ancienne position vers sa nouvelle position. Les autres mots disparaissent et apparaissent en fondu. Un signe comme une virgule compte comme un mot à part entière.
- **Caractères.** Même principe, une lettre ou un chiffre à la fois.

**Mots** et **Caractères** fonctionnent sur les zones de texte et les formes. Un texte de plus de 16 000 caractères, un texte orienté verticalement et un texte dans une forme retournée sont traités comme **Objets**. C'est aussi le cas d'une paire qui n'a aucun mot ni caractère en commun.

La morphose est également une transition dans d'autres programmes de présentation (versions 2019 et ultérieures). JeffDeck la lit et l'enregistre de la même manière qu'eux. Les programmes plus anciens exécutent une morphose sous forme de fondu.

## Définir le minutage

Le groupe **Minutage** définit la durée de l'effet et la manière dont le diaporama quitte la diapositive.

- **Durée** correspond au nombre de secondes que dure l'effet. Une nouvelle transition dure 0,7 seconde. Vous pouvez saisir une valeur de 0,01 à 59.
- **Manuellement** est activé par défaut. Le diaporama quitte cette diapositive lorsque le présentateur clique. Désactivez cette option pour passer à la suite avec une touche comme Espace ou Flèche droite. Les animations de la diapositive s'exécutent toujours au clic.
- **Après** fait avancer le diaporama automatiquement au bout du nombre de secondes indiqué. Vous pouvez utiliser **Après** en même temps que **Manuellement** : le diaporama avance dès que l'une des deux conditions est remplie.

**Appliquer partout** donne à toutes les diapositives la transition et le minutage de la diapositive active.

Chacune de ces actions constitue une étape que Ctrl+Z permet d'annuler.

## Transitions provenant d'autres fichiers

Une présentation créée dans un autre programme peut contenir une transition pour laquelle JeffDeck ne dispose d'aucun effet. Elle s'exécute sous forme de fondu et est enregistrée telle quelle, de sorte qu'elle fonctionne toujours dans le programme d'origine.

## Voir aussi

- [Onglet Transitions](tab-transitions)
- [Animations](animations)
- [Lancer le diaporama et utiliser le mode Présentateur](show)
