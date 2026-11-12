# Fichiers PDF et imprimeurs professionnels
<!-- keywords: pdf, créer un pdf, enregistrer en pdf, exporter pdf, pdf/x, pdf/a, archivage, imprimeur, imprimerie, impression commerciale, cmjn, quadrichromie, fond perdu, traits de coupe, profil colorimétrique, couleur d'accompagnement, surimpression, mot de passe, chiffrer, protéger par un mot de passe, protection par mot de passe -->

Un **PDF** est un fichier qui présente le même aspect sur tous les ordinateurs. Utilisez-le pour envoyer une composition par courriel, la publier en ligne ou la transmettre à un imprimeur.

## Créer un PDF

1. Cliquez sur **Fichier > Exporter**, puis sur **Créer un PDF**.
2. Sous **Optimiser pour**, choisissez une qualité. **ppp** signifie points par pouce. Plus il y a de points, plus les images sont nettes et plus le fichier est volumineux.
3. Sous **Pages**, choisissez **Toutes les pages**, **Page actuelle** ou **Pages de** une page **à** une autre.
4. Cliquez sur **Enregistrer le PDF…**, saisissez un nom, puis cliquez sur **Enregistrer**.

| Choix | Images | Idéal pour |
|---|---|---|
| Taille minimale | 96 ppp | Affichage à l'écran |
| Standard | 150 ppp | Partage en ligne |
| Impression de haute qualité | 300 ppp | Imprimantes de bureau |
| Impression commerciale | Pleine résolution | Un imprimeur. Ajoute les traits de coupe, les repères de fond perdu, les repères de montage, les barres de couleurs et les informations de travail. |

Autres options :

- **Inclure les propriétés du document** place le titre, l'auteur, l'objet et les mots clés dans le PDF.
- **Balises de structure du document pour l'accessibilité** (activée par défaut) crée un PDF *balisé*. Les balises indiquent à un lecteur d'écran ce qui est un titre, un paragraphe, une image ou un tableau, ainsi que l'ordre de lecture : l'ordre des objets sur chaque page, que vous pouvez consulter et modifier dans la liste **Ordre de lecture** du Vérificateur d'accessibilité. Une image est lue avec son texte de remplacement. Un fichier PDF/X n'a pas de balises.
- **Créer des signets** (activée par défaut) ajoute un signet dans le PDF pour chaque page. Il reprend le nom de la page, ou « Page 3 » pour une page sans nom. Un lecteur PDF les affiche à côté des pages.
- **Ouvrir le PDF après l'enregistrement** affiche aussitôt le PDF dans votre lecteur.
- **Feuilles de livret** apparaît pour les livrets. Elle place deux pages sur chaque face dans l'ordre de pliage. Décochez-la pour une page par page PDF.

## PDF/A pour conserver des archives

**PDF/A** est un PDF conçu pour durer des années. Il incorpore toutes les polices et utilise des couleurs standard, sans effets de transparence. Cochez **PDF/A pour l'archivage à long terme**, ou cliquez sur **Fichier > Exporter**, puis sur **Créer un PDF/A pour l'archivage**.

## PDF/X pour un imprimeur

**PDF/X** est un PDF plus strict, demandé par les imprimeurs. Les couleurs sont données en quantités d'encre, les polices sont incorporées, et le format de coupe et les fonds perdus sont indiqués. Le **fond perdu** est une image ou une couleur qui dépasse du bord de la page, afin que la découpe ne laisse aucun bord blanc.

1. Cliquez sur **Fichier > Exporter**, puis sur **Créer un PDF/X pour un imprimeur professionnel**. Vous pouvez aussi cocher **PDF/X pour un imprimeur commercial** dans la fenêtre **Créer un PDF**. Il est impossible de l'utiliser avec PDF/A.
2. Sous **Condition d'impression**, choisissez celle que votre imprimeur indique.
3. Si l'imprimeur demande des traits de coupe et un fond perdu, choisissez aussi **Impression commerciale** sous **Optimiser pour**.
4. Cliquez sur **Enregistrer le PDF…**.

Les conditions disponibles sont :

- **PDF/X-1a** pour l'offset rotative américaine (SWOP) ou l'offset européen (FOGRA39). Il aplatit les effets de transparence.
- **PDF/X-4** pour le papier couché haut de gamme (FOGRA51, PSO Coated v3), ou avec le profil colorimétrique de votre imprimeur (un fichier .icc que vous choisissez). Il conserve les effets de transparence.

Le profil colorimétrique PSO Coated v3 n'est pas fourni avec JeffPub. La première fois que vous l'utilisez, JeffPub vous demande l'autorisation de le télécharger, une seule fois, depuis l'European Color Initiative.

Avant de créer le fichier, JeffPub vérifie les images dont la résolution est inférieure à 150 ppi (pixels par pouce) à leur taille d'impression, ainsi que les polices absentes de cet ordinateur. Cliquez sur **Créer le PDF quand même**, **Ouvrir le vérificateur de conception** ou **Annuler**.

## Protéger un PDF par un mot de passe

Dans la fenêtre **Créer un PDF**, cliquez sur **Chiffrer avec un mot de passe…**. Saisissez le mot de passe deux fois. Toute personne qui ouvre le PDF devra alors le saisir. Le mot de passe protège réellement le fichier : les pages sont brouillées par un chiffrement fort (AES-256), et personne ne peut les lire sans lui.

Vous pouvez aussi cocher **Restreindre l'impression, la copie et la modification**, saisir un second mot de passe et choisir ce qui est autorisé. Ces restrictions ne sont qu'une demande. La plupart des lecteurs PDF les respectent, mais elles n'empêchent pas une personne déterminée de les contourner ; ne comptez donc pas sur elles pour garder un fichier privé.

Un mot de passe oublié ne peut pas être récupéré : conservez-le en lieu sûr. Le mot de passe ne sert que pour ce PDF et n'est jamais enregistré. Un PDF protégé par un mot de passe ne peut pas être PDF/A ni PDF/X ; ces deux options sont donc désactivées tant qu'un mot de passe est défini. Dans **Fichier > Partager**, **Chiffrer avec un mot de passe…** placé au-dessus de **Envoyer au format PDF** protège le PDF joint à un courriel.

Lorsque vous placez comme image un PDF protégé par un mot de passe, JeffPub demande ce mot de passe. S'il est incorrect, JeffPub le redemande ; cliquez sur **Annuler** pour arrêter. La page de votre composition est une copie et ne nécessite pas de mot de passe. Si le propriétaire du PDF demande qu'il ne soit pas copié, JeffPub le place tout de même et affiche un message l'indiquant ; cette demande n'est pas un verrou.

Une image liée à un PDF protégé par un mot de passe s'affiche comme **Non mise à jour** à l'ouverture de la composition, et JeffPub ne demande jamais le mot de passe de lui-même. Cliquez sur **Mettre à jour le lien** pour qu'il vous le demande.

## Régler les couleurs pour un imprimeur

Cliquez sur **Fichier > Infos**, puis sur la carte **Paramètres d'impression commerciale** :

- Le **Modèle de couleurs** est **RVB**, **Une seule couleur (d'accompagnement)**, **Couleurs d'accompagnement**, **Couleurs quadri (CMJN)** ou **Couleurs quadri et couleurs d'accompagnement**. Les couleurs quadri sont les encres cyan, magenta, jaune et noir. Une couleur d'accompagnement est une encre spéciale mélangée par l'imprimeur. Ajoutez-les une à une sous **Couleurs d'accompagnement**.
- **Surimpression du noir dans les fichiers destinés à l'imprimeur (PDF/X)** fait imprimer le noir par-dessus les couleurs situées dessous, afin qu'un décalage des plaques ne laisse aucun bord blanc. Vous choisissez ce qu'elle couvre : le texte en dessous d'une certaine taille, les traits ou les remplissages.

## Voir aussi

- [Imprimer une composition](printing)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
- [Vérifier votre conception](design-checker)
