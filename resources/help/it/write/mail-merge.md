# Stampa unione, buste ed etichette
<!-- keywords: stampa unione, unione, lettere, destinatari, elenco indirizzi, lettera tipo, etichette, buste, campo unione, blocco di indirizzi, formula di apertura, regole, se allora altrimenti, salta record, record successivo, anteprima risultati, avvia stampa unione, rubrica, messaggi di posta elettronica, csv, xlsx, ods, vcard, unione posta elettronica, unione catalogo -->

Una **stampa unione** crea molte copie di una stessa lettera, ciascuna con i dati di una persona diversa. Scrivete la lettera una sola volta. Conservate i dati delle persone in un **elenco**. JeffWrite inserisce nome e indirizzo della prima persona nella prima copia, quelli della seconda persona nella seconda copia, e così via.

Gli strumenti si trovano nella scheda **Lettere**. Per una procedura guidata, fate clic su **Lettere > Avvia stampa unione > Procedura guidata passo passo per stampa unione**.

## Scegliete che cosa creare

Fate clic su **Lettere > Avvia stampa unione** e scegliete una delle opzioni:

- **Lettere**: una copia della pagina per ogni persona. Ogni copia inizia su una nuova pagina.
- **Messaggi di posta elettronica**: un messaggio per ogni persona. Vedete "Inviare messaggi di posta elettronica" più avanti.
- **Buste** ed **Etichette**: una pagina preparata per questi formati. Vedete le sezioni seguenti.
- **Elenco**: un unico elenco di tutte le persone, senza interruzioni di pagina tra una persona e l'altra. Usatelo per un elenco telefonico o un catalogo.
- **Cancella tipo di unione** riporta il documento a un normale documento.

JeffWrite salva nel file il tipo di unione e il nome dell'elenco. Quando riaprite il file, l'unione è ancora presente.

## Scegliete i destinatari

Un **destinatario** è una persona che riceve una copia. Fate clic su **Lettere > Seleziona destinatari**:

- **Digita un nuovo elenco**: inserite le persone nella finestra che si apre. Fate clic su **Nuova voce** per ciascuna. JeffWrite vi propone poi di salvare l'elenco come file .csv.
- **Usa un elenco esistente**: scegliete un file che è già un elenco. Può essere un file .csv o un file di testo con tabulazioni (una riga per ogni persona, e la prima riga contiene i nomi delle colonne), un foglio di calcolo (.xlsx, .xls o .ods, da JeffCrunch o da un altro programma), oppure un file di contatti (.vcf).
- **Seleziona dai contatti**: scegliete un file di contatti (.vcf). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato.

Poi fate clic su **Lettere > Modifica elenco destinatari** per controllare l'elenco:

- Deselezionate una riga per escludere quella persona.
- Fate clic su un'intestazione di colonna per ordinare.
- Digitate in **Filtra destinatari** per mostrare solo le righe che contengono le parole indicate.
- **Distingui i destinatari in base a** sceglie la colonna che distingue una persona dall'altra, ad esempio un indirizzo e-mail. Quando riaprite il documento e l'elenco viene letto di nuovo, le persone che avete deselezionato restano escluse, anche se l'elenco ha un ordine diverso o nuove persone. Con **Posizione nell'elenco**, le persone vengono ricordate dal numero della riga nel file dell'elenco, anche se avete ordinato o filtrato le righe in questa finestra. Se l'elenco è cambiato al punto che questo non è più valido, JeffWrite ve lo segnala e include tutte le persone che non è riuscito a riconoscere. Il documento conserva una forma codificata di ogni indirizzo, non l'indirizzo stesso; tuttavia, chi ha il documento e indovina un indirizzo può comunque verificare la sua ipotesi. **Ispeziona documento** (File > Informazioni) elimina la registrazione delle persone escluse.
- **Trova duplicati** deseleziona le ripetizioni.
- **Convalida indirizzi** colora le righe che non hanno un indirizzo o che hanno un CAP che sembra errato.

## Inserite i dati nella lettera

Un **campo unione** è un segnaposto. Mostra il nome di una colonna tra doppie virgolette angolari, ad esempio «Nome». In ogni copia viene sostituito con il dato di quella persona.

Fate clic dove deve comparire il dato, quindi usate **Lettere > Scrivi e inserisci campi**:

- **Inserisci campo unione** elenca le colonne del vostro elenco. Scegliete una colonna.
  - **Codice a barre da campo**, nello stesso elenco, inserisce un codice a barre il cui valore è una colonna, come un codice articolo o l'ISBN a 13 cifre di un libro. Scegliete la colonna, quindi il tipo, e controllate nella finestra **Inserisci codice a barre** (quello che digitate lì è solo un esempio). La finestra offre solo ciò che un campo può contenere: il tipo, la dimensione, l'altezza delle barre, le cifre e il livello QR. Non propone il prezzo di un libro, un codice aggiuntivo, un carattere di controllo né lo sfondo chiaro, che un campo non può indicare; l'ISBN di un libro è un EAN-13. Nel documento il campo appare come un codice a barre, con un esempio del tipo finché non eseguite l'unione; per cambiare tipo o aspetto, fate doppio clic su di esso oppure fate clic con il pulsante destro e scegliete **Modifica codice a barre**. Quando eseguite l'unione, la copia di ogni persona riceve un codice a barre con il suo valore, disegnato come immagine nella riga, quindi viene stampato e salvato in PDF come qualsiasi immagine. Il valore è solo un dato: virgolette o barre rovesciate al suo interno non cambiano nulla. Se un valore è vuoto, non viene creato alcun codice a barre. Se un codice non può contenere il valore (una lettera in un EAN-13, oppure più di 500 caratteri in Code 128), al suo posto la copia mostra un breve messaggio tra parentesi quadre, senza alterare le altre copie. Una unione crea al massimo 2.000 codici a barre, contando anche i valori che non possono essere rappresentati; se il programma segnala che sono troppi, unite meno persone. In un file `.docx` salvato, il campo è un campo DISPLAYBARCODE con la colonna al suo interno, quindi viene conservato quando salvate e riaprite il file. Un campo con un valore digitato a mano, o di un tipo che JeffWrite non disegna (UPC-E, ad esempio), resta invariato. Nelle unioni per posta elettronica il valore viene inserito come testo.
- **Blocco di indirizzi** inserisce un intero indirizzo postale: nome, azienda, via e città. Scegliete le righe che volete. Una riga vuota viene omessa. Potete anche indicare un Paese da omettere, ad esempio il vostro. **Anteprima** mostra l'indirizzo della prima persona.
- **Formula di apertura** inserisce una riga come "Gentile dott.ssa Fenwick,". Scegliete voi le parole. La prima scelta, "Gentile dott.ssa Fenwick,", usa il titolo della persona. Se una persona non ha un titolo, viene usato al suo posto il nome e il cognome, come in "Gentile Nora Pell,". Se una persona non ha un nome, la formula dice qualcos'altro, come "Gentile Signore/Signora,", che potete modificare.
- **Abbina campi** indica al blocco di indirizzi e alla formula di apertura quale colonna contiene ciascuna parte, quando i nomi delle colonne sono insoliti.

## Regole

Una **regola** rende diversa la copia per alcune persone. Fate clic su **Regole** e scegliete una delle opzioni:

- **Se…Allora…Altrimenti**: mostra un testo quando una colonna ha un valore e un altro testo quando non lo ha. Ad esempio, se Provincia è "MI", scrivete "Bentornati a casa".
- **Salta record se**: esclude una persona quando una colonna soddisfa una condizione che impostate, ad esempio quando il cognome è vuoto.
- **Record successivo** e **Record successivo se**: usano i dati della persona successiva nello stesso punto. Le etichette usano questa regola per inserire una persona diversa in ogni etichetta.
- **Numero record unione** inserisce il numero della riga della persona nell'elenco. **Numero sequenza unione** conta le copie create fino a quel momento.
- **Chiedi** e **Campo da compilare**: vi chiedono una risposta al termine dell'unione. Chiedi conserva la risposta con un nome. Campo da compilare inserisce la risposta nel testo.
- **Imposta segnalibro**: assegna a un nome un valore. Un riferimento incrociato al nome mostra il valore.

## Visualizzate l'anteprima dei risultati

Fate clic su **Lettere > Anteprima risultati**. Si apre un riquadro che mostra la pagina per una persona. Usate **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure digitate un numero. **Trova destinatario** salta alla persona successiva che ha le parole indicate in uno dei dati.

**Evidenzia campi unione** ombreggia in grigio i campi unione nella pagina, così potete vedere dove si trovano. Gli altri campi, come i numeri di pagina, non vengono ombreggiati. L'ombreggiatura appare sullo schermo e non viene mai stampata.

Il vostro documento non viene modificato. **Verifica errori** vi avvisa quando l'elenco non ha una colonna per un campo, oppure quando non è inclusa nessuna persona.

## Completate l'unione

Fate clic su **Lettere > Completa e unisci**:

- **Modifica documenti singoli** crea un nuovo documento con la copia di ogni persona. Potete modificarlo prima di stamparlo.
- **Stampa documenti** crea quel documento e apre la pagina **Stampa**.
- **Salva come PDF** salva tutte le copie in un unico file PDF.
- **Invia messaggi di posta elettronica** crea un file di messaggio .eml per ogni persona, in una cartella che scegliete. Scegliete la colonna che contiene gli indirizzi e digitate un oggetto. JeffWrite non invia i messaggi: apriteli con il vostro programma di posta.
- **Salva ciascuno come file** crea un file per ogni persona, in una cartella che scegliete, in formato .docx o PDF. Date ai file un nome con colonne tra parentesi graffe, ad esempio {Cognome}, {Nome}; un nome usato due volte riceve un numero. Una finestra mostra i nomi dei primi file e quanti ne verranno creati. Viene chiesta conferma oltre 200 file, e una volta se nella cartella ci sono già file con gli stessi nomi. Fate clic su **Annulla** per fermarvi tra un file e l'altro: i file già creati restano; chiudere la finestra produce lo stesso effetto. Un file che è un collegamento a un altro file viene sostituito da un nuovo file, mentre il file a cui puntava non viene toccato. I file contengono la lettera di una sola persona: non contengono l'elenco, la sua posizione né l'elenco delle persone escluse. Una unione crea al massimo 100.000 file.

Le persone che avete deselezionato nell'elenco vengono escluse.

Un file di messaggio viene creato solo per una persona il cui indirizzo è un unico indirizzo e-mail. Se un indirizzo ne contiene due (separati da una virgola o da un punto e virgola), ha un nome insieme all'indirizzo oppure non ha un dominio, a quella persona non viene creato alcun messaggio, e JeffWrite ve ne indica il numero. Un oggetto lungo viene salvato in più parti, che i programmi di posta ricompongono.

Durante un'unione appare una finestra di avanzamento, e **Annulla** la interrompe. Se una regola pone una domanda (Chiedi o Campo da compilare), vi viene chiesta ogni domanda diversa una sola volta, fino a 20 domande. Dopo, le restanti usano le risposte predefinite. Se annullate una domanda, l'intera unione si interrompe. Un'unione si interrompe e ve lo segnala se produrrebbe più di 10.000 pagine, così un documento molto lungo con un elenco molto lungo non richiede ore. In questo caso unite una parte dell'elenco alla volta.

## Buste

Fate clic su **Lettere > Buste**. Digitate l'indirizzo del destinatario e il vostro indirizzo di mittente, quindi scegliete il formato della busta dall'elenco. I formati sono indicati con larghezza e altezza. Fate clic su **Aggiungi al documento** per inserire la busta su una pagina a sé all'inizio del documento, oppure su **Stampa** per stamparla. La pagina ha la larghezza del lato lungo della busta, che è il modo in cui la maggior parte delle stampanti accetta una busta.

Per creare buste per un'unione, fate clic su **Avvia stampa unione > Buste**. Ogni busta contiene un blocco di indirizzi.

## Etichette

Fate clic su **Lettere > Etichette**. Digitate l'indirizzo e scegliete il foglio dall'elenco. Un foglio è indicato con il numero di etichette che contiene e con le dimensioni di ciascuna etichetta. Se il vostro foglio non è nell'elenco, scegliete **Foglio personalizzato** e digitate le misure: quante etichette stanno in larghezza e in altezza, le dimensioni di un'etichetta, lo spazio sopra e a lato della prima etichetta, e la distanza tra le etichette.

- **Pagina intera della stessa etichetta** inserisce lo stesso indirizzo su ogni etichetta del foglio.
- **Singola etichetta** inserisce l'indirizzo su una sola etichetta, così potete usare un foglio già parzialmente utilizzato. Indicate la riga e la colonna.

Fate clic su **Nuovo documento** per creare un documento che potete vedere e modificare, oppure su **Stampa** per stamparlo.

Per creare etichette per un'unione, fate clic su **Avvia stampa unione > Etichette**. La prima etichetta contiene un blocco di indirizzi, e ogni altra etichetta inizia con una regola **Record successivo**. Se modificate la prima etichetta, fate clic su **Aggiorna etichette** per copiare la modifica su tutte le altre.

## Che cosa contiene il file

Il tipo di unione, il nome e la posizione dell'elenco, l'oggetto e la colonna degli indirizzi e-mail vengono salvati nel file .docx. Anche le persone che avete deselezionate vengono salvate, come brevi codici della colonna scelta, e non con nome o indirizzo (solo un file .docx conserva questa informazione; per gli altri formati lo segnala **Verifica compatibilità**). Anche le scelte di Abbina campi vengono salvate. Un elenco viene letto di nuovo dal suo file ogni volta. Se il file non si trova più nella posizione originale, JeffWrite cerca accanto al documento e, se non lo trova, vi chiede di sceglierlo.

Un elenco può avere fino a 1.000 colonne e 4.000.000 di celle in totale, e un'unione usa le prime 100.000 persone. Se un file ne contiene di più, JeffWrite usa quelle che rientrano nel limite e ve lo segnala. Le righe completamente vuote vengono escluse dall'elenco.

## Vedi anche

- [Usare i campi](fields)
- [Stampare](print)
- [Creare e modificare tabelle](tables)
