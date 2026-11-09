# Modificare le impostazioni di JeffCrunch
<!-- keywords: recupero, recupera lavoro non salvato, recupero automatico, correzione, modifica, completamento automatico, maniglia di riempimento, trascinamento, posizioni decimali, opzioni, impostazioni, preferenze, statistiche di utilizzo, numero di installazione, privacy, informativa sulla privacy, JP_NO_TELEMETRY, elimina statistiche, aggiornamenti, informazioni, modifica direttamente nelle celle, colore griglia, zoom predefinito, larghezza standard, dati personali -->

Le impostazioni si trovano nella finestra **Opzioni di JeffCrunch**. Fai clic su **File > Opzioni** per aprirla. Fai clic su **OK** per confermare le modifiche oppure su **Annulla** per scartarle.

## Modifica

La pagina **Modifica** cambia il modo in cui funziona la digitazione nelle celle.

- **Dopo aver premuto Invio, sposta la selezione** indica dove va la selezione quando completi un inserimento: giù, a destra, su, a sinistra, oppure resta dove si trova.
- **Completa automaticamente un testo dalle celle sopra e sotto** si chiama completamento automatico. Quando digiti le prime lettere di un testo e in quella colonna un solo testo inizia con esse, JeffCrunch mostra il resto, selezionato. Premi **Invio** per confermarlo. Continua a digitare per ignorarlo, oppure premi **Backspace** per eliminarlo. Il completamento considera solo il blocco di celle accanto a quella in cui stai digitando, e solo i testi, non i numeri.
- **Usa la maniglia di riempimento e trascina le celle per spostarle o copiarle** attiva il piccolo quadratino nell'angolo della selezione e lo spostamento trascinando il bordo della selezione. Disattivalo se sposti le celle per errore. Copia e Incolla continuano a funzionare.
- **Mostra uno zero nelle celle che hanno valore zero (questo foglio)** attiva o disattiva la visualizzazione degli zeri nel foglio che stai guardando. Quando è disattivata, una cella che contiene 0 appare vuota, ma contiene comunque lo 0 e viene sommata come 0. La scelta viene salvata con il foglio.
- **Inserisci una virgola decimale nei numeri interi digitati** si chiama posizioni decimali automatiche. Se imposti **Posizioni decimali** su 2, digitando 1234 viene inserito 12,34. Se lo imposti su -2, digitando 5 viene inserito 500. Un numero digitato con la virgola decimale viene inserito così come lo digiti.
- **Consenti la modifica direttamente nelle celle** è attivata per impostazione predefinita. Quando la disattivi, un inserimento digitato o il tasto F2 portano alla barra della formula, e la cella mostra il risultato solo quando completi l'inserimento.

## Opzioni di salvataggio

**Salva informazioni di recupero automatico ogni** imposta ogni quanto JeffCrunch conserva una copia di una cartella di lavoro con modifiche non salvate, da 1 a 120 minuti. La copia è un file .xlsx in una cartella che JeffCrunch usa per questo scopo, creata in background così che tu possa continuare a digitare. Viene eliminata quando salvi o chiudi la cartella di lavoro. Se JeffCrunch si chiude all'improvviso, alla successiva apertura propone le copie disponibili. Puoi anche fare clic su **File > Apri > Recupera lavoro non salvato**. Una cartella di lavoro protetta da password non viene copiata, perché la copia non sarebbe protetta.

## Opzioni di correzione

La pagina **Correzione** è condivisa dalle app di JeffOffice. Stabilisce se l'ortografia viene controllata durante la digitazione in una cella (disattiva **Controlla ortografia durante la digitazione** per eliminare le sottolineature rosse ondulate; **Controllo ortografia** nella scheda Revisione continua a funzionare), se le parole in maiuscolo e le parole con numeri vengono ignorate nel controllo ortografico, se la correzione automatica corregge le parole mentre digiti, se le virgolette dritte diventano curve, se trattini, frazioni e ordinali vengono modificati e se un indirizzo web diventa un collegamento. Vedi [Controllo ortografia](notes-links) e [Digitare nelle celle](edit-cells).

## Opzioni di visualizzazione

- **Colore griglia** è il colore delle linee tra le celle sullo schermo. È una scelta solo per il tuo schermo e non viene salvata nella cartella di lavoro.
- **Zoom di una nuova cartella di lavoro** indica quanto grandi appaiono i fogli di una nuova cartella di lavoro, da 10% a 400%.
- **Larghezza colonna standard (questo foglio)** è la larghezza di ogni colonna di questo foglio che non ha una larghezza propria. Corrisponde a **Larghezza predefinita** nella scheda Home.

## Opzioni di privacy

- **Rimuovi il mio nome e la mia società dalle proprietà di una cartella di lavoro a ogni salvataggio** elimina, al salvataggio, l'autore, chi l'ha salvata per ultima, la società e il responsabile dalle proprietà della cartella di lavoro. Le proprietà sono i dati che un file conserva su se stesso.

## Statistiche di utilizzo

**Invia statistiche di utilizzo anonime** ti permette di condividere informazioni su come viene usato JeffCrunch. Anonime significa che le informazioni non dicono nulla su chi sei. Quando è attivata, JeffCrunch invia questi dati una volta al giorno e nel giorno in cui aggiorni: che si tratta di JeffCrunch, la sua versione, il sistema operativo e la lingua, quante volte è stato avviato, con quale frequenza viene usato ciascun comando e che tipo di file hai aperto o salvato. Se JeffCrunch si chiude all'improvviso, alla successiva apertura invia anche una segnalazione di arresto anomalo. JeffCrunch non invia mai i tuoi file, i loro nomi o il loro contenuto, né quello che digiti. Se l'opzione è disattivata, non viene inviata nemmeno alcuna segnalazione di arresto anomalo.

La scelta vale per tutte le app di JeffOffice: se la cambi qui, cambia in ciascuna di esse. Al primo avvio JeffCrunch ti chiede una decisione nella finestra **Aiutaci a migliorare JeffCrunch**.

- **Informativa sulla privacy** accanto alla casella di controllo apre la nostra informativa sulla privacy all'indirizzo jeffoffice.com/privacy. Spiega cosa viene inviato, dove va e per quanto tempo lo conserviamo. Conserviamo le statistiche per 24 mesi.
- **Numero di installazione** è mostrato sotto la casella di controllo. È un numero casuale generato per la tua copia di JeffOffice, uguale in tutte le app di JeffOffice. Non dice nulla su di te. Fai clic su **Copia** per copiarlo. Se le statistiche sono disattivate, oppure se non è ancora stato generato alcun numero, appare **nessuno**.
- Per far eliminare le statistiche che già conserviamo, invia il tuo numero di installazione a info@jeffoffice.com **prima** di disattivare le statistiche. Disattivandole, il numero viene cancellato dal computer e, senza di esso, non possiamo stabilire quali registrazioni sono tue.
- Per disattivare le statistiche senza aprire JeffCrunch, imposta la variabile d'ambiente `JP_NO_TELEMETRY` su un valore qualsiasi prima di avviarlo. Da quel momento non viene conteggiato né inviato nulla, e JeffCrunch non pone la domanda del primo avvio.

## Vedi anche

- [Benvenuti in JeffCrunch](start)
