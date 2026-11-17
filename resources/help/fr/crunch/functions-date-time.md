# Fonctions de date et d'heure
<!-- keywords: fonctions, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC, fonction, date, heure, jour, mois, année, jours ouvrés, semaine, durée, calendrier -->

Voici les fonctions de date et d'heure que JeffCrunch peut calculer. Chacune indique sa syntaxe, ce qu'elle fait et le rôle de chacun de ses arguments. Un argument entre crochets est facultatif, et « ... » signifie que vous pouvez ajouter autant d'arguments supplémentaires que nécessaire après le dernier. Pour essayer l'une d'elles, consultez [Choisir une fonction](insert-function). Pendant que vous saisissez une fonction dans une cellule, une petite info-bulle sous la cellule affiche ses arguments, et un clic sur le nom de la fonction dans l'info-bulle ouvre sa fiche ici.

## DATE

`DATE(année; mois; jour)`

Crée une date à partir d'une année, d'un mois et d'un jour.

- `année` : L'année, par exemple 2027.
- `mois` : Le mois, de 1 à 12.
- `jour` : Le jour du mois, de 1 à 31.

## DATEDIF

`DATEDIF(date_début; date_fin; unité)`

Donne le nombre d'années, de mois ou de jours entre deux dates.

- `date_début` : La date de début de la période.
- `date_fin` : La date de fin de la période.
- `unité` : Ce qu'il faut compter : « Y » pour les années, « M » pour les mois, « D » pour les jours, « MD » pour les jours sans tenir compte des mois ni des années, « YM » pour les mois sans tenir compte des années, « YD » pour les jours sans tenir compte des années.

## DATEVALUE

`DATEVALUE(texte_date)`

Convertit une date saisie sous forme de texte en date.

- `texte_date` : Une date écrite sous forme de texte, par exemple « 15/03/2027 ».

## DAY

`DAY(numéro_de_série)`

Donne le jour du mois d'une date.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.

## DAYS

`DAYS(date_fin; date_début)`

Compte le nombre de jours entre deux dates.

- `date_fin` : La date de fin de la période.
- `date_début` : La date de début de la période.

## DAYS360

`DAYS360(date_début; date_fin; [méthode])`

Compte le nombre de jours entre deux dates sur une année de douze mois de 30 jours.

- `date_début` : La date de début de la période.
- `date_fin` : La date de fin de la période.
- `méthode` (facultatif) : Mode de comptage des mois : FAUX ou omis pour la méthode américaine, VRAI pour la méthode européenne.

## EDATE

`EDATE(date_début; mois)`

Donne la date située un certain nombre de mois avant ou après une date.

- `date_début` : La date de début de la période.
- `mois` : Le nombre de mois à ajouter (négatif pour reculer).

## EOMONTH

`EOMONTH(date_début; mois)`

Donne le dernier jour du mois situé un certain nombre de mois avant ou après une date.

- `date_début` : La date de début de la période.
- `mois` : Le nombre de mois à ajouter (négatif pour reculer).

## HOUR

`HOUR(numéro_de_série)`

Donne l'heure d'une valeur horaire.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.

## ISOWEEKNUM

`ISOWEEKNUM(date)`

Donne le numéro de semaine ISO d'une date.

- `date` : Une date, ou une cellule qui en contient une.

## MINUTE

`MINUTE(numéro_de_série)`

Donne les minutes d'une valeur horaire.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.

## MONTH

`MONTH(numéro_de_série)`

Donne le mois d'une date, de 1 à 12.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.

## NETWORKDAYS

`NETWORKDAYS(date_début; date_fin; [jours_fériés])`

Compte le nombre de jours ouvrés entre deux dates.

- `date_début` : La date de début de la période.
- `date_fin` : La date de fin de la période.
- `jours_fériés` (facultatif) : Une plage de cellules contenant des dates qui ne sont pas des jours ouvrés.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(date_début; date_fin; [week-end]; [jours_fériés])`

Compte le nombre de jours ouvrés entre deux dates, en choisissant les jours du week-end.

- `date_début` : La date de début de la période.
- `date_fin` : La date de fin de la période.
- `week-end` (facultatif) : Les jours du week-end : un code de 1 à 7 ou de 11 à 17, ou sept caractères 0 et 1 à partir du lundi, avec 1 pour un jour non travaillé.
- `jours_fériés` (facultatif) : Une plage de cellules contenant des dates qui ne sont pas des jours ouvrés.

## NOW

`NOW()`

Donne la date et l'heure actuelles.

## SECOND

`SECOND(numéro_de_série)`

Donne les secondes d'une valeur horaire.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.

## TIME

`TIME(heures; minutes; secondes)`

Crée une heure à partir d'un nombre d'heures, de minutes et de secondes.

- `heures` : L'heure, de 0 à 23.
- `minutes` : Les minutes, de 0 à 59.
- `secondes` : Les secondes, de 0 à 59.

## TIMEVALUE

`TIMEVALUE(texte_heure)`

Convertit une heure saisie sous forme de texte en heure.

- `texte_heure` : Une heure écrite sous forme de texte, par exemple « 14:30 ».

## TODAY

`TODAY()`

Donne la date du jour.

## WEEKDAY

`WEEKDAY(numéro_de_série; [type_retour])`

Donne le jour de la semaine d'une date sous forme de nombre.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.
- `type_retour` (facultatif) : Le jour de début de la semaine : 1 ou omis pour dimanche, 2 pour lundi.

## WEEKNUM

`WEEKNUM(numéro_de_série; [type_retour])`

Donne le numéro de la semaine de l'année à laquelle appartient une date.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.
- `type_retour` (facultatif) : Le jour de début de la semaine : 1 ou omis pour dimanche, 2 pour lundi.

## WORKDAY

`WORKDAY(date_début; jours; [jours_fériés])`

Donne la date située un certain nombre de jours ouvrés avant ou après une date.

- `date_début` : La date de début de la période.
- `jours` : Le nombre de jours à ajouter (négatif pour reculer).
- `jours_fériés` (facultatif) : Une plage de cellules contenant des dates qui ne sont pas des jours ouvrés.

## WORKDAY.INTL

`WORKDAY.INTL(date_début; jours; [week-end]; [jours_fériés])`

Donne la date située un certain nombre de jours ouvrés à partir d'une date de début, en choisissant les jours du week-end.

- `date_début` : La date de début de la période.
- `jours` : Le nombre de jours à ajouter (négatif pour reculer).
- `week-end` (facultatif) : Les jours du week-end : un code de 1 à 7 ou de 11 à 17, ou sept caractères 0 et 1 à partir du lundi, avec 1 pour un jour non travaillé.
- `jours_fériés` (facultatif) : Une plage de cellules contenant des dates qui ne sont pas des jours ouvrés.

## YEAR

`YEAR(numéro_de_série)`

Donne l'année d'une date.

- `numéro_de_série` : Une date ou une heure, ou une cellule qui en contient une.

## YEARFRAC

`YEARFRAC(date_début; date_fin; [base])`

Donne la fraction d'année comprise entre deux dates.

- `date_début` : La date de début de la période.
- `date_fin` : La date de fin de la période.
- `base` (facultatif) : Mode de comptage des jours d'une année : 0 ou omis pour 30/360, 1 pour les jours réels, 2 pour les jours réels sur une année de 360 jours, 3 pour les jours réels sur une année de 365 jours, 4 pour 30/360 européen.
