# Protéger un document et verrouiller le suivi
<!-- keywords: protéger, protection, restreindre la modification, lecture seule, mot de passe, verrouiller le suivi, options de suivi des modifications, nom d'utilisateur, initiales, modifications suivies uniquement, commentaires uniquement, arrêter la protection, verrouiller, sécuriser, limiter la modification, lecture seule -->

La protection limite ce que les utilisateurs peuvent faire dans un document. Ces outils se trouvent sous l'onglet **Révision**.

## Restreindre la modification

1. Cliquez sur **Restreindre la modification** dans le groupe **Protéger**.
2. Choisissez ce que les utilisateurs peuvent faire :
   - **Aucune modification (lecture seule)** : le texte ne peut pas être modifié.
   - **Modifications suivies** : les utilisateurs peuvent modifier le document, mais chaque modification est suivie, et le suivi des modifications ne peut pas être désactivé.
   - **Commentaires** : les utilisateurs peuvent ajouter des commentaires, mais ne peuvent pas modifier le texte.
   - **Remplissage de formulaires** : le texte est en lecture seule, mais les utilisateurs peuvent remplir les champs de formulaire qu'il contient.
3. Saisissez un mot de passe, deux fois. Vous pouvez laisser le mot de passe vide. Dans ce cas, tout le monde peut arrêter la protection.
4. Cliquez sur **Appliquer la protection**.

La barre de titre affiche alors « Protégé ». Pour arrêter la protection, cliquez de nouveau sur **Restreindre la modification**, puis sur **Arrêter la protection**. Saisissez le mot de passe, s'il y en a un.

Un mot de passe évite que des personnes de bonne foi modifient le document par erreur. Ce n'est pas une sécurité forte. N'utilisez pas la protection pour masquer des informations confidentielles.

Le mot de passe est conservé dans le fichier sous la forme d'un nombre brouillé, appelé empreinte. Le mot de passe lui-même n'est jamais enregistré. Si vous l'oubliez, JeffWrite ne peut pas le récupérer.

## Remplir un formulaire

Un document peut contenir des champs de formulaire : des zones à saisir, des cases à cocher et des listes où choisir. JeffWrite permet de les remplir, mais n'en ajoute pas de nouveaux.

- **Saisir dans un champ.** Cliquez dans le champ, puis saisissez du texte. Un champ peut n'accepter qu'un certain nombre de lettres, convertir ce que vous tapez en majuscules, ou n'accepter qu'un nombre ou une date. JeffWrite applique ces règles lors de la saisie, du collage ou de la saisie avec une méthode d'entrée. Un texte collé trop long pour le champ est tronqué, et ce sont ses premiers caractères qui sont conservés. Un saut de ligne dans le texte collé devient une espace. Un saut de ligne déjà présent dans le champ reste en place, sauf si vous le sélectionnez et écrivez par-dessus ; la saisie n'en ajoute jamais. Un nombre ou une date prend son format défini lorsque vous quittez le champ : appuyez sur Tab, appuyez sur Entrée, cliquez ailleurs, sortez avec les touches fléchées, enregistrez ou imprimez.
- **Cocher une case.** Cliquez dessus, ou placez-vous dessus et appuyez sur Espace.
- **Choisir dans une liste.** Cliquez dessus, ou placez-vous dessus et appuyez sur Entrée. Choisissez ensuite une entrée. Appuyer sur une lettre sélectionne l'entrée suivante qui commence par cette lettre.
- **Passer d'un champ à l'autre.** Lorsque le document est protégé pour le remplissage de formulaires, appuyez sur Tab ou sur Entrée pour aller au champ suivant, et sur Maj+Tab pour revenir en arrière. Dans tout document, Entrée dans un champ ne crée pas de nouveau paragraphe. Seuls les champs peuvent être modifiés. Une partie du document que l'auteur du formulaire a exclue de la protection peut aussi être modifiée ; dans cette partie, Tab est la touche habituelle, et Remplacer tout ne modifie que les occurrences trouvées dans cette partie. JeffWrite indique alors le nombre d'occurrences non traitées et précise que le texte des en-têtes, des pieds de page, des notes et des zones de texte est laissé tel quel. Ce que vous saisissez, collez, déposez ou corrigez est contrôlé selon l'endroit où il se trouverait, et non selon l'emplacement du curseur. Les images, les formes et les tableaux peuvent être ajoutés et modifiés dans une partie exclue de la protection, comme dans tout document, et ne sont modifiés nulle part ailleurs.
- **Annuler une saisie.** Appuyez sur Annuler. Les lettres saisies dans un même champ forment une seule étape. Annuler revient en arrière sur les étapes effectuées depuis le début de la protection, c'est-à-dire vos saisies et vos modifications dans une partie exclue de la protection, et rien de ce qui précède.

Un champ désactivé dans le formulaire, ou un champ que le formulaire calcule lui-même, ne peut pas être rempli. Dans tout document, les marques de début, de fin et les autres marques d'un champ ne sont jamais supprimées une à une : sélectionnez le champ entier pour le supprimer. Un champ situé dans un document en lecture seule, ou ouvert pour les commentaires uniquement, ne peut pas non plus être rempli. Si le suivi des modifications est activé dans un document non protégé, ce que vous saisissez dans un champ de texte est une modification suivie, et la longueur maximale et les majuscules du texte ne sont pas imposées.

## Verrouiller le suivi

Cliquez sur **Options de suivi**, puis sur **Verrouiller le suivi**. Chaque modification est alors suivie à partir de ce moment. Personne ne peut désactiver le suivi des modifications, ni accepter ou refuser une modification, sans le mot de passe. Cela revient à protéger le document pour les modifications suivies.

## Changer votre nom dans les modifications

Cliquez sur **Options de suivi**, puis sur **Options de suivi des modifications**. Saisissez votre nom et vos initiales. Ils signent les modifications et les commentaires que vous faites à partir de ce moment.
