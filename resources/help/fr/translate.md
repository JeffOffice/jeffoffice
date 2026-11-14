# Traduire du texte
<!-- keywords: traduire, traduction, traducteur, langue, langues, espagnol, français, allemand, italien, portugais, néerlandais, russe, ukrainien, indonésien, hors ligne, sur cet ordinateur, télécharger, modèles, remplacer, insérer en dessous -->

**Révision > Langue > Traduire** ouvre le volet **Traduire**. Il traduit votre texte sur votre propre ordinateur. Vos mots ne sont envoyés nulle part, et aucune connexion Internet n'est nécessaire une fois que les fichiers de traduction d'une langue sont sur votre ordinateur.

## Traduire un texte

1. Sélectionnez le texte à traduire. Si rien n'est sélectionné, le volet utilise le paragraphe où se trouve le curseur. Dans une zone de texte qui est sélectionnée sans être en cours de modification, il utilise la totalité du texte de la zone.
2. Cliquez sur **Révision > Langue > Traduire**. Le volet affiche votre texte sous **Original**.
3. Sous **De**, choisissez la langue de votre texte. Sous **À**, choisissez la langue souhaitée. Le bouton **Permuter** inverse les deux.
4. Cliquez sur **Traduire**. La traduction apparaît sous **Traduction**. Les mots en gras, en italique et soulignés restent en gras, en italique et soulignés.
5. Cliquez sur **Remplacer** pour placer la traduction à l'emplacement de votre texte, ou sur **Insérer en dessous** pour l'ajouter en nouveaux paragraphes sous votre texte. Chacune de ces actions se fait en une seule étape : **Annuler** la retire.

Si vous déplacez le curseur ou modifiez le texte après avoir cliqué sur **Traduire**, cliquez sur **Utiliser la sélection** pour relire le texte, puis cliquez de nouveau sur **Traduire**. Le volet ne remplacera pas une traduction sur un texte qui n'est plus celui qu'il a traduit.

**Annuler** interrompt une traduction qui dure trop longtemps, et la fermeture du volet interrompt également la traduction : la fenêtre n'attend jamais qu'elle se termine. Les textes longs prennent plus de temps : une page de texte demande quelques secondes sur la plupart des ordinateurs. Le volet traduit au maximum 1 000 000 de lettres à la fois (environ 200 pages) ; si vous en sélectionnez davantage, il le signale et vous demande de sélectionner moins de texte. Il découpe les longs paragraphes en morceaux d'environ 1 500 lettres, à la fin d'une phrase lorsque c'est possible, puis les réassemble.

## La première utilisation d'une langue

Une traduction nécessite un ensemble de fichiers, appelé modèle, pour chaque sens de traduction : un pour l'anglais vers l'allemand, un autre pour l'allemand vers l'anglais. Chaque modèle fait environ 37 Mo. Ces fichiers ne font pas partie de JeffPub. La première fois que vous traduisez dans une langue, le volet indique ce dont il a besoin et sa taille, par exemple « Télécharger l'allemand (74 Mo) ? ». Rien n'est téléchargé tant que vous n'avez pas cliqué sur ce bouton. Le volet indique l'avancement du téléchargement, et **Annuler** l'interrompt sans rien laisser sur l'ordinateur.

JeffPub vérifie chaque fichier à l'aide d'une empreinte (une somme de contrôle SHA-256) avant de l'utiliser. Un fichier qui ne correspond pas est supprimé, et la langue n'est pas installée.

Les fichiers sont conservés dans un seul dossier que partagent toutes les applications JeffOffice (un dossier nommé JeffOffice dans le dossier de données de votre utilisateur, dans Translation/models). Ainsi, une langue téléchargée dans une application est disponible dans les autres, et elle n'est téléchargée qu'une seule fois. Deux langues qui n'ont pas de modèle propre, comme l'allemand et le français, passent par l'anglais ; le volet a donc besoin des deux modèles.

## Lorsqu'une langue n'a pas de modèle

Si une paire de langues n'a pas de modèle, le volet l'indique et propose **Ouvrir plutôt dans votre navigateur**. Cela ouvre un site de traduction gratuit, LibreTranslate (ou le service dont vous avez choisi l'adresse dans le paramètre prévu à cet effet), avec votre texte. Ce n'est qu'à ce moment que votre texte quitte votre ordinateur, puisqu'il est envoyé à ce site, et le volet n'indique plus qu'il reste sur votre ordinateur. Rien n'est envoyé tant que vous n'avez pas cliqué sur ce bouton.

## À propos de la qualité

Les modèles sont petits, ils fonctionnent donc rapidement sur un ordinateur ordinaire. Ils traduisent bien les phrases, mais moins bien les étiquettes courtes : un mot isolé ou le nom d'un bouton peut être mal traduit. Relisez la traduction avant de l'utiliser. Les noms, les nombres et les tabulations sont conservés tels quels.

Les images, les tabulations et les sauts de ligne à l'intérieur d'un paragraphe sont laissés intacts. La traduction remplit les mots situés entre eux.

## Voir aussi

- [Orthographe, dictionnaire des synonymes, recherche et coupure de mots](proofing)
