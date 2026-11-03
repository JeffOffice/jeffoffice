# Campi
<!-- keywords: campo, campi, aggiorna campo, aggiornare campi, F9, scollega, scollegare, blocca, bloccare, codici di campo, numero di pagina, data, NUMPAGES, SEQ, REF, PAGEREF, formula, Inserisci campo -->

Un **campo** è una porzione di testo che il programma calcola automaticamente, ad esempio un numero di pagina, la data odierna o il numero di una figura. È composto da due parti: un **codice**, che indica cosa calcolare, e un **risultato**, ovvero il testo visualizzato. Il codice di un numero di pagina è `PAGE` e il suo risultato nella terza pagina è `3`.

## Inserire un campo nel testo

1. Fai clic nel punto in cui inserire il campo.
2. Fai clic su **Inserisci > Campo**.
3. Scegli un campo dall'elenco. Il relativo codice viene visualizzato nella casella sottostante, dove puoi modificarlo.
4. Fai clic su **OK**.

Ad esempio, scegli **Data**, modifica il codice in `DATE \@ "d MMMM yyyy"` e fai clic su **OK**. Il testo mostra la data come `2 novembre 2026`.

## Aggiornare i campi

Un campo mostra il risultato ottenuto dall'ultimo calcolo. Premi **F9** per ricalcolare il campo in cui si trova il cursore. Per aggiornare più campi contemporaneamente, seleziona il testo che li contiene e premi **F9**. Premi **CTRL+A** e quindi **F9** per aggiornare tutti i campi del documento.

L'aggiornamento è un'unica operazione: **Annulla** ripristina tutti i risultati precedenti. L'aggiornamento non viene registrato come revisione, nemmeno quando le revisioni sono attivate, perché il risultato non è testo digitato da te.

Un campo in un'intestazione o in un piè di pagina che mostra il numero di pagina, il numero totale di pagine o un titolo è sempre corretto e non richiede aggiornamenti.

## Visualizzare i codici

Premi **MAIUSC+F9** per visualizzare, al posto del risultato, il codice del campo in cui si trova il cursore, racchiuso tra parentesi graffe, ad esempio `{ PAGE }`. Premi **ALT+F9** per visualizzare i codici di tutti i campi e premilo di nuovo per tornare ai risultati. Questa operazione cambia solo la visualizzazione: il file viene salvato come prima e il documento non viene considerato modificato.

## Scollegare o bloccare un campo

- **CTRL+MAIUSC+F9** trasforma il campo in testo normale. Il risultato rimane, ma non è più un campo.
- **CTRL+F11** blocca un campo, in modo che **F9** non lo modifichi. **CTRL+MAIUSC+F11** lo sblocca.

## Formati

Aggiungi opzioni al codice per modificare l'aspetto di un risultato:

- `\* roman`, `\* ROMAN`, `\* alphabetic`, `\* Ordinal`, `\* CardText`, `\* Upper`, `\* Lower` e `\* Caps` modificano le maiuscole/minuscole o il tipo di numerazione. `{ = 21 \* CardText }` mostra `ventuno`.
- `\# "0,00"` imposta l'aspetto di un numero. `{ = 1234,5 \# "#.##0,00" }` mostra `1.234,50`.
- `\@ "dddd d MMMM"` imposta l'aspetto di una data. `M` indica il mese e `m` i minuti.

## Campi calcolati da JeffWrite

PAGE, NUMPAGES, SECTIONPAGES, SECTION, PAGEREF, REF, NOTEREF, STYLEREF, SEQ, DATE, TIME, CREATEDATE, SAVEDATE, PRINTDATE, AUTHOR, TITLE, SUBJECT, FILENAME, FILESIZE, DOCPROPERTY, NUMWORDS, NUMCHARS, IF, COMPARE, QUOTE, le formule che iniziano con `=` e TOC. Un campo di qualsiasi altro tipo mantiene il risultato che ha e viene salvato così com'è.

DATE, TIME, FILENAME e AUTHOR dipendono dal computer e dal momento, quindi il loro risultato può cambiare a ogni aggiornamento.
