# Trova e sostituisci parole
<!-- keywords: trova avanzato, suona come, soundex, forme della parola, tutte le forme di una parola, ignora punteggiatura, ignora spazi, speciale, segno di paragrafo, tabulazione, trova per formato, formato, grassetto, carattere, stile, caratteri jolly, carattere jolly, modello, espressione regolare, gruppi, conteggio corrispondenze, evidenzia tutte le corrispondenze, sostituisci, sostituisci tutto, cerca, cerca parole, cambia parole, maiuscole/minuscole, parole intere, trova successivo, ctrl+f, ctrl+h, modifica, riquadro, testo nascosto, annulla -->

Usa Trova per andare alle parole nel documento. Usa Sostituisci per cambiarle. Entrambe funzionano in un unico riquadro a destra della finestra.

## Trovare parole

1. Premi **Ctrl+F**, oppure fai clic su **Home > Modifica > Trova**. Si apre il riquadro **Trova e sostituisci**, con il cursore in **Trova**.
2. Digita le parole che vuoi cercare. Poi premi **Invio**, oppure fai clic su **Trova successivo**.

JeffWrite parte dalla posizione del cursore. Seleziona il punto successivo in cui trova le parole e fa scorrere le pagine in modo che tu possa vederlo. Ogni punto trovato si chiama **corrispondenza**. Fai clic di nuovo su **Trova successivo** per passare alla corrispondenza successiva. Dopo l'ultima corrispondenza, torna all'inizio del documento.

Mentre digiti, il riquadro conta le corrispondenze, ad esempio "9 corrispondenze", e ogni corrispondenza riceve una fascia gialla sulle pagine. Le fasce compaiono anche nelle celle di tabella, nelle intestazioni, nei piè di pagina e nelle note a piè di pagina. Dopo **Trova successivo**, il riquadro indica quale corrispondenza stai visualizzando, ad esempio "Corrispondenza 2 di 4". Il conteggio e le fasce cambiano quando fai clic su **Maiuscole/minuscole** o **Solo parole intere**, oppure quando modifichi il testo. Scompaiono quando chiudi il riquadro.

## Trovare con caratteri jolly

Seleziona **Usa caratteri jolly** per cercare un modello, e non solo delle parole. Un **carattere jolly** è un segno che rappresenta altre lettere. Questi sono i segni:

- **?** rappresenta una lettera qualsiasi. **c?t** trova "cat" e "cot".
- **\*** rappresenta una sequenza qualsiasi di lettere. **c\*t** trova "cat" e "cart".
- **<** indica l'inizio di una parola, e **>** indica la fine. **<the>** trova "the" ma non "other".
- **[abc]** è una qualsiasi di quelle lettere, **[a-c]** è una qualsiasi lettera da a a c, e **[!abc]** è una lettera qualsiasi tranne quelle.
- **{2}** ripete due volte la lettera che lo precede. **{1,3}** la ripete da una a tre volte.
- **@** indica una o più ripetizioni della lettera che lo precede.
- **\\** davanti a un segno trova il segno stesso. **\\?** trova un punto interrogativo.
- **^p** è un segno di paragrafo, **^t** è una tabulazione, **^#** è una cifra, **^$** è una lettera e **^w** è uno spazio bianco.

Racchiudi una parte del modello tra **( )** per creare un **gruppo**. In **Sostituisci con**, **\1** è il primo gruppo, **\2** il secondo, e così via fino a **\9**. **^&** è tutto ciò che è stato trovato. Ad esempio, per cercare **(<[A-Z]@>), (<[A-Z]@>)** e sostituire con **\2 \1**, trasformi "Rossi, Mario" in "Mario Rossi". **Solo parole intere** è disattivata mentre i caratteri jolly sono attivi, perché **<** e **>** svolgono già quella funzione.

Se il modello contiene un errore, il riquadro ne indica la natura.

Se le parole non sono nel documento, il riquadro lo segnala e nulla si sposta.

Puoi continuare a lavorare nel documento mentre il riquadro è aperto. Fai clic sulla pagina per scrivere. Fai clic sulla **X** in alto nel riquadro per chiuderlo.

## Trova avanzato

Queste opzioni si trovano nel riquadro sotto **Usa caratteri jolly**. Ognuna cambia il modo in cui JeffWrite legge le parole in **Trova**.

- **Suona come** trova le parole che si pronunciano come la parola digitata. Assegna a ogni parola un codice basato sui suoni e trova le parole con lo stesso codice. Digita **Rossi**, e trova anche "Rosi".
- **Trova tutte le forme della parola** trova ogni forma della parola. Digita **correre**, e trova "corri", "corre" e "corso". Funziona sulle parole intere. Se digiti due parole, come **cane grosso**, le trova una accanto all'altra, in una qualsiasi delle loro forme.
- **Ignora caratteri di punteggiatura** salta segni come trattini e punti tra le lettere. Digita **e-mail**, e trova anche "email".
- **Ignora caratteri di spazio** salta spazi e tabulazioni tra le lettere. Digita **e mail**, e trova "email".

Possono essere attive solo una tra **Suona come**, **Trova tutte le forme della parola** e **Usa caratteri jolly** alla volta. **Suona come** e **Trova tutte le forme della parola** trovano sempre parole intere, quindi **Solo parole intere** e **Maiuscole/minuscole** sono disattivate.

### Speciale

Fai clic su **Speciale** per inserire un codice in **Trova**: **Segno di paragrafo** (^p), **Carattere di tabulazione** (^t), **Qualsiasi cifra** (^#), **Qualsiasi lettera** (^$), **Spazio bianco** (^w), **Interruzione di sezione** (^b), **Campo** (^d), **Segno di nota a piè di pagina** (^f), **Segno di nota di chiusura** (^e), oppure **Carattere accento circonflesso** (^^). I codici funzionano anche senza **Usa caratteri jolly**. Un'**interruzione di sezione** è il segno che termina una sezione. Un **campo** è un pezzo di testo che il programma calcola, come un numero di pagina o una data, e **^d** trova il punto in cui ciascuno inizia. I segni di nota sono i numeretti nel testo che rimandano a una nota a piè di pagina o di chiusura. Questi quattro vengono trovati, ma **Sostituisci** e **Sostituisci tutto** non li modificano, perché cambiarne uno farebbe perdere ciò che rappresenta. Gli altri segni nella casella, come **?** e **\***, valgono allora solo come caratteri normali.

### Formato

Fai clic su **Formato** per trovare solo il testo con un certo aspetto: **Grassetto**, **Corsivo**, **Sottolineatura**, **Barrato**, **Evidenziato**, un **Carattere** oppure uno **Stile** di paragrafo. Il riquadro elenca sotto i pulsanti gli aspetti scelti. Scegli **Nessuna formattazione** per cancellarli.

Puoi lasciare **Trova** vuoto. Allora ogni porzione di testo con quell'aspetto è una corrispondenza. Con parole in **Trova**, vengono trovate solo le parole con quell'aspetto. **Sostituisci tutto** modifica soltanto quelle parole.

## Restringere la ricerca

- **Maiuscole/minuscole** trova solo le parole con le stesse maiuscole. Con l'opzione attiva, "Piano" non trova "piano".
- **Solo parole intere** salta le parole più lunghe che contengono quella cercata. Con l'opzione attiva, "pari" non trova "parità".
- **Cerca** stabilisce in quale direzione va **Trova successivo** a partire dal cursore. **Giù** va fino alla fine del documento, e **Su** fino all'inizio. Ciascuna si ferma lì e lo segnala. **Tutto** prosegue dall'altro capo ed è la scelta abituale. **Sostituisci tutto** cambia sempre l'intero documento.

## Sostituire parole

1. Premi **Ctrl+H**, oppure fai clic su **Home > Modifica > Sostituisci**. Il cursore va in **Sostituisci con**.
2. Digita le parole nuove in **Sostituisci con**. Per eliminare le parole trovate, lascia la casella vuota.
3. Digita le parole vecchie in **Trova**.
4. Fai clic su **Sostituisci** per vedere la prima corrispondenza. Fai clic di nuovo su **Sostituisci** per cambiarla. JeffWrite passa poi alla corrispondenza successiva. Ogni modifica è un passaggio, quindi **Annulla** (Ctrl+Z) le riporta indietro una alla volta.
5. Fai clic su **Sostituisci tutto** per cambiare tutte le corrispondenze in una volta sola. Il riquadro indica quante ne ha cambiate. **Sostituisci tutto** modifica anche le parole nelle caselle di testo e nelle forme, ed è un unico passaggio, quindi un solo **Annulla** riporta tutte le parole al loro posto. Quando **Revisioni** è attivo, ogni sostituzione viene registrata come modifica tracciata, nel testo e nelle caselle. Una casella troppo grande per essere modificata viene segnalata nel messaggio. **Sostituisci tutto** non raggiunge le caselle di testo nelle intestazioni e nei piè di pagina, le forme raggruppate né le caselle collegate tra loro. **Trova successivo** non cerca ancora nelle caselle di testo.

Le nuove parole assumono l'iniziale delle parole che sostituiscono. Se sostituisci una parola in grassetto, la nuova parola è in grassetto.

Ad esempio, per cambiare "color" in "colour" in tutto il documento: digita **color** in **Trova**, digita **colour** in **Sostituisci con**, e fai clic su **Sostituisci tutto**.

## Che cosa esamina la ricerca

- Esamina il testo del documento, comprese le tabelle. Un segnalibro nel mezzo di alcune parole, presente in alcuni file, non impedisce di trovarle. **Sostituisci** e **Sostituisci tutto** conservano il segnalibro.
- **Trova successivo** esamina prima il testo. Poi passa alle intestazioni, quindi ai piè di pagina, poi alle note a piè di pagina, e infine torna all'inizio del testo. Quando una corrispondenza si trova in una di queste parti, JeffWrite la apre per la modifica e seleziona la corrispondenza. Il riquadro indica dove si trova, ad esempio "Trovato nell'intestazione della pagina 3." **Sostituisci** cambia la corrispondenza su cui ti trovi, nel testo o in un'intestazione, in un piè di pagina o in una nota. **Sostituisci tutto** cambia ogni corrispondenza in tutte queste parti, e un solo **Annulla** le riporta tutte indietro. Le note di chiusura fanno parte del testo, quindi vengono cercate insieme ad esso. Vedi [Intestazioni e piè di pagina](headers-footers).
- Salta il testo impostato come **Nascosto**, perché quel testo non viene mostrato. Vedi [Modificare l'aspetto del testo](font).

## Argomenti correlati

- [Modificare l'aspetto del testo](font)
- [Usare JeffWrite con la tastiera](keyboard)
