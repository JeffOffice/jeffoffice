# Trova e sostituisci parole
<!-- keywords: ricerca avanzata, suona come, soundex, forme della parola, tutte le forme di una parola, ignora punteggiatura, ignora spazi, speciale, segno di paragrafo, tabulazione, trova per formato, formato, grassetto, carattere, stile, caratteri jolly, carattere jolly, modello, espressione regolare, gruppi, conteggio corrispondenze, evidenzia tutte le corrispondenze, sostituisci, sostituisci tutto, cerca, trova, cambia parole, maiuscole/minuscole, parole intere, trova successivo, ctrl+f, ctrl+h, modifica, riquadro, testo nascosto, annulla, trova e sostituisci, ricerca e sostituzione -->

Usa Trova per passare alle parole nel documento. Usa Sostituisci per cambiarle. Entrambe funzionano in un unico riquadro sulla destra della finestra.

## Trovare parole

1. Premi **Ctrl+F**, oppure fai clic su **Home > Modifica > Trova**. Si apre il riquadro **Trova e sostituisci**, con il cursore in **Trova**.
2. Digita le parole che vuoi cercare. Poi premi **Invio**, oppure fai clic su **Trova successivo**.

JeffWrite parte dalla posizione del cursore. Seleziona il punto successivo in cui trova le parole e scorre le pagine in modo che tu lo veda. Questo punto si chiama **corrispondenza**. Fai di nuovo clic su **Trova successivo** per passare alla corrispondenza seguente. Dopo l'ultima corrispondenza, il programma torna all'inizio del documento.

Mentre digiti, il riquadro conta le corrispondenze, ad esempio "9 corrispondenze", e ogni corrispondenza riceve una fascia gialla nelle pagine. Le fasce compaiono anche nelle celle delle tabelle, nelle intestazioni, nei piè di pagina e nelle note a piè di pagina. Dopo **Trova successivo**, il riquadro indica quale corrispondenza stai visualizzando, ad esempio "Corrispondenza 2 di 4". Il conteggio e le fasce cambiano quando fai clic su **Maiuscole/minuscole** o **Solo parole intere**, oppure quando modifichi il testo. Scompaiono quando chiudi il riquadro.

## Trovare con i caratteri jolly

Seleziona **Usa caratteri jolly** per cercare un modello, non solo parole. Un **carattere jolly** è un segno che rappresenta altre lettere. Questi sono i segni:

- **?** rappresenta una lettera qualsiasi. **c?t** trova "cat" e "cot".
- **\*** rappresenta una sequenza qualsiasi di lettere. **c\*t** trova "cat" e "cart".
- **<** è l'inizio di una parola, e **>** è la fine. **<the>** trova "the" ma non "other".
- **[abc]** è una qualsiasi di queste lettere, **[a-c]** è una lettera da a a c, e **[!abc]** è una lettera diversa da quelle.
- **{2}** ripete due volte la lettera che lo precede. **{1,3}** la ripete da una a tre volte.
- **@** indica una o più ripetizioni della lettera che lo precede.
- **\\** davanti a un segno trova il segno stesso. **\\?** trova un punto interrogativo.
- **^p** è un segno di paragrafo, **^t** è una tabulazione, **^#** è una cifra, **^$** è una lettera, e **^w** è uno spazio bianco.

Racchiudi una parte del modello tra **( )** per creare un **gruppo**. In **Sostituisci con**, **\1** è il primo gruppo, **\2** il secondo, e così via fino a **\9**. **^&** è tutto ciò che è stato trovato. Ad esempio, cerca **(<[A-Z]@>), (<[A-Z]@>)** e sostituisci con **\2 \1** per trasformare "Rossi, Mario" in "Mario Rossi". **Solo parole intere** è disattivato mentre i caratteri jolly sono attivi, perché **<** e **>** svolgono già quella funzione.

Se il modello contiene un errore, il riquadro indica di che errore si tratta.

Se le parole non sono nel documento, il riquadro lo segnala e il cursore non si sposta.

Puoi continuare a lavorare nel documento mentre il riquadro è aperto. Fai clic nella pagina per scrivere. Fai clic sulla **X** in alto nel riquadro per chiuderlo.

## Trovare con le espressioni regolari

Seleziona **Usa espressioni regolari** per modelli più complessi dei caratteri jolly. Un'**espressione regolare** è un breve modello composto da lettere e segni. Si attiva al posto di **Usa caratteri jolly**: possono essere attivi solo uno dei due. Questi sono i segni più comuni:

- **.** è un carattere qualsiasi, **\d** è una cifra, **\w** è una lettera, una cifra o un trattino basso, e **\s** è uno spazio o una tabulazione. **[a-c]** è una lettera da a a c, e **[^abc]** è un carattere diverso da quelli.
- **^** è l'inizio di un paragrafo, **$** è la fine, e **\b** è il confine di una parola. **\bgatto\b** trova "gatto" ma non "gattino."
- **\*** indica nessuna o più ripetizioni di ciò che lo precede, **+** indica una o più ripetizioni, e **?** indica nessuna o una ripetizione. **{2}** significa esattamente due volte, e **{2,5}** significa da due a cinque volte; il numero può arrivare fino a 200. Aggiungi **?** dopo una ripetizione, come in **.+?**, per trovare la sequenza più breve possibile.
- **|** significa "oppure": **gatto|cane** trova una delle due parole.
- **( )** crea un **gruppo**. Usa **(?: )** per un gruppo che non ti serve in **Sostituisci con**.
- **\\** davanti a un segno trova il segno stesso: **\.** trova un punto. **\t** è una tabulazione.

**Maiuscole/minuscole** funziona come per le parole. Le lettere accentate e le lettere di altri alfabeti sono considerate lettere.

Il modello viene cercato un paragrafo alla volta, quindi **^** e **$** indicano l'inizio e la fine di ciascun paragrafo. Per cercare oltre la fine di un paragrafo, inserisci **\n** (un segno di paragrafo) nel modello: **casa\.\nRossi** trova "casa." alla fine di un paragrafo e "Rossi" all'inizio del successivo.

In **Sostituisci con**, **$1** o **\1** è il primo gruppo, **$2** o **\2** il secondo, e così via fino a nove gruppi. **$0** o **\0** è tutto ciò che è stato trovato. **$$** è un segno di dollaro, **\\** è una barra rovesciata, e **\t** è una tabulazione. Ad esempio, cerca **(\w+), (\w+)** e sostituisci con **$2 $1** per trasformare "Rossi, Mario" in "Mario Rossi."

**Sostituisci tutto** modifica tutte le corrispondenze nel testo, nelle intestazioni, nei piè di pagina, nelle note e nelle caselle di testo in un'unica operazione, quindi un solo **Annulla** le riporta tutte com'erano. Con **Revisioni** attivo, ogni modifica viene registrata come revisione, come accade per le parole. Il testo nascosto non viene modificato.

Se il modello contiene un errore, il riquadro indica vicino a quale carattere si trova. Un modello può non corrispondere a nessun carattere, come **^** (l'inizio di un paragrafo) o **$** (la fine): in quel caso **Sostituisci con** inserisce le sue parole in quel punto, quindi **^** con **> ** aggiunge quei segni all'inizio di ogni paragrafo. Un modello che richiederebbe troppo tempo sul tuo testo, come **(a+)+$** su una lunga riga di a, viene interrotto da JeffWrite, che lo segnala; non viene modificato nulla. Una ricerca lunga mostra una finestra **Ricerca in corso** con un pulsante **Annulla**; annullandola il documento resta com'era, e non puoi scrivere nel documento finché la ricerca non è terminata. I modelli che guardano avanti o indietro senza consumare ciò che controllano, come **x(?=\d)**, e i modelli in modalità estesa, come **(?x)**, non sono supportati; JeffWrite lo segnala. Un paragrafo molto lungo viene cercato a parti, e ogni corrispondenza è quella che una ricerca sull'intero paragrafo troverebbe. Un modello che corrisponde a un tratto molto lungo di un paragrafo molto lungo (decine di migliaia di caratteri) può essere interrotto con il messaggio "richiede troppo lavoro", e una singola corrispondenza di oltre 200.000 caratteri non viene sostituita: JeffWrite si ferma, lo segnala e non modifica nulla.

## Trova avanzata

Queste opzioni si trovano nel riquadro sotto **Usa espressioni regolari**. Ognuna cambia il modo in cui JeffWrite interpreta le parole in **Trova**.

- **Suona come** trova le parole che si pronunciano come la tua parola. Assegna a ogni parola un codice basato sui suoni e trova le parole con lo stesso codice. Digita **Rossi**, e trova anche "Rosi".
- **Trova tutte le forme della parola** trova ogni forma della parola. Digita **correre**, e trova "corre," "correva," "corso". Funziona sulle parole intere. Se digiti due parole, come **cane grande**, le trova una accanto all'altra, in qualsiasi loro forma.
- **Ignora i caratteri di punteggiatura** salta segni come trattini e punti tra le lettere. Digita **email**, e trova anche "e-mail".
- **Ignora i caratteri di spazio** salta gli spazi e le tabulazioni tra le lettere. Digita **e mail**, e trova "email".

Solo una tra **Suona come**, **Trova tutte le forme della parola**, **Usa caratteri jolly** e **Usa espressioni regolari** può essere attiva alla volta. **Suona come** e **Trova tutte le forme della parola** trovano sempre parole intere, quindi **Solo parole intere** e **Maiuscole/minuscole** sono disattivati.

### Speciale

Fai clic su **Speciale** per inserire un codice in **Trova**: **Segno di paragrafo** (^p), **Carattere di tabulazione** (^t), **Qualsiasi cifra** (^#), **Qualsiasi lettera** (^$), **Spazio bianco** (^w), **Interruzione di sezione** (^b), **Campo** (^d), **Segno di nota a piè di pagina** (^f), **Segno di nota di chiusura** (^e), o **Carattere di accento circonflesso** (^^). I codici funzionano anche senza **Usa caratteri jolly**. Un'**interruzione di sezione** è il segno che termina una sezione. Un **campo** è una porzione di testo che il programma calcola, come un numero di pagina o una data, e **^d** trova l'inizio di ciascuno. I segni di nota sono i piccoli numeri nel testo che rimandano a una nota a piè di pagina o di chiusura. Questi quattro vengono trovati, ma **Sostituisci** e **Sostituisci tutto** non li modificano, perché cambiarli farebbe perdere ciò che rappresentano. Gli altri segni nell'elenco, come **?** e **\***, sono considerati semplici caratteri.

### Formato

Fai clic su **Formato** per trovare solo il testo con un certo aspetto: **Grassetto**, **Corsivo**, **Sottolineato**, **Barrato**, **Evidenziato**, un **Carattere**, oppure uno **Stile** di paragrafo. Il riquadro elenca sotto i pulsanti gli aspetti scelti. Seleziona **Nessuna formattazione** per cancellarli.

Puoi lasciare vuoto **Trova**. In questo caso, ogni porzione di testo con quell'aspetto è una corrispondenza. Se in **Trova** ci sono parole, vengono trovate solo quelle con quell'aspetto. **Sostituisci tutto** modifica solo quelle parole.

## Restringere la ricerca

- **Maiuscole/minuscole** trova solo le parole con le stesse maiuscole. Con questa opzione attiva, "Piano" non trova "piano".
- **Solo parole intere** salta le parole più lunghe che contengono la tua. Con questa opzione attiva, "pari" non trova "parità".
- **Cerca** stabilisce in quale direzione va **Trova successivo** a partire dal cursore. **Giù** va fino alla fine del documento, e **Su** fino all'inizio. In entrambi i casi il programma si ferma e lo indica. **Tutto** continua dall'altra estremità ed è la scelta abituale. **Sostituisci tutto** modifica sempre l'intero documento.

## Sostituire parole

1. Premi **Ctrl+H**, oppure fai clic su **Home > Modifica > Sostituisci**. Il cursore va in **Sostituisci con**.
2. Digita le nuove parole in **Sostituisci con**. Per eliminare le parole trovate, lascia il campo vuoto.
3. Digita le parole da cambiare in **Trova**.
4. Fai clic su **Sostituisci** per vedere la prima corrispondenza. Fai di nuovo clic su **Sostituisci** per modificarla. JeffWrite passa poi alla corrispondenza successiva. Ogni modifica è un'operazione distinta, quindi **Annulla** (Ctrl+Z) la riporta indietro una alla volta.
5. Fai clic su **Sostituisci tutto** per modificare tutte le corrispondenze in una volta. Il riquadro indica quante ne ha modificate. **Sostituisci tutto** modifica anche le parole nelle caselle di testo e nelle forme, ed è un'unica operazione, quindi un solo **Annulla** rimette a posto tutte le parole. Con **Revisioni** attivo, ogni sostituzione viene registrata come modifica tracciata, nel testo e nelle caselle. Una casella troppo grande per essere modificata viene conteggiata nel messaggio. **Sostituisci tutto** non raggiunge le caselle di testo nelle intestazioni e nei piè di pagina, le forme raggruppate né le caselle collegate tra loro. **Trova successivo** non cerca ancora nelle caselle di testo.

Le nuove parole assumono l'iniziale maiuscola o minuscola delle parole che sostituiscono. Se sostituisci una parola in grassetto, la nuova parola sarà in grassetto.

Ad esempio, per cambiare "color" in "colour" ovunque: digita **color** in **Trova**, digita **colour** in **Sostituisci con**, e fai clic su **Sostituisci tutto**.

## Cosa cerca la ricerca

- Cerca nel testo del documento, comprese le tabelle. Un segnalibro inserito nel mezzo di alcune parole, che alcuni file contengono, non impedisce di trovarle. **Sostituisci** e **Sostituisci tutto** mantengono il segnalibro.
- **Trova successivo** cerca prima nel testo. Poi passa alle intestazioni, quindi ai piè di pagina, alle note a piè di pagina, e infine torna all'inizio del testo. Quando una corrispondenza si trova in una di queste parti, JeffWrite la apre per la modifica e seleziona la corrispondenza. Il riquadro indica dove si trova, ad esempio "Trovato nell'intestazione della pagina 3". **Sostituisci** sostituisce la corrispondenza su cui ti trovi, nel testo o in un'intestazione, un piè di pagina o una nota a piè di pagina. **Sostituisci tutto** modifica ogni corrispondenza in tutte queste parti, e un solo **Annulla** le riporta tutte indietro. Le note di chiusura fanno parte del testo, quindi vengono cercate insieme ad esso. Vedi [Intestazioni e piè di pagina](headers-footers).
- Salta il testo impostato come **Nascosto**, perché quel testo non viene visualizzato. Vedi [Modificare l'aspetto del testo](font).

## Argomenti correlati

- [Modificare l'aspetto del testo](font)
- [Usare JeffWrite con la tastiera](keyboard)
