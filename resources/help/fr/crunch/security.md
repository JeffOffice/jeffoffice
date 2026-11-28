# Sécurité : ce qu'un fichier peut atteindre

<!-- keywords: centre de gestion de la confidentialité, sécurité, macros, fichiers liés, liens, images liées, médias liés, internet, téléchargé, lecture seule, protégé, liens externes, programmes, sûr, approbations, mots de passe -->

Un fichier peut renvoyer vers d'autres éléments : fichiers, pages web, vidéos, voire programmes. La page **Sécurité** vous permet de décider ce que fait JeffCrunch dans ce cas. Cliquez sur **Fichier > Options**, puis sur l'onglet **Sécurité**. Cliquez sur **OK** pour conserver vos modifications, ou sur **Annuler** pour les abandonner.

Ces choix concernent toutes les applications JeffOffice : une modification dans l'une d'elles s'applique à toutes. Une ligne correspondant à une fonction absente de l'application indique **Ne s'applique pas** et apparaît grisée.

## Macros

Les macros sont de petits programmes intégrés à un fichier. JeffCrunch les conserve dans le fichier, afin qu'elles restent présentes à l'enregistrement, et il **ne les exécute jamais**. Aucun paramètre ne permet de les exécuter.

## Fichiers liés et liaisons de classeurs

Il s'agit des autres fichiers qu'un classeur lit, comme les classeurs dont il récupère des nombres, appelés **liaisons**, les fichiers que lisent ses requêtes et les fichiers à partir desquels ses tableaux croisés dynamiques sont créés. Ce choix détermine ce que fait JeffCrunch lorsqu'il ouvre un classeur qui lit de tels fichiers, et lorsque vous utilisez **Actualiser** ou **Actualiser tout** sur ses requêtes et ses tableaux croisés dynamiques.

- **Me demander d'abord** est le choix habituel. Avant de lire l'un de ces fichiers, JeffCrunch affiche le chemin complet du fichier et vous demande confirmation. Cliquez sur **Lire le fichier** uniquement si vous faites confiance à l'origine du classeur. **Ne pas lire**, le bouton par défaut, ne lit rien et laisse le classeur inchangé. Ce choix demande confirmation à chaque ouverture d'un classeur, puis une fois pour chaque fichier. Après avoir cliqué sur **Lire le fichier** pour un fichier, JeffCrunch ne redemande pas pour ce même fichier dans cette fenêtre, jusqu'à ce que vous fermiez la fenêtre.
- **Les mettre à jour** lit un fichier sans demander seulement si vous avez déjà accepté que ce même classeur lise ce même fichier. Pour tous les autres fichiers, y compris ceux mentionnés dans un classeur que quelqu'un vient de vous envoyer, JeffCrunch demande d'abord, de la même manière.
- **Ne jamais les mettre à jour** ne lit rien à l'ouverture d'un classeur et n'affiche aucune question. Les nombres enregistrés dans le fichier restent inchangés. Lorsque vous cliquez vous-même sur **Actualiser**, il s'agit d'une demande explicite, et JeffCrunch affiche alors la question. Après avoir cliqué sur **Lire le fichier**, JeffCrunch ne redemande pas pour ce même fichier dans cette fenêtre.

## Ce qui est mémorisé

Lorsque vous cliquez sur **Lire le fichier** ou **Lire les fichiers**, JeffCrunch mémorise ce couple : ce classeur et ce fichier, chacun désigné par son chemin complet. Seul le couple est mémorisé. Si le classeur est déplacé ou renommé, modifié pour lire un autre fichier, ou si le fichier est remplacé par un raccourci vers un autre fichier, JeffCrunch demande à nouveau. Un classeur que vous n'avez jamais enregistré n'a pas de chemin, donc rien à son sujet n'est mémorisé après sa fermeture.

Les couples sont conservés uniquement dans vos propres paramètres, sur cet ordinateur, jamais dans un classeur. Un classeur ne peut donc pas en ajouter, et les personnes qui reçoivent un classeur de votre part n'en reçoivent aucun. Les paramètres conservent une forme brouillée de chaque chemin, et non le chemin lui-même. Jusqu'à 2 000 couples sont conservés ; au-delà, le couple utilisé le plus anciennement est oublié en premier.

**Options > Sécurité** indique le nombre de couples mémorisés (**Approbations mémorisées**). Cliquez sur **Tout oublier** pour les oublier tous en une seule fois. Aucun classeur n'est modifié, et chacun demandera à nouveau la prochaine fois.

Un classeur qui indique « jamais » dans son propre **Message au démarrage** (**Données > Modifier les liaisons**) est toujours laissé tel quel. Un classeur provenant d'un autre ordinateur n'est jamais lu à l'ouverture d'un fichier, quel que soit votre choix. Vous pouvez toujours utiliser **Mettre à jour les valeurs** dans **Modifier les liaisons** vous-même, à tout moment.

## Vidéos et sons liés

Ceci ne s'applique pas dans JeffCrunch : il n'a pas de vidéos ni de sons liés à lire. La ligne est grisée.

## Fichiers provenant d'Internet ou d'autres ordinateurs

Un fichier téléchargé, ou qui se trouve sur un autre ordinateur de votre réseau, peut ne pas être sûr. Ce choix détermine la manière dont JeffCrunch l'ouvre.

- **Ouvrir d'abord en lecture seule** est le choix habituel. Le fichier s'ouvre pour que vous puissiez le lire, mais vous ne pouvez pas le modifier. Une barre sous le fichier l'indique. Cliquez sur **Activer la modification** dans cette barre pour modifier ce seul fichier (jusqu'à la fermeture du programme). Lorsque vous l'enregistrez ensuite à son emplacement, la marque de téléchargement est retirée du fichier. Vous pouvez aussi utiliser **Enregistrer sous** pour créer une copie, ou modifier ce paramètre pour choisir l'option suivante.
- **Ouvrir normalement** ouvre tous les fichiers de la même manière.

Un fichier ouvert avec **Ouvrir une copie**, **Ouvrir et réparer** ou **Ouvrir en lecture seule** depuis l'écran **Fichier** s'ouvre de cette manière, quel que soit ce paramètre.

## Liens qui lancent un programme

Un lien qui lancerait un programme ou un script (un fichier dont l'extension est .exe, .bat, .sh, etc.) est **toujours refusé**. JeffCrunch vous en informe, et rien n'est exécuté. Un lien vers un fichier situé sur un autre ordinateur n'est jamais suivi sans que vous soyez d'abord consulté. Aucun paramètre ne modifie ce comportement.

## Voir aussi

- [Modifier les paramètres de JeffCrunch](options)
- [Notes et liens](notes-links)
