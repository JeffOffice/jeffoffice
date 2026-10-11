# Fichiers PDF et imprimeurs professionnels
<!-- keywords: pdf, créer un pdf, enregistrer en pdf, exporter en pdf, pdf/x, pdf/a, archivage, archive, imprimeur, imprimerie, impression professionnelle, cmjn, cmyk, fond perdu, traits de coupe, profil de couleur, profil colorimétrique, couleur d'accompagnement, surimpression -->

Un **PDF** est un fichier qui s'affiche de la même façon sur tous les ordinateurs. Utilisez-le pour envoyer une composition par e-mail, la publier en ligne ou la confier à un imprimeur.

## Créer un PDF

1. Cliquez sur **Fichier > Exporter**, puis sur **Créer un PDF**.
2. Sous **Optimiser pour**, choisissez une qualité. **ppp** signifie points par pouce. Plus il y a de points, plus les images sont nettes et plus les fichiers sont volumineux.
3. Sous **Pages**, choisissez **Toutes les pages**, **Page active** ou **Pages de** une page **à** une autre.
4. Cliquez sur **Enregistrer le PDF…**, tapez un nom, puis cliquez sur **Enregistrer**.

| Choix | Images | Idéal pour |
|---|---|---|
| Taille minimale | 96 ppp | L'affichage à l'écran |
| Standard | 150 ppp | Le partage en ligne |
| Impression haute qualité | 300 ppp | Les imprimantes de bureau |
| Impression commerciale | Pleine résolution | Un imprimeur professionnel. Ajoute les traits de coupe, les repères de fond perdu, les repères de montage, les barres de couleurs et les informations sur le travail. |

Autres options :

- **Inclure les propriétés du document** ajoute le titre, l'auteur, l'objet et les mots clés au PDF.
- **Ouvrir le PDF après l'enregistrement** affiche aussitôt le PDF dans votre visionneuse.
- **Feuilles de livret** apparaît pour les livrets. Cette option place deux pages sur chaque face, dans l'ordre de pliage. Désactivez-la pour obtenir une page par page PDF.

## PDF/A pour l'archivage

Le **PDF/A** est un PDF conçu pour durer des années. Il contient toutes les polices et utilise des couleurs standard, sans effets de transparence. Cochez **PDF/A pour l'archivage à long terme**, ou cliquez sur **Fichier > Exporter**, puis sur **Créer un PDF/A pour l'archivage**.

## PDF/X pour un imprimeur professionnel

Le **PDF/X** est un PDF plus strict, demandé par les imprimeurs. Les couleurs y sont exprimées en quantités d'encre, les polices y sont incorporées, et le format fini et le fond perdu y sont indiqués. Le **fond perdu** est la partie de l'image ou de la couleur qui dépasse du bord de la page, afin que la coupe ne laisse aucun bord blanc.

1. Cliquez sur **Fichier > Exporter**, puis sur **Créer un PDF/X pour un imprimeur professionnel**. Vous pouvez aussi cocher **PDF/X pour un imprimeur commercial** dans la fenêtre **Créer un PDF**. Cette option n'est pas compatible avec le PDF/A.
2. Sous **Condition d'impression**, choisissez celle indiquée par votre imprimeur.
3. Si l'imprimeur demande des traits de coupe et un fond perdu, choisissez aussi **Impression commerciale** sous **Optimiser pour**.
4. Cliquez sur **Enregistrer le PDF…**.

Les conditions disponibles sont les suivantes :

- **PDF/X-1a** pour l'offset rotative américaine (SWOP) ou l'offset européen (FOGRA39). Les effets de transparence sont aplatis.
- **PDF/X-4** pour le papier couché premium (FOGRA51, PSO Coated v3) ou avec le profil de couleur propre à votre imprimeur (un fichier .icc que vous choisissez). Les effets de transparence sont conservés.

Le profil de couleur PSO Coated v3 n'est pas fourni avec JeffPub. La première fois que vous l'utilisez, JeffPub vous demande votre accord avant de le télécharger, une seule fois, depuis l'European Color Initiative.

Avant de créer le fichier, JeffPub recherche les images de moins de 150 ppp (pixels par pouce) à leur taille d'impression ainsi que les polices absentes de cet ordinateur. Cliquez sur **Créer le PDF quand même**, **Ouvrir le vérificateur de conception** ou **Annuler**.

## Configurer les couleurs pour un imprimeur professionnel

Cliquez sur **Fichier > Propriétés**, puis sur la carte **Paramètres d'impression commerciale** :

- **Modèle de couleurs** peut être **RVB**, **Une seule couleur (d'accompagnement)**, **Couleurs d'accompagnement**, **Couleurs quadri (CMJN)** ou **Couleurs quadri et couleurs d'accompagnement**. Les couleurs quadri sont les encres cyan, magenta, jaune et noire. Une couleur d'accompagnement est une encre spéciale préparée par l'imprimeur. Ajoutez chacune d'elles sous **Couleurs d'accompagnement**.
- **Surimpression du noir dans les fichiers destinés à l'imprimeur (PDF/X)** imprime le noir par-dessus les couleurs situées en dessous, afin qu'un décalage de plaque ne laisse aucun bord blanc. Vous choisissez ce qui est concerné : le texte inférieur à une certaine taille, les traits ou les remplissages.

## Voir aussi

- [Imprimer une composition](printing)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
- [Vérifier votre conception](design-checker)
