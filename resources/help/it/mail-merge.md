# Stampa unione e unione catalogo
<!-- keywords: stampa unione, unione, unisci, destinatari, elenco indirizzi, etichette, lettere tipo, lettere circolari, csv, xlsx, vcard, campo unione, blocco di indirizzi, catalogo, elenco prodotti, unione posta elettronica, e-mail, anteprima -->

La stampa unione compila una pubblicazione con i dati di un elenco. Si scrive una lettera una sola volta e JeffPub ne crea una copia per ogni persona, inserendo il nome e l'indirizzo di quella persona. L'unione catalogo ripete uno stesso layout per ogni elemento di un elenco.

Gli strumenti si trovano nella scheda **Lettere**. Per una procedura guidata, fare clic su **Lettere > Inizia > Stampa unione > Creazione guidata stampa unione** per aprire il riquadro **Stampa unione**.

## Scegliere i destinatari

Fare clic su **Lettere > Inizia > Seleziona destinatari**:

- **Usa un elenco esistente…**: selezionare un file CSV, un file di testo delimitato da tabulazioni, un foglio di calcolo .xlsx (viene usato il primo foglio) o un file di contatti .vcf. In un file CSV, di testo o di foglio di calcolo, la prima riga deve contenere i nomi delle colonne.
- **Seleziona dai contatti…**: selezionare un file di contatti (.vcf o .vcard). La maggior parte delle rubriche e dei telefoni può salvare i contatti in un file di questo tipo; ogni contatto diventa un destinatario, con le colonne nome, indirizzo, telefono e indirizzo di posta elettronica.
- **Digita un nuovo elenco…**: compilare la finestra **Nuovo elenco indirizzi**, usando **Nuova voce** per ogni persona. JeffPub propone quindi di salvare l'elenco come file CSV.

Per controllare l'elenco, fare clic su **Lettere > Inizia > Modifica elenco destinatari**. Nella finestra **Destinatari stampa unione**, deselezionare una riga per escludere quella persona, fare clic su un'intestazione per ordinare e digitare in **Filtra destinatari** per restringere l'elenco. **Trova duplicati** deseleziona le voci ripetute e **Convalida indirizzi** evidenzia le righe con un indirizzo mancante o un codice postale anomalo.

## Aggiungere campi unione

Un campo unione è un segnaposto, ad esempio «Nome», che viene sostituito da un valore reale per ogni persona. Fare clic in una casella di testo (se non lo si fa, JeffPub ne aggiunge una), quindi usare **Lettere > Scrivi e inserisci campi**:

- **Blocco di indirizzi**: un indirizzo postale composto da colonne con nomi come Nome, Cognome e Indirizzo riga 1.
- **Formula di apertura**: "Caro" seguito dal nome della persona oppure "Caro amico," se il nome manca.
- **Inserisci campo unione**: consente di scegliere una colonna qualsiasi.
- **Campo immagine**: consente di scegliere una colonna contenente nomi di file immagine. JeffPub aggiunge una cornice vuota e cerca i file nella stessa cartella dell'elenco. Dopo aver salvato e riaperto la pubblicazione, un'immagine il cui file si trova fuori dalla cartella della pubblicazione non viene visualizzata finché non si seleziona di nuovo l'elenco destinatari (**Lettere > Seleziona destinatari > Usa un elenco esistente…**). Da quel momento le immagini presenti nella cartella dell'elenco e nelle relative sottocartelle vengono visualizzate fino alla chiusura della pubblicazione. JeffPub non memorizza questa impostazione per la volta successiva. In alternativa, tenere i file immagine nella cartella della pubblicazione o in una sua sottocartella: verranno visualizzati non appena la pubblicazione viene aperta. In questo modo una pubblicazione ricevuta da altri non può leggere di propria iniziativa immagini dal computer. L'immagine di ciascun record viene visualizzata quando si visualizza l'anteprima dei risultati, si stampa, si salva un PDF o si esegue l'unione in una nuova pubblicazione o in un catalogo.

## Visualizzare l'anteprima dei risultati

Fare clic su **Lettere > Anteprima risultati > Anteprima risultati** per visualizzare i dati reali al posto dei campi. Usare **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone oppure **Trova destinatario** per passare a una corrispondenza.

## Completare l'unione

Fare clic su **Lettere > Fine > Completa e unisci**:

- **Unisci in stampante…** apre la pagina **Stampa**. Selezionare **Stampa tutti i record della stampa unione**, quindi stampare.
- **Unisci in una nuova pubblicazione** crea una nuova pubblicazione con una copia delle pagine per ogni persona.
- **Unisci in PDF…** salva un unico PDF con le pagine di tutte le persone.
- **Unisci in posta elettronica…** crea un file di posta elettronica .eml per ogni persona, con la prima pagina come immagine. Scegliere la colonna dell'indirizzo di posta elettronica, digitare un oggetto e selezionare una cartella. JeffPub non invia i messaggi: aprire i file nel programma di posta elettronica.
- **Esporta elenco destinatari…** salva l'elenco come file CSV.

## Unione catalogo

Fare clic su **Inserisci > Pagine > Pagine catalogo** per aprire il riquadro **Unione catalogo**.

1. Scegliere l'elenco prodotti, come descritto sopra.
2. Fare clic su **Inserisci area catalogo**. Viene visualizzato un riquadro con l'etichetta "Area catalogo". Impostarne **Righe** e **Colonne**, oltre a posizione e dimensioni. Inserire i campi di un elemento nella prima cella facendo doppio clic su di essi nell'elenco. I campi vengono ripetuti nelle altre celle. Per un'immagine, scegliere la versione "(immagine)" del campo.
3. Visualizzare l'anteprima con le frecce. Ogni pagina mostra tanti elementi quante sono le celle dell'area. Quindi eseguire l'unione in una nuova pubblicazione, in un PDF o nella stampante.

**Rimuovi area catalogo** elimina l'area.

## Argomenti correlati

- [Stampare una pubblicazione](printing)
- [Aggiungere e formattare caselle di testo](text-boxes)
- [La scheda Lettere](tab-mailings)
