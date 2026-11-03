# Usare i fogli
<!-- keywords: foglio, fogli, foglio di lavoro, scheda, schede, linguetta, aggiungere, inserire, nuovo foglio, eliminare, rimuovere, cancellare, rinominare, nome, spostare, copiare, duplicare, riordinare, trascinare, nascondere, scoprire, mostrare, colore scheda, colore, selezionare più fogli, Maiusc+F11, #RIF!, riferimento 3D, Sposta o copia -->

Una cartella di lavoro è composta da **fogli** e le **schede** in fondo alla finestra ne riportano i nomi. Fai clic su una scheda per visualizzare il foglio corrispondente. Ognuna delle modifiche descritte di seguito è un'operazione che puoi annullare e viene salvata nella cartella di lavoro.

## Aggiungere un foglio

- Fai clic sul pulsante **+** dopo l'ultima scheda per aggiungere un foglio alla fine.
- Premi **Maiusc+F11** oppure usa **Home > Inserisci > Inserisci foglio** per aggiungere un foglio prima di quello visualizzato.

Il nuovo foglio è vuoto, riceve il primo nome libero (Foglio4, Foglio5 e così via) e si apre subito. Una cartella di lavoro può contenere fino a 1.000 fogli; oltre questo limite JeffCrunch lo segnala e non aggiunge nulla, perché non sarebbe più in grado di aprire un file con più fogli.

## Rinominare un foglio

**Fai doppio clic** sulla scheda, digita il nuovo nome e premi **Invio** (**Esc** annulla l'operazione). Puoi anche usare **Home > Formato > Rinomina foglio**.

Un nome può contenere fino a 31 caratteri. Non può essere vuoto, non può contenere nessuno dei caratteri **: \ / ? * [ ]**, non può iniziare o terminare con un apostrofo e non può coincidere con il nome di un altro foglio (le maiuscole non contano come differenza). Il nome "History" è riservato al programma. Se il nome viene rifiutato, JeffCrunch ne indica il motivo e il foglio mantiene il nome precedente.

Tutte le formule che citano il foglio si aggiornano con il nuovo nome. Esempio: **=Budget!B2** diventa **='Budget T1'!B2** quando rinomini Budget in Budget T1. Si aggiornano anche i nomi che fanno riferimento al foglio, le regole dei formati condizionali e della convalida, e i grafici.

## Eliminare un foglio

Fai clic con il pulsante destro del mouse sulla scheda e scegli **Elimina foglio**, oppure usa **Home > Elimina > Elimina foglio**. Se il foglio contiene qualcosa, JeffCrunch chiede prima conferma. Le formule di altri fogli che leggevano il foglio eliminato mostrano **#RIF!**, a indicare che i dati letti non esistono più. **Annulla** ripristina il foglio e quelle formule.

Una cartella di lavoro mantiene sempre almeno un foglio visibile.

## Spostare o copiare un foglio

- **Trascina** una scheda in una nuova posizione per spostare il foglio.
- In alternativa, usa **Home > Formato > Sposta o copia foglio**. Scegli il foglio davanti al quale inserire il foglio selezionato (oppure **(dopo l'ultimo foglio)**) e seleziona **Copia, lasciando gli originali dove sono** per copiarlo anziché spostarlo. La copia riceve un nome come **Budget (2)** e si apre.

Le formule continuano a leggere gli stessi fogli, ovunque si trovino. Una copia ha le stesse celle, formule, formati e dimensioni del foglio di origine. Un riferimento senza nome di foglio (ad esempio **A1**) legge la cella della copia stessa; un riferimento che cita un foglio (ad esempio **Foglio1!A1**) continua a citare quel foglio, anche se si tratta del foglio di origine della copia.

La copia mantiene anche la protezione, il filtro, i collegamenti, l'impostazione della pagina e le interruzioni di pagina, i gruppi di righe e colonne e l'aspetto delle barre dei dati dell'originale; i collegamenti seguono le righe inserite o eliminate, nell'originale prima della copia e nella copia dopo. Non vengono invece copiati le immagini, i grafici, le note, le tabelle pivot, i grafici sparkline e i nomi che appartengono solo all'originale, perché JeffCrunch non li copia ancora; in questi casi il programma indica quali elementi sono esclusi. Un foglio che contiene tabelle viene copiato insieme a copie delle tabelle (ognuna con nome, ID e parte propri, e le formule della copia citano le sue tabelle). Eliminando un foglio vengono eliminate anche le sue tabelle; una formula di un altro foglio che ne citava una mostra allora #RIF!, e Annulla la ripristina.

## Nascondere e mostrare un foglio

Fai clic con il pulsante destro del mouse sulla scheda e scegli **Nascondi foglio** (oppure usa **Home > Formato > Nascondi foglio**). Il foglio non è visibile, ma resta nella cartella di lavoro e le formule continuano a leggerlo. Per visualizzarlo di nuovo, fai clic con il pulsante destro del mouse su una scheda qualsiasi, scegli **Scopri foglio** e fai clic sul nome del foglio nell'elenco. L'elenco è vuoto se non ci sono fogli nascosti.

## Colorare una scheda

Fai clic con il pulsante destro del mouse sulla scheda, scegli **Colore scheda** e seleziona un colore, oppure scegli **Nessun colore**. Il colore compare come una linea sotto il nome. Un colore scelto dalla tavolozza del tema viene salvato come il colore fisso che è in quel momento.

## Più fogli contemporaneamente

Usa **Ctrl+clic** sulle schede per selezionarne più di una, oppure fai clic su una scheda e poi **Maiusc+clic** su un'altra per selezionare tutte le schede comprese tra le due. Le schede selezionate appaiono ombreggiate. A quel punto **Elimina**, **Nascondi**, **Colore scheda** e **Sposta o copia** agiscono su tutte in un'unica operazione. Fai clic su una scheda non selezionata per selezionare solo quella.

Ciò che digiti o formatti mentre sono selezionati più fogli modifica solo il foglio visualizzato.

## Limitazioni

- In una cartella di lavoro che contiene una formula su un intervallo di fogli, ad esempio **=SOMMA(Foglio1:Foglio3!A1)**, per ora non è possibile aggiungere, eliminare, rinominare o spostare fogli, perché cambierebbero i fogli inclusi nella formula.
- Dopo il salvataggio, **Annulla** non può tornare oltre un foglio aggiunto, copiato, eliminato, rinominato o spostato prima del salvataggio. Il file salvato contiene i fogli così come sono. L'occultamento, la visualizzazione e il colore delle schede possono invece ancora essere annullati.
