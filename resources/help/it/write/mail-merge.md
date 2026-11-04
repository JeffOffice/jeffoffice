# Stampa unione, buste ed etichette
<!-- keywords: stampa unione, unione, lettere, destinatari, elenco indirizzi, lettera tipo, etichette, buste, campo unione, blocco di indirizzi, formula di apertura, regole, se allora altrimenti, salta record, record successivo, anteprima risultati, completa e unisci, catalogo, messaggi di posta elettronica, csv, xlsx, ods, vcard -->

Una **stampa unione** crea molte copie di una stessa lettera, ciascuna con i dati di una persona diversa. Scrivi la lettera una sola volta. I dati delle persone sono conservati in un **elenco**. JeffWrite inserisce nome e indirizzo della prima persona nella prima copia, quelli della seconda persona nella seconda copia, e così via.

Gli strumenti si trovano nella scheda **Lettere**. Per un percorso guidato, fai clic su **Lettere > Avvia stampa unione > Procedura guidata passo passo per stampa unione**.

## Scegli che cosa creare

Fai clic su **Lettere > Avvia stampa unione** e scegli una voce:

- **Lettere**: una copia della pagina per ogni persona. Ogni copia inizia su una nuova pagina.
- **Messaggi di posta elettronica**: un messaggio per ogni persona. Vedi "Inviare i messaggi di posta elettronica" più avanti.
- **Buste** ed **Etichette**: una pagina preparata per loro. Vedi le sezioni seguenti.
- **Catalogo**: un unico elenco di tutte le persone, senza interruzioni di pagina tra una persona e l'altra. Usalo per un elenco telefonico o un catalogo.
- **Cancella tipo di unione** trasforma il documento in un documento normale.

JeffWrite salva nel file il tipo di unione e il nome dell'elenco. Quando riapri il file, l'unione è ancora presente.

## Scegli i destinatari

Un **destinatario** è una persona che riceve una copia. Fai clic su **Lettere > Seleziona destinatari**:

- **Digita un nuovo elenco**: digita le persone nella finestra che si apre. Fai clic su **Nuova voce** per ognuna. JeffWrite ti propone quindi di salvare l'elenco come file .csv.
- **Usa un elenco esistente**: scegli un file che è già un elenco. Può essere un file .csv o un file di testo con tabulazioni (una riga per ogni persona, e la prima riga contiene i nomi delle colonne), un foglio di calcolo (.xlsx, .xls o .ods, da JeffCrunch o da un altro programma) oppure un file di contatti (.vcf).
- **Seleziona dai contatti**: scegli un file di contatti (.vcf). La maggior parte delle rubriche e dei telefoni può salvare i contatti in questo formato.

Poi fai clic su **Lettere > Modifica elenco destinatari** per controllare l'elenco:

- Deseleziona una riga per escludere quella persona.
- Fai clic su un'intestazione di colonna per ordinare.
- Digita in **Filtra destinatari** per mostrare solo le righe che contengono le parole indicate.
- **Trova duplicati** deseleziona i ripetuti.
- **Convalida indirizzi** colora le righe che non hanno un indirizzo o che hanno un CAP dall'aspetto errato.

## Inserisci i dati nella lettera

Un **campo unione** è un segnaposto. Mostra il nome di una colonna tra virgolette basse doppie, come «Nome». In ogni copia viene sostituito con il dato di quella persona.

Fai clic dove deve andare il dato, poi usa **Lettere > Scrivi e inserisci campi**:

- **Inserisci campo unione** elenca le colonne del tuo elenco. Scegline una.
- **Blocco di indirizzi** inserisce un intero indirizzo postale: nome, società, via e città. Scegli le righe che vuoi. Una riga senza dati viene omessa. Puoi anche indicare un paese da omettere, ad esempio il tuo. **Anteprima** mostra l'indirizzo della prima persona.
- **Formula di apertura** inserisce una frase come "Gentile Dott. Fenwick,". Scegli tu le parole. La prima scelta, "Gentile Dott. Fenwick,", usa il titolo della persona. Se una persona non ha un titolo, usa invece nome e cognome, come in "Gentile Nora Pell,". Se una persona non ha un nome, la formula di apertura dice altro, come "Gentile Signore/Signora,", che puoi modificare.
- **Abbina campi** indica al blocco di indirizzi e alla formula di apertura quale colonna contiene ciascuna parte, quando i nomi delle colonne sono insoliti.

## Regole

Una **regola** rende diversa la copia per alcune persone. Fai clic su **Regole** e scegli:

- **Se…Allora…Altrimenti**: mostra un testo quando una colonna ha un valore e un altro testo quando non ce l'ha. Ad esempio, se Provincia è "MI", scrivi "Benvenuto a casa".
- **Salta record se**: escludi una persona quando una colonna ha un valore, ad esempio quando il cognome è vuoto.
- **Record successivo** e **Record successivo se**: usa i dati della persona successiva nello stesso punto. Le etichette usano questa regola per mettere una persona diversa su ogni etichetta.
- **Numero record di unione** inserisce il numero della riga della persona nell'elenco. **Numero sequenza unione** conta le copie create finora.
- **Chiedi** e **Campo da compilare**: ti chiedono una risposta al termine dell'unione. Chiedi conserva la risposta con un nome. Campo da compilare inserisce la risposta nel testo.
- **Imposta segnalibro**: assegna a un nome un valore. Un riferimento incrociato al nome mostra il valore.

## Anteprima dei risultati

Fai clic su **Lettere > Anteprima risultati**. Si apre un riquadro che mostra la pagina per una persona. Usa **Primo record**, **Record precedente**, **Record successivo** e **Ultimo record** per scorrere le persone, oppure digita un numero. **Trova destinatario** passa alla persona successiva che contiene le parole indicate in un dato.

**Evidenzia campi unione** ombreggia in grigio i campi unione nella pagina, così vedi dove si trovano. Gli altri campi, come i numeri di pagina, non vengono ombreggiati. L'ombreggiatura si vede sullo schermo e non viene mai stampata.

Il tuo documento originale non viene modificato. **Verifica errori** ti avvisa quando l'elenco non ha una colonna per un campo o quando nessuno è incluso.

## Completa l'unione

Fai clic su **Lettere > Completa e unisci**:

- **Modifica documenti singoli** crea un nuovo documento con la copia di ogni persona. Puoi modificarlo prima di stamparlo.
- **Stampa documenti** crea quel documento e apre la pagina **Stampa**.
- **Salva come PDF** salva tutte le copie in un unico file PDF.
- **Invia messaggi di posta elettronica** crea un file di messaggio .eml per ogni persona, in una cartella che scegli. Scegli la colonna che contiene gli indirizzi di posta elettronica e digita un oggetto. JeffWrite non invia i messaggi: aprili con il tuo programma di posta.

Le persone che hai deselezionato nell'elenco vengono escluse.

Un file di messaggio viene creato solo per una persona il cui indirizzo contiene un unico indirizzo di posta elettronica. Se un indirizzo ne contiene due (separati da una virgola o da un punto e virgola), contiene un nome oppure non ha un dominio, quella persona non riceve alcun messaggio, e JeffWrite te lo segnala indicando quante sono. Un oggetto lungo viene salvato in più parti che i programmi di posta ricompongono.

Durante l'unione compare una finestra di avanzamento, e **Annulla** la interrompe. Se una regola ti fa una domanda (Chiedi o Campo da compilare), ti viene chiesta ogni domanda diversa una sola volta, fino a un massimo di 20. Dopo, le restanti usano le risposte predefinite. Se annulli una domanda, l'intera unione si interrompe. L'unione si arresta e lo segnala se produrrebbe più di 10.000 pagine, così un documento molto lungo con un elenco molto lungo non richiede ore. Unisci una parte dell'elenco alla volta.

## Buste

Fai clic su **Lettere > Buste**. Digita l'indirizzo del destinatario e il tuo indirizzo di mittente, quindi scegli il formato della busta dall'elenco. I formati sono indicati con la larghezza e l'altezza. Fai clic su **Aggiungi al documento** per inserire la busta in una pagina separata all'inizio del documento, oppure su **Stampa** per stamparla. La pagina ha la larghezza del lato lungo della busta, che è il modo in cui la maggior parte delle stampanti accetta una busta.

Per creare buste per un'unione, fai clic su **Avvia stampa unione > Buste**. Ogni busta contiene un blocco di indirizzi.

## Etichette

Fai clic su **Lettere > Etichette**. Digita l'indirizzo e scegli il foglio dall'elenco. Un foglio è indicato dal numero di etichette che contiene e dalle dimensioni di ciascuna. Se il tuo foglio non è nell'elenco, scegli **Foglio personalizzato** e digita le misure: quante etichette vanno in orizzontale e in verticale, quanto è grande un'etichetta, lo spazio sopra e a fianco della prima etichetta, e la distanza tra le etichette. In centimetri, un'etichetta da 2,5 × 5 cm si misura così.

- **Pagina piena della stessa etichetta** mette lo stesso indirizzo su ogni etichetta del foglio.
- **Singola etichetta** inserisce l'indirizzo in una sola etichetta, così puoi usare un foglio già parzialmente utilizzato. Indica riga e colonna.

Fai clic su **Nuovo documento** per creare un documento che puoi vedere e modificare, oppure su **Stampa** per stamparlo.

Per creare etichette per un'unione, fai clic su **Avvia stampa unione > Etichette**. La prima etichetta contiene un blocco di indirizzi, e ogni altra etichetta inizia con una regola **Record successivo**. Se modifichi la prima etichetta, fai clic su **Aggiorna etichette** per copiare la modifica in tutte le altre.

## Che cosa contiene il file

Il tipo di unione, il nome e la posizione dell'elenco, l'oggetto e la colonna degli indirizzi di posta elettronica sono salvati nel tuo file .docx. Le persone che hai deselezionato e le scelte di Abbina campi non vengono salvate: ti verranno richieste di nuovo dopo aver aperto il file. Un elenco viene riletto ogni volta dal suo file. Se il file non si trova più nel punto in cui era, JeffWrite cerca accanto al tuo documento e, se non lo trova, ti chiede di sceglierlo.

Un elenco può contenere fino a 1.000 colonne e 4.000.000 di celle in totale, e una stampa unione usa le prime 100.000 persone. Se un file ne contiene di più, JeffWrite usa quanto rientra nei limiti e te lo segnala. Le righe vuote vengono escluse dall'elenco.

## Vedi anche

- [Usare i campi](fields)
- [Stampare](print)
- [Creare e modificare tabelle](tables)
