# Stampa unione e unione catalogo
<!-- keywords: stampa unione, unione, destinatari, elenco indirizzi, etichette, lettere personalizzate, csv, xlsx, vcard, campo unione, blocco di indirizzi, catalogo, elenco prodotti, unione posta elettronica, anteprima -->

La stampa unione riempie una pubblicazione con i dati di un elenco. Si scrive una lettera una sola volta, e JeffPub ne crea una copia per ogni persona, con nome e indirizzo di quella persona al posto giusto. L'unione catalogo ripete un unico layout per ogni voce di un elenco.

Gli strumenti si trovano nella scheda **Lettere**. Per un percorso guidato, fare clic su **Lettere > Inizia > Stampa unione > Creazione guidata stampa unione passo passo** per aprire il riquadro **Stampa unione**.

## Scegliere i destinatari

Fare clic su **Lettere > Inizia > Seleziona destinatari**:

- **Usa elenco esistente…**: scegliere un file CSV, un file di testo con tabulazioni, un foglio di calcolo .xlsx (il primo foglio) o un file di contatti .vcf. In un file CSV, di testo o di foglio di calcolo, la prima riga deve contenere i nomi delle colonne.
- **Seleziona dai contatti…**: scegliere un file di contatti (.vcf o .vcard). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato; ogni contatto diventa un destinatario, con colonne per nome, indirizzo, telefono ed e-mail.
- **Digita un nuovo elenco…**: compilare la finestra **Nuovo elenco indirizzi**, usando **Nuova voce** per ogni persona. JeffPub propone quindi di salvare l'elenco come file CSV.

Per controllare l'elenco, fare clic su **Lettere > Inizia > Modifica elenco destinatari**. Nella finestra **Destinatari stampa unione** deselezionare una riga per escludere quella persona, fare clic su un'intestazione per ordinare, e digitare in **Filtra destinatari** per restringere l'elenco. **Trova duplicati** deseleziona le ripetizioni, mentre **Convalida indirizzi** evidenzia le righe con un indirizzo mancante o con un CAP insolito.

## Aggiungere campi unione

Un campo unione è un segnaposto, come «Nome», che diventa un valore reale per ogni persona. Fare clic in una casella di testo (se non ce n'è una, JeffPub ne aggiunge una), quindi usare **Lettere > Scrivi e inserisci campi**:

- **Blocco di indirizzi**: un indirizzo postale composto da colonne con nomi come Nome, Cognome e Indirizzo riga 1.
- **Formula di apertura**: «Gentile» e il nome della persona, oppure «Gentile destinatario,» se il nome manca.
- **Inserisci campo unione**: scegliere una colonna qualsiasi.
- **Campo immagine**: scegliere una colonna con i nomi dei file immagine. JeffPub aggiunge un riquadro vuoto e cerca i file accanto all'elenco. Dopo aver salvato la pubblicazione e averla riaperta, un'immagine il cui file si trova fuori dalla cartella della pubblicazione non viene mostrata finché non si sceglie di nuovo l'elenco destinatari (**Lettere > Seleziona destinatari > Usa elenco esistente…**). Allora vengono mostrate le immagini presenti nella cartella dell'elenco e nelle cartelle al suo interno, fino alla chiusura della pubblicazione. JeffPub non ricorda questa scelta per la volta successiva. In alternativa, conservare i file immagine nella cartella della pubblicazione o in una cartella al suo interno: in questo caso appaiono non appena la pubblicazione viene aperta. Così una pubblicazione inviata da altri non legge immagini dal proprio computer senza autorizzazione. L'immagine di ogni record viene mostrata in anteprima dei risultati, alla stampa, al salvataggio in PDF o all'unione in una nuova pubblicazione o in un catalogo.

## Mostrare un numero, un prezzo, una data o un'ora nel formato desiderato

Un campo unione mostra il valore esattamente come è nell'elenco. Per cambiarlo, fare clic accanto al campo (o selezionarlo, oppure selezionarne più di uno) e fare clic su **Lettere > Scrivi e inserisci campi > Formato campo unione…**. In **Mostra il valore come** scegliere:

- **Numero**, con da 0 a 4 **Cifre decimali**. Ad esempio, 1234,5 con 1 cifra decimale appare come 1.234,5.
- **Valuta**, con il simbolo di valuta della propria area: 1234,5 appare come 1.234,50 € dove quello è il simbolo.
- **Percentuale**, dove 0,256 appare come 25,6% con 1 cifra decimale.
- **Data**, in uno **Stile** come Breve, Esteso, Anno, Mese, giorno, Mese e giorno o Mese e anno.
- **Ora**, in uno **Stile** Breve, a 12 ore o a 24 ore.

La riga **Esempio** mostra il risultato della scelta. I valori vengono letti e mostrati secondo la lingua del testo nel punto del campo (inglese americano, se non è impostata alcuna lingua), non secondo la lingua del computer. In inglese americano 03/04/2026 è il 4 marzo, mentre in una lingua che mette prima il giorno, come il tedesco o l'inglese britannico, è il 3 aprile; in tedesco, 1.234,5 è un numero con la virgola decimale. Un anno di due cifre viene letto come 2000-2029 per 00-29 e come 1930-1999 per 30-99. Un numero tra parentesi, come (500), è negativo. Un valore che non è un numero, una data o un'ora del tipo scelto viene mostrato esattamente come è nell'elenco: una parola, «nan» o «inf» (una persona può chiamarsi Nan), un valore vuoto, un'ora con un formato data o una data con un formato ora. L'anteprima e l'unione completata mostrano entrambe il valore formattato. La scelta costituisce un passaggio di **Annulla** ed è salvata con la pubblicazione.

## Visualizzare l'anteprima dei risultati

Fare clic su **Lettere > Anteprima risultati > Anteprima risultati** per vedere i dati reali al posto dei campi. Usare **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure **Trova destinatario** per saltare a una corrispondenza.

## Completare l'unione

Fare clic su **Lettere > Fine > Completa e unisci**:

- **Unisci in stampante…** apre la pagina **Stampa**. Selezionare **Stampa tutti i record della stampa unione**, quindi stampare.
- **Unisci in una nuova pubblicazione** crea una nuova pubblicazione con una copia delle pagine per ogni persona.
- **Unisci in PDF…** salva un unico PDF con le pagine di tutte le persone.
- **Unisci in posta elettronica…** crea un file e-mail .eml per ogni persona, con la prima pagina come immagine. Scegliere la colonna dell'e-mail, digitare un oggetto e selezionare una cartella. JeffPub non invia i messaggi: aprire i file con il proprio programma di posta.
- **Esporta elenco destinatari…** salva l'elenco come file CSV.

## Unione catalogo

Fare clic su **Inserisci > Pagine > Pagine catalogo** per aprire il riquadro **Unione catalogo**.

1. Scegliere l'elenco dei prodotti, come descritto sopra.
2. Fare clic su **Inserisci area catalogo**. Compare un riquadro con l'etichetta «Area catalogo». Impostare **Righe** e **Colonne**, oltre a posizione e dimensioni. Inserire i campi di una voce nella prima cella facendo doppio clic su di essi nell'elenco: si ripeteranno nelle altre celle. Per un'immagine, scegliere la versione «(immagine)» di un campo.
3. Visualizzare l'anteprima con le frecce. Ogni pagina mostra tante voci quante sono le celle dell'area. Quindi unire in una nuova pubblicazione, in un PDF o inviare alla stampante.

**Rimuovi area catalogo** elimina l'area.

## Vedi anche

- [Stampare una pubblicazione](printing)
- [Aggiungere e formattare le caselle di testo](text-boxes)
- [La scheda Lettere](tab-mailings)
