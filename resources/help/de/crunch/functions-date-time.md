# Funktionen für Datum und Uhrzeit
<!-- keywords: Funktionen, DATUM, DATEDIF, DATWERT, TAG, TAGE, TAGE360, EDATUM, MONATSENDE, STUNDE, ISOKALENDERWOCHE, MINUTE, MONAT, NETTOARBEITSTAGE, NETTOARBEITSTAGE.INTL, JETZT, SEKUNDE, ZEIT, ZEITWERT, HEUTE, WOCHENTAG, KALENDERWOCHE, ARBEITSTAG, ARBEITSTAG.INTL, JAHR, JAHRESBRUCH, Datumsfunktion, Zeitfunktion, Datum berechnen, Arbeitstage, Wochentag -->

Dies sind die Funktionen für Datum und Uhrzeit, die JeffCrunch berechnen kann. Jede zeigt, wie sie geschrieben wird, was sie tut und was jedes ihrer Argumente bedeutet. Ein Argument in eckigen Klammern ist optional, und "..." bedeutet, dass Sie beliebig viele weitere des letzten Arguments angeben können. Um eine Funktion auszuprobieren, lesen Sie [Funktion auswählen](insert-function). Während Sie in einer Zelle eine Funktion eingeben, zeigt ein kleiner Hinweis unter der Zelle ihre Argumente an. Ein Klick auf den Namen der Funktion im Hinweis öffnet ihren Eintrag hier.

## DATUM

`DATUM(Jahr; Monat; Tag)`

Erstellt ein Datum aus einem Jahr, einem Monat und einem Tag.

- `Jahr`: Das Jahr, z. B. 2027.
- `Monat`: Der Monat, von 1 bis 12.
- `Tag`: Der Tag des Monats, von 1 bis 31.

## DATEDIF

`DATEDIF(Anfangsdatum; Enddatum; Einheit)`

Gibt die Jahre, Monate oder Tage zwischen zwei Datumsangaben an.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Enddatum`: Das Datum, an dem der Zeitraum endet.
- `Einheit`: Was gezählt werden soll: „J“ für Jahre, „M“ für Monate, „T“ für Tage, „MT“ für Tage ohne Monate und Jahre, „YM“ für Monate ohne Jahre, „YD“ für Tage ohne Jahre.

## DATWERT

`DATWERT(Datumstext)`

Wandelt ein als Text geschriebenes Datum in ein Datum um.

- `Datumstext`: Ein als Text geschriebenes Datum, z. B. "15.03.2027".

## TAG

`TAG(Zahl)`

Gibt den Tag des Monats eines Datums an.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.

## TAGE

`TAGE(Enddatum; Anfangsdatum)`

Zählt die Tage zwischen zwei Datumsangaben.

- `Enddatum`: Das Datum, an dem der Zeitraum endet.
- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.

## TAGE360

`TAGE360(Anfangsdatum; Enddatum; [Methode])`

Zählt die Tage zwischen zwei Datumsangaben in einem Jahr mit zwölf Monaten zu je 30 Tagen.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Enddatum`: Das Datum, an dem der Zeitraum endet.
- `Methode` (optional): Wie die Monate gezählt werden: FALSCH oder weggelassen für die US-Methode, WAHR für die europäische Methode.

## EDATUM

`EDATUM(Anfangsdatum; Monate)`

Gibt das Datum an, das eine bestimmte Anzahl Monate vor oder nach einem Datum liegt.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Monate`: Wie viele Monate addiert werden (negativ, um zurückzugehen).

## MONATSENDE

`MONATSENDE(Anfangsdatum; Monate)`

Gibt den letzten Tag des Monats an, der eine bestimmte Anzahl Monate vor oder nach einem Datum liegt.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Monate`: Wie viele Monate addiert werden (negativ, um zurückzugehen).

## STUNDE

`STUNDE(Zahl)`

Gibt die Stunde einer Uhrzeit an.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.

## ISOKALENDERWOCHE

`ISOKALENDERWOCHE(Datum)`

Gibt die internationale Kalenderwoche eines Datums an.

- `Datum`: Ein Datum oder eine Zelle, die ein Datum enthält.

## MINUTE

`MINUTE(Zahl)`

Gibt die Minuten einer Uhrzeit an.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.

## MONAT

`MONAT(Zahl)`

Gibt den Monat eines Datums an, von 1 bis 12.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.

## NETTOARBEITSTAGE

`NETTOARBEITSTAGE(Anfangsdatum; Enddatum; [Feiertage])`

Zählt die Arbeitstage zwischen zwei Datumsangaben.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Enddatum`: Das Datum, an dem der Zeitraum endet.
- `Feiertage` (optional): Ein Zellbereich mit Datumsangaben, die keine Arbeitstage sind.

## NETTOARBEITSTAGE.INTL

`NETTOARBEITSTAGE.INTL(Anfangsdatum; Enddatum; [Wochenende]; [Feiertage])`

Zählt die Arbeitstage zwischen zwei Datumsangaben, mit den Wochentagen des Wochenendes Ihrer Wahl.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Enddatum`: Das Datum, an dem der Zeitraum endet.
- `Wochenende` (optional): Welche Tage das Wochenende sind: ein Code von 1 bis 7 oder von 11 bis 17 oder sieben Zeichen aus 0 und 1, beginnend mit Montag, wobei 1 für einen freien Tag steht.
- `Feiertage` (optional): Ein Zellbereich mit Datumsangaben, die keine Arbeitstage sind.

## JETZT

`JETZT()`

Gibt das aktuelle Datum und die aktuelle Uhrzeit an.

## SEKUNDE

`SEKUNDE(Zahl)`

Gibt die Sekunden einer Uhrzeit an.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.

## ZEIT

`ZEIT(Stunde; Minute; Sekunde)`

Erstellt eine Uhrzeit aus einer Stunde, Minuten und Sekunden.

- `Stunde`: Die Stunde, von 0 bis 23.
- `Minute`: Die Minute, von 0 bis 59.
- `Sekunde`: Die Sekunde, von 0 bis 59.

## ZEITWERT

`ZEITWERT(Zeittext)`

Wandelt eine als Text geschriebene Uhrzeit in eine Uhrzeit um.

- `Zeittext`: Eine als Text geschriebene Uhrzeit, z. B. "14:30".

## HEUTE

`HEUTE()`

Gibt das heutige Datum an.

## WOCHENTAG

`WOCHENTAG(Zahl; [Rückgabetyp])`

Gibt den Wochentag eines Datums als Zahl an.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.
- `Rückgabetyp` (optional): Mit welchem Tag die Woche beginnt: 1 oder weggelassen für Sonntag, 2 für Montag.

## KALENDERWOCHE

`KALENDERWOCHE(Zahl; [Rückgabetyp])`

Gibt die Nummer der Woche des Jahres an, in die ein Datum fällt.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.
- `Rückgabetyp` (optional): Mit welchem Tag die Woche beginnt: 1 oder weggelassen für Sonntag, 2 für Montag.

## ARBEITSTAG

`ARBEITSTAG(Anfangsdatum; Tage; [Feiertage])`

Gibt das Datum an, das eine bestimmte Anzahl Arbeitstage vor oder nach einem Datum liegt.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Tage`: Die Anzahl der Tage, die addiert werden (negativ, um zurückzugehen).
- `Feiertage` (optional): Ein Zellbereich mit Datumsangaben, die keine Arbeitstage sind.

## ARBEITSTAG.INTL

`ARBEITSTAG.INTL(Anfangsdatum; Tage; [Wochenende]; [Feiertage])`

Gibt das Datum an, das eine bestimmte Anzahl Arbeitstage von einem Anfangsdatum entfernt liegt, mit den Wochentagen des Wochenendes Ihrer Wahl.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Tage`: Die Anzahl der Tage, die addiert werden (negativ, um zurückzugehen).
- `Wochenende` (optional): Welche Tage das Wochenende sind: ein Code von 1 bis 7 oder von 11 bis 17 oder sieben Zeichen aus 0 und 1, beginnend mit Montag, wobei 1 für einen freien Tag steht.
- `Feiertage` (optional): Ein Zellbereich mit Datumsangaben, die keine Arbeitstage sind.

## JAHR

`JAHR(Zahl)`

Gibt das Jahr eines Datums an.

- `Zahl`: Ein Datum oder eine Uhrzeit oder eine Zelle, die eine solche enthält.

## JAHRESBRUCH

`JAHRESBRUCH(Anfangsdatum; Enddatum; [Basis])`

Gibt den Teil eines Jahres zwischen zwei Datumsangaben an.

- `Anfangsdatum`: Das Datum, an dem der Zeitraum beginnt.
- `Enddatum`: Das Datum, an dem der Zeitraum endet.
- `Basis` (optional): Wie die Tage eines Jahres gezählt werden: 0 oder weggelassen für 30/360, 1 für tatsächliche Tage, 2 für tatsächliche Tage in einem Jahr mit 360 Tagen, 3 für tatsächliche Tage in einem Jahr mit 365 Tagen, 4 für europäische 30/360.
