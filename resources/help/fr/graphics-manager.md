# Gestionnaire de graphiques
<!-- keywords: gestionnaire de graphiques, images, photos, image liée, lier au fichier, mettre à jour le lien, modifier le lien, incorporer l'image, incorporée, manquante, introuvable, modifiée, non mise à jour, hors du dossier, relier, retrouver le fichier image, état de l'image, résolution -->

Le **Gestionnaire de graphiques** répertorie toutes les images de la composition et indique la provenance de chacune. Il est surtout utile pour les images liées à leur fichier, car une image liée dépend d'un fichier situé hors de la composition. Pour savoir comment lier une image, consultez [Ajouter et ajuster des images](pictures).

## L'ouvrir

Cliquez sur **Affichage > Afficher > Gestionnaire de graphiques**. Le volet s'ouvre à droite. Pour le fermer, cliquez de nouveau sur le même bouton, ou sur le **X** en haut du volet.

## Lire la liste

Chaque ligne affiche une miniature, le nom du fichier, la page, le type de fichier, sa taille en Ko et son **état**. La zone au-dessus de la liste trie les lignes par page, nom, taille ou type. Sous la liste, les détails de l'image sélectionnée indiquent son état, son fichier, si la composition conserve l'image entière, sa taille en pixels et sa **résolution effective** : le nombre de pixels de l'image par pouce de la page. Environ 300 convient pour l'impression.

| État | Signification |
|---|---|
| Incorporée | L'image est stockée dans la composition. Aucun fichier extérieur n'est nécessaire. |
| Liée | L'image est liée à un fichier, et ce fichier se trouve là où JeffPub l'attend, sans modification. |
| Manquante | L'image est liée à un fichier qui n'existe plus à cet emplacement. Si la composition n'en conserve aucune copie, la page affiche la petite miniature, qui paraît floue, et c'est elle qui est imprimée. |
| Modification | Le fichier a changé depuis la création ou la dernière mise à jour du lien. Cliquez sur **Mettre à jour le lien** pour intégrer les modifications. |
| Non mise à jour | L'image est liée à un fichier situé hors du dossier de la composition ; JeffPub n'a donc pas ouvert ce fichier. La page affiche ce que la composition a stocké. Cliquez sur **Mettre à jour le lien** pour utiliser le fichier. Voir « Images situées hors du dossier de la composition », plus bas. |

## Rôle des boutons

- **Atteindre** sélectionne l'image sur sa page.
- **Enregistrer sous…** enregistre une copie de l'image dans un fichier.
- **Remplacer…** place un autre fichier dans le même cadre.
- **Mettre à jour le lien** relit le fichier. Utilisez-le pour une image à l'état **Modification**, après avoir remis en place un fichier **Manquante**, ou pour une image **Non mise à jour**. Toutes les images partageant ce lien sont mises à jour, et l'image conserve sa position et sa largeur.
- **Modifier le lien…** associe l'image à un autre fichier, par exemple si vous avez déplacé les fichiers dans un autre dossier.
- **Incorporer l'image** transforme le lien en copie stockée dans la composition. L'image apparaît alors comme **Incorporée**. Si le fichier est manquant, seule la petite miniature peut être incorporée, ce que signale la barre d'état.

Les trois boutons de lien sont grisés si l'image sélectionnée n'est pas liée. Chacun constitue une seule opération : **Ctrl+Z** l'annule.

## Images situées hors du dossier de la composition

À l'ouverture d'une composition, JeffPub lit les fichiers des images liées qui se trouvent dans le même dossier que la composition ou dans un de ses sous-dossiers. Il ne consulte aucun fichier situé ailleurs, sur votre ordinateur ou sur votre réseau.

La raison : une composition que l'on vous envoie peut désigner n'importe quel fichier. Si JeffPub ouvrait tout fichier désigné, la composition d'un inconnu pourrait afficher l'une de vos images privées sur ses pages, que vous risqueriez ensuite d'enregistrer ou de renvoyer sans le savoir. Sur un réseau, le simple fait de vérifier la présence d'un fichier peut aussi transmettre vos identifiants Windows à un autre ordinateur.

Une image liée à un fichier situé ailleurs affiche donc ce que la composition a stocké : l'image entière si la composition en conserve une, sinon la petite miniature, qui paraît floue. Son état est **Non mise à jour**, et les détails précisent que le fichier se trouve hors du dossier.

Pour utiliser le fichier, sélectionnez l'image et cliquez sur **Mettre à jour le lien**, ou cliquez sur **Modifier le lien…** et choisissez vous-même le fichier. Cela vaut autorisation pour cette seule image, jusqu'à la fermeture de la composition. Les autres images restent **Non mise à jour** tant que vous ne faites pas de même pour chacune.

L'enregistrement conserve le lien tel quel : à la prochaine ouverture de la composition, l'image sera de nouveau **Non mise à jour**. Pour que les images se chargent automatiquement, placez-les dans le dossier de la composition ou dans un de ses sous-dossiers.

L'impression, l'enregistrement au format PDF et l'enregistrement en livre électronique utilisent ce qu'affiche la page. Si une image est **Non mise à jour**, cliquez sur **Mettre à jour le lien** avant d'imprimer. Le [Vérificateur de conception](design-checker) répertorie ces images.

## Réparer une image manquante

1. Sélectionnez l'image **Manquante** dans la liste.
2. Cliquez sur **Modifier le lien…** et choisissez le fichier. Ou remettez le fichier à son emplacement d'origine, puis cliquez sur **Mettre à jour le lien**.
3. Si le fichier a définitivement disparu, cliquez sur **Incorporer l'image** pour conserver la miniature, ou sur **Remplacer…** pour utiliser une autre image.

Le [Vérificateur de conception](design-checker) répertorie également les images liées manquantes.

## Voir aussi

- [Ajouter et ajuster des images](pictures)
- [Vérifier votre conception](design-checker)
- [Enregistrer en tant qu'image, page web ou livre électronique](export)
