# Fonctions de base de données
<!-- keywords: fonctions, base de données, fonctions de base de données, DAVERAGE, DCOUNT, DCOUNTA, DGET, DMAX, DMIN, DPRODUCT, DSTDEV, DSTDEVP, DSUM, DVAR, DVARP -->

Voici les fonctions de base de données que JeffCrunch peut calculer. Chacune indique son écriture, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires que nécessaire après le dernier. Pour essayer l'une d'elles, reportez-vous à [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments ; un clic sur le nom de la fonction dans cette info-bulle ouvre sa fiche ici.

## DAVERAGE

`DAVERAGE(database, field, criteria)`

Calcule la moyenne des nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DCOUNT

`DCOUNT(database, field, criteria)`

Compte les nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DCOUNTA

`DCOUNTA(database, field, criteria)`

Compte les cellules non vides d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DGET

`DGET(database, field, criteria)`

Donne l'unique valeur d'une colonne d'une liste, pour la seule ligne qui répond aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DMAX

`DMAX(database, field, criteria)`

Donne le plus grand nombre d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DMIN

`DMIN(database, field, criteria)`

Donne le plus petit nombre d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DPRODUCT

`DPRODUCT(database, field, criteria)`

Multiplie les nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DSTDEV

`DSTDEV(database, field, criteria)`

Estime l'écart type des nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DSTDEVP

`DSTDEVP(database, field, criteria)`

Donne l'écart type des nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DSUM

`DSUM(database, field, criteria)`

Additionne les nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DVAR

`DVAR(database, field, criteria)`

Estime la variance des nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".

## DVARP

`DVARP(database, field, criteria)`

Donne la variance des nombres d'une colonne d'une liste, pour les lignes qui répondent aux critères.

- `database` : la plage de cellules qui contient la liste, avec les noms de ses colonnes dans la première ligne.
- `field` : la colonne à utiliser : son nom entre guillemets, ou son numéro, la première colonne comptant pour 1.
- `criteria` : ce que vous recherchez : un nombre, un texte ou un test, par exemple ">5".
