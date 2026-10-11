# Orthographe, dictionnaire des synonymes, recherche et coupure de mots
<!-- keywords: vérification orthographique, orthographe, correcteur orthographique, dictionnaire des synonymes, synonymes, recherche, dictionnaire, langue de vérification, coupure de mots, césure, couper les mots, traduire, traduction, correction automatique, faute d'orthographe, mot mal orthographié, soulignement rouge, langue -->

L'onglet **Révision** propose des outils qui vous aident à bien choisir et écrire vos mots.

## Vérifier l'orthographe lors de la frappe

JeffPub souligne d'un trait ondulé rouge les mots qu'il ne connaît pas. Cliquez avec le bouton droit sur le mot pour choisir une suggestion, choisissez **Ignorer tout** (pour cette session) ou choisissez **Ajouter au dictionnaire**.

Pour désactiver cette fonction, cliquez sur la flèche à côté de **Révision > Vérification > Orthographe**, puis cliquez sur **Vérifier l'orthographe lors de la frappe**.

## Vérifier toute la composition

1. Cliquez sur **Révision > Vérification > Orthographe** ou appuyez sur F7.
2. La boîte de dialogue **Vérifier l'orthographe** affiche le premier mot posant problème sous **Absent du dictionnaire**, avec des **Suggestions** en dessous.
3. Choisissez une suggestion ou tapez votre propre correction dans **Remplacer par**.
4. Cliquez sur **Remplacer** pour ce mot ou sur **Remplacer tout** pour toutes les occurrences. Cliquez sur **Ignorer** pour passer le mot, sur **Ignorer tout** pour l'ignorer pendant cette session, ou sur **Ajouter** pour l'enregistrer dans votre propre liste de mots.

JeffPub vérifie chaque zone de texte, forme et tableau, en commençant par la première page. Il vous indique quand la vérification est terminée. Il ignore les mots en MAJUSCULES et les mots qui contiennent des chiffres. Vous pouvez modifier ce comportement dans l'onglet **Vérification** de **Fichier > Options** (la fenêtre **Options de JeffPub**).

## Définir la langue

L'orthographe et la coupure automatique des mots suivent la langue du texte. Un texte sans langue définie est vérifié comme de l'Anglais (États-Unis). JeffPub dispose de dictionnaires pour l'anglais (États-Unis, Royaume-Uni, Canada et Australie), l'espagnol (Espagne et Mexique), le français, l'allemand, l'italien, le néerlandais et le portugais (Brésil et Portugal).

1. Sélectionnez le texte.
2. Cliquez sur **Révision > Langue > Définir la langue de vérification…**
3. Choisissez une langue. Choisissez **Ne pas vérifier l'orthographe** pour un texte tel que des noms ou des codes.

## Trouver un meilleur mot

- **Dictionnaire des synonymes :** cliquez dans un mot, puis cliquez sur **Révision > Vérification > Dictionnaire des synonymes** (Maj+F7). Choisissez un synonyme, c'est-à-dire un mot de sens proche. Cliquez sur **Insérer** pour remplacer votre mot. **Rechercher** lance une recherche sur le synonyme choisi.
- **Recherche :** cliquez sur **Révision > Vérification > Recherche** (Alt+F7) pour ouvrir le volet **Recherche**. Tapez un mot dans **Rechercher** et cliquez sur **Rechercher**.

Les deux outils utilisent un dictionnaire des synonymes intégré à JeffPub, en anglais uniquement. Rien de ce que vous tapez n'est envoyé en ligne.

## Coupure de mots

La coupure de mots coupe les mots longs par un trait d'union en fin de ligne, pour que les lignes paraissent plus régulières. Les nouvelles zones de texte coupent les mots par défaut. Pour modifier ce comportement pour les nouvelles zones, utilisez **Coupure de mots automatique dans les nouvelles zones de texte** dans l'onglet **Options avancées** de **Fichier > Options** (la fenêtre **Options de JeffPub**).

Pour modifier un seul article, cliquez dans sa zone de texte, puis cliquez sur **Révision > Vérifier > Coupure de mots…** (Ctrl+Maj+H). Vous pouvez aussi utiliser **Zone de texte > Texte > Coupure de mots**. Dans la boîte de dialogue **Coupure de mots** :

- **Coupure automatique des mots dans cet article** active ou désactive la coupure.
- **Zone de coupure** définit l'espace vide qu'une ligne peut laisser à sa fin avant que JeffPub ne coupe plutôt un mot.
- **Insérer un trait d'union conditionnel** ajoute un trait d'union à l'emplacement du curseur. Il n'apparaît que si le mot est coupé à cet endroit.
- **Ajouter des traits d'union conditionnels** en ajoute un à chaque endroit où un mot long peut être coupé.

## Traduire du texte

Sélectionnez du texte et cliquez sur **Révision > Langue > Traduire**. Choisissez une langue et cliquez sur **OK**. JeffPub ouvre alors votre navigateur web sur LibreTranslate, un site de traduction gratuit et open source, avec votre texte. Le texte étant envoyé à ce site, n'utilisez pas cette fonction pour des contenus privés.

## Voir aussi

- [Rechercher et remplacer du texte](find-replace)
- [L'onglet Révision](tab-review)
- [Ajouter et mettre en forme des zones de texte](text-boxes)
