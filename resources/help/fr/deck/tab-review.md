# Onglet Révision

<!-- keywords: onglet révision, comparer, volet Révision, accepter, refuser, ruban, vérification, orthographe, dictionnaire des synonymes, nombre de mots, commentaires, nouveau commentaire, supprimer commentaire, précédent, suivant, afficher les commentaires, langue, langue de vérification, définir la langue, ne pas vérifier l'orthographe, traduire, traduction, libretranslate, grammaire, vérification grammaticale, style, trait bleu, ligne pointillée, vérifier la grammaire pendant la frappe, comparaison, révisions, correction -->

L'onglet **Révision** vérifie vos mots, les traduit et gère les commentaires.

- **Vérification** contient **Orthographe** (F7), **Dictionnaire des synonymes** (Maj+F7), **Statistiques** et **Vérifier l'accessibilité**. Voir [Mettre en forme le texte](text).
- **Langue** contient **Traduire**, **Définir la langue de vérification** et **Ne pas vérifier l'orthographe**. Voir ci-dessous.
- **Comparer** contient **Comparer**, **Accepter**, **Refuser**, **Précédent**, **Suivant** et **Volet Révision**. Voir [Comparer des présentations](compare).
- **Commentaires** contient **Nouveau commentaire** (Ctrl+Alt+M), **Supprimer** avec son menu, **Précédent**, **Suivant** et **Afficher les commentaires**. Voir [Commentaires](comments).

## Vérifier la grammaire pendant la frappe

JeffDeck recherche aussi les erreurs de grammaire et de style évidentes dans le texte, sur les diapositives et dans les commentaires du présentateur. Un problème est souligné d'une ligne bleue pointillée, et un mot mal orthographié conserve son trait rouge ondulé. Faites un clic droit sur le problème pour voir ce qui ne va pas et choisir une suggestion, ou cliquez sur **Ignorer ce problème** ou **Tout ignorer**. **Orthographe** (F7) passe aux problèmes de grammaire des diapositives après l'orthographe. Le texte dans une langue non vérifiée et le texte marqué « ne pas vérifier » sont ignorés. Pour désactiver cette fonction, cliquez sur **Fichier > Options > Vérification**, puis décochez **Vérifier la grammaire pendant la frappe**. Ce choix est commun à toutes les applications JeffOffice.

**Langues vérifiées.** L'anglais, le français, l'allemand et l'espagnol, chacun avec ses propres règles. Le français contrôle l'espace insécable avant ; : ! ? et à l'intérieur des « », l'apostrophe dans des mots comme « l'ami » et « qu'il », ainsi que les confusions comme « a » et « à ». L'allemand contrôle « das » et « dass », « seid » et « seit », « wider » et « wieder », la virgule devant « dass » et « weil », ainsi que les majuscules des noms communs. L'espagnol contrôle l'absence de ¿ ou de ¡, les accents des mots interrogatifs, « a ver » et « haber », ainsi que « del » et « al ». Un paragraphe est vérifié dans la langue qui lui est attribuée, et un texte sans langue est vérifié en anglais. Dans un paragraphe qui comporte plusieurs langues, la première langue vérifiée est utilisée et le reste est ignoré.

**Choisir ce qui est vérifié.** Cliquez sur **Fichier > Options > Vérification > Paramètres de grammaire** pour choisir les types de problèmes : **Grammaire** (verbes qui ne s'accordent pas, mauvaise forme verbale, « a » et « an »), **Ponctuation** (espaces, virgules et traits d'union superflus ou manquants), **Majuscules**, **Mots confondus** (mots qui se prononcent de la même façon), **Répétition** et **Style**. **Style** est désactivé tant que vous ne l'activez pas. Il fait des suggestions et ne signale pas de fautes : phrases de plus de 40 mots, voix passive, trois phrases consécutives commençant par le même mot, et formulations lourdes (« afin de », « en raison du fait que »). Votre choix est conservé lorsque vous cliquez sur **OK** dans les options. Il s'applique immédiatement aux traits bleus et au volet Orthographe (F7) dans les fenêtres ouvertes de cette application, et aux autres applications JeffOffice à leur prochain démarrage.

## Définir la langue d'un texte

L'orthographe vérifie chaque texte dans sa propre langue. Pour modifier la langue de certains mots, sélectionnez-les, ou sélectionnez une zone de texte ou un espace réservé complet, puis cliquez sur **Définir la langue de vérification** dans le groupe **Langue**. Choisissez la langue dans la liste, puis cliquez sur **OK**. Les mots de cette langue sont vérifiés avec le dictionnaire correspondant, et le fichier conserve ce choix lorsque vous l'enregistrez au format de présentation ou de présentation OpenDocument. Le texte que vous n'avez pas marqué est vérifié en anglais américain.

## Ne pas vérifier certains textes

Certains textes ne doivent pas être vérifiés : un code, un nom ou une ligne dans une langue inventée. Sélectionnez-le, puis cliquez sur **Ne pas vérifier l'orthographe** dans le groupe **Langue**. Les traits rouges ondulés disparaissent. Cliquez à nouveau dessus pour vérifier de nouveau le texte. La zone de notes ne contient que du texte brut : elle n'a donc pas de langue propre.

## Traduire un texte

1. Sélectionnez les mots dans une zone de texte ou dans les notes. Pour traduire tout le contenu d'une zone de texte, sélectionnez la zone.
2. Cliquez sur **Traduire** dans le groupe **Langue**.
3. Choisissez la langue de destination.
4. Cliquez sur **OK**.

Votre navigateur web ouvre alors un service de traduction nommé LibreTranslate, avec votre texte. Rien n'est envoyé tant que vous ne cliquez pas sur **OK**. Vous pouvez modifier l'adresse du service dans la zone prévue, par exemple pour utiliser un service que vous hébergez vous-même. Seules les adresses web sécurisées (https) sont utilisées, ainsi que les adresses http de cet ordinateur. 5 000 lettres au maximum sont envoyées.

## Vérifier l'accessibilité

**Vérifier l'accessibilité** ouvre une petite fenêtre qui liste ce qui peut être difficile à utiliser pour certaines personnes : une image ou une forme sans texte de remplacement, une diapositive sans titre, un tableau sans ligne d'en-tête, une vidéo sans sous-titres, un texte difficile à lire sur son arrière-plan, et des objets que le lecteur d'écran lit dans un ordre différent de leur position. Le texte doit présenter un contraste de 4,5 pour 1 avec son arrière-plan (3 pour 1 pour un texte de 18 points ou plus, ou de 14 points ou plus en gras). Lorsque le texte se trouve sur une image, une astuce vous invite à vérifier le contraste à l'œil. Un lecteur d'écran lit les objets d'une diapositive de l'arrière vers l'avant. Cliquez sur **Ordre de lecture** pour les afficher dans cet ordre, et utilisez **Lire plus tôt** et **Lire plus tard** (ou faites glisser) pour le modifier ; chaque déplacement correspond à une étape d'annulation. Un lecteur d'écran utilise la première ligne d'un tableau pour nommer ses colonnes : sélectionnez un problème de tableau, puis cliquez sur **Utiliser la première ligne comme en-tête** pour activer la ligne d'en-tête (une étape d'annulation). Une vidéo sans sous-titres est signalée sous forme d'astuce : les vidéos doivent avoir des sous-titres pour les personnes qui n'entendent pas. Il n'existe pas de correction en un clic ; sélectionnez la vidéo, puis utilisez **Insérer des légendes** dans l'onglet **Lecture** : l'astuce disparaît. Cliquez sur un problème pour accéder à sa diapositive et sélectionner l'objet. Donnez à une image un texte de remplacement dans le volet **Format de l'image**, ou marquez-la comme décorative, puis cliquez sur **Vérifier à nouveau**. Vous pouvez aussi lancer la vérification depuis **Fichier > Infos**.

## Voir aussi

- [Comparer des présentations](compare)
- [Commentaires](comments)
- [Mettre en forme le texte](text)
