# Proteggere un documento e bloccare le revisioni
<!-- keywords: proteggi, protezione, limita modifiche, sola lettura, password, blocca revisioni, opzioni revisioni, nome utente, iniziali, solo modifiche registrate, solo commenti, interrompi protezione, proteggi, campi modulo, modulo -->

La protezione limita ciò che gli utenti possono fare in un documento. Questi strumenti si trovano nella scheda **Revisione**.

## Limitare le modifiche

1. Fai clic su **Limita modifiche** nel gruppo **Proteggi**.
2. Scegli che cosa possono fare gli utenti:
   - **Nessuna modifica (sola lettura)**: il testo non può essere modificato.
   - **Modifiche registrate**: gli utenti possono modificare, ma ogni modifica viene registrata e le revisioni non possono essere disattivate.
   - **Commenti**: gli utenti possono aggiungere commenti, ma non modificare il testo.
   - **Compilazione di moduli**: il testo è di sola lettura, ma gli utenti possono compilare i campi modulo che contiene.
3. Digita una password, due volte. Puoi lasciare vuoto il campo della password. In tal caso chiunque può interrompere la protezione.
4. Fai clic su **Avvia applicazione restrizioni**.

La barra del titolo mostra quindi "Protetto". Per interrompere la protezione, fai di nuovo clic su **Limita modifiche**, quindi su **Interrompi protezione**. Digita la password, se ne è stata impostata una.

Una password impedisce alle persone in buona fede di modificare il documento per errore. Non è una protezione robusta. Non usare la protezione per nascondere informazioni riservate.

La password viene conservata nel file come numero codificato, detto hash. La password stessa non viene mai salvata. Se la dimentichi, JeffWrite non è in grado di recuperarla.

## Compilare un modulo

Un documento può contenere campi modulo: caselle in cui digitare, caselle di controllo ed elenchi da cui scegliere. JeffWrite li compila, ma non ne aggiunge di nuovi.

- **Digitare in un campo.** Fai clic sul campo e digita. Un campo può accettare solo un certo numero di caratteri, oppure convertire ciò che digiti in maiuscole, in un numero o in una data. JeffWrite applica queste regole quando digiti, incolli o inserisci testo con un metodo di input. Il testo incollato che viene troncato per entrare nel campo conserva i primi caratteri. Un'interruzione di riga nel testo incollato diventa uno spazio. Un'interruzione di riga già presente nel campo resta, a meno che non la selezioni e la sovrascriva; la digitazione non ne aggiunge mai. Un numero o una data viene riportato nel formato impostato quando esci dal campo: premi Tab, premi Invio, fai clic altrove, esci con i tasti freccia, salva o stampa.
- **Selezionare una casella di controllo.** Fai clic su di essa, oppure spostati fino a essa e premi la BARRA SPAZIATRICE.
- **Scegliere da un elenco.** Fai clic sull'elenco, oppure spostati fino a esso e premi Invio. Poi scegli una voce. Premendo una lettera si sceglie la voce successiva che inizia con quella lettera.
- **Spostarsi tra i campi.** Quando il documento è protetto per la compilazione di moduli, premi Tab o Invio per passare al campo successivo e MAIUSC+Tab per tornare indietro. In qualsiasi documento, premere Invio all'interno di un campo non crea un nuovo paragrafo. Possono essere modificati solo i campi. Anche una parte del documento che chi ha creato il modulo ha escluso dalla protezione può essere modificata; in quella parte il tasto Tab funziona normalmente, e Sostituisci tutto modifica solo le corrispondenze presenti in quella parte, indica quante ne ha lasciate e segnala che il testo nelle intestazioni, nei piè di pagina, nelle note e nelle caselle di testo è stato lasciato invariato. Ciò che digiti, incolli, trascini o correggi viene verificato in base al punto in cui andrebbe a finire, non in base alla posizione del cursore. Immagini, forme e tabelle possono essere aggiunte e modificate in una parte esclusa dalla protezione, come in qualsiasi documento, e non vengono modificate altrove.
- **Annullare.** Premi Annulla. Le lettere digitate in un campo costituiscono un'unica operazione. Annulla ripristina le operazioni eseguite dall'inizio della protezione, cioè le voci inserite e le modifiche in una parte esclusa dalla protezione, e nulla di ciò che c'era prima.

Non è possibile compilare un campo disattivato nel modulo o un campo che il modulo calcola automaticamente. In qualsiasi documento, i contrassegni di inizio, fine e altri contrassegni di un campo non vengono mai eliminati uno alla volta: per eliminare il campo, selezionalo per intero. Non è possibile compilare un campo nemmeno in un documento di sola lettura o aperto solo per i commenti. Con le revisioni attive in un documento non protetto, ciò che digiti in un campo di testo viene registrato come modifica, e la lunghezza massima del testo e le maiuscole del campo non vengono applicate.

## Bloccare le revisioni

Fai clic su **Opzioni revisioni**, quindi su **Blocca revisioni**. Da quel momento ogni modifica viene registrata. Nessuno può disattivare Revisioni, né accettare o rifiutare una modifica, senza la password. Equivale a proteggere il documento per le modifiche registrate.

## Cambiare il proprio nome nelle revisioni

Fai clic su **Opzioni revisioni**, quindi su **Cambia opzioni revisioni**. Digita il tuo nome e le tue iniziali. Firmeranno le modifiche e i commenti che farai da quel momento in poi.
