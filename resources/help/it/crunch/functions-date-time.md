# Funzioni di data e ora
<!-- keywords: funzioni, DATE, DATEDIF, DATEVALUE, DAY, DAYS, DAYS360, EDATE, EOMONTH, HOUR, ISOWEEKNUM, MINUTE, MONTH, NETWORKDAYS, NETWORKDAYS.INTL, NOW, SECOND, TIME, TIMEVALUE, TODAY, WEEKDAY, WEEKNUM, WORKDAY, WORKDAY.INTL, YEAR, YEARFRAC -->

Queste sono le funzioni di data e ora che JeffCrunch può calcolare. Ogni funzione mostra come si scrive, che cosa fa e quali sono i suoi argomenti. Un argomento tra parentesi quadre è facoltativo, e "..." indica che si possono aggiungere quanti altri argomenti servono dopo l'ultimo. Per provarne una, vedi [Scegli una funzione](insert-function). Mentre si digita una funzione in una cella, un piccolo suggerimento sotto la cella mostra i suoi argomenti, e un clic sul nome della funzione nel suggerimento apre la sua scheda in questa guida.

## DATE

`DATE(anno, mese, giorno)`

Crea una data a partire da un anno, un mese e un giorno.

- `anno`: L'anno, ad esempio 2027.
- `mese`: Il mese, da 1 a 12.
- `giorno`: Il giorno del mese, da 1 a 31.

## DATEDIF

`DATEDIF(data_inizio, data_fine, unità)`

Restituisce gli anni, i mesi o i giorni tra due date.

- `data_inizio`: La data in cui inizia il periodo.
- `data_fine`: La data in cui termina il periodo.
- `unità`: Che cosa contare: "Y" anni, "M" mesi, "D" giorni, "MD" giorni ignorando mesi e anni, "YM" mesi ignorando gli anni, "YD" giorni ignorando gli anni.

## DATEVALUE

`DATEVALUE(testo_data)`

Converte una data scritta come testo in una data.

- `testo_data`: Una data scritta come testo, ad esempio "15/3/2027".

## DAY

`DAY(numero_seriale)`

Restituisce il giorno del mese di una data.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.

## DAYS

`DAYS(data_fine, data_inizio)`

Conta i giorni tra due date.

- `data_fine`: La data in cui termina il periodo.
- `data_inizio`: La data in cui inizia il periodo.

## DAYS360

`DAYS360(data_inizio, data_fine, [metodo])`

Conta i giorni tra due date in un anno di dodici mesi da 30 giorni.

- `data_inizio`: La data in cui inizia il periodo.
- `data_fine`: La data in cui termina il periodo.
- `metodo` (facoltativo): Come si contano i mesi: falso o omesso per il metodo USA, vero per il metodo europeo.

## EDATE

`EDATE(data_inizio, mesi)`

Restituisce la data che si trova un certo numero di mesi prima o dopo una data.

- `data_inizio`: La data in cui inizia il periodo.
- `mesi`: Quanti mesi aggiungere (negativo per tornare indietro).

## EOMONTH

`EOMONTH(data_inizio, mesi)`

Restituisce l'ultimo giorno del mese che si trova un certo numero di mesi prima o dopo una data.

- `data_inizio`: La data in cui inizia il periodo.
- `mesi`: Quanti mesi aggiungere (negativo per tornare indietro).

## HOUR

`HOUR(numero_seriale)`

Restituisce l'ora di un orario.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.

## ISOWEEKNUM

`ISOWEEKNUM(data)`

Restituisce il numero della settimana ISO di una data.

- `data`: Una data, oppure una cella che la contiene.

## MINUTE

`MINUTE(numero_seriale)`

Restituisce i minuti di un orario.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.

## MONTH

`MONTH(numero_seriale)`

Restituisce il mese di una data, da 1 a 12.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.

## NETWORKDAYS

`NETWORKDAYS(data_inizio, data_fine, [festività])`

Conta i giorni lavorativi tra due date.

- `data_inizio`: La data in cui inizia il periodo.
- `data_fine`: La data in cui termina il periodo.
- `festività` (facoltativo): Un intervallo di celle con le date che non sono giorni lavorativi.

## NETWORKDAYS.INTL

`NETWORKDAYS.INTL(data_inizio, data_fine, [fine_settimana], [festività])`

Conta i giorni lavorativi tra due date, con i giorni del fine settimana a scelta.

- `data_inizio`: La data in cui inizia il periodo.
- `data_fine`: La data in cui termina il periodo.
- `fine_settimana` (facoltativo): Quali giorni sono il fine settimana: un codice da 1 a 7 o da 11 a 17, oppure sette caratteri di 0 e 1 a partire dal lunedì, con 1 per un giorno libero.
- `festività` (facoltativo): Un intervallo di celle con le date che non sono giorni lavorativi.

## NOW

`NOW()`

Restituisce la data e l'ora correnti.

## SECOND

`SECOND(numero_seriale)`

Restituisce i secondi di un orario.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.

## TIME

`TIME(ore, minuti, secondi)`

Crea un orario a partire da ore, minuti e secondi.

- `ore`: L'ora, da 0 a 23.
- `minuti`: Il minuto, da 0 a 59.
- `secondi`: Il secondo, da 0 a 59.

## TIMEVALUE

`TIMEVALUE(testo_ora)`

Converte un orario scritto come testo in un orario.

- `testo_ora`: Un orario scritto come testo, ad esempio "14:30".

## TODAY

`TODAY()`

Restituisce la data odierna.

## WEEKDAY

`WEEKDAY(numero_seriale, [tipo_restituito])`

Restituisce il giorno della settimana di una data sotto forma di numero.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.
- `tipo_restituito` (facoltativo): Il giorno con cui inizia la settimana: 1 o omesso per domenica, 2 per lunedì.

## WEEKNUM

`WEEKNUM(numero_seriale, [tipo_restituito])`

Restituisce il numero della settimana dell'anno in cui cade una data.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.
- `tipo_restituito` (facoltativo): Il giorno con cui inizia la settimana: 1 o omesso per domenica, 2 per lunedì.

## WORKDAY

`WORKDAY(data_inizio, giorni, [festività])`

Restituisce la data che si trova un certo numero di giorni lavorativi prima o dopo una data.

- `data_inizio`: La data in cui inizia il periodo.
- `giorni`: Il numero di giorni da aggiungere (negativo per tornare indietro).
- `festività` (facoltativo): Un intervallo di celle con le date che non sono giorni lavorativi.

## WORKDAY.INTL

`WORKDAY.INTL(data_inizio, giorni, [fine_settimana], [festività])`

Restituisce la data che si trova un certo numero di giorni lavorativi da una data di inizio, con i giorni del fine settimana a scelta.

- `data_inizio`: La data in cui inizia il periodo.
- `giorni`: Il numero di giorni da aggiungere (negativo per tornare indietro).
- `fine_settimana` (facoltativo): Quali giorni sono il fine settimana: un codice da 1 a 7 o da 11 a 17, oppure sette caratteri di 0 e 1 a partire dal lunedì, con 1 per un giorno libero.
- `festività` (facoltativo): Un intervallo di celle con le date che non sono giorni lavorativi.

## YEAR

`YEAR(numero_seriale)`

Restituisce l'anno di una data.

- `numero_seriale`: Una data o un orario, oppure una cella che lo contiene.

## YEARFRAC

`YEARFRAC(data_inizio, data_fine, [base])`

Restituisce la parte di anno compresa tra due date.

- `data_inizio`: La data in cui inizia il periodo.
- `data_fine`: La data in cui termina il periodo.
- `base` (facoltativo): Come si contano i giorni di un anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 europeo 30/360.
