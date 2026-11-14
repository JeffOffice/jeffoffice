# Stampa unione, buste ed etichette
<!-- keywords: stampa unione, unione, lettere, destinatari, elenco indirizzi, lettera tipo, etichette, buste, campo unione, blocco di indirizzi, formula di apertura, regole, se allora altrimenti, salta record, record successivo, anteprima risultati, completa e unisci, catalogo, messaggi di posta elettronica, csv, xlsx, ods, vcard, stampa unione, unione posta elettronica, elenco destinatari, campo, unisci -->

Una **stampa unione** crea molte copie di una stessa lettera, ciascuna con i dati di una persona diversa. Si scrive la lettera una sola volta. I dati delle persone si conservano in un **elenco**. JeffWrite inserisce nome e indirizzo della prima persona nella prima copia, quelli della seconda nella seconda copia, e così via.

Gli strumenti si trovano nella scheda **Lettere**. Per una procedura guidata, fare clic su **Lettere > Avvia stampa unione > Procedura guidata passo passo per stampa unione**.

## Scegliere che cosa creare

Fare clic su **Lettere > Avvia stampa unione** e scegliere una delle opzioni:

- **Lettere**: una copia della pagina per ogni persona. Ogni copia inizia su una nuova pagina.
- **Messaggi di posta elettronica**: un messaggio per ogni persona. Vedere "Inviare messaggi di posta elettronica" più avanti.
- **Buste** ed **Etichette**: una pagina preparata per loro. Vedere le sezioni seguenti.
- **Catalogo**: un unico elenco di tutte le persone, senza interruzioni di pagina tra una persona e l'altra. Utile per un elenco telefonico o un catalogo.
- **Rimuovi tipo di unione** riporta il documento a un normale documento.

JeffWrite salva nel file il tipo di unione e il nome dell'elenco. Riaprendo il file, la stampa unione è ancora presente.

## Scegliere i destinatari

Un **destinatario** è una persona che riceve una copia. Fare clic su **Lettere > Seleziona destinatari**:

- **Digita un nuovo elenco**: digitare le persone nella finestra che si apre. Fare clic su **Nuova voce** per ciascuna. JeffWrite propone poi di salvare l'elenco come file .csv.
- **Usa elenco esistente**: scegliere un file che è già un elenco. Può essere un file .csv o un file di testo con tabulazioni (una riga per ogni persona, e la prima riga contiene i nomi delle colonne), un foglio di calcolo (.xlsx, .xls o .ods, da JeffCrunch o da un altro programma), oppure un file di contatti (.vcf).
- **Seleziona dai contatti**: scegliere un file di contatti (.vcf). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato.

Poi fare clic su **Lettere > Modifica elenco destinatari** per controllare l'elenco:

- Deselezionare una riga per escludere quella persona.
- Fare clic su un'intestazione di colonna per ordinare.
- Digitare in **Filtra destinatari** per mostrare solo le righe che contengono le parole indicate.
- **Trova duplicati** deseleziona le ripetizioni.
- **Convalida indirizzi** evidenzia le righe senza indirizzo o con un CAP che sembra errato.

## Inserire i dati nella lettera

Un **campo unione** è un segnaposto. Mostra il nome di una colonna tra doppie virgolette basse, ad esempio «Nome». In ogni copia, il campo viene sostituito con il dato della persona.

Fare clic dove deve andare il dato, quindi usare **Lettere > Scrivi e inserisci campi**:

- **Inserisci campo unione** elenca le colonne del proprio elenco. Sceglierne una.
  - **Codice a barre da campo**, nello stesso elenco, inserisce un codice a barre il cui valore è una colonna, ad esempio un codice articolo o l'ISBN a 13 cifre di un libro. Scegliere la colonna, quindi il tipo, e controllare nella finestra **Inserisci codice a barre** (quanto vi si digita è solo un esempio). La finestra offre solo ciò che un campo può contenere: il tipo, le dimensioni, l'altezza delle barre, le cifre e il livello QR. Non propone il prezzo di un libro, il codice aggiuntivo, la cifra di controllo né lo sfondo bianco, che un campo non può specificare; l'ISBN di un libro è un EAN-13. Nel documento il campo appare come un codice a barre, di esempio per il tipo scelto fino alla stampa unione; per cambiare tipo o aspetto, fare doppio clic sul campo oppure fare clic destro e scegliere **Modifica codice a barre**. Alla stampa unione, la copia di ogni persona riceve un codice a barre con il proprio valore, disegnato come immagine nella riga, quindi viene stampato e salvato in PDF come qualsiasi immagine. Il valore è solo un dato: virgolette o barre rovesciate al suo interno non cambiano nulla. Se un valore è vuoto, non viene creato alcun codice a barre. Se un codice non può contenere il valore (una lettera in un EAN-13, o più di 500 caratteri in Code 128), la copia mostra al suo posto un breve messaggio tra parentesi quadre, senza influire sulle altre copie. Una stampa unione crea al massimo 2.000 codici a barre, contando anche i valori che non possono essere rappresentati; se viene segnalato un numero eccessivo, unire un numero minore di persone. In un file `.docx` salvato, il campo è un campo DISPLAYBARCODE che contiene la colonna, quindi viene conservato al salvataggio e alla riapertura del file. Un campo con un valore digitato a mano, o di un tipo che JeffWrite non disegna (UPC-E, ad esempio), resta invariato. Nelle unioni per posta elettronica il valore viene inserito come testo.
- **Blocco di indirizzi** inserisce un intero indirizzo postale: nome, società, via e città. Scegliere le righe desiderate. Una riga vuota viene omessa. È anche possibile indicare un paese da omettere, ad esempio il proprio. **Anteprima** mostra l'indirizzo della prima persona.
- **Formula di apertura** inserisce una riga come "Gentile Dott.ssa Fenwick,". Si scelgono le parole. La prima opzione, "Gentile Dott.ssa Fenwick,", usa il titolo della persona. Se una persona non ha un titolo, usa nome e cognome, come in "Gentile Nora Pell,". Se una persona non ha un nome, la formula di apertura dice altro, ad esempio "Gentili Signore e Signori,", che si può modificare.
- **Associa campi** indica al blocco di indirizzi e alla formula di apertura quale colonna contiene ciascuna parte, quando i nomi delle colonne non sono quelli standard.

## Regole

Una **regola** rende diversa una copia per alcune persone. Fare clic su **Regole** e scegliere:

- **Se…Allora…Altrimenti**: mostra un testo quando una colonna contiene un valore, e un altro testo quando non lo contiene. Ad esempio, se Provincia è "MI", scrivi "Benvenuti a casa".
- **Salta record se**: esclude una persona quando una colonna contiene un valore, ad esempio quando il cognome è vuoto.
- **Record successivo** e **Record successivo se**: usano i dati della persona successiva nello stesso punto. Le etichette usano questa regola per mettere una persona diversa su ciascuna etichetta.
- **Numero record di unione** inserisce il numero della riga della persona nell'elenco. **Numero sequenza di unione** conta le copie create finora.
- **Chiedi** e **Campo da compilare**: chiedono una risposta al termine dell'unione. Chiedi conserva la risposta con un nome. Campo da compilare inserisce la risposta nel testo.
- **Imposta segnalibro**: assegna un valore a un nome. Un riferimento incrociato a quel nome mostra il valore.

## Visualizzare l'anteprima dei risultati

Fare clic su **Lettere > Anteprima risultati**. Si apre un riquadro che mostra la pagina per una persona. Usare **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure digitare un numero. **Trova destinatario** passa alla persona successiva che ha le parole indicate in un dato.

**Evidenzia campi unione** ombreggia in grigio i campi unione nella pagina, per vedere dove si trovano. Gli altri campi, come i numeri di pagina, non vengono ombreggiati. L'ombreggiatura si vede sullo schermo e non viene mai stampata.

Il documento originale non viene modificato. **Verifica errori** segnala quando l'elenco non ha una colonna per un campo, oppure quando nessuna persona è inclusa.

## Completare l'unione

Fare clic su **Lettere > Completa e unisci**:

- **Modifica documenti singoli** crea un nuovo documento con la copia di ogni persona. È possibile modificarlo prima di stamparlo.
- **Stampa documenti** crea quel documento e apre la pagina **Stampa**.
- **Salva come PDF** salva tutte le copie in un unico file PDF.
- **Invia messaggi di posta elettronica** crea un file di messaggio .eml per ogni persona, in una cartella scelta. Scegliere la colonna con gli indirizzi di posta elettronica e digitare un oggetto. JeffWrite non invia i messaggi: aprire i file con il proprio programma di posta.

Le persone deselezionate nell'elenco vengono escluse.

Un file di messaggio viene creato solo per una persona il cui indirizzo è un unico indirizzo di posta elettronica. Se un indirizzo ne contiene due (separati da virgola o punto e virgola), include un nome oppure non ha un dominio, quella persona non riceve alcun messaggio e JeffWrite ne indica il numero. Un oggetto lungo viene salvato in più parti che i programmi di posta ricompongono.

Durante un'unione compare una finestra di avanzamento, e **Annulla** la interrompe. Se una regola pone una domanda (Chiedi o Campo da compilare), viene chiesta una volta ciascuna domanda diversa, fino a 20. Le altre usano le risposte predefinite. Se si annulla una domanda, l'intera unione si interrompe. L'unione si arresta, segnalandolo, se produrrebbe più di 10.000 pagine, così un documento molto lungo con un elenco molto lungo non richiede ore. In tal caso, unire una parte dell'elenco alla volta.

## Buste

Fare clic su **Lettere > Buste**. Digitare l'indirizzo del destinatario e il proprio indirizzo di mittente, quindi scegliere il formato della busta dall'elenco. I formati sono indicati con la larghezza e l'altezza. Fare clic su **Aggiungi al documento** per inserire la busta in una pagina propria all'inizio del documento, oppure su **Stampa** per stamparla. La pagina ha la larghezza del lato lungo della busta, che è il modo in cui la maggior parte delle stampanti accetta le buste.

Per creare buste per una stampa unione, fare clic su **Avvia stampa unione > Buste**. Ogni busta contiene un blocco di indirizzi.

## Etichette

Fare clic su **Lettere > Etichette**. Digitare l'indirizzo e scegliere il foglio dall'elenco. Un foglio è indicato con il numero di etichette che contiene e con le dimensioni di ciascuna. Se il foglio non è nell'elenco, scegliere **Foglio personalizzato** e digitare le misure: quante etichette stanno in orizzontale e in verticale, le dimensioni di un'etichetta, lo spazio sopra e a lato della prima etichetta, e la distanza tra le etichette.

- **Pagina intera della stessa etichetta** inserisce lo stesso indirizzo su ogni etichetta del foglio.
- **Singola etichetta** inserisce l'indirizzo su una sola etichetta, per utilizzare un foglio già parzialmente usato. Indicare riga e colonna.

Fare clic su **Nuovo documento** per creare un documento da visualizzare e modificare, oppure su **Stampa** per stamparlo.

Per creare etichette per una stampa unione, fare clic su **Avvia stampa unione > Etichette**. La prima etichetta contiene un blocco di indirizzi, e ogni altra etichetta inizia con una regola **Record successivo**. Se si modifica la prima etichetta, fare clic su **Aggiorna etichette** per copiare la modifica su tutte le altre.

## Che cosa contiene il file

Il tipo di unione, il nome e la posizione dell'elenco, l'oggetto e la colonna degli indirizzi di posta elettronica sono salvati nel file .docx. Le persone deselezionate e le scelte di **Associa campi** non vengono salvate: vengono richieste di nuovo dopo l'apertura del file. L'elenco viene riletto ogni volta dal proprio file. Se il file non si trova più nella posizione originale, JeffWrite cerca accanto al documento e poi chiede di sceglierlo.

Un elenco può avere fino a 1.000 colonne e 4.000.000 di celle in totale, e una stampa unione usa le prime 100.000 persone. Se un file ne contiene di più, JeffWrite usa ciò che rientra nel limite e lo segnala. Le righe vuote vengono omesse dall'elenco.

## Vedere anche

- [Usare i campi](fields)
- [Stampare](print)
- [Creare e modificare tabelle](tables)
