# Riempire celle e sommarle

<!-- keywords: riempi, serie, trascina, maniglia, doppio clic, elenco personalizzato, riempimento automatico, AutoFill, Ctrl+D, Ctrl+R, giù, destra, su, sinistra, numero, data, giorno feriale, mese, anno, passo, arresta, copia, somma, Somma, media, conteggio, massimo, minimo, Alt+=, totale, riempimento rapido, Ctrl+E, motivo, dividi nomi, riempi, riempimento lampo, tra fogli, fogli di lavoro -->

## Riempire

Il riempimento continua ciò che hai iniziato. Seleziona le celle che hai compilato, poi trascina il piccolo quadratino nell'angolo della selezione (la **maniglia di riempimento**) lungo una colonna o una riga, e rilascia. Un bordo tratteggiato mostra fino a dove arriverà il riempimento.

| Inizi con | Il riempimento produce |
| --- | --- |
| 1, 2 | 3, 4, 5 e così via: il passo tra i valori continua |
| Un solo numero, come 5 | 5, 5, 5: copie (tieni premuto Ctrl mentre trascini per ottenere l'effetto opposto) |
| Una data | I giorni successivi (usa il pulsante **Opzioni di riempimento**, oppure **Riempi > Serie**, per giorni feriali, mesi o anni) |
| Lun, oppure Gennaio | Mar, Mer, ... oppure Febbraio, Marzo, ... nella tua lingua |
| Voce 1 | Voce 2, Voce 3, ... |
| Una parola di un elenco personalizzato, come Piccolo | La parola successiva dell'elenco, e di nuovo la prima dopo l'ultima |
| Una formula | La formula copiata, con i riferimenti spostati come in un incolla |

Fai doppio clic sulla maniglia di riempimento per riempire verso il basso fino a dove arrivano i dati accanto alla selezione. JeffCrunch controlla prima la colonna a sinistra, poi quella a destra. Si ferma alla prima cella vuota di quella colonna.

## Elenchi personalizzati

Un **elenco personalizzato** è una serie di parole che il riempimento scorre, come i giorni della settimana. Per crearne uno, fai clic su **File > Opzioni > Elenchi personalizzati**. Digita un elenco per riga, con le parole separate da virgole, ad esempio `Piccolo, Medio, Grande`. Fai clic su **OK**. Ora digita **Piccolo** in una cella e trascina la maniglia di riempimento: le celle si riempiranno con Medio, Grande, Piccolo, e così via. Le maiuscole non contano quando JeffCrunch cerca una parola in un elenco, e le nuove celle copiano le maiuscole della cella di partenza. Un elenco richiede almeno due parole. I giorni della settimana e i mesi sono già predefiniti.

**Riempi > Giù** (**Ctrl+D**) e **Riempi > Destra** (**Ctrl+R**) copiano la prima riga o colonna della selezione sulle altre. **Riempi > Su** e **Riempi > Sinistra** lavorano partendo dall'ultima. **Riempi > Serie** chiede il tipo (lineare, crescita, data), l'unità di una data, il passo e il punto in cui fermarsi. Ogni cella riempita assume il formato della cella che la precede. Una cella che non può essere modificata (parte di una matrice, una cella bloccata o l'intestazione di una tabella) interrompe il riempimento, e non cambia nulla.

Dopo aver trascinato la maniglia di riempimento, accanto all'angolo di ciò che è stato riempito compare il piccolo pulsante **Opzioni di riempimento**. Il suo menu ripete il riempimento in un altro modo, e il riempimento resta un'unica operazione annullabile con Annulla: **Copia celle**, **Riempi serie**, **Riempi solo formattazione** (le celle conservano il contenuto e assumono il formato), **Riempi senza formattazione** (le celle conservano il proprio formato) e, per le date, **Riempi giorni**, **Riempi giorni feriali**, **Riempi mesi** e **Riempi anni**. Il pulsante scompare quando fai qualsiasi altra operazione.

Il riempimento non entra né esce da celle unite, perché lascerebbe valori in celle nascoste dall'area unita. Separale prima.

**Riempi > Giustifica** distribuisce il testo delle celle selezionate di una colonna su righe larghe quanto la colonna, nelle celle a partire dalla prima verso il basso; le celle rimanenti della selezione vengono svuotate. Se il testo richiede più celle di quelle selezionate, JeffCrunch chiede se usare le celle sottostanti. Funziona solo con il testo.

**Riempi > Riempimento rapido** (**Ctrl+E**) riempie una colonna per te a partire da un modello che gli mostri. Digita a mano il risultato desiderato nella prima o nelle prime due celle di una colonna, accanto alle celle da cui deriva. Poi posiziona il cursore in quella colonna e scegli Riempimento rapido. JeffCrunch ricava come sono stati creati i tuoi esempi a partire dalle celle accanto, e riempie le celle vuote nello stesso modo. Esempio: la colonna A contiene nomi come **Rossi, Anna**. In B1 digiti **Anna Rossi**. Il Riempimento rapido compila B2 e le celle successive con gli altri nomi invertiti allo stesso modo. Può prendere una parola da una cella, la prima lettera di una parola, o le prime o ultime lettere, in maiuscolo, minuscolo o come sono scritte, e può aggiungere un testo uguale in ogni esempio, come uno spazio o un punto. Se non trova un modello, lo segnala. Digita uno o due esempi in più e riprova. È un'unica operazione annullabile con Annulla.

**Riempi > Riempi tra fogli di lavoro** copia le celle selezionate nelle stesse celle dei fogli che selezioni in un elenco: tutto il contenuto, solo il contenuto o solo i formati. È un'unica operazione annullabile con Annulla.

## Somma

**Home > Somma** (**Alt e il segno di uguale**) inserisce un totale nella cella selezionata. Esamina i numeri sopra la cella; se non ce ne sono, quelli a sinistra. La formula viene inserita nella cella perché tu possa verificarla, e **Invio** la conferma. La freccia accanto a Somma offre gli altri totali: **Media**, **Conteggio numeri**, **Max** e **Min**.

Se selezioni prima un intervallo, ogni colonna dell'intervallo riceve la propria formula nelle celle vuote sotto di essa (oppure, per una sola riga, nelle celle a destra).
