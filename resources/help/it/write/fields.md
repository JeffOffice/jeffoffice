# Campi

<!-- keywords: campo, campi, indice analitico, INDEX, sommario, TOA, CITATION, BIBLIOGRAPHY, XE, TA, TC, aggiorna campo, F9, scollega, blocca, codici di campo, numero di pagina, data, NUMPAGES, SEQ, REF, PAGEREF, formula, Inserisci campo, modifica campo, clic destro -->

Un **campo** è una parte di testo che il programma calcola per te, come un numero di pagina, la data di oggi o il numero di una figura. Ha due parti: un **codice**, che indica cosa calcolare, e un **risultato**, che è il testo che vedi. Il codice per il numero di pagina è `PAGE`, e il suo risultato nella terza pagina è `3`.

## Inserire un campo nel testo

1. Fai clic nel punto in cui deve comparire il campo.
2. Fai clic su **Inserisci > Campo**.
3. Scegli un campo dall'elenco. Il suo codice compare nella casella sottostante e puoi modificarlo.
4. Fai clic su **OK**.

Per esempio, scegli **Data**, cambia il codice in `DATE \@ "MMMM d, yyyy"` e fai clic su **OK**. Nel testo la data compare come `2 novembre 2026`.

## Modificare il codice di un campo

Fai clic con il pulsante destro del mouse su un campo e scegli **Modifica campo**. Una casella mostra il codice. Modificalo e fai clic su **OK**: il campo viene calcolato di nuovo. **Annulla** ripristina il campo precedente. Un campo bloccato non può essere modificato finché non lo sblocchi. Il menu del clic destro contiene anche **Aggiorna campo**, **Attiva/disattiva codice di campo**, **Blocca campo**, **Sblocca campo** e **Scollega campo**.

## Aggiornare i campi

Un campo mostra il risultato che aveva quando è stato calcolato l'ultima volta. Premi **F9** per calcolare di nuovo il campo in cui si trova il cursore. Per aggiornare più campi insieme, seleziona il testo che li contiene e premi **F9**. Premi **Ctrl+A** e poi **F9** per aggiornare tutti i campi del documento.

L'aggiornamento è un'unica operazione: **Annulla** ripristina tutti i risultati precedenti. L'aggiornamento di un campo non è una modifica registrata, anche se le Revisioni sono attive, perché il risultato non è qualcosa che hai digitato.

Un campo in un'intestazione o in un piè di pagina che mostra il numero di pagina, il numero totale di pagine o un titolo è sempre corretto e non richiede aggiornamenti.

## Mostrare i codici

Premi **Maiusc+F9** per mostrare, al posto del risultato, il codice del campo in cui si trova il cursore, racchiuso tra parentesi graffe, come `{ PAGE }`. Premi **Alt+F9** per mostrare i codici di tutti i campi; premilo di nuovo per tornare ai risultati. Questa operazione cambia solo ciò che vedi: il file viene salvato come era e il documento non viene segnato come modificato.

## Scollegare o bloccare un campo

- **Ctrl+Maiusc+F9** trasforma il campo in testo normale. Il risultato resta, ma non è più un campo.
- **Ctrl+F11** blocca un campo, così **F9** non lo modifica. **Ctrl+Maiusc+F11** lo sblocca.

## Formati

Aggiungi opzioni al codice per cambiare l'aspetto di un risultato:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` e `\* Caps` cambiano le maiuscole o il tipo di numero. `{ = 21 \* CardText }` mostra `ventuno`.
- `\# "0.00"` imposta l'aspetto di un numero. `{ = 1234.5 \# "#,##0.00" }` mostra `1.234,50`.
- `\@ "dddd, MMMM d"` imposta l'aspetto di una data. `M` indica il mese e `m` indica i minuti.

## Campi calcolati da JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, una formula che inizia con `=`, TOC (anche per un elenco di figure), INDEX, TOA, CITATION e BIBLIOGRAPHY. I campi nascosti TC, XE e TA non hanno un risultato da calcolare: indicano a una tabella, a un indice o a un indice delle fonti giuridiche cosa inserire nell'elenco. Un campo di qualsiasi altro tipo conserva il risultato che ha ed è salvato così com'è.

DATE, TIME, FILENAME e AUTHOR dipendono dal computer e dal momento, quindi possono cambiare ogni volta che li aggiorni.
