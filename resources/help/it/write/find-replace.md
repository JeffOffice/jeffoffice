# Trova e sostituisci parole
<!-- keywords: trova avanzata, suona come, soundex, forme della parola, tutte le forme di una parola, ignora punteggiatura, ignora spazi, speciale, contrassegno di paragrafo, tabulazione, trova per formato, formato, grassetto, carattere, stile, caratteri jolly, carattere jolly, modello, espressione regolare, gruppi, conteggio corrispondenze, evidenzia tutte le corrispondenze, sostituisci, sostituisci tutto, cerca, cerca parole, cambia parole, maiuscole/minuscole, parole intere, trova successivo, ctrl+f, ctrl+h, modifica, riquadro, testo nascosto, annulla -->

Usa **Trova** per spostarti sulle parole del documento. Usa **Sostituisci** per modificarle. Entrambe le funzioni operano in un unico riquadro a destra della finestra.

## Trovare parole

1. Premi **Ctrl+F**, oppure fai clic su **Home > Modifica > Trova**. Si apre il riquadro **Trova e sostituisci**, con il cursore in **Trova:**.
2. Digita le parole che vuoi cercare. Poi premi **Invio**, oppure fai clic su **Trova successivo**.

JeffWrite parte dalla posizione del cursore. Seleziona la prossima occorrenza delle parole cercate e fa scorrere le pagine in modo che sia visibile. Ogni occorrenza si chiama **corrispondenza**. Fai clic di nuovo su **Trova successivo** per passare alla corrispondenza successiva. Dopo l'ultima corrispondenza, il programma torna all'inizio del documento.

Mentre digiti, il riquadro conta le corrispondenze, ad esempio "9 corrispondenze", e ogni corrispondenza riceve una fascia gialla nelle pagine. Le fasce compaiono anche nelle celle delle tabelle, nelle intestazioni, nei piè di pagina e nelle note a piè di pagina. Dopo **Trova successivo**, il riquadro indica quale corrispondenza stai visualizzando, ad esempio "Corrispondenza 2 di 4". Il conteggio e le fasce cambiano quando fai clic su **Maiuscole/minuscole** o **Solo parole intere**, oppure quando modifichi il testo. Scompaiono quando chiudi il riquadro.

## Trovare con i caratteri jolly

Seleziona **Usa caratteri jolly** per cercare un modello e non solo parole. Un **carattere jolly** è un segno che rappresenta altre lettere. Questi sono i segni disponibili:

- **?** rappresenta una lettera qualsiasi. **c?t** trova "cat" e "cot".
- **\*** rappresenta una sequenza qualsiasi di lettere. **c\*t** trova "cat" e "cart".
- **<** indica l'inizio di una parola, e **>** indica la fine. **<the>** trova "the" ma non "other".
- **[abc]** indica una qualsiasi di quelle lettere, **[a-c]** indica una lettera da a a c, e **[!abc]** indica una lettera diversa da quelle.
- **{2}** ripete due volte la lettera che lo precede. **{1,3}** la ripete da una a tre volte.
- **@** indica una o più ripetizioni della lettera che lo precede.
- **\\** davanti a un segno trova il segno stesso. **\\?** trova un punto interrogativo.
- **^p** è un contrassegno di paragrafo, **^t** è una tabulazione, **^#** è una cifra, **^$** è una lettera e **^w** è uno spazio vuoto.

Racchiudi una parte del modello tra **( )** per creare un **gruppo**. In **Sostituisci con:**, **\1** è il primo gruppo, **\2** il secondo, e così via fino a **\9**. **^&** è tutto ciò che è stato trovato. Ad esempio, cerca **(<[A-Z]@>), (<[A-Z]@>)** e sostituisci con **\2 \1** per trasformare "Rossi, Mario" in "Mario Rossi". **Solo parole intere** è disattivata mentre sono attivi i caratteri jolly, perché **<** e **>** svolgono già questa funzione.

Se il modello contiene un errore, il riquadro ne indica la causa.

Se le parole cercate non sono nel documento, il riquadro lo segnala e il cursore non si sposta.

Puoi continuare a lavorare nel documento mentre il riquadro è aperto. Fai clic nella pagina per digitare. Fai clic sulla **X** in alto nel riquadro per chiuderlo.

## Trova avanzata

Queste opzioni si trovano nel riquadro sotto **Usa caratteri jolly**. Ognuna cambia il modo in cui JeffWrite interpreta le parole in **Trova:**.

- **Suona come** trova le parole che si pronunciano come la parola digitata. Assegna a ogni parola un codice basato sui suoni e trova le parole con lo stesso codice. Digita **Rossi** e trova anche "Rosi".
- **Trova tutte le forme della parola** trova ogni forma della parola. Digita **correre** e trova "corro", "corri", "correva" e così via. Lavora con parole intere. Se digiti due parole, come **cane grosso**, le trova una accanto all'altra, in una qualsiasi delle loro forme.
- **Ignora caratteri di punteggiatura** salta segni come trattini e punti tra le lettere. Digita **email** e trova anche "e-mail".
- **Ignora caratteri di spazio** salta spazi e tabulazioni tra le lettere. Digita **e mail** e trova "email".

Tra **Suona come**, **Trova tutte le forme della parola** e **Usa caratteri jolly** può essere attiva una sola opzione alla volta. **Suona come** e **Trova tutte le forme della parola** trovano sempre parole intere, quindi **Solo parole intere** e **Maiuscole/minuscole** sono disattivate.

### Speciale

Fai clic su **Speciale** per inserire in **Trova:** un codice: **Contrassegno di paragrafo** (^p), **Carattere di tabulazione** (^t), **Qualsiasi cifra** (^#), **Qualsiasi lettera** (^$), **Spazio vuoto** (^w), **Interruzione di sezione** (^b), **Campo** (^d), **Segno di nota a piè di pagina** (^f), **Segno di nota di chiusura** (^e) oppure **Carattere di accento circonflesso** (^^). I codici funzionano anche senza **Usa caratteri jolly**. Un'**interruzione di sezione** è il segno che termina una sezione. Un **campo** è un elemento di testo che il programma calcola, come un numero di pagina o una data, e **^d** trova l'inizio di ciascuno. I segni di nota sono i piccoli numeri nel testo che rimandano a una nota a piè di pagina o a una nota di chiusura. Questi quattro elementi vengono trovati, ma **Sostituisci** e **Sostituisci tutto** non li modificano, perché cambiarne uno farebbe perdere ciò che rappresenta. Gli altri segni nell'elenco, come **?** e **\***, vengono trattati come semplici caratteri.

### Formato

Fai clic su **Formato** per trovare solo il testo con un determinato aspetto: **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Evidenziato**, un **Carattere** oppure uno **Stile** di paragrafo. Il riquadro elenca sotto i pulsanti gli aspetti scelti. Seleziona **Nessuna formattazione** per cancellarli.

Puoi lasciare vuoto **Trova:**. In questo caso ogni porzione di testo con quell'aspetto è una corrispondenza. Se in **Trova:** digiti delle parole, vengono trovate solo quelle con quell'aspetto. **Sostituisci tutto** modifica solo quelle parole.

## Restringere la ricerca

- **Maiuscole/minuscole** trova solo le parole con le stesse maiuscole. Con questa opzione attiva, "Piano" non trova "piano".
- **Solo parole intere** salta le parole più lunghe che contengono quella cercata. Con questa opzione attiva, "pari" non trova "parità".

## Sostituire parole

1. Premi **Ctrl+H**, oppure fai clic su **Home > Modifica > Sostituisci**. Il cursore passa in **Sostituisci con:**.
2. Digita le nuove parole in **Sostituisci con:**. Per eliminare le parole trovate, lascia il campo vuoto.
3. Digita le parole da sostituire in **Trova:**.
4. Fai clic su **Sostituisci** per vedere la prima corrispondenza. Fai clic di nuovo su **Sostituisci** per modificarla. JeffWrite passa poi alla corrispondenza successiva. Ogni modifica è un passaggio a sé, quindi **Annulla** (Ctrl+Z) le annulla una alla volta.
5. Fai clic su **Sostituisci tutto** per modificare tutte le corrispondenze in una sola volta. Il riquadro indica quante ne ha modificate. **Sostituisci tutto** è un unico passaggio, quindi un solo **Annulla** ripristina tutte le parole.

Le nuove parole assumono l'aspetto della prima lettera delle parole che sostituiscono. Se sostituisci una parola in grassetto, la nuova parola sarà in grassetto.

Ad esempio, per cambiare "color" in "colour" in tutto il documento: digita **color** in **Trova:**, digita **colour** in **Sostituisci con:** e fai clic su **Sostituisci tutto**.

## Che cosa cerca la ricerca

- Cerca nel testo del documento, comprese le tabelle. Un segnalibro inserito in mezzo ad alcune parole, presente in alcuni file, non impedisce di trovarle. **Sostituisci** e **Sostituisci tutto** mantengono il segnalibro.
- **Trova successivo** cerca prima nel testo. Poi passa alle intestazioni, quindi ai piè di pagina, alle note a piè di pagina e infine torna all'inizio del testo. Quando una corrispondenza si trova in una di queste parti, JeffWrite la apre per la modifica e seleziona la corrispondenza. Il riquadro indica dove si trova, ad esempio "Trovato nell'intestazione della pagina 3." **Sostituisci** sostituisce la corrispondenza su cui ti trovi, nel testo oppure in un'intestazione, un piè di pagina o una nota a piè di pagina. **Sostituisci tutto** modifica ogni corrispondenza in tutte queste parti, e un solo **Annulla** le ripristina tutte. Le note di chiusura fanno parte del testo, quindi vengono cercate insieme ad esso. Vedi [Intestazioni e piè di pagina](headers-footers).
- Salta il testo impostato come **Nascosto**, perché non viene visualizzato. Vedi [Modificare l'aspetto del testo](font).

## Argomenti correlati

- [Modificare l'aspetto del testo](font)
- [Usare JeffWrite con la tastiera](keyboard)
