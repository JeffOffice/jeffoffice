# Orthographe, dictionnaire des synonymes, recherche et coupure de mots

<!-- keywords: vérifier l'orthographe, orthographe, correcteur, fautes d'orthographe, dictionnaire des synonymes, synonymes, recherche, dictionnaire, langue de vérification, coupure de mots, couper les mots, traduire, correction automatique, mot mal orthographié, trait ondulé rouge, langue, grammaire, vérifier la grammaire, style, pointillé bleu, ligne pointillée, vérifier la grammaire pendant la frappe -->

L'onglet **Révision** contient des outils qui vous aident à bien écrire vos mots.

## Vérifier l'orthographe pendant la frappe

JeffPub souligne d'un trait ondulé rouge tout mot qu'il ne connaît pas. Faites un clic droit sur le mot pour choisir une suggestion, cliquez sur **Ignorer tout** (pour cette session) ou sur **Ajouter au dictionnaire**.

Pour désactiver cette fonction, cliquez sur la flèche située à côté de **Révision > Vérification > Orthographe**, puis cliquez sur **Vérifier l'orthographe lors de la frappe**. Le même choix se trouve dans **Fichier > Options > Vérification**. Ce réglage est commun à toutes les applications JeffOffice et à toutes les fenêtres, et JeffPub s'en souvient au prochain démarrage.

## Vos propres mots

Cliquez sur **Fichier > Options > Vérification > Dictionnaire personnalisé** pour voir les mots que vous avez ajoutés et pour ajouter, supprimer, rechercher, importer ou exporter des mots. La même boîte de dialogue contient une liste de **Mots exclus** : ce sont des mots toujours signalés comme mal orthographiés, même si un dictionnaire les accepte, et qui ne sont jamais proposés comme suggestions. Un mot exclu désigne uniquement ce mot exact, et non ses autres formes. Ajouter un mot au dictionnaire le retire de la liste des mots exclus. L'importation et l'exportation utilisent un fichier texte brut, avec un mot par ligne (UTF-8). Vos listes sont identiques dans toutes les applications JeffOffice et ne sont jamais enregistrées dans un document.

## Vérifier la grammaire pendant la frappe

JeffPub recherche aussi les erreurs de grammaire et de style évidentes dans le texte, dans les zones de texte, les formes et les tableaux. Un problème reçoit un pointillé bleu, et un mot mal orthographié conserve son trait ondulé rouge. Faites un clic droit sur le problème pour voir ce qui ne va pas et choisir une suggestion, ou choisissez **Ignorer ce problème** ou **Ignorer tout**. Le texte dans une langue non vérifiée, et le texte marqué « ne pas vérifier », est ignoré. Pour désactiver cette fonction, cliquez sur **Fichier > Options > Vérification** et décochez **Vérifier la grammaire pendant la frappe**. Ce choix est commun à toutes les applications JeffOffice.

**Langues vérifiées.** La grammaire n'est vérifiée qu'en anglais, pour toutes les variantes régionales (américain, britannique, canadien, australien et les autres). Le texte dans toute autre langue n'est vérifié que pour l'orthographe et ne reçoit aucune marque de grammaire. Un paragraphe est vérifié selon la langue qui lui est attribuée. Un texte sans langue est vérifié en anglais, sauf si la plupart de ses mots ne sont pas anglais, comme un passage dans une autre langue que personne n'a désignée. Les espaces superflues et les espaces avant les signes de ponctuation ne sont recherchées que dans les phrases, et non dans les titres, les étiquettes, les listes de noms, les cellules de tableau qui ne sont pas des phrases complètes, ni les références bibliographiques. Dans un paragraphe comportant plusieurs langues, seul l'anglais est vérifié, et le reste est ignoré. Le texte qu'un fichier marque « ne pas vérifier » est également ignoré.

**Choisir ce qui est recherché.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les types de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme d'un verbe, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union superflus ou manquants), **Majuscules**, **Mots confondus** (mots qui se ressemblent à l'oral), **Répétition** et **Style**. Le style est désactivé tant que vous ne l'activez pas. Il formule des suggestions, et non des fautes : phrases de plus de 40 mots, voix passive, trois phrases de suite commençant par le même mot, et formulations lourdes (« in order to », « due to the fact that »). Le choix est conservé lorsque vous cliquez sur **OK** dans les options. Il s'applique immédiatement aux traits bleus et au volet Orthographe (F7) dans les fenêtres ouvertes de cette application, et aux autres applications JeffOffice à leur prochain démarrage.

## Vérifier toute la composition

1. Cliquez sur **Révision > Vérification > Orthographe**, ou appuyez sur F7.
2. La boîte de dialogue **Vérifier l'orthographe** affiche le premier mot problématique sous **Absent du dictionnaire**, avec les **Suggestions** en dessous.
3. Choisissez une suggestion, ou saisissez votre propre correction dans **Remplacer par**.
4. Cliquez sur **Remplacer** pour ce mot, ou sur **Tout remplacer** pour toutes les occurrences. Cliquez sur **Ignorer** pour passer ce mot, sur **Ignorer tout** pour le passer pendant toute la session, ou sur **Ajouter** pour l'enregistrer dans votre liste de mots personnelle.

JeffPub vérifie chaque zone de texte, forme et tableau, en commençant par la première page. Lorsque **Vérifier la grammaire pendant la frappe** est activé, la vérification passe aux problèmes de grammaire après l'orthographe et indique le problème à la place de **Absent du dictionnaire**. Elle indique quand elle est terminée. Elle ignore les mots en MAJUSCULES et les mots contenant des chiffres. Vous pouvez modifier ce comportement dans l'onglet **Vérification** de **Fichier > Options** (fenêtre **Options de JeffPub**).

## Choisir la langue

L'orthographe et la coupure automatique des mots suivent la langue du texte. Un texte sans langue définie est vérifié en anglais (États-Unis). JeffPub dispose de dictionnaires pour l'anglais (États-Unis, Royaume-Uni, Canada et Australie), l'espagnol (21 variantes régionales, de l'Argentine au Venezuela, dont le Mexique et l'Espagne), le français, l'allemand, l'italien, le néerlandais et le portugais (Brésil et Portugal).

1. Sélectionnez le texte.
2. Cliquez sur **Révision > Langue > Définir la langue de vérification…**
3. Choisissez une langue. Choisissez **Ne pas vérifier l'orthographe** pour un texte tel que des noms ou des codes.

## Trouver un meilleur mot

- **Dictionnaire des synonymes :** cliquez dans un mot, puis cliquez sur **Révision > Vérification > Dictionnaire des synonymes** (Maj+F7). Choisissez un synonyme, c'est-à-dire un mot de sens voisin. Cliquez sur **Insérer** pour remplacer votre mot. **Rechercher** lance une recherche sur le synonyme choisi.
- **Recherche :** cliquez sur **Révision > Vérification > Recherche** (Alt+F7) pour ouvrir le volet **Recherche**. Saisissez un mot dans **Rechercher** et cliquez sur **Rechercher**.

Les deux fonctions utilisent un dictionnaire des synonymes intégré à JeffPub, uniquement en anglais. Rien de ce que vous saisissez n'est envoyé sur Internet.

## Coupure de mots

La coupure de mots découpe les mots longs par un trait d'union en fin de ligne, pour que les lignes paraissent régulières. Les nouvelles zones de texte utilisent la coupure par défaut. Pour modifier ce réglage pour les nouvelles zones, voir **Coupure de mots automatique dans les nouvelles zones de texte** dans l'onglet **Options avancées** de **Fichier > Options** (fenêtre **Options de JeffPub**).

Pour modifier un seul article, cliquez dans sa zone de texte, puis cliquez sur **Révision > Vérifier > Coupure de mots…** (Ctrl+Maj+H). Vous pouvez aussi utiliser **Zone de texte > Texte > Coupure de mots**. Dans la boîte de dialogue **Coupure de mots** :

- **Coupure automatique des mots dans cet article** active ou désactive la coupure pour cet article.
- **Zone de coupure** définit l'espace vide qu'une ligne peut laisser en fin de ligne avant que JeffPub coupe un mot à la place.
- **Insérer un trait d'union conditionnel** ajoute un trait d'union à la position du curseur. Il n'apparaît que si le mot se coupe à cet endroit.
- **Ajouter des traits d'union conditionnels** en ajoute un à chaque endroit où un mot long peut être coupé.

## Traduire du texte

Sélectionnez du texte, puis cliquez sur **Révision > Langue > Traduire**. Le volet Traduction le traduit sur votre ordinateur et peut remplacer le texte ou ajouter la traduction sous celui-ci. Voir [Traduire du texte](translate).

## Voir aussi

- [Rechercher et remplacer du texte](find-replace)
- [L'onglet Révision](tab-review)
- [Ajouter et mettre en forme des zones de texte](text-boxes)
