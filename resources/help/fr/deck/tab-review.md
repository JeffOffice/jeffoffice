# Onglet Révision
<!-- keywords: onglet révision, comparer, volet révision, accepter, refuser, ruban, vérification, orthographe, dictionnaire des synonymes, synonymes, nombre de mots, statistiques, commentaires, nouveau commentaire, supprimer commentaire, précédent, suivant, afficher les commentaires, langue, langue de vérification, définir la langue, ne pas vérifier l'orthographe, traduire, traduction, libretranslate, grammaire, vérification grammaticale, style, trait bleu, trait pointillé, vérifier la grammaire pendant la frappe, correction grammaticale -->

L'onglet **Révision** vérifie vos mots, les traduit et gère les commentaires.

- **Vérification** contient **Orthographe** (F7), **Dictionnaire des synonymes** (Maj+F7), **Statistiques** et **Vérifier l'accessibilité**. Voir [Mettre en forme le texte](text).
- **Langue** contient **Traduire**, **Définir la langue de vérification** et **Ne pas vérifier l'orthographe**. Voir ci-dessous.
- **Comparer** contient **Comparer**, **Accepter**, **Refuser**, **Précédent**, **Suivant** et **Volet Révision**. Voir [Comparer des présentations](compare).
- **Commentaires** contient **Nouveau commentaire** (Ctrl+Alt+M), **Supprimer** avec son menu, **Précédent**, **Suivant** et **Afficher les commentaires**. Voir [Commentaires](comments).

## Vérifier la grammaire pendant la frappe

JeffDeck repère aussi les erreurs de grammaire et de style évidentes dans le texte, sur les diapositives et dans les commentaires du présentateur. Un problème est souligné d'un trait bleu pointillé, et un mot mal orthographié conserve son trait rouge ondulé. Faites un clic droit sur le problème pour voir ce qui ne va pas et choisir une suggestion, ou choisissez **Ignorer ce problème** ou **Tout ignorer**. **Orthographe** (F7) passe aux problèmes de grammaire des diapositives après l'orthographe. Le texte dans une langue non vérifiée, et le texte marqué « ne pas vérifier », est ignoré. Pour désactiver cette fonction, cliquez sur **Fichier > Options > Vérification**, puis décochez **Vérifier la grammaire au fur et à mesure de la saisie**. Ce choix est commun à toutes les applications JeffOffice.

**Langues vérifiées.** La grammaire n'est vérifiée qu'en anglais, dans toutes ses variantes régionales (américain, britannique, canadien, australien et autres). Le texte dans une autre langue n'est vérifié que pour l'orthographe, sans marque de grammaire. Un paragraphe est vérifié dans la langue qui lui est attribuée, et le texte sans langue est vérifié en anglais. Dans un paragraphe qui comporte plusieurs langues, seul l'anglais est vérifié et le reste est ignoré. Le texte que le fichier marque « ne pas vérifier » est également ignoré.

**Choisir ce qui est recherché.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les types de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme de verbe, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union en trop ou manquants), **Majuscules**, **Mots confondus** (mots qui se prononcent de la même façon), **Répétitions** et **Style**. **Style** est désactivé tant que vous ne l'activez pas. Il formule des suggestions et ne signale pas des fautes : phrases de plus de 40 mots, voix passive, trois phrases consécutives commençant par le même mot, et formulations lourdes (« in order to », « due to the fact that »). Le choix est conservé lorsque vous cliquez sur **OK** dans les Options. Il s'applique immédiatement aux traits bleus et au volet Orthographe (F7) des fenêtres ouvertes de cette application, et aux autres applications JeffOffice à leur prochain démarrage.

## Définir la langue d'un texte

La vérification orthographique contrôle chaque texte dans sa propre langue. Pour changer la langue de certains mots, sélectionnez-les, ou sélectionnez toute une zone de texte ou un espace réservé, puis cliquez sur **Définir la langue de vérification** dans le groupe **Langue**. Choisissez la langue dans la liste, puis cliquez sur **OK**. Les mots dans cette langue sont vérifiés avec le dictionnaire correspondant, et le fichier conserve ce choix lorsque vous l'enregistrez au format de présentation ou au format de présentation OpenDocument. Le texte que vous n'avez pas marqué est vérifié en anglais américain.

## Ne pas vérifier certains textes

Certains textes ne doivent pas être vérifiés : un code, un nom ou une ligne dans une langue inventée. Sélectionnez-le, puis cliquez sur **Ne pas vérifier l'orthographe** dans le groupe **Langue**. Les traits rouges ondulés disparaissent. Cliquez à nouveau dessus pour vérifier de nouveau le texte. La zone de commentaires ne contient que du texte brut : elle n'a donc pas de langue propre.

## Traduire un texte

1. Sélectionnez les mots dans une zone de texte ou dans les commentaires. Pour traduire tout le contenu d'une zone de texte, sélectionnez la zone.
2. Cliquez sur **Traduire** dans le groupe **Langue**.
3. Choisissez la langue de destination.
4. Cliquez sur **OK**.

Votre navigateur web ouvre alors un service de traduction, LibreTranslate, avec votre texte. Rien n'est envoyé tant que vous n'avez pas cliqué sur **OK**. Vous pouvez modifier l'adresse du service dans la zone prévue, par exemple pour utiliser un service que vous hébergez vous-même. Seules les adresses web sécurisées (https) sont utilisées, ainsi que les adresses http de cet ordinateur. Au maximum 5 000 lettres sont envoyées.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** ouvre une petite fenêtre qui liste ce qui peut être difficile à utiliser pour certaines personnes : une image ou une forme sans texte de remplacement, une diapositive sans titre, un tableau sans ligne d'en-tête, un tableau avec des cellules fusionnées, une vidéo sans légendes, une diapositive dont le titre est aussi celui d'une autre diapositive (conseil), un lien dont le texte est sa propre adresse ou ne dit rien de sa destination (comme « ici » ou « en savoir plus »), un texte difficile à lire sur son arrière-plan, et des objets qu'un lecteur d'écran lit dans un ordre différent de leur position. Le texte doit présenter un contraste de 4,5 pour 1 avec son arrière-plan (3 pour 1 pour un texte d'au moins 18 points, ou d'au moins 14 points en gras) ; lorsque le texte se trouve sur une image, un conseil vous invite à vérifier le contraste à l'œil. Un lecteur d'écran lit les objets d'une diapositive de l'arrière vers l'avant de la pile. Cliquez sur **Ordre de lecture** pour les afficher dans cet ordre, puis utilisez **Lire plus tôt** et **Lire plus tard** (ou le glisser-déplacer) pour le modifier ; chaque déplacement correspond à une étape d'annulation. Un lecteur d'écran utilise la première ligne d'un tableau pour nommer ses colonnes : sélectionnez un problème de tableau et cliquez sur **Utiliser la première ligne comme en-tête** pour activer la ligne d'en-tête (une étape d'annulation). Une vidéo sans légendes est signalée par un conseil : les vidéos doivent avoir des légendes pour les personnes qui n'entendent pas. Il n'existe pas de correction en un clic ; sélectionnez la vidéo et utilisez **Insérer des légendes** dans l'onglet **Lecture**, et le conseil disparaît. Cliquez sur un problème pour accéder à sa diapositive et sélectionner l'objet. Ajoutez un texte de remplacement à une image dans le volet **Format de l'image**, ou marquez-la comme décorative, puis cliquez sur **Vérifier à nouveau**. Vous pouvez aussi lancer la vérification depuis **Fichier > Informations**.

## Voir aussi

- [Comparer des présentations](compare)
- [Commentaires](comments)
- [Mettre en forme le texte](text)
