# Stampa unione, buste ed etichette
<!-- keywords: stampa unione, unione, lettere, destinatari, elenco indirizzi, lettera tipo, etichette, buste, campo unione, blocco di indirizzi, formula di apertura, regole, se allora altrimenti, salta record, record successivo, anteprima risultati, completa e unisci, directory, messaggi di posta elettronica, csv, xlsx, ods, vcard, unione posta elettronica, unione catalogo, elenco, indirizzi, codice a barre -->

Una **stampa unione** crea molte copie di una stessa lettera, ciascuna con i dati di una persona diversa. Scrivi la lettera una sola volta. Conservi i dati delle persone in un **elenco**. JeffWrite inserisce nome e indirizzo della prima persona nella prima copia, quelli della seconda persona nella seconda copia, e così via.

Gli strumenti si trovano nella scheda **Lettere**. Per un percorso guidato, fai clic su **Lettere > Avvia stampa unione > Procedura guidata passo passo per stampa unione**.

## Scegli che cosa creare

Fai clic su **Lettere > Avvia stampa unione** e scegli una voce:

- **Lettere**: una copia della pagina per ogni persona. Ogni copia inizia su una nuova pagina.
- **Messaggi di posta elettronica**: un messaggio per ogni persona. Vedi "Crea messaggi di posta elettronica" più avanti.
- **Buste** ed **Etichette**: una pagina preparata per esse. Vedi le sezioni seguenti.
- **Directory**: un unico elenco di tutti, senza interruzioni di pagina tra le persone. Usalo per un elenco telefonico o un catalogo.
- **Cancella tipo di unione** riporta il documento a un normale documento.

JeffWrite salva nel file il tipo di unione e il nome dell'elenco. Quando riapri il file, l'unione è ancora presente.

## Scegli i destinatari

Un **destinatario** è una persona che riceve una copia. Fai clic su **Lettere > Seleziona destinatari**:

- **Digita un nuovo elenco**: digita le persone nella finestra che si apre. Fai clic su **Nuova voce** per ciascuna. JeffWrite ti propone poi di salvare l'elenco come file .csv.
- **Usa elenco esistente**: scegli un file che è già un elenco. Può essere un file .csv o un file di testo con tabulazioni (una riga per ogni persona, e la prima riga contiene i nomi delle colonne), un foglio di calcolo (.xlsx, .xls o .ods, da JeffCrunch o da un altro programma), oppure un file di contatti (.vcf).
- **Seleziona dai contatti**: scegli un file di contatti (.vcf). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato.

Poi fai clic su **Lettere > Modifica elenco destinatari** per controllare l'elenco:

- Deseleziona una riga per escludere quella persona.
- Fai clic su un'intestazione di colonna per ordinare.
- Digita in **Filtra destinatari** per mostrare solo le righe che contengono le parole indicate.
- **Trova duplicati** deseleziona le ripetizioni.
- **Convalida indirizzi** colora le righe che non hanno indirizzo o che hanno un CAP dall'aspetto errato.

## Inserisci i dati nella lettera

Un **campo unione** è un segnaposto. Mostra il nome di una colonna tra doppie virgolette angolari, come «Nome». In ogni copia viene sostituito con il dato della persona.

Fai clic dove deve andare il dato, poi usa **Lettere > Scrivi e inserisci campi**:

- **Inserisci campo unione** elenca le colonne del tuo elenco. Scegline una.
  - **Codice a barre da campo**, nello stesso elenco, inserisce un codice a barre il cui valore è una colonna, ad esempio un codice articolo o l'ISBN a 13 cifre di un libro. Scegli la colonna, poi il tipo, e controlla la finestra **Inserisci codice a barre** (quanto digiti lì è solo un esempio). La finestra offre solo ciò che un campo può contenere: il tipo, la dimensione, l'altezza delle barre, le cifre e il livello QR. Non include il prezzo di un libro, un codice aggiuntivo, un carattere di controllo né lo sfondo chiaro, perché un campo non può indicarli; l'ISBN di un libro è un EAN-13. Nel documento il campo è disegnato come codice a barre, come esempio del tipo fino alla unione; per cambiare tipo o aspetto, fai doppio clic sul campo oppure fai clic destro e scegli **Modifica codice a barre**. Alla unione, la copia di ogni persona riceve un codice a barre con il valore di quella persona, disegnato come immagine nella riga, quindi viene stampato e salvato in PDF come una qualsiasi immagine. Il valore è solo un dato: virgolette o barre rovesciate al suo interno non cambiano nulla. Se un valore è vuoto, non c'è alcun codice a barre. Se un codice non può contenere un valore (una lettera in un EAN-13, o più di 500 caratteri in Code 128), la copia mostra al suo posto un breve messaggio tra parentesi quadre, e le altre copie non ne risentono. Una unione crea al massimo 2.000 codici a barre, contando anche i valori che non possono essere contenuti; se viene segnalato che sono troppi, unisci meno persone. In un file `.docx` salvato, il campo è un campo DISPLAYBARCODE con la colonna al suo interno, quindi viene conservato quando salvi e riapri il file. Un campo con un valore digitato a mano, o di un tipo che JeffWrite non disegna (UPC-E, ad esempio), resta invariato. Nelle unioni per posta elettronica il valore viene inserito come testo.
  - **Blocco di indirizzi** inserisce un intero indirizzo postale: nome, azienda, via e città. Scegli le righe che vuoi. Una riga vuota viene omessa. Puoi anche indicare un paese da omettere, come il tuo. **Anteprima** mostra l'indirizzo della prima persona.
  - **Formula di apertura** inserisce una riga come "Gentile dott. Fenwick,". Scegli tu le parole. La prima scelta, "Gentile dott. Fenwick,", usa il titolo della persona. Se una persona non ha un titolo, usa invece nome e cognome, come in "Gentile Nora Pell,". Se una persona non ha un nome, la formula dice qualcosa d'altro, come "Gentile Signore o Signora,", che puoi modificare.
  - **Associa campi** indica al blocco di indirizzi e alla formula di apertura quale colonna contiene ciascuna parte, quando i nomi delle colonne sono insoliti.

## Regole

Una **regola** rende diversa una copia per alcune persone. Fai clic su **Regole** e scegli:

- **Se…Allora…Altrimenti**: mostra un testo quando una colonna ha un valore, e un altro testo quando non ce l'ha. Ad esempio, se Provincia è "MI", scrivi "Benvenuti a casa".
- **Salta record se**: escludi una persona quando una colonna ha un valore, ad esempio quando il cognome è vuoto.
- **Record successivo** e **Record successivo se**: usano i dati della persona successiva nello stesso punto. Le etichette usano questa regola per mettere una persona diversa su ogni etichetta.
- **Numero record di unione** inserisce il numero della riga della persona nell'elenco. **Numero sequenza di unione** conta le copie create finora.
- **Chiedi** e **Campo da compilare**: ti chiedono una risposta al termine dell'unione. Chiedi conserva la risposta con un nome. Campo da compilare inserisce la risposta nel testo.
- **Imposta segnalibro**: assegna un valore a un nome. Un riferimento incrociato al nome mostra il valore.

## Anteprima dei risultati

Fai clic su **Lettere > Anteprima risultati**. Si apre un riquadro che mostra la pagina per una persona. Usa **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure digita un numero. **Trova destinatario** salta alla persona successiva che contiene le parole indicate in uno dei suoi dati.

**Evidenzia campi unione** ombreggia in grigio i campi unione nella pagina, così puoi vedere dove si trovano. Gli altri campi, come i numeri di pagina, non vengono ombreggiati. L'ombreggiatura compare sullo schermo e non viene mai stampata.

Il tuo documento non viene modificato. **Verifica errori** ti avvisa quando nell'elenco manca una colonna richiesta da un campo, oppure quando non è incluso nessun destinatario.

## Completa l'unione

Fai clic su **Lettere > Completa e unisci**:

- **Modifica documenti singoli** crea un nuovo documento con la copia di ogni persona. Puoi modificarlo prima di stampare.
- **Stampa documenti** crea quel documento e apre la pagina **Stampa**.
- **Salva come PDF** salva tutte le copie in un unico file PDF.
- **Invia messaggi di posta elettronica** crea un file messaggio .eml per ogni persona, in una cartella che scegli. Scegli la colonna con gli indirizzi e digita un oggetto. JeffWrite non invia i messaggi: aprili con il tuo programma di posta.

Le persone che hai deselezionato nell'elenco restano escluse.

Un file messaggio viene creato solo per una persona il cui indirizzo è un unico indirizzo di posta elettronica. Se un indirizzo ne contiene due (separati da una virgola o da un punto e virgola), ha un nome insieme all'indirizzo oppure non ha il dominio, quella persona non riceve alcun messaggio, e JeffWrite te lo segnala con il numero di casi. Un oggetto lungo viene salvato in più parti, che i programmi di posta ricompongono.

Durante l'unione compare una finestra di avanzamento, e **Annulla** la interrompe. Se una regola fa una domanda (Chiedi o Campo da compilare), ti viene chiesta ciascuna domanda diversa una sola volta, fino a 20 domande. Dopo, le altre usano le risposte predefinite. Se annulli una domanda, l'intera unione si interrompe. L'unione si interrompe e te lo segnala se produrrebbe più di 10.000 pagine, così un documento molto lungo con un elenco molto lungo non richiede ore. In questo caso, unisci una parte dell'elenco alla volta.

## Buste

Fai clic su **Lettere > Buste**. Digita l'indirizzo del destinatario e il tuo indirizzo di mittente, e scegli il formato della busta dall'elenco. I formati sono indicati con la larghezza e l'altezza. Fai clic su **Aggiungi al documento** per mettere la busta su una pagina a sé all'inizio del documento, oppure su **Stampa** per stamparla. La pagina ha la larghezza del lato lungo della busta, che è il modo in cui la maggior parte delle stampanti accetta le buste.

Per creare buste per un'unione, fai clic su **Avvia stampa unione > Buste**. Ogni busta contiene un blocco di indirizzi.

## Etichette

Fai clic su **Lettere > Etichette**. Digita l'indirizzo e scegli il foglio dall'elenco. Un foglio è indicato dal numero di etichette che contiene e dalla dimensione di ciascuna. Se il tuo foglio non è nell'elenco, scegli **Foglio personalizzato** e digita le misure: quante etichette stanno in larghezza e in altezza, la dimensione di un'etichetta, lo spazio sopra e a fianco della prima etichetta, e la distanza tra le etichette.

- **Pagina intera della stessa etichetta** mette lo stesso indirizzo su ogni etichetta del foglio.
- **Singola etichetta** mette l'indirizzo su una sola etichetta, così puoi usare un foglio già parzialmente utilizzato. Indica riga e colonna.

Fai clic su **Nuovo documento** per creare un documento che puoi vedere e modificare, oppure su **Stampa** per stamparlo.

Per creare etichette per un'unione, fai clic su **Avvia stampa unione > Etichette**. La prima etichetta contiene un blocco di indirizzi, e ogni altra etichetta inizia con una regola **Record successivo**. Se modifichi la prima etichetta, fai clic su **Aggiorna etichette** per copiare la modifica su tutte le altre.

## Che cosa contiene il file

Nel file .docx vengono salvati il tipo di unione, il nome e la posizione dell'elenco, l'oggetto e la colonna degli indirizzi di posta elettronica. Non vengono salvate le persone che hai deselezionato né le scelte di **Associa campi**: al riavvio del file ti verranno richieste di nuovo. L'elenco viene riletto ogni volta dal proprio file. Se il file non si trova più nella posizione originale, JeffWrite cerca accanto al documento e, se non lo trova, ti chiede di sceglierlo.

Un elenco può avere fino a 1.000 colonne e 4.000.000 di celle in totale, e una unione usa le prime 100.000 persone. Se un file ne contiene di più, JeffWrite usa quanto rientra nel limite e te lo segnala. Le righe completamente vuote vengono omesse dall'elenco.

## Argomenti correlati

- [Usare i campi](fields)
- [Stampare](print)
- [Creare e modificare tabelle](tables)
