# Lavorare con i fogli
<!-- keywords: foglio, fogli, foglio di lavoro, scheda, schede, linguetta, aggiungere, inserire, nuovo foglio, eliminare, rimuovere, cancellare, rinominare, nome, spostare, copiare, duplicare, riordinare, trascinare, nascondere, scoprire, mostrare, colore scheda, colore, selezionare più fogli, MAIUSC+F11, #RIF!, #REF!, riferimento 3D, Sposta o copia -->

Una cartella di lavoro è composta da **fogli**, e le **schede** in fondo alla finestra ne riportano i nomi. Fai clic su una scheda per visualizzare il foglio corrispondente. Ognuna delle modifiche descritte di seguito è un'unica operazione che puoi annullare e viene salvata nella cartella di lavoro.

## Aggiungere un foglio

- Fai clic sul pulsante **+** dopo l'ultima scheda per aggiungere un foglio alla fine.
- Premi **MAIUSC+F11**, oppure usa **Home > Inserisci > Inserisci foglio**, per aggiungere un foglio prima di quello visualizzato.

Il nuovo foglio è vuoto, riceve il primo nome libero (Foglio4, Foglio5 e così via) e si apre subito. Una cartella di lavoro può contenere fino a 1.000 fogli; oltre questo limite JeffCrunch lo segnala e non aggiunge nulla, perché non è in grado di aprire un file con più fogli.

## Rinominare un foglio

**Fai doppio clic** sulla scheda, digita il nuovo nome e premi **INVIO** (**ESC** annulla la modifica). Puoi anche usare **Home > Formato > Rinomina foglio**.

Un nome può contenere fino a 31 caratteri. Non può essere vuoto, non può contenere nessuno dei caratteri **: \ / ? * [ ]**, non può iniziare o terminare con un apostrofo e non può essere uguale al nome di un altro foglio (le maiuscole e le minuscole non contano come differenza). Il nome "History" è riservato al programma. Se il nome non viene accettato, JeffCrunch ne spiega il motivo e il foglio mantiene il nome precedente.

Tutte le formule che fanno riferimento al foglio seguono il nuovo nome. Esempio: **=Budget!B2** diventa **='Budget T1'!B2** quando rinomini Budget in Budget T1. Lo stesso vale per i nomi che fanno riferimento al foglio, per le regole dei formati condizionali e della convalida e per i grafici.

## Eliminare un foglio

Fai clic con il pulsante destro del mouse sulla scheda e scegli **Elimina foglio**, oppure usa **Home > Elimina > Elimina foglio**. Se il foglio contiene qualcosa, JeffCrunch chiede prima una conferma. Le formule di altri fogli che leggono il foglio eliminato mostrano **#RIF!**, a indicare che ciò che leggevano non esiste più. **Annulla** ripristina il foglio e quelle formule.

Una cartella di lavoro conserva sempre almeno un foglio visibile.

## Spostare o copiare un foglio

- **Trascina** una scheda in una nuova posizione per spostare il foglio.
- In alternativa, usa **Home > Formato > Sposta o copia foglio**. Scegli il foglio prima del quale inserire il foglio selezionato (oppure **(dopo l'ultimo foglio)**) e seleziona **Copiali e lascia gli originali dove sono** per copiarlo anziché spostarlo. La copia riceve un nome come **Budget (2)** e si apre.

Le formule continuano a leggere gli stessi fogli, ovunque si trovino. Una copia ha le stesse celle, formule, formati e dimensioni del foglio di origine. Un riferimento senza nome di foglio (ad esempio **A1**) legge la cella corrispondente della copia; un riferimento che specifica un foglio (ad esempio **Foglio1!A1**) continua a fare riferimento a quel foglio, anche se si tratta del foglio da cui proviene la copia.

Una copia mantiene anche la protezione, il filtro, i collegamenti, l'impostazione della pagina e le interruzioni di pagina, i gruppi di righe e colonne e l'aspetto delle barre dei dati dell'originale; i collegamenti seguono le righe inserite o eliminate, nell'originale prima della copia e nella copia dopo la sua creazione. Non include invece le immagini, i grafici, le note, le tabelle pivot, i grafici sparkline e i nomi che appartengono solo all'originale, perché JeffCrunch non è ancora in grado di copiarli; in questi casi indica quali elementi non sono stati copiati. Un foglio che contiene tabelle viene copiato insieme a copie delle tabelle (ognuna con un nome, un ID e una parte propri, e le formule della copia fanno riferimento alle sue tabelle). Eliminando un foglio vengono eliminate anche le sue tabelle; una formula di un altro foglio che faceva riferimento a una di esse mostra allora #RIF!, e Annulla la ripristina.

## Nascondere e mostrare un foglio

Fai clic con il pulsante destro del mouse sulla scheda e scegli **Nascondi foglio** (oppure usa **Home > Formato > Nascondi foglio**). Il foglio non viene visualizzato, ma rimane nella cartella di lavoro e le formule continuano a leggerlo. Per visualizzarlo di nuovo, fai clic con il pulsante destro del mouse su una scheda qualsiasi, scegli **Scopri foglio** e fai clic sul nome del foglio nell'elenco. L'elenco è vuoto quando non ci sono fogli nascosti.

## Colorare una scheda

Fai clic con il pulsante destro del mouse sulla scheda, scegli **Colore scheda** e seleziona un colore, oppure scegli **Nessun colore**. Il colore appare come una linea sotto il nome. Un colore scelto dalla tavolozza del tema viene salvato come il colore semplice che è in quel momento.

## Più fogli contemporaneamente

Fai **CTRL+clic** sulle schede per selezionarne più di una, oppure fai clic su una scheda e poi **MAIUSC+clic** su un'altra per selezionare tutte le schede comprese tra le due. Le schede selezionate sono evidenziate. **Elimina**, **Nascondi**, **Colore scheda** e **Sposta o copia** agiscono quindi su tutte in un'unica operazione. Fai clic su una scheda non selezionata per selezionare solo quella.

Ciò che digiti o formatti mentre sono selezionati più fogli modifica solo il foglio visualizzato.

## Operazioni non possibili

- In una cartella di lavoro che contiene una formula su un intervallo di fogli, ad esempio **=SOMMA(Foglio1:Foglio3!A1)**, non è ancora possibile aggiungere, eliminare, rinominare o spostare fogli, perché cambierebbero i fogli inclusi nella formula.
- Dopo il salvataggio, **Annulla** non può tornare oltre un foglio aggiunto, copiato, eliminato, rinominato o spostato prima del salvataggio. Il file salvato li contiene così come sono. L'occultamento, la visualizzazione e il colore delle schede possono ancora essere annullati.
