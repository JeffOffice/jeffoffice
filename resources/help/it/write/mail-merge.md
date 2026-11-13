# Stampa unione, buste ed etichette
<!-- keywords: stampa unione, unione, lettere, destinatari, elenco indirizzi, lettera tipo, etichette, buste, campo unione, blocco di indirizzi, formula di apertura, regole, se allora altrimenti, salta record, record successivo, anteprima risultati, completa e unisci, elenco, email, csv, xlsx, ods, vcard, codice a barre, unione email, indirizzi, modulo -->

Una **stampa unione** crea molte copie di una stessa lettera, ciascuna con i dati di una persona diversa. Si scrive la lettera una sola volta e si conservano i dati delle persone in un **elenco**. JeffWrite inserisce nome e indirizzo della prima persona nella prima copia, quelli della seconda persona nella seconda copia, e così via.

Gli strumenti si trovano nella scheda **Lettere**. Per una procedura guidata, fare clic su **Lettere > Avvia stampa unione > Procedura guidata passo passo per stampa unione**.

## Scegliere il tipo di documento

Fare clic su **Lettere > Avvia stampa unione** e scegliere una voce:

- **Lettere**: una copia della pagina per ogni persona. Ogni copia inizia su una nuova pagina.
- **Messaggi email**: un messaggio per ogni persona. Vedere "Inviare messaggi email" più avanti.
- **Buste** ed **Etichette**: una pagina preparata per l'uso. Vedere le sezioni seguenti.
- **Directory**: un unico elenco di tutte le persone, senza interruzioni di pagina tra una persona e l'altra. Usarlo per un elenco telefonico o un catalogo.
- **Cancella tipo di unione** riporta il documento a un normale documento.

JeffWrite memorizza nel file il tipo di unione e il nome dell'elenco al momento del salvataggio. Riaprendo il file, l'unione è ancora presente.

## Scegliere i destinatari

Un **destinatario** è una persona che riceve una copia. Fare clic su **Lettere > Seleziona destinatari**:

- **Digita un nuovo elenco**: digitare le persone nella finestra che si apre. Fare clic su **Nuova voce** per ciascuna. JeffWrite propone quindi di salvare l'elenco come file .csv.
- **Usa un elenco esistente**: scegliere un file che è già un elenco. Può essere un file .csv o un file di testo con tabulazioni (una riga per ogni persona, e la prima riga contiene i nomi delle colonne), un foglio di calcolo (.xlsx, .xls o .ods, creato con JeffCrunch o con un altro programma), oppure un file di contatti (.vcf).
- **Seleziona dai contatti**: scegliere un file di contatti (.vcf). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato.

Poi fare clic su **Lettere > Modifica elenco destinatari** per controllare l'elenco:

- Deselezionare una riga per escludere quella persona.
- Fare clic su un'intestazione di colonna per ordinare.
- Digitare in **Filtra destinatari** per mostrare solo le righe che contengono le parole indicate.
- **Trova duplicati** deseleziona le ripetizioni.
- **Convalida indirizzi** evidenzia in colore le righe senza indirizzo o con un CAP che sembra errato.

## Inserire i dati nella lettera

Un **campo unione** è un segnaposto. Mostra il nome di una colonna tra virgolette basse a caporale, ad esempio «Nome». In ogni copia viene sostituito con il dato di quella persona.

Fare clic nel punto in cui va il dato, quindi usare **Lettere > Scrivi e inserisci campi**:

- **Inserisci campo unione** elenca le colonne dell'elenco. Scegliere una colonna.
  - **Codice a barre dal campo**, nello stesso elenco, inserisce un codice a barre il cui valore è una colonna, ad esempio un codice articolo o l'ISBN a 13 cifre di un libro. Scegliere la colonna, poi il tipo, e controllare nella finestra **Inserisci codice a barre** (quanto si digita lì è solo un esempio). La finestra offre solo ciò che un campo può contenere: il tipo, le dimensioni, l'altezza delle barre, le cifre e il livello QR. Non propone il prezzo di un libro, il codice aggiuntivo, la cifra di controllo né lo sfondo bianco, che un campo non può specificare; l'ISBN di un libro è un EAN-13. Nel documento il campo mostra il nome della colonna tra virgolette basse a caporale. Durante la stampa unione, la copia di ogni persona riceve un codice a barre con il valore proprio di quella persona, disegnato come immagine nella riga, quindi viene stampato e salvato in PDF come qualsiasi immagine. Il valore è solo un dato: virgolette o barre inverse al suo interno non cambiano nulla. Se un valore è vuoto, non viene creato alcun codice a barre. Se un codice non può contenere il valore (una lettera in un EAN-13, o più di 500 caratteri in Code 128), la copia mostra al suo posto un breve messaggio tra parentesi quadre, senza influire sulle altre copie. Una stampa unione crea al massimo 2.000 codici a barre, contando anche i valori che non possono essere rappresentati; se viene segnalato che sono troppi, unire meno persone. In un file `.docx` salvato, il campo è un campo DISPLAYBARCODE che contiene la colonna, quindi viene conservato al salvataggio e alla riapertura del file. Un campo con un valore digitato a mano, o di un tipo che JeffWrite non disegna (UPC-E, per esempio), resta invariato. Nelle stampe unione per email il valore viene inserito come testo.
- **Blocco di indirizzi** inserisce un indirizzo postale completo: nome, azienda, via e città. Scegliere le righe da includere. Una riga vuota viene omessa. È anche possibile indicare un paese da omettere, come il proprio. **Anteprima** mostra l'indirizzo della prima persona.
- **Formula di apertura** inserisce una riga come "Gentile dott.ssa Fenwick,". Si scelgono le parole. La prima scelta, "Gentile dott.ssa Fenwick,", usa il titolo della persona. Se una persona non ha un titolo, usa nome e cognome, come in "Gentile Nora Pell,". Se una persona non ha un nome, la formula di apertura riporta un'altra frase, come "Gentile Signore/Signora,", che è possibile modificare.
- **Associa campi** indica al blocco di indirizzi e alla formula di apertura quale colonna contiene ciascuna parte, quando i nomi delle colonne sono insoliti.

## Regole

Una **regola** rende diversa la copia per alcune persone. Fare clic su **Regole** e scegliere una voce:

- **Se…Allora…Altrimenti**: mostra un testo quando una colonna ha un valore e un altro testo quando non ce l'ha. Ad esempio, se Provincia è "MI", scrivere "Benvenuto a casa".
- **Salta record se**: esclude una persona quando una colonna ha un valore, ad esempio quando il cognome è vuoto.
- **Record successivo** e **Record successivo se**: usano i dati della persona successiva nello stesso punto. Le etichette usano questa regola per inserire una persona diversa in ogni etichetta.
- **Numero record di unione** inserisce il numero della riga della persona nell'elenco. **Numero sequenza di unione** conta le copie create finora.
- **Chiedi** e **Campo da compilare**: chiedono una risposta al termine della stampa unione. Chiedi conserva la risposta con un nome. Campo da compilare inserisce la risposta nel testo.
- **Imposta segnalibro**: assegna un valore a un nome. Un riferimento incrociato al nome mostra il valore.

## Visualizzare l'anteprima dei risultati

Fare clic su **Lettere > Anteprima risultati**. Si apre un riquadro che mostra la pagina per una persona. Usare **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure digitare un numero. **Trova destinatario** passa alla persona successiva che ha le parole indicate in uno dei suoi dati.

**Evidenzia campi unione** colora in grigio i campi unione della pagina, così da vederne la posizione. Gli altri campi, come i numeri di pagina, non vengono evidenziati. L'evidenziazione si vede sullo schermo e non viene mai stampata.

Il documento originale non viene modificato. **Verifica errori** segnala quando l'elenco non ha una colonna per un campo, oppure quando non è incluso nessun destinatario.

## Completare la stampa unione

Fare clic su **Lettere > Completa e unisci**:

- **Modifica documenti singoli** crea un nuovo documento con la copia di ogni persona. È possibile modificarlo prima di stamparlo.
- **Stampa documenti** crea tale documento e apre la pagina **Stampa**.
- **Salva come PDF** salva tutte le copie in un unico file PDF.
- **Invia messaggi di posta elettronica** crea un file di messaggio .eml per ogni persona, in una cartella scelta. Scegliere la colonna che contiene gli indirizzi email e digitare un oggetto. JeffWrite non invia i messaggi: aprire i file con il proprio programma di posta.

Le persone deselezionate nell'elenco vengono escluse.

Un file di messaggio viene creato solo per una persona il cui indirizzo è un unico indirizzo email. Se un indirizzo ne contiene due (con una virgola o un punto e virgola tra i due), include un nome oppure non ha un dominio, quella persona non riceve alcun messaggio, e JeffWrite ne indica il numero. Un oggetto lungo viene salvato in più parti, che i programmi di posta ricompongono.

Durante la stampa unione compare una finestra di avanzamento, e **Annulla** la interrompe. Se una regola pone una domanda (Chiedi o Campo da compilare), viene chiesta ciascuna domanda diversa una sola volta, fino a 20 domande. Le altre usano poi le risposte predefinite. Se si annulla una domanda, l'intera stampa unione si interrompe. La stampa unione si arresta, segnalandolo, se produrrebbe più di 10.000 pagine, così un documento molto lungo con un elenco molto esteso non richiede ore. In questo caso, unire una parte dell'elenco alla volta.

## Buste

Fare clic su **Lettere > Buste**. Digitare l'indirizzo del destinatario e il proprio indirizzo di mittente, quindi scegliere il formato della busta dall'elenco. I formati sono indicati con larghezza e altezza. Fare clic su **Aggiungi al documento** per inserire la busta su una pagina a sé all'inizio del documento, oppure su **Stampa** per stamparla. La pagina ha la larghezza del lato lungo della busta, che è il modo in cui la maggior parte delle stampanti accetta le buste.

Per creare buste per una stampa unione, fare clic su **Avvia stampa unione > Buste**. Ogni busta contiene un blocco di indirizzi.

## Etichette

Fare clic su **Lettere > Etichette**. Digitare l'indirizzo e scegliere il foglio dall'elenco. Un foglio è indicato dal numero di etichette che contiene e dalle dimensioni di ciascuna. Se il foglio non compare nell'elenco, scegliere **Foglio personalizzato** e digitare le misure: quante etichette stanno in orizzontale e in verticale, le dimensioni di un'etichetta, lo spazio sopra e a lato della prima etichetta, e la distanza tra le etichette.

- **Pagina intera della stessa etichetta** inserisce lo stesso indirizzo su ogni etichetta del foglio.
- **Singola etichetta** inserisce l'indirizzo su una sola etichetta, così da utilizzare un foglio già parzialmente usato. Indicare riga e colonna.

Fare clic su **Nuovo documento** per creare un documento da visualizzare e modificare, oppure su **Stampa** per stamparlo.

Per creare etichette per una stampa unione, fare clic su **Avvia stampa unione > Etichette**. La prima etichetta contiene un blocco di indirizzi, e ogni altra etichetta inizia con una regola **Record successivo**. Se si modifica la prima etichetta, fare clic su **Aggiorna etichette** per copiare la modifica in tutte le altre.

## Cosa contiene il file

Il tipo di unione, il nome e il percorso dell'elenco, l'oggetto e la colonna degli indirizzi email vengono salvati nel file .docx. Le persone deselezionate e le scelte di Associa campi non vengono salvate: vengono richieste di nuovo dopo l'apertura del file. Un elenco viene letto di nuovo dal suo file ogni volta. Se il file non si trova più nella posizione originaria, JeffWrite cerca accanto al documento e, se non lo trova, chiede di sceglierlo.

Un elenco può contenere fino a 1.000 colonne e 4.000.000 di celle in totale, e una stampa unione usa le prime 100.000 persone. Se un file ne contiene di più, JeffWrite usa quelle che rientrano nel limite e lo segnala. Le righe vuote vengono escluse dall'elenco.

## Vedi anche

- [Usare i campi](fields)
- [Stampa](print)
- [Creare e modificare tabelle](tables)
