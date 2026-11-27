# Gestire i fogli

<!-- keywords: foglio, fogli, scheda, schede, aggiungi, inserisci, nuovo foglio, elimina, rimuovi, rinomina, nome, sposta, copia, duplica, riordina, trascina, nascondi, scopri, mostra, colore scheda, colore, seleziona più fogli, Maiusc+F11, #RIF!, riferimento 3D, Sposta o copia -->

Una cartella di lavoro è formata da **fogli**, e le **schede** in basso nella finestra ne riportano i nomi. Fai clic su una scheda per visualizzare quel foglio. Ogni modifica descritta di seguito è un singolo passaggio che puoi annullare, ed è salvata nella cartella di lavoro.

## Aggiungere un foglio

- Fai clic sul pulsante **+** dopo l'ultima scheda per aggiungere un foglio alla fine.
- Premi **Maiusc+F11**, oppure usa **Home > Inserisci > Inserisci foglio**, per aggiungere un foglio prima di quello che stai visualizzando.

Il nuovo foglio è vuoto, ha il primo nome libero (Foglio4, Foglio5 e così via) e si apre subito. Una cartella di lavoro può contenere fino a 1.000 fogli; oltre questo limite JeffCrunch lo segnala e non ne aggiunge altri, perché un file con più fogli non è uno che JeffCrunch apre.

## Rinominare un foglio

**Fai doppio clic** sulla scheda, digita il nuovo nome e premi **Invio** (**Esc** annulla). Puoi anche usare **Home > Formato > Rinomina foglio**.

Un nome può contenere fino a 31 caratteri. Non può essere vuoto, non può contenere nessuno dei caratteri **: \ / ? * [ ]**, non può iniziare o terminare con un apostrofo e non può coincidere con il nome di un altro foglio (le lettere maiuscole non fanno differenza). Il nome "History" è riservato al programma. Se il nome non viene accettato, JeffCrunch ne indica il motivo e il foglio mantiene il nome precedente.

Ogni formula che cita il foglio segue il nuovo nome. Esempio: **=Budget!B2** diventa **='Q1 Budget'!B2** quando rinomini Budget in Q1 Budget. Anche i nomi che puntano al foglio, le regole delle formattazioni condizionali e della convalida, e i grafici seguono il nuovo nome.

## Eliminare un foglio

Fai clic con il pulsante destro sulla scheda e scegli **Elimina foglio**, oppure usa **Home > Elimina > Elimina foglio**. Se il foglio contiene dati, JeffCrunch chiede prima conferma. Le formule di altri fogli che leggono il foglio eliminato mostrano **#RIF!**, che indica che ciò che leggevano non esiste più. **Annulla** ripristina il foglio e quelle formule.

Una cartella di lavoro conserva sempre almeno un foglio visibile.

## Spostare o copiare un foglio

- **Trascina** una scheda in una nuova posizione per spostare il foglio.
- Oppure usa **Home > Formato > Sposta o copia foglio**. Scegli il foglio davanti al quale collocare quello selezionato (oppure **(dopo l'ultimo foglio)**) e spunta **Copia e lascia gli originali dove si trovano** per copiarlo invece di spostarlo. Una copia riceve un nome come **Budget (2)** e si apre.

Le formule continuano a leggere i fogli che leggevano, qualunque sia la loro posizione. Una copia ha le stesse celle, formule, formati e dimensioni del foglio da cui deriva. Un riferimento senza nome di foglio (come **A1**) legge la cella della copia; un riferimento che cita un foglio (come **Foglio1!A1**) continua a indicare quel foglio, anche se è il foglio da cui la copia deriva.

Una copia ha anche la protezione, il filtro, i collegamenti, l'impostazione di pagina e le interruzioni di pagina, i gruppi di righe e colonne e l'aspetto delle barre dei dati dell'originale; i collegamenti seguono le righe inserite o eliminate, nell'originale prima della copia e nella copia dopo di essa. Non ha le immagini, i grafici, le note, le tabelle pivot, gli sparkline né i nomi che appartengono solo all'originale, perché JeffCrunch non li copia ancora; quando succede, lo indica. Un foglio con tabelle viene copiato insieme alle sue copie (ognuna con un nome, un identificativo e una parte propria, e le formule della copia fanno riferimento alle proprie tabelle). Eliminare un foglio elimina anche le sue tabelle; una formula di un altro foglio che ne citava una mostra allora #RIF!, e **Annulla** la ripristina.

Per copiare un foglio in **un'altra cartella di lavoro** aperta, sceglila in **Alla cartella di lavoro** nella stessa finestra. L'elenco mostra ciò che la copia non avrà (per ora, le immagini, i grafici, le formattazioni condizionali, le tabelle, le note e la modalità di stampa del foglio) e lo indica prima di copiare qualsiasi dato. Le formule che leggono altri fogli della prima cartella di lavoro diventano collegamenti al suo file, quindi la prima cartella di lavoro deve essere stata salvata almeno una volta in formato .xlsx. La copia mostra i valori che la prima cartella di lavoro ha ora, che cambiano quando i collegamenti vengono aggiornati (**Aggiorna valori** in **Dati > Modifica collegamenti**). Se la prima cartella di lavoro contiene modifiche non salvate, i collegamenti leggono il suo file salvato al momento dell'aggiornamento, quindi salvala per conservare questi valori. Le formule che leggono il foglio copiato restano sulla copia. Le celle che usano nomi, tabelle, una matrice o un intervallo a overflow, oppure che leggono una terza cartella di lavoro, vengono copiate come valori visualizzati, e l'elenco indica quante sono. Il modo in cui il foglio è visualizzato (righe e colonne bloccate, zoom, griglia e direzione di lettura), l'altezza predefinita delle righe e la larghezza predefinita delle colonne vengono copiati insieme ad esso. La copia ha lo stesso aspetto: formati, tipi di carattere e colori vengono aggiunti all'altra cartella di lavoro (un colore del tema della prima cartella di lavoro viene mantenuto come il colore che mostra quando i due temi sono diversi). Le due cartelle di lavoro devono contare le date dallo stesso anno (1900 o 1904). **Annulla** nell'altra cartella di lavoro elimina la copia; la prima cartella di lavoro non viene modificata. Lo spostamento di un foglio in un'altra cartella di lavoro non è ancora disponibile.

## Nascondere e scoprire un foglio

Fai clic con il pulsante destro sulla scheda e scegli **Nascondi foglio** (oppure usa **Home > Formato > Nascondi foglio**). Il foglio non viene visualizzato, ma resta nella cartella di lavoro e le formule continuano a leggerlo. Per mostrarlo di nuovo, fai clic con il pulsante destro su una scheda qualsiasi e scegli **Scopri foglio**, quindi fai clic sul suo nome nell'elenco. L'elenco è vuoto quando nessun foglio è nascosto.

## Colorare una scheda

Fai clic con il pulsante destro sulla scheda e scegli **Colore scheda**, poi scegli un colore oppure **Nessun colore**. Il colore appare come una linea sotto il nome. Un colore scelto dalla tavolozza del tema viene salvato come colore semplice, cioè come appare in quel momento.

## Più fogli contemporaneamente

Fai **Ctrl+clic** su più schede per selezionarne più di una, oppure fai clic su una scheda e poi **Maiusc+clic** su un'altra per selezionare tutte le schede comprese tra le due. Le schede selezionate sono evidenziate con un colore di sfondo. **Elimina**, **Nascondi**, **Colore scheda** e **Sposta o copia** agiscono allora su tutte come un unico passaggio. Fai clic su una scheda non selezionata per selezionare solo quella.

Ciò che digiti o formatti mentre sono selezionati più fogli cambia soltanto il foglio che stai visualizzando.

## Operazioni non disponibili

- Una cartella di lavoro che contiene una formula su un intervallo di fogli, come **=SOMMA(Foglio1:Foglio3!A1)**, non può avere fogli aggiunti, eliminati, rinominati o spostati, perché ciò cambierebbe i fogli coperti dalla formula.
- Dopo il salvataggio, **Annulla** non può tornare indietro oltre un foglio aggiunto, copiato, eliminato, rinominato o spostato prima del salvataggio. Il file salvato contiene questi fogli così come sono. Nascondere, scoprire e il colore della scheda possono ancora essere annullati.
