# Stampa unione, buste ed etichette
<!-- keywords: stampa unione, unione, lettere, destinatari, elenco indirizzi, lettera tipo, etichette, buste, campo unione, blocco di indirizzi, formula di apertura, regole, se allora altrimenti, salta record, record successivo, anteprima risultati, completa e unisci, catalogo, messaggi di posta elettronica, csv, xlsx, ods, vcard, unione, stampa unione, lettera, mail merge -->

Una **stampa unione** crea molte copie di una stessa lettera, ciascuna con i dati di una persona diversa. Scrivi la lettera una sola volta. Conservi i dati delle persone in un **elenco**. JeffWrite inserisce nome e indirizzo della prima persona nella prima copia, quelli della seconda persona nella seconda copia, e così via.

Gli strumenti si trovano nella scheda **Lettere**. Per un percorso guidato, fai clic su **Lettere > Avvia stampa unione > Procedura guidata passo passo per stampa unione**.

## Scegli cosa creare

Fai clic su **Lettere > Avvia stampa unione** e scegli una delle opzioni:

- **Lettere**: una copia della pagina per ogni persona. Ogni copia inizia su una nuova pagina.
- **Messaggi di posta elettronica**: un messaggio per ogni persona. Vedi "Inviare messaggi di posta elettronica" più avanti.
- **Buste** ed **Etichette**: una pagina preparata per loro. Vedi le sezioni seguenti.
- **Catalogo**: un unico elenco di tutte le persone, senza interruzioni di pagina tra una persona e l'altra. Usalo per un elenco telefonico o un catalogo.
- **Cancella tipo di unione** riporta il documento a un normale documento.

JeffWrite salva nel file il tipo di unione e il nome dell'elenco. Quando riapri il file, l'unione è ancora presente.

## Scegli i destinatari

Un **destinatario** è una persona che riceve una copia. Fai clic su **Lettere > Seleziona destinatari**:

- **Digita un nuovo elenco**: digita le persone nella finestra che si apre. Fai clic su **Nuova voce** per ciascuna. JeffWrite ti propone poi di salvare l'elenco come file .csv.
- **Usa un elenco esistente**: scegli un file che è già un elenco. Può essere un file .csv o un file di testo con tabulazioni (una riga per ogni persona, e la prima riga contiene i nomi delle colonne), un foglio di calcolo (.xlsx, .xls o .ods, creato con JeffCrunch o con un altro programma), oppure un file di contatti (.vcf).
- **Seleziona dai contatti**: scegli un file di contatti (.vcf). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato.

Poi fai clic su **Lettere > Modifica elenco destinatari** per controllare l'elenco:

- Deseleziona una riga per escludere quella persona.
- Fai clic su un'intestazione di colonna per ordinare.
- Digita in **Filtra destinatari** per mostrare solo le righe che contengono le tue parole.
- **Trova duplicati** deseleziona le ripetizioni.
- **Convalida indirizzi** colora le righe senza indirizzo o con un CAP che sembra errato.

## Inserisci i dati nella lettera

Un **campo unione** è un segnaposto. Mostra il nome di una colonna tra virgolette doppie a sinistra e a destra, come «Nome». In ogni copia viene sostituito con il dato di quella persona.

Fai clic dove deve andare il dato, quindi usa **Lettere > Scrivi e inserisci campi**:

- **Inserisci campo unione** elenca le colonne del tuo elenco. Scegline una.
  - **Codice a barre da campo**, nello stesso elenco, inserisce un codice a barre il cui valore è una colonna, ad esempio un codice articolo o l'ISBN a 13 cifre di un libro. Scegli la colonna, quindi il tipo, e controlla nella finestra **Inserisci codice a barre** (quello che digiti lì è solo un esempio). La finestra offre solo ciò che un campo può contenere: il tipo, le dimensioni, l'altezza delle barre, le cifre e il livello QR. Non include il prezzo di un libro, un codice aggiuntivo, un carattere di controllo né lo sfondo chiaro, che un campo non può specificare; l'ISBN di un libro è un EAN-13. Nel documento il campo viene disegnato come codice a barre, con un esempio del tipo scelto finché non esegui l'unione; per cambiare tipo o aspetto, fai doppio clic su di esso oppure fai clic destro e scegli **Modifica codice a barre**. Quando esegui l'unione, la copia di ogni persona riceve un codice a barre con il valore proprio di quella persona, disegnato come immagine nella riga, così viene stampato e salvato in PDF come qualsiasi immagine. Il valore è solo un dato: virgolette o barre rovesciate al suo interno non cambiano nulla. Se un valore è vuoto, non viene creato alcun codice a barre. Se un codice non può contenere un valore (una lettera in un EAN-13, o più di 500 caratteri in Code 128), la copia mostra al suo posto un breve messaggio tra parentesi quadre, senza influire sulle altre copie. Una unione crea al massimo 2.000 codici a barre, contando anche i valori che non è possibile rappresentare; se viene segnalato che sono troppi, unisci meno persone. In un file .docx salvato, il campo è un campo DISPLAYBARCODE che contiene la colonna, quindi viene conservato quando salvi e riapri il file. Un campo con un valore digitato direttamente, o di un tipo che JeffWrite non disegna (UPC-E, ad esempio), resta così com'è. Le unioni per posta elettronica inseriscono il valore come testo.
- **Blocco di indirizzi** inserisce un intero indirizzo postale: nome, azienda, via e città. Scegli le righe che ti servono. Una riga senza dati viene omessa. Puoi anche indicare un paese da omettere, ad esempio il tuo. **Anteprima** mostra l'indirizzo della prima persona.
- **Formula di apertura** inserisce una riga come "Gentile dott. Fenwick,". Scegli tu le parole. La prima scelta, "Gentile dott. Fenwick,", usa il titolo della persona. Se una persona non ha un titolo, usa invece nome e cognome, come in "Gentile Nora Pell,". Se una persona non ha un nome, la formula di apertura dice qualcos'altro, ad esempio "Gentile Signore/Signora,", che puoi modificare.
- **Abbina campi** indica al blocco di indirizzi e alla formula di apertura quale colonna contiene ciascuna parte, quando i nomi delle colonne sono insoliti.

## Regole

Una **regola** rende diversa la copia per alcune persone. Fai clic su **Regole** e scegline una:

- **Se…Allora…Altrimenti**: mostra un testo quando una colonna ha un valore e un altro testo quando non ce l'ha. Ad esempio, se Provincia è "MI", scrivi "Benvenuto a casa".
- **Salta record se**: escludi una persona quando una colonna soddisfa una condizione che imposti, ad esempio quando il cognome è vuoto.
- **Record successivo** e **Record successivo se**: usa i dati della persona successiva nello stesso punto. Le etichette usano questa regola per inserire una persona diversa in ogni etichetta.
- **Numero record unione** inserisce il numero della riga della persona nell'elenco. **Numero sequenza unione** conta le copie create finora.
- **Chiedi** e **Campo da compilare**: ti chiedono una risposta al termine della unione. Chiedi conserva la risposta con un nome. Campo da compilare inserisce la risposta nel testo.
- **Imposta segnalibro**: assegna a un nome un valore. Un riferimento incrociato al nome mostra il valore.

## Controlla i risultati

Fai clic su **Lettere > Anteprima risultati**. Si apre un riquadro che mostra la pagina per una persona. Usa **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure digita un numero. **Trova destinatario** salta alla persona successiva che ha le parole indicate in uno dei suoi dati.

**Evidenzia campi unione** colora di grigio i campi unione nella pagina, così puoi vedere dove si trovano. Gli altri campi, come i numeri di pagina, non vengono colorati. L'evidenziazione compare sullo schermo e non viene mai stampata.

Il tuo documento non viene modificato. **Verifica errori** ti avvisa quando l'elenco non ha una colonna per un campo, oppure quando non è incluso nessuno.

## Completa la unione

Fai clic su **Lettere > Completa e unisci**:

- **Modifica documenti singoli** crea un nuovo documento con la copia di ogni persona. Puoi modificarlo prima di stamparlo.
- **Stampa documenti** crea quel documento e apre la pagina **Stampa**.
- **Salva come PDF** salva tutte le copie in un unico file PDF.
- **Invia messaggi di posta elettronica** crea un file di messaggio .eml per ogni persona, in una cartella che scegli. Scegli la colonna che contiene gli indirizzi di posta elettronica e digita un oggetto. JeffWrite non invia i messaggi: apri i file con il tuo programma di posta.

Le persone che hai deselezionato nell'elenco vengono escluse.

Un file di messaggio viene creato solo per una persona il cui indirizzo è un unico indirizzo di posta elettronica. Se un indirizzo ne contiene due (separati da una virgola o da un punto e virgola), include un nome, oppure non ha un dominio, quella persona non riceve alcun messaggio, e JeffWrite te ne indica il numero. Un oggetto lungo viene salvato in più parti, che i programmi di posta ricompongono.

Durante una unione compare una finestra di avanzamento, e **Annulla** la interrompe. Se una regola pone una domanda (Chiedi o Campo da compilare), ti viene chiesta ogni domanda diversa una sola volta, fino a 20 domande. Dopo, le altre usano le risposte predefinite. Se annulli una domanda, l'intera unione si interrompe. Una unione si arresta, avvisandoti, se produrrebbe più di 10.000 pagine, così un documento molto lungo con un elenco molto lungo non richiede ore. Esegui l'unione su una parte dell'elenco alla volta.

## Buste

Fai clic su **Lettere > Buste**. Digita l'indirizzo del destinatario e il tuo indirizzo di mittente, quindi scegli il formato della busta dall'elenco. I formati sono indicati con la larghezza e l'altezza. Fai clic su **Aggiungi al documento** per inserire la busta in una pagina separata all'inizio del documento, oppure su **Stampa** per stamparla. La pagina ha la larghezza del lato lungo della busta, che è il modo in cui la maggior parte delle stampanti alimenta una busta.

Per creare buste per una unione, fai clic su **Avvia stampa unione > Buste**. Ogni busta contiene un blocco di indirizzi.

## Etichette

Fai clic su **Lettere > Etichette**. Digita l'indirizzo e scegli il foglio dall'elenco. Un foglio è indicato dal numero di etichette che contiene e dalle dimensioni di ciascuna. Se il tuo foglio non è nell'elenco, scegli **Foglio personalizzato** e digita le misure: quante etichette ci sono in larghezza e in altezza, le dimensioni di un'etichetta, lo spazio sopra e a lato della prima etichetta e la distanza tra le etichette.

- **Pagina intera della stessa etichetta** inserisce lo stesso indirizzo su ogni etichetta del foglio.
- **Singola etichetta** inserisce l'indirizzo su una sola etichetta, così puoi usare un foglio già parzialmente utilizzato. Indica la riga e la colonna.

Fai clic su **Nuovo documento** per creare un documento che puoi vedere e modificare, oppure su **Stampa** per stamparlo.

Per creare etichette per una unione, fai clic su **Avvia stampa unione > Etichette**. La prima etichetta contiene un blocco di indirizzi, e ogni altra etichetta inizia con una regola **Record successivo**. Se modifichi la prima etichetta, fai clic su **Aggiorna etichette** per copiare la modifica su tutte le altre.

## Cosa contiene il file

Il tipo di unione, il nome e il percorso dell'elenco, l'oggetto e la colonna della posta elettronica vengono salvati nel file .docx. Le persone che hai deselezionato e le scelte di Abbina campi non vengono salvate: ti verranno chieste di nuovo dopo aver riaperto il file. Un elenco viene riletto ogni volta dal suo file. Se il file non si trova più nella posizione originale, JeffWrite cerca accanto al tuo documento e poi ti chiede di sceglierlo.

Un elenco può avere fino a 1.000 colonne e 4.000.000 di celle in totale, e una unione usa le prime 100.000 persone. Se un file ne contiene di più, JeffWrite usa ciò che rientra nei limiti e te lo segnala. Le righe completamente vuote vengono omesse dall'elenco.

## Argomenti correlati

- [Usa i campi](fields)
- [Stampa](print)
- [Crea e modifica tabelle](tables)
