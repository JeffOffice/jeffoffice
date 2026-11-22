# Info: proprietà, password e Verifica documento
<!-- keywords: info, proprietà, titolo, autore, oggetto, parole chiave, categorie, commenti, società, responsabile, proprietà personalizzate, contrassegna come finale, finale, sola lettura, password, cifra, proteggi, verifica cartella di lavoro, informazioni personali, fogli nascosti, note, collegamenti esterni, XML personalizzato, mostra nella cartella, copia percorso, cartella, percorso, cronologia versioni, versioni, torna indietro, info, informazioni, metadati, protezione, condividi, ispeziona cartella di lavoro -->

Fai clic su **File > Info** per vedere i dati della cartella di lavoro e proteggerla prima di condividerla. Premi **Esc** per tornare alla cartella di lavoro.

## Dove si trova il file

In alto, la pagina mostra dove è salvato il file. Fai clic su **Mostra nella cartella** per aprire quella cartella. Fai clic su **Copia percorso** per copiare il percorso, così puoi incollarlo in un messaggio di posta elettronica o in una chat.

## Proprietà

Le proprietà sono dati sulla cartella di lavoro, come il titolo e l'autore. La pagina ha una casella per ciascuna: **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**. Digita in una casella e premi **Tab** o **Invio**. JeffCrunch conserva la modifica e la scrive nel file al successivo salvataggio della cartella di lavoro. La pagina mostra anche quando la cartella di lavoro è stata creata, salvata e stampata.

Fai clic su **Proprietà personalizzate** per creare proprietà personali, come **Progetto** o **Cliente**. Fai clic su **Aggiungi**, digita un nome, scegli il tipo di valore (testo, numero intero, numero, sì o no, oppure data) e digita il valore. JeffCrunch le salva come le hai digitate.

## Contrassegna come finale

Fai clic su **Contrassegna come finale** per indicare ai lettori che la cartella di lavoro è completata. JeffCrunch salva la cartella di lavoro e la mantiene poi solo in lettura. Quando qualcuno la apre, la barra del titolo indica che è di sola lettura. È un modo gentile per dire "per favore, non modificare questo". Non è un blocco, perché chiunque può scegliere di nuovo **Contrassegnata come finale** per modificarla.

## Apri sempre in sola lettura

Fai clic su **Apri sempre in sola lettura** per far sì che la cartella di lavoro si apra ogni volta solo in lettura, così nessuno la modifica per sbaglio. La scelta viene salvata con il file al salvataggio. La prossima volta che qualcuno apre il file, la barra di stato indica che è aperto in sola lettura. Per modificarlo, apri **File > Info** e fai clic su **Si apre sempre in sola lettura**. Basta un clic, quindi si tratta di un promemoria e non di un blocco. Gli altri programmi non leggono questa scelta. Un file di testo, un file .xls e un file .ods non possono contenerla, quindi salva prima la cartella di lavoro come file .xlsx.

## Cifra con password

Fai clic su **Cifra con password** per chiedere una password ogni volta che il file viene aperto. Equivale a **Proteggi con password**. Vedi [Aprire cartelle di lavoro protette da password](open-password). Una password protegge un file **.xlsx** con una protezione robusta. Una password **non può essere recuperata** se la dimentichi. Le versioni conservate e la copia di backup seguono la password.

Per impedire modifiche a un foglio o al suo layout, vedi [Proteggere un foglio o la cartella di lavoro](protect).

## Proteggi il foglio o i fogli

**Proteggi foglio corrente** ti permette di scegliere cosa possono modificare le persone nel foglio in cui ti trovi, con una password se lo desideri. **Proteggi struttura cartella di lavoro** impedisce di aggiungere, eliminare, spostare, nascondere o rinominare i fogli. Entrambe sono disponibili anche nella scheda **Revisione**.

## Cronologia versioni

**Cronologia versioni** conserva copie della cartella di lavoro su questo computer. Non viene inviato nulla altrove. Una copia si chiama versione. Conservane una prima di una modifica importante.

- **Conserva una versione** salva la cartella di lavoro così com'è ora, con un nome a tua scelta.
- **Apri una copia** apre una versione in una finestra separata, così puoi guardarla accanto alla cartella di lavoro.
- **Torna a questa versione** rimette una versione come cartella di lavoro. La cartella di lavoro com'è ora viene prima conservata come versione, così non si perde nulla.
- **Elimina** cancella una versione.

La cartella di lavoro deve essere salvata prima, perché le versioni appartengono a un file. Il ritorno a una versione funziona per un file .xlsx.

## Verifica cartella di lavoro

Fai clic su **Verifica cartella di lavoro** prima di inviare una cartella di lavoro a qualcuno. Elenca ciò che il file contiene e che potresti non voler condividere, con il numero di elementi trovati per ciascuna voce:

- **Note** sono le note lasciate dalle persone sulle celle. **Rimuovi tutto** le elimina.
- **Proprietà del documento e informazioni personali** sono l'autore, la persona che ha salvato per ultima, la società e il responsabile. **Rimuovi tutto** le cancella.
- **Fogli nascosti** sono fogli che non compaiono come schede. **Mostra tutto** li rende visibili, così puoi vedere cosa contengono ed eliminare quelli che non ti servono.
- **Righe e colonne nascoste** sono righe o colonne che hai nascosto a mano. Le righe nascoste da un filtro non vengono conteggiate. **Mostra tutto** le rende visibili, così puoi vedere cosa contengono.
- **Intestazioni e piè di pagina** sono il testo stampato in alto e in basso in ogni pagina. **Rimuovi tutto** le cancella in tutti i fogli.
- **Collegamenti esterni** sono collegamenti a valori di altre cartelle di lavoro. **Rimuovi tutto** trasforma ogni cella che ne legge uno nel numero o nel testo che mostra ora, ed esclude i collegamenti al salvataggio. Un nome che indicava una posizione in un'altra cartella di lavoro diventa **#RIF!**, un errore che segnala che la posizione non esiste più. Questa operazione non si può annullare, quindi salva prima una copia se potresti voler riavere i collegamenti.
- **Dati XML personalizzati** sono dati aggiunti al file da altri programmi, che non vedi in un foglio. **Rimuovi tutto** li esclude al momento del salvataggio.

Salva poi la cartella di lavoro per conservare la copia pulita.

**Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, lo fa ogni volta che salvi, per qualsiasi file, senza modificare il documento aperto. È la stessa casella che si trova in **Opzioni**, ed è uguale in ogni app di JeffOffice.

## Verifica compatibilità per .csv

Scegli **.csv** in **Verifica compatibilità** per vedere cosa lascerebbe fuori un file .csv dalla cartella di lavoro. Un file .csv contiene solo i valori di un foglio. L'elenco indica gli altri fogli, le formule (il file ne contiene i risultati), le immagini, i grafici, le note, le tabelle pivot e il resto di ciò che contiene la cartella di lavoro, i nomi definiti e le tabelle, e sempre tutto ciò che non è un valore, come i formati numero, i tipi di carattere, i colori, i bordi, la larghezza delle colonne, le celle unite, i riquadri bloccati, i filtri, la formattazione condizionale, la convalida dei dati, i collegamenti e le impostazioni di stampa. Elenca anche le scelte mantenute nelle proprietà del file, come Contrassegna come finale e Apri sempre in sola lettura. Non salva nulla. Scegli **Salva come .xlsx** al salvataggio per conservarle tutte.

## Verifica compatibilità

Fai clic su **Verifica compatibilità**, poi scegli **.xls**, **.ods** o **.csv** per vedere cosa non potrebbe contenere quel tipo di file. Elenca le parti della cartella di lavoro che verrebbero lasciate fuori o ridotte per adattarsi, come grafici, note o righe oltre il vecchio limite, e le scelte mantenute nel file stesso, Contrassegna come finale e Apri sempre in sola lettura, quando sono attive. Non salva nulla, e la cartella di lavoro resta com'è.
