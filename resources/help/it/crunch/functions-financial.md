# Funzioni finanziarie
<!-- keywords: funzioni, ACCRINT, ACCRINTM, AMORDEGRC, AMORLINC, COUPDAYBS, COUPDAYS, COUPDAYSNC, COUPNCD, COUPNUM, COUPPCD, CUMIPMT, CUMPRINC, DB, DDB, DISC, DOLLARDE, DOLLARFR, DURATION, EFFECT, FV, FVSCHEDULE, INTRATE, IPMT, IRR, ISPMT, MDURATION, MIRR, NOMINAL, NPER, NPV, ODDFPRICE, ODDFYIELD, ODDLPRICE, ODDLYIELD, PDURATION, PMT, PPMT, PRICE, PRICEDISC, PRICEMAT, PV, RATE, RECEIVED, RRI, SLN, SYD, TBILLEQ, TBILLPRICE, TBILLYIELD, VDB, XIRR, XNPV, YIELD, YIELDDISC, YIELDMAT -->

Queste sono le funzioni finanziarie che JeffCrunch può calcolare. Ciascuna mostra come si scrive, cosa fa e il significato di ciascun argomento. Un argomento tra parentesi quadre è facoltativo, e "..." indica che si possono aggiungere quanti altri argomenti servono, dopo l'ultimo. Per provarne una, vedi [Scegli una funzione](insert-function). Mentre digiti una funzione in una cella, sotto la cella compare un piccolo suggerimento con i suoi argomenti; se fai clic sul nome della funzione nel suggerimento, si apre la sua scheda in questa pagina.

## ACCRINT

`ACCRINT(issue, first_interest, settlement, rate, par, frequency, [basis], [calc_method])`

Restituisce gli interessi maturati su un titolo che paga interessi secondo un calendario.

- `issue`: La data di emissione del titolo.
- `first_interest`: La data in cui viene pagato il primo interesse.
- `settlement`: La data di acquisto del titolo.
- `rate`: Il tasso di interesse per periodo.
- `par`: Il valore nominale del titolo (1.000 se omesso).
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.
- `calc_method` (facoltativo): VERO o omesso per calcolare gli interessi dalla data di emissione; FALSO per calcolarli dalla data del primo interesse quando la data di regolamento è successiva.

## ACCRINTM

`ACCRINTM(issue, settlement, rate, [par], [basis])`

Restituisce gli interessi maturati su un titolo che paga interessi alla scadenza.

- `issue`: La data di emissione del titolo.
- `settlement`: La data di acquisto del titolo.
- `rate`: Il tasso di interesse per periodo.
- `par` (facoltativo): Il valore nominale del titolo (1.000 se omesso).
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## AMORDEGRC

`AMORDEGRC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Restituisce l'ammortamento di un periodo con il metodo francese a quote decrescenti.

- `cost`: Il costo del bene al momento dell'acquisto.
- `date_purchased`: La data di acquisto del bene.
- `first_period`: La data in cui termina il primo periodo.
- `salvage`: Il valore del bene alla fine.
- `period`: Il periodo da elaborare, contando il primo come 1.
- `rate`: Il tasso di interesse per periodo.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## AMORLINC

`AMORLINC(cost, date_purchased, first_period, salvage, period, rate, [basis])`

Restituisce l'ammortamento di un periodo con il metodo francese a quote costanti.

- `cost`: Il costo del bene al momento dell'acquisto.
- `date_purchased`: La data di acquisto del bene.
- `first_period`: La data in cui termina il primo periodo.
- `salvage`: Il valore del bene alla fine.
- `period`: Il periodo da elaborare, contando il primo come 1.
- `rate`: Il tasso di interesse per periodo.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## COUPDAYBS

`COUPDAYBS(settlement, maturity, frequency, [basis])`

Restituisce i giorni dall'inizio del periodo di cedola alla data di regolamento.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## COUPDAYS

`COUPDAYS(settlement, maturity, frequency, [basis])`

Restituisce i giorni del periodo di cedola che contiene la data di regolamento.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## COUPDAYSNC

`COUPDAYSNC(settlement, maturity, frequency, [basis])`

Restituisce i giorni dalla data di regolamento alla cedola successiva.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## COUPNCD

`COUPNCD(settlement, maturity, frequency, [basis])`

Restituisce la data della cedola successiva alla data di regolamento.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## COUPNUM

`COUPNUM(settlement, maturity, frequency, [basis])`

Restituisce il numero di cedole da pagare tra la data di regolamento e la data di scadenza.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## COUPPCD

`COUPPCD(settlement, maturity, frequency, [basis])`

Restituisce la data della cedola precedente alla data di regolamento.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## CUMIPMT

`CUMIPMT(rate, nper, pv, start_period, end_period, type)`

Restituisce gli interessi pagati su un prestito tra due periodi.

- `rate`: Il tasso di interesse per periodo.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `start_period`: Il periodo in cui inizia l'intervallo.
- `end_period`: Il periodo in cui termina l'intervallo.
- `type`: Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## CUMPRINC

`CUMPRINC(rate, nper, pv, start_period, end_period, type)`

Restituisce la quota di capitale pagata su un prestito tra due periodi.

- `rate`: Il tasso di interesse per periodo.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `start_period`: Il periodo in cui inizia l'intervallo.
- `end_period`: Il periodo in cui termina l'intervallo.
- `type`: Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## DB

`DB(cost, salvage, life, period, [month])`

Restituisce l'ammortamento di un bene per un periodo, con il metodo del saldo decrescente fisso.

- `cost`: Il costo del bene al momento dell'acquisto.
- `salvage`: Il valore del bene alla fine.
- `life`: Quanti periodi dura il bene.
- `period`: Il periodo da elaborare, contando il primo come 1.
- `month` (facoltativo): Il mese, da 1 a 12.

## DDB

`DDB(cost, salvage, life, period, [factor])`

Restituisce l'ammortamento di un periodo con il metodo del saldo decrescente.

- `cost`: Il costo del bene al momento dell'acquisto.
- `salvage`: Il valore del bene alla fine.
- `life`: Quanti periodi dura il bene.
- `period`: Il periodo da elaborare, contando il primo come 1.
- `factor` (facoltativo): Quanto rapidamente decresce il saldo (2 se omesso, cioè doppio saldo decrescente).

## DISC

`DISC(settlement, maturity, pr, redemption, [basis])`

Restituisce il tasso di sconto di un titolo.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `pr`: Il prezzo del titolo per 100 di valore nominale.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## DOLLARDE

`DOLLARDE(fractional_dollar, fraction)`

Converte un prezzo scritto come frazione, come 1,02, in un numero decimale.

- `fractional_dollar`: Un prezzo scritto come frazione, come 1,02 per una unità e due sedicesimi.
- `fraction`: Il numero per cui dividere una unità: 16 per i sedicesimi, ad esempio.

## DOLLARFR

`DOLLARFR(decimal_dollar, fraction)`

Converte un prezzo decimale in uno scritto come frazione, come 1,02.

- `decimal_dollar`: Un prezzo scritto come numero decimale, come 1,125.
- `fraction`: Il numero per cui dividere una unità: 16 per i sedicesimi, ad esempio.

## DURATION

`DURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Restituisce il tempo medio necessario per recuperare il costo di un'obbligazione, ponderato sui pagamenti.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `coupon`: Il tasso cedolare annuo dell'obbligazione.
- `yld`: Il rendimento annuo del titolo.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## EFFECT

`EFFECT(nominal_rate, npery)`

Restituisce il tasso annuo effettivo corrispondente a un tasso nominale con più periodi all'anno.

- `nominal_rate`: Il tasso annuo nominale.
- `npery`: Quanti periodi ci sono in un anno.

## FV

`FV(rate, nper, pmt, [pv], [type])`

Restituisce il valore finale di pagamenti periodici regolari.

- `rate`: Il tasso di interesse per periodo.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pmt`: Il pagamento effettuato in ogni periodo.
- `pv` (facoltativo): Il valore attuale dell'importo (l'intero prestito o investimento).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## FVSCHEDULE

`FVSCHEDULE(principal, schedule)`

Restituisce il valore futuro di un importo dopo una serie di tassi di interesse.

- `principal`: L'importo iniziale.
- `schedule`: L'intervallo di celle con i tassi di interesse da applicare, uno dopo l'altro.

## INTRATE

`INTRATE(settlement, maturity, investment, redemption, [basis])`

Restituisce il tasso di interesse di un titolo interamente investito.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `investment`: L'importo investito nel titolo.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## IPMT

`IPMT(rate, per, nper, pv, [fv], [type])`

Restituisce la quota di interessi di un pagamento.

- `rate`: Il tasso di interesse per periodo.
- `per`: La rata da elaborare, contando la prima come 1.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv` (facoltativo): Quanto resta dopo l'ultimo pagamento (0 se omesso).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## IRR

`IRR(values, [guess])`

Restituisce il tasso di rendimento di una serie di importi che arrivano a intervalli regolari.

- `values`: L'intervallo di celle che contiene gli importi.
- `guess` (facoltativo): La tua stima del risultato, da cui far partire la ricerca (0,1 se omesso).

## ISPMT

`ISPMT(rate, per, nper, pv)`

Restituisce gli interessi pagati in un periodo di un prestito che viene rimborsato in quote di capitale uguali.

- `rate`: Il tasso di interesse per periodo.
- `per`: La rata da elaborare, contando la prima come 1.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).

## MDURATION

`MDURATION(settlement, maturity, coupon, yld, frequency, [basis])`

Restituisce la duration modificata di un'obbligazione, rettificata per il suo rendimento.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `coupon`: Il tasso cedolare annuo dell'obbligazione.
- `yld`: Il rendimento annuo del titolo.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## MIRR

`MIRR(values, finance_rate, reinvest_rate)`

Restituisce il tasso di rendimento di flussi di cassa, con un tasso per il finanziamento e un altro per il reinvestimento.

- `values`: L'intervallo di celle che contiene gli importi.
- `finance_rate`: Il tasso che paghi sul denaro preso in prestito.
- `reinvest_rate`: Il tasso che guadagni sul denaro reinvestito.

## NOMINAL

`NOMINAL(effect_rate, npery)`

Restituisce il tasso nominale corrispondente a un tasso annuo effettivo e a un numero di periodi all'anno.

- `effect_rate`: Il tasso annuo effettivo.
- `npery`: Quanti periodi ci sono in un anno.

## NPER

`NPER(rate, pmt, pv, [fv], [type])`

Restituisce quanti pagamenti servono per estinguere un prestito.

- `rate`: Il tasso di interesse per periodo.
- `pmt`: Il pagamento effettuato in ogni periodo.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv` (facoltativo): Quanto resta dopo l'ultimo pagamento (0 se omesso).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## NPV

`NPV(rate, value1, ...)`

Restituisce il valore attuale di una serie di importi futuri.

- `rate`: Il tasso di interesse per periodo.
- `value1`: Il primo valore o intervallo di celle.
- `...`: Quanti altri ne servono, dello stesso tipo del precedente.

## ODDFPRICE

`ODDFPRICE(settlement, maturity, issue, first_coupon, rate, yld, redemption, frequency, [basis])`

Restituisce il prezzo per 100 di un'obbligazione con un primo periodo irregolare.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `issue`: La data di emissione del titolo.
- `first_coupon`: La data della prima cedola.
- `rate`: Il tasso di interesse per periodo.
- `yld`: Il rendimento annuo del titolo.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## ODDFYIELD

`ODDFYIELD(settlement, maturity, issue, first_coupon, rate, pr, redemption, frequency, [basis])`

Restituisce il rendimento di un'obbligazione con un primo periodo irregolare.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `issue`: La data di emissione del titolo.
- `first_coupon`: La data della prima cedola.
- `rate`: Il tasso di interesse per periodo.
- `pr`: Il prezzo del titolo per 100 di valore nominale.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## ODDLPRICE

`ODDLPRICE(settlement, maturity, last_interest, rate, yld, redemption, frequency, [basis])`

Restituisce il prezzo per 100 di un'obbligazione con un ultimo periodo irregolare.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `last_interest`: La data in cui è stato pagato l'ultimo interesse.
- `rate`: Il tasso di interesse per periodo.
- `yld`: Il rendimento annuo del titolo.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## ODDLYIELD

`ODDLYIELD(settlement, maturity, last_interest, rate, pr, redemption, frequency, [basis])`

Restituisce il rendimento di un'obbligazione con un ultimo periodo irregolare.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `last_interest`: La data in cui è stato pagato l'ultimo interesse.
- `rate`: Il tasso di interesse per periodo.
- `pr`: Il prezzo del titolo per 100 di valore nominale.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## PDURATION

`PDURATION(rate, pv, fv)`

Restituisce i periodi necessari perché un investimento raggiunga un valore.

- `rate`: Il tasso di interesse per periodo.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv`: Quanto resta dopo l'ultimo pagamento (0 se omesso).

## PMT

`PMT(rate, nper, pv, [fv], [type])`

Restituisce il pagamento periodico regolare di un prestito.

- `rate`: Il tasso di interesse per periodo.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv` (facoltativo): Quanto resta dopo l'ultimo pagamento (0 se omesso).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## PPMT

`PPMT(rate, per, nper, pv, [fv], [type])`

Restituisce la parte di un pagamento che estingue il capitale del prestito.

- `rate`: Il tasso di interesse per periodo.
- `per`: La rata da elaborare, contando la prima come 1.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv` (facoltativo): Quanto resta dopo l'ultimo pagamento (0 se omesso).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## PRICE

`PRICE(settlement, maturity, rate, yld, redemption, frequency, [basis])`

Restituisce il prezzo per 100 di un'obbligazione che paga interessi secondo un calendario.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `rate`: Il tasso di interesse per periodo.
- `yld`: Il rendimento annuo del titolo.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## PRICEDISC

`PRICEDISC(settlement, maturity, discount, redemption, [basis])`

Restituisce il prezzo per 100 di un titolo acquistato con uno sconto.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `discount`: Il tasso di sconto del titolo.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## PRICEMAT

`PRICEMAT(settlement, maturity, issue, rate, yld, [basis])`

Restituisce il prezzo per 100 di un titolo che paga interessi alla scadenza.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `issue`: La data di emissione del titolo.
- `rate`: Il tasso di interesse per periodo.
- `yld`: Il rendimento annuo del titolo.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## PV

`PV(rate, nper, pmt, [fv], [type])`

Restituisce il valore odierno di una serie di pagamenti futuri.

- `rate`: Il tasso di interesse per periodo.
- `nper`: Quanti pagamenti sono previsti in tutto.
- `pmt`: Il pagamento effettuato in ogni periodo.
- `fv` (facoltativo): Quanto resta dopo l'ultimo pagamento (0 se omesso).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.

## RATE

`RATE(nper, pmt, pv, [fv], [type], [guess])`

Restituisce il tasso di interesse per periodo di un prestito o di un investimento.

- `nper`: Quanti pagamenti sono previsti in tutto.
- `pmt`: Il pagamento effettuato in ogni periodo.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv` (facoltativo): Quanto resta dopo l'ultimo pagamento (0 se omesso).
- `type` (facoltativo): Quando vengono effettuati i pagamenti: 0 o omesso alla fine di ogni periodo, 1 all'inizio.
- `guess` (facoltativo): La tua stima del risultato, da cui far partire la ricerca (0,1 se omesso).

## RECEIVED

`RECEIVED(settlement, maturity, investment, discount, [basis])`

Restituisce l'importo incassato alla scadenza per un titolo interamente investito.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `investment`: L'importo investito nel titolo.
- `discount`: Il tasso di sconto del titolo.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## RRI

`RRI(nper, pv, fv)`

Restituisce il tasso di interesse che fa crescere un investimento fino a un valore.

- `nper`: Quanti pagamenti sono previsti in tutto.
- `pv`: Il valore attuale dell'importo (l'intero prestito o investimento).
- `fv`: Quanto resta dopo l'ultimo pagamento (0 se omesso).

## SLN

`SLN(cost, salvage, life)`

Restituisce l'ammortamento annuo a quote costanti.

- `cost`: Il costo del bene al momento dell'acquisto.
- `salvage`: Il valore del bene alla fine.
- `life`: Quanti periodi dura il bene.

## SYD

`SYD(cost, salvage, life, per)`

Restituisce l'ammortamento annuo con il metodo della somma delle cifre degli anni.

- `cost`: Il costo del bene al momento dell'acquisto.
- `salvage`: Il valore del bene alla fine.
- `life`: Quanti periodi dura il bene.
- `per`: La rata da elaborare, contando la prima come 1.

## TBILLEQ

`TBILLEQ(settlement, maturity, discount)`

Restituisce il rendimento equivalente obbligazionario di un buono del Tesoro.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `discount`: Il tasso di sconto del titolo.

## TBILLPRICE

`TBILLPRICE(settlement, maturity, discount)`

Restituisce il prezzo per 100 di un buono del Tesoro.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `discount`: Il tasso di sconto del titolo.

## TBILLYIELD

`TBILLYIELD(settlement, maturity, pr)`

Restituisce il rendimento di un buono del Tesoro.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `pr`: Il prezzo del titolo per 100 di valore nominale.

## VDB

`VDB(cost, salvage, life, start_period, end_period, [factor], [no_switch])`

Restituisce l'ammortamento di un intervallo qualsiasi di periodi con il metodo del saldo decrescente.

- `cost`: Il costo del bene al momento dell'acquisto.
- `salvage`: Il valore del bene alla fine.
- `life`: Quanti periodi dura il bene.
- `start_period`: Il periodo in cui inizia l'intervallo.
- `end_period`: Il periodo in cui termina l'intervallo.
- `factor` (facoltativo): Quanto rapidamente decresce il saldo (2 se omesso, cioè doppio saldo decrescente).
- `no_switch` (facoltativo): VERO per continuare con il metodo del saldo decrescente anche quando la quota a quote costanti risulterebbe maggiore.

## XIRR

`XIRR(values, dates, [guess])`

Restituisce il tasso di rendimento di importi che arrivano in date scelte da te.

- `values`: L'intervallo di celle che contiene gli importi.
- `dates`: Le date degli importi, nello stesso ordine degli importi.
- `guess` (facoltativo): La tua stima del risultato, da cui far partire la ricerca (0,1 se omesso).

## XNPV

`XNPV(rate, values, dates)`

Restituisce il valore attuale di importi che arrivano in date scelte da te.

- `rate`: Il tasso di interesse per periodo.
- `values`: L'intervallo di celle che contiene gli importi.
- `dates`: Le date degli importi, nello stesso ordine degli importi.

## YIELD

`YIELD(settlement, maturity, rate, pr, redemption, frequency, [basis])`

Restituisce il rendimento di un'obbligazione che paga interessi secondo un calendario.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `rate`: Il tasso di interesse per periodo.
- `pr`: Il prezzo del titolo per 100 di valore nominale.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `frequency`: Quante cedole vengono pagate in un anno: 1, 2 o 4.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## YIELDDISC

`YIELDDISC(settlement, maturity, pr, redemption, [basis])`

Restituisce il rendimento annuo di un titolo acquistato con uno sconto.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `pr`: Il prezzo del titolo per 100 di valore nominale.
- `redemption`: Il valore del titolo alla scadenza, per 100 di valore nominale.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.

## YIELDMAT

`YIELDMAT(settlement, maturity, issue, rate, pr, [basis])`

Restituisce il rendimento annuo di un titolo che paga interessi alla scadenza.

- `settlement`: La data di acquisto del titolo.
- `maturity`: La data in cui il titolo scade e viene rimborsato.
- `issue`: La data di emissione del titolo.
- `rate`: Il tasso di interesse per periodo.
- `pr`: Il prezzo del titolo per 100 di valore nominale.
- `basis` (facoltativo): Come vengono conati i giorni dell'anno: 0 o omesso per 30/360, 1 giorni effettivi, 2 giorni effettivi in un anno di 360 giorni, 3 giorni effettivi in un anno di 365 giorni, 4 30/360 europeo.
