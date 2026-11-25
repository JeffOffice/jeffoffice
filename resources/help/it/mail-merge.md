# Unione posta e unione catalogo
<!-- keywords: unione posta, unione, destinatari, elenco indirizzi, etichette, lettere tipo, csv, xlsx, vcard, campo unione, blocco di indirizzi, catalogo, elenco prodotti, unione posta elettronica, anteprima, stampa unione -->

L'unione posta riempie una pubblicazione con i dati di un elenco. Scrivi una lettera una sola volta, e JeffPub ne crea una copia per ogni persona, con nome e indirizzo di quella persona al loro posto. Un'unione catalogo ripete un unico layout per ogni voce di un elenco.

Gli strumenti si trovano nella scheda **Lettere**. Per un percorso guidato, fai clic su **Lettere > Avvia > Stampa unione passo passo** per aprire il riquadro **Stampa unione**.

## Scegliere i destinatari

Fai clic su **Lettere > Avvia > Seleziona destinatari**:

- **Usa un elenco esistente…**: scegli un file CSV, un file di testo con tabulazioni, un foglio di calcolo .xlsx (il primo foglio) o un file di contatti .vcf. In un file CSV, di testo o di foglio di calcolo, la prima riga deve contenere i nomi delle colonne.
- **Seleziona dai contatti…**: scegli un file di contatti (.vcf o .vcard). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato; ogni contatto diventa un destinatario, con colonne per nome, indirizzo, telefono ed e-mail.
- **Digita un nuovo elenco…**: compila la finestra **Nuovo elenco indirizzi**, usando **Nuova voce** per ogni persona. JeffPub ti propone quindi di salvare l'elenco come file CSV.

Per controllare l'elenco, fai clic su **Lettere > Avvia > Modifica elenco destinatari**. Nella finestra **Destinatari stampa unione** deseleziona una riga per escludere quella persona, fai clic su un'intestazione per ordinare, e digita in **Filtra destinatari** per restringere l'elenco. **Distingui i destinatari in base a** sceglie una colonna che distingue le persone; l'elenco viene conservato con le righe deselezionate nella pubblicazione e, quando scegli di nuovo lo stesso file, le stesse persone restano escluse, anche se nel frattempo hai ordinato le righe. **Trova duplicati** deseleziona le ripetizioni, mentre **Convalida indirizzi** evidenzia le righe con un indirizzo mancante o con un CAP insolito.

## Aggiungere campi unione

Un campo unione è un segnaposto, come «Nome», che diventa un valore reale per ogni persona. Fai clic in una casella di testo (JeffPub ne aggiunge una se non ce n'è), quindi usa **Lettere > Scrivi e inserisci campi**:

- **Blocco di indirizzi**: un indirizzo postale composto da colonne con nomi come Nome, Cognome e Indirizzo riga 1.
- **Formula di apertura**: "Gentile" seguito dal nome della persona, oppure "Caro amico," se il nome manca.
- **Inserisci campo unione**: scegli una colonna qualsiasi.
- **Campo immagine**: scegli una colonna con i nomi dei file immagine. JeffPub aggiunge una cornice vuota e cerca i file accanto al tuo elenco. Dopo aver salvato la pubblicazione e averla riaperta, un'immagine il cui file si trova al di fuori della cartella della pubblicazione non viene mostrata finché non scegli di nuovo l'elenco destinatari (**Lettere > Seleziona destinatari > Usa un elenco esistente…**). A quel punto vengono mostrate le immagini presenti nella cartella dell'elenco e in quelle al suo interno, fino alla chiusura della pubblicazione. JeffPub non ricorda questa scelta per la volta successiva. Oppure conserva i file immagine nella cartella della pubblicazione, o in una cartella al suo interno: in quel caso vengono mostrati non appena la pubblicazione si apre. Così una pubblicazione che qualcuno ti invia non legge da sola immagini dal tuo computer. L'immagine di ogni record viene mostrata quando visualizzi l'anteprima dei risultati, quando stampi, quando salvi un PDF o quando unisci in una nuova pubblicazione o in un catalogo.

## Mostrare un numero, un prezzo, una data o un'ora a modo tuo

Un campo unione mostra il valore esattamente come è nel tuo elenco. Per cambiarlo, fai clic accanto al campo (oppure selezionalo, o selezionane diversi) e fai clic su **Lettere > Scrivi e inserisci campi > Formatta campo unione…**. In **Mostra il valore come**, scegli:

- **Numero**, con da 0 a 4 **Cifre decimali**. Ad esempio, 1234,5 con 1 cifra decimale viene mostrato come 1.234,5.
- **Valuta**, con il simbolo di valuta della tua area: 1234,5 viene mostrato come 1.234,50 € dove quello è il simbolo.
- **Percentuale**, dove 0,256 viene mostrato come 25,6% con 1 cifra decimale.
- **Data**, in uno **Stile** come Breve, Lungo, Anno, Mese, giorno, Mese e giorno o Mese e anno.
- **Ora**, in uno **Stile** Breve, a 12 ore o a 24 ore.

La riga **Esempio** mostra il risultato della tua scelta. I valori vengono letti e mostrati nella lingua del testo nel campo (inglese americano se non ne è impostata nessuna), non nella lingua del tuo computer. Con l'inglese americano 03/04/2026 è il 4 marzo, mentre in una lingua che mette il giorno per primo, come il tedesco o l'inglese britannico, è il 3 aprile; in tedesco 1.234,5 è un numero con virgola decimale. Un anno a due cifre viene letto come 2000–2029 per 00–29, e come 1930–1999 per 30–99. Un numero tra parentesi, come (500), è negativo. Un valore che non è un numero, una data o un'ora del tipo scelto viene mostrato esattamente come è nel tuo elenco: una parola, "nan" o "inf" (una persona potrebbe chiamarsi Nan), un valore vuoto, un'ora con un formato data, o una data con un formato ora. L'anteprima e l'unione completata mostrano entrambe il valore formattato. La scelta è un passaggio di Annulla e viene salvata con la pubblicazione.

## Visualizzare l'anteprima dei risultati

Fai clic su **Lettere > Anteprima risultati > Anteprima risultati** per vedere i dati reali al posto dei campi. Usa **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure **Trova destinatario** per saltare a una corrispondenza.

## Completare l'unione

Fai clic su **Lettere > Fine > Completa e unisci**:

- **Unisci in stampante…** apre la pagina **Stampa**. Seleziona **Stampa tutti i record della stampa unione**, quindi stampa.
- **Unisci in una nuova pubblicazione** crea una nuova pubblicazione con una copia delle tue pagine per ogni persona.
- **Unisci in PDF…** salva un unico PDF con le pagine di ogni persona.
- **Unisci in posta elettronica…** crea un file e-mail .eml per ogni persona, con la prima pagina come immagine. Scegli la colonna dell'indirizzo e-mail, digita un oggetto e scegli una cartella. JeffPub non invia i messaggi: apri i file con il tuo programma di posta.
- **Salva ciascuno come file…** crea una pubblicazione .jpub per ogni persona, in una cartella a tua scelta. Assegna i nomi dei file usando colonne tra parentesi graffe, come {Cognome}, {Nome}. Ti viene chiesta conferma prima di più di 200 file, e una volta se nella cartella ci sono già file con gli stessi nomi. Fai clic su **Annulla** per interrompere tra un file e l'altro.
- **Esporta elenco destinatari…** salva l'elenco come file CSV.

## Unione catalogo

Fai clic su **Inserisci > Pagine > Pagine catalogo** per aprire il riquadro **Unione catalogo**.

1. Scegli l'elenco dei prodotti, come descritto sopra.
2. Fai clic su **Inserisci area catalogo**. Compare una casella con l'etichetta "Area catalogo". Imposta **Righe** e **Colonne**, oltre a posizione e dimensioni. Inserisci i campi di una voce nella prima cella, facendo doppio clic su di essi nell'elenco. Si ripetono nelle altre celle. Per un'immagine, scegli la versione "(immagine)" di un campo.
3. Visualizza l'anteprima con le frecce. Ogni pagina mostra tante voci quante sono le celle dell'area. Poi unisci in una nuova pubblicazione, in un PDF o in stampante.

**Rimuovi area catalogo** elimina l'area.

## Argomenti correlati

- [Stampare una pubblicazione](printing)
- [Aggiungere e formattare le caselle di testo](text-boxes)
- [La scheda Lettere](tab-mailings)
