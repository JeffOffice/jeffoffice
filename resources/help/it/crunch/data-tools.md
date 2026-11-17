# Strumenti dati

<!-- keywords: modulo, modulo dati, inserimento dati, record, registrazioni, nuovo record, trova successivo, trova precedente, criteri, gestione scenari, scenari, what-if, che cosa succede se, riepilogo scenari, dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, ricerca obiettivo, subtotale, totali, filtro avanzato, condizioni, convalida dati, cerchia dati non validi, ovale rosso, cancella cerchi di convalida, consolida, combina intervalli, consenti, elenco, menu a discesa, messaggio di input, avviso di errore, righe ripetute, univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette, modifica collegamenti, aggiorna valori, cambia origine, collegamenti, esterno, origine, interrompi collegamento, verifica stato, apri origine

Il gruppo **Dati** della scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Equivale a **Apri file di testo** nella scheda **File**. JeffCrunch chiede come è suddiviso il file in colonne e che tipo di testo contiene, quindi lo mostra come nuova cartella di lavoro.

## Modifica collegamenti

Se una cartella di lavoro contiene formule che leggono un'altra cartella di lavoro (ad esempio ='[Prezzi.xlsx]Foglio1'!A1), **Modifica collegamenti** nel gruppo **Recupera dati** elenca le altre cartelle di lavoro, dette origini. Ogni riga indica dove si trova l'origine e il suo stato. Finché la cartella di lavoro non viene riaperta con le sue origini, mostra i numeri che il suo file ha memorizzato da esse.

- **Verifica stato** cerca ogni origine su questo computer e indica **Trovata**, oppure che non è stata trovata. Un'origine sul Web non viene scaricata, quindi il suo stato indica che non è stata verificata.
- **Aggiorna valori** legge dal computer l'origine scelta e assegna a ogni cella che è solo un riferimento a una sua cella il valore che tale cella ha ora. Una cella che svolge un calcolo sul valore, come `=[1]Prezzi!A1*2`, mantiene il numero memorizzato nel file; il messaggio sotto l'elenco indica quante celle sono state aggiornate e quante no. Aggiorna valori è un'unica operazione, quindi **Annulla** ripristina i valori precedenti. I nuovi valori vengono scritti anche nella parte del file dedicata al collegamento al salvataggio, così la cartella di lavoro li mostra alla successiva apertura, anche quando l'origine non è presente.
- **Cambia origine** consente di scegliere un altro file per il collegamento selezionato, ad esempio quando la cartella di lavoro è stata spostata. Le formule restano invariate. Il nuovo percorso viene scritto nel file al salvataggio, e **Annulla** ripristina il percorso precedente.
- **Apri origine** apre l'origine scelta in una nuova finestra, quando può essere trovata.
- **Interrompi collegamento** trasforma ogni cella che legge l'origine scelta nel valore che mostra ora, e un nome che indicava una posizione al suo interno in **#RIF!**. JeffCrunch chiede conferma prima. Interrompi collegamento è un'unica operazione, quindi **Annulla** ripristina formule, nomi e collegamenti, fino al salvataggio della cartella di lavoro. Quando non rimane alcun collegamento, il file viene salvato senza le parti che li contenevano.

## Modulo

**Modulo** (nel gruppo **Strumenti dati** e nella casella di ricerca) mostra una riga di un elenco alla volta, come un piccolo modulo. Un elenco è una tabella, oppure un intervallo di celle con una riga di nomi in alto e una riga di dati sotto ciascun nome. Fai prima clic su una cella qualsiasi dell'elenco.

- Ogni casella corrisponde a una colonna ed è indicata dal nome nella riga superiore. Digita in una casella per modificare il valore. Una casella in grigio contiene una formula: il modulo mostra il risultato, e la formula va modificata nel foglio.
- **Trova successivo** e **Trova precedente** passano alla riga successiva e a quella precedente. Passando a un'altra riga, oppure premendo **Invio**, viene scritto ciò che hai digitato in questa riga.
- **Nuovo** mostra un modulo vuoto. Compilalo, quindi premi di nuovo **Nuovo**, **Trova successivo** o **Chiudi**. La riga viene aggiunta sotto l'ultima riga dell'elenco. Una tabella cresce di una riga. Se una cella sotto l'elenco non è vuota, il modulo lo segnala e non aggiunge nulla.
- **Elimina** rimuove la riga visualizzata, dopo aver chiesto conferma. Le righe sotto di essa salgono di una posizione. Una tabella mantiene sempre almeno una riga di dati. Una riga nascosta, o nascosta da un filtro, non viene mostrata nel modulo, non viene conteggiata e non viene mai eliminata dal modulo. Se l'elenco è troppo lungo per essere letto per intero, il modulo indica che mostra solo le prime righe.
- **Ripristina** annulla ciò che hai digitato in questa riga, prima di lasciarla.
- **Criteri** consente di cercare righe. Digita una condizione in una o più caselle, quindi premi **Trova successivo** o **Trova precedente**. Una condizione è un valore, come **Ann** (il testo corrisponde all'inizio della parola; **?** e **\*** sostituiscono lettere), oppure un valore preceduto da un segno: **>100**, **<=5**, **<>0**, **=Oslo**. Una riga deve soddisfare tutte le condizioni digitate. Premi **Modulo** per tornare indietro, oppure **Cancella** per ricominciare. Mentre sono impostati dei criteri, **Trova successivo** e **Trova precedente** saltano le righe che non li soddisfano.
- **Chiudi** scrive la riga e chiude il modulo.

Ogni riga che modifichi, aggiungi o elimini è un'unica operazione, quindi **Annulla** nel foglio la ripristina in un solo passaggio.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda **Home**. Vedi [Ordinamento e filtro](sort-filter).

## Testo in colonne

Usa questa funzione quando una colonna contiene più parti di testo che vuoi inserire in colonne separate, come "Rossi, Anna, 31".

1. Seleziona le celle di **una sola** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi con un separatore**: seleziona i segni che si trovano tra le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il segno desiderato. **Considera separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Virgolette di testo** indica il segno che racchiude una parte che non deve essere divisa, come "Lee, Bo". All'interno delle virgolette, due segni consecutivi valgono come uno.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri a partire da 1. Ad esempio, 5, 12 inizia una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ogni parte: **Generale** legge numeri e date come se li avessi digitati, mentre **Testo** mantiene ogni parte così com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente corrisponde alla prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise secondo le scelte fatte. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella, sostituendo il contenuto che vi si trovava. Testo in colonne è un'unica operazione che puoi annullare.

## Rimuovi duplicati

Usa questa funzione per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco, oppure una cella al suo interno. JeffCrunch prende l'intero blocco di dati attorno a una singola cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno una riga di intestazione** se la prima riga contiene i nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo quando ogni colonna selezionata è uguale. Le lettere maiuscole e minuscole sono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i rispettivi formati e formule. Le celle sotto la selezione non si spostano. Le righe vuote lasciate in fondo alla selezione vengono svuotate. Rimuovi duplicati è un'unica operazione che puoi annullare.

## Ricerca obiettivo

Usa questa funzione quando conosci il risultato che vuoi ma non il numero che lo produce. Ad esempio, una cella di pagamento contiene una formula che usa un tasso di interesse, e vuoi che il pagamento sia 500. Ricerca obiettivo prova dei numeri nella cella del tasso finché il pagamento non è 500.

1. Scegli **Ricerca obiettivo** nella scheda **Dati**.
2. In **Cella con la formula**, digita la cella che contiene la formula, ad esempio B3. Inizialmente corrisponde alla cella selezionata.
3. In **Valore desiderato**, digita il risultato che la formula deve restituire.
4. In **Cella da modificare**, digita la cella che Ricerca obiettivo può modificare, ad esempio B1. Deve contenere un numero, non una formula.
5. Premi **OK**.

Se Ricerca obiettivo trova un numero, questo resta nella cella da modificare, e la barra di stato ne indica il valore. Se non riesce, nulla cambia e viene segnalato. Prova con un valore iniziale diverso nella cella da modificare, e verifica che la formula usi davvero quella cella. Ricerca obiettivo è un'unica operazione che puoi annullare.

## Gestione scenari

Uno scenario è un insieme con nome di valori per alcune celle. Usa gli scenari per provare ipotesi "what if" su un unico foglio. Ad esempio, conserva uno scenario "Vendite basse" e uno "Vendite alte" per le stesse celle di input, e mostra quello che ti interessa.

1. Scegli **Gestione scenari** nella scheda **Dati**.
2. Premi **Aggiungi**. Digita un nome, ad esempio Vendite alte. In **Celle modificate** digita le celle che lo scenario imposta, ad esempio B1:B3 oppure D5. Uno scenario può modificare fino a 32 celle. Le celle selezionate vengono inserite automaticamente. Ogni cella parte con il valore che contiene ora. Nella tabella digita il valore che questo scenario deve usare, quindi premi **OK**.
3. Scegli uno scenario nell'elenco e premi **Mostra** per inserire i suoi valori nelle celle. Anche un doppio clic sul nome ha lo stesso effetto. Premi **Modifica** per cambiarne il nome, il commento o i valori. Premi **Elimina** per rimuoverlo.

Mostra, Aggiungi, Modifica ed Elimina sono ciascuna un'operazione che puoi annullare.

**Riepilogo** colloca tutti gli scenari affiancati in un nuovo foglio chiamato Riepilogo scenari. Chiede le celle risultato, ossia le celle che vuoi confrontare, ad esempio il totale in B10. Per ogni scenario, il riepilogo mostra i valori delle celle modificate e i risultati delle celle risultato, calcolati con i valori di quello scenario. Una colonna chiamata Valori correnti mostra come erano le celle quando hai creato il riepilogo. Le tue celle restano come erano.

Gli scenari vengono salvati nella cartella di lavoro. Una cartella creata con un altro programma conserva i suoi scenari, che qui vengono visualizzati. Una copia salvata come file .xls meno recente o come .ods non li conserva, e il salvataggio lo segnala.

## Subtotale

Usa questa funzione per inserire una riga di totali sotto ogni gruppo di righe che hanno la stessa voce in una colonna, e un totale generale in fondo. L'elenco deve avere una riga di intestazione e dovrebbe essere ordinato in base alla colonna di raggruppamento, così le voci uguali sono vicine.

1. Seleziona una cella dell'elenco.
2. Scegli **Subtotale** nel gruppo **Struttura** della scheda **Dati**.
3. In **A ogni variazione di**, scegli la colonna che raggruppa le righe, ad esempio Regione.
4. In **Usa funzione**, scegli cosa calcolare: **Somma**, **Conta**, **Media**, **Max**, **Min** e altre.
5. In **Aggiungi subtotale a**, seleziona ogni colonna da totalizzare.
6. Lascia selezionata l'opzione **Sostituisci subtotali correnti** per sostituire gli eventuali subtotali già presenti. Premi **OK**.

Ogni riga di totale riporta "Totale Est" (o la voce a cui si riferisce) e contiene una formula SUBTOTALE, così esclude le altre righe di totale. Le righe di ogni gruppo vengono raggruppate, quindi puoi usare i pulsanti 1, 2 e 3 accanto alle righe per mostrare solo i totali. Per eliminare i totali, apri di nuovo **Subtotale** e premi **Rimuovi tutto**. Aggiungere o rimuovere subtotali è un'unica operazione che puoi annullare.

## Filtro avanzato

I pulsanti di filtro scelgono le righe una colonna alla volta. **Avanzate** (nel gruppo **Ordina e filtra**) consente di scrivere le condizioni nel foglio, così possono essere lunghe quanto serve.

1. Accanto all'elenco, copia le intestazioni delle colonne da verificare in una riga di celle vuote.
2. Sotto ogni intestazione, scrivi una condizione. Le condizioni nella **stessa riga** devono essere tutte vere (è la "e"). Le condizioni in **righe diverse** sono alternative: una riga dell'elenco deve soddisfarne solo una di quelle righe (è la "o").
3. Scegli **Avanzate**. Verifica che **Intervallo elenco** sia l'elenco con la sua riga di intestazione, e digita nella casella **Intervallo condizioni** le celle delle intestazioni e delle condizioni.
4. Scegli **Nascondi le altre righe nell'elenco**, oppure **Copia le righe in un'altra posizione** e digita la prima cella della posizione, ad esempio H1. Seleziona **Escludi le righe che ripetono una precedente** per copiarne solo una per ciascuna.
5. Premi **OK**.

Come scrivere una condizione:

- Una parola, ad esempio **An**, seleziona il testo che inizia con essa. Le lettere maiuscole e minuscole sono considerate uguali. **?** sostituisce un solo carattere qualsiasi e **\*** una sequenza qualsiasi di caratteri.
- Un numero seleziona le celle che gli sono uguali.
- **=Ann** seleziona il testo esattamente uguale ad Ann. **=** da solo seleziona le celle vuote.
- **>**, **>=**, **<**, **<=** e **<>** (diverso) davanti a un numero o a una parola li confrontano, ad esempio **>25**. **<>** da solo seleziona le celle non vuote.

Una riga di condizioni vuota seleziona tutte le righe. Per mostrare di nuovo le righe nascoste, scegli **Cancella**. La copia viene creata subito e non cambia quando cambia l'elenco. Filtro avanzato è un'unica operazione che puoi annullare.

## Convalida dati

Usa questa funzione per stabilire cosa può essere digitato in alcune celle, in modo che un inserimento errato venga bloccato o messo in discussione.

1. Seleziona le celle.
2. Scegli **Convalida dati** nel gruppo **Strumenti dati** della scheda **Dati**.
3. Nella scheda **Impostazioni**, scegli cosa **Consenti**: **Qualsiasi valore** (nessuna regola), **Numero intero**, **Decimale**, **Elenco**, **Data**, **Ora**, **Lunghezza testo** oppure **Personalizzata**.
4. Per un numero, una data, un'ora o una lunghezza del testo, scegli **Dati** (**compreso tra**, **non compreso tra**, **uguale a** e così via) e digita i limiti. Un limite può essere un valore o una formula che inizia con =.
5. Per un **Elenco**, digita le scelte separate da virgole, ad esempio Rosso,Verde,Blu, oppure digita un intervallo di celle dopo =, ad esempio =$A$1:$A$5. La cella mostrerà una freccia con le scelte. Disattiva **Menu a discesa nella cella** per nascondere la freccia.
6. Per **Personalizzata**, digita una formula che inizia con =. L'inserimento è consentito quando la formula è vera.
7. **Ignora celle vuote** consente di lasciare vuota una cella.

Nella scheda **Messaggio di input**, seleziona la casella e scrivi un titolo e un messaggio. Compaiono accanto alla cella ogni volta che viene selezionata.

Nella scheda **Avviso di errore**, scegli cosa succede a un inserimento non consentito. **Arresta** lo rifiuta. **Avviso** chiede se mantenerlo. **Informazioni** segnala la situazione e lo mantiene. Scrivi un titolo e un messaggio per l'avviso.

**Cerchia dati non validi** (accanto a **Convalida dati**) disegna un ovale rosso intorno a ogni cella del foglio il cui valore non è consentito dalla sua regola, ad esempio un numero inserito prima della regola o il risultato di una formula. Un messaggio indica se tutte le celle sono corrette. Una cella che correggi perde il suo ovale. **Cancella cerchi di convalida** elimina tutti gli ovali.

**Cancella tutto** elimina le regole dalle celle selezionate. La nuova regola sostituisce quelle che le celle selezionate avevano; le altre celle mantengono le loro. Modificare le regole è un'unica operazione che puoi annullare, e le regole vengono salvate nel file.

## Consolida

Usa questa funzione per combinare intervalli simili, ad esempio la stessa tabella presente in più fogli, in un'unica tabella.

1. Scegli **Consolida** nel gruppo **Strumenti dati** della scheda **Dati**.
2. Scegli la **Funzione** con cui combinare, ad esempio **Somma** o **Media**.
3. Digita un intervallo nella casella e premi **Aggiungi**. Usa **Foglio2!A1:C10** per un intervallo in un altro foglio. Aggiungi ogni intervallo da combinare. Seleziona un intervallo nell'elenco e premi **Elimina** per rimuoverlo.
4. Se gli intervalli hanno nomi nella prima riga, seleziona **La riga superiore contiene i nomi delle colonne**. Se hanno nomi nella prima colonna, seleziona **La colonna sinistra contiene i nomi delle righe**.
5. In **Inserisci il risultato in**, digita la prima cella per il risultato.
6. Seleziona **Crea collegamenti ai dati di origine** se ogni risultato deve essere una formula che legge le sue celle di origine, così cambia quando cambiano queste. Senza questa opzione, i risultati sono numeri semplici.
7. Premi **OK**.

Se non è selezionata alcuna opzione sui nomi, viene combinata la cella nella stessa posizione di ogni intervallo (la prima cella di ogni intervallo, poi la seconda e così via). Con i nomi selezionati, vengono combinate le celle con gli stessi nomi, ovunque si trovino in ciascun intervallo, e un nome presente solo in alcuni intervalli ottiene comunque una riga o una colonna. Consolida è un'unica operazione che puoi annullare.
