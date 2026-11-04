# Campi

<!-- keywords: campo, campi, INDEX, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, aggiorna campo, F9, scollega, blocca, codici di campo, numero di pagina, data, NUMPAGES, SEQ, REF, PAGEREF, formula, Inserisci campo -->

Un **campo** è un elemento di testo che il programma calcola per te, come il numero di pagina, la data odierna o il numero di una figura. È composto da due parti: un **codice**, che indica che cosa calcolare, e un **risultato**, cioè il testo che vedi. Il codice per il numero di pagina è `PAGE`, e il suo risultato nella terza pagina è `3`.

## Inserire un campo nel testo

1. Fai clic nel punto in cui deve comparire il campo.
2. Fai clic su **Inserisci > Campo**.
3. Scegli un campo dall'elenco. Il suo codice compare nella casella sotto, e puoi modificarlo.
4. Fai clic su **OK**.

Per esempio, scegli **Data**, cambia il codice in `DATE \@ "MMMM d, yyyy"` e fai clic su **OK**. Il testo mostra la data come `2 novembre 2026`.

## Aggiornare i campi

Un campo mostra il risultato che aveva l'ultima volta che è stato calcolato. Premi **F9** per calcolare di nuovo il campo in cui si trova il cursore. Per aggiornare contemporaneamente più campi, seleziona il testo che li contiene e premi **F9**. Premi **Ctrl+A** e poi **F9** per aggiornare tutti i campi del documento.

L'aggiornamento è un'unica operazione: **Annulla** ripristina tutti i risultati precedenti. L'aggiornamento non è una modifica registrata, anche con le revisioni attive, perché il risultato non è qualcosa che hai digitato.

Un campo in un'intestazione o in un piè di pagina che mostra il numero di pagina, il numero totale di pagine o un titolo è sempre corretto e non richiede aggiornamenti.

## Mostrare i codici

Premi **Maiusc+F9** per mostrare, al posto del risultato, il codice del campo in cui si trova il cursore, racchiuso tra parentesi graffe, come `{ PAGE }`. Premi **Alt+F9** per mostrare i codici di tutti i campi, e premilo di nuovo per mostrare i risultati. Questo cambia solo ciò che vedi: il file viene salvato come era e il documento non viene segnato come modificato.

## Scollegare o bloccare un campo

- **Ctrl+Maiusc+F9** trasforma il campo in testo normale. Il risultato resta, ma non è più un campo.
- **Ctrl+F11** blocca un campo, così **F9** non lo modifica. **Ctrl+Maiusc+F11** lo sblocca.

## Formati

Aggiungi opzioni al codice per cambiare l'aspetto del risultato:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` e `\* Caps` cambiano le maiuscole o il tipo di numero. `{ = 21 \* CardText }` mostra `ventuno`.
- `\# "0.00"` imposta l'aspetto di un numero. `{ = 1234.5 \# "#,##0.00" }` mostra `1.234,50`.
- `\@ "dddd, MMMM d"` imposta l'aspetto di una data. `M` indica il mese, e `m` indica il minuto.

## Campi calcolati da JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, una formula che inizia con `=`, TOC (anche per un elenco di figure), INDEX, TOA, CITATION e BIBLIOGRAPHY. I campi nascosti TC, XE e TA non hanno un risultato da calcolare: indicano a una tabella, a un indice o a una tabella delle fonti che cosa elencare. Un campo di qualsiasi altro tipo mantiene il risultato che ha e viene salvato così com'è.

DATE, TIME, FILENAME e AUTHOR dipendono dal computer e dal momento, quindi possono cambiare ogni volta che li aggiorni.
