# Trova, sostituisci e vai a
<!-- keywords: trova, sostituisci, cambia, cerca, ricerca, caratteri jolly, asterisco, punto interrogativo, tilde, maiuscole/minuscole, intero contenuto cella, vai a, riferimento, nome, seleziona per tipo, celle vuote, costanti, formule, errori, area corrente, ultima cella, celle visibili, formati condizionali, formattazione condizionale, convalida, convalida dati, CTRL+F, CTRL+H, F5, note -->

## Trova

Premere **CTRL+F** (oppure scegliere **Trova** nella scheda Home, quindi di nuovo **Trova**). Digitare il testo e premere **INVIO** per passare alla cella successiva che lo contiene, oppure scegliere **Trova tutti** per ottenere un elenco di tutte le celle corrispondenti. Fare clic su una riga dell'elenco per passare a quella cella.

| Opzione | Funzione |
| --- | --- |
| **In** | Cerca in questo foglio o in tutti i fogli visibili. Se sono selezionate più celle, la ricerca in un foglio viene eseguita solo in tali celle. |
| **Ordine** | Scorre le celle per righe (da sinistra a destra, poi verso il basso) o per colonne (dall'alto in basso, poi verso destra). **Successivo** si ferma alla prima corrispondenza in quell'ordine. |
| **Cerca in** | **Formule** cerca in ciò che è digitato in ogni cella, **Valori** in ciò che la cella mostra e **Note** nelle note delle celle. |
| **Maiuscole/minuscole** | Le lettere maiuscole e minuscole devono corrispondere. |
| **Confronta intero contenuto cella** | L'intero contenuto della cella deve corrispondere al testo, non solo una parte. |

Nel testo da cercare, **?** sostituisce un singolo carattere qualsiasi e **\*** una sequenza qualsiasi di caratteri (anche nessuno). Digitare **~** prima di ?, \* o ~ per cercare il carattere stesso: **a~\*b** trova a\*b.

La ricerca viene eseguita un po' alla volta, quindi è possibile continuare a lavorare. Il tasto **ESC** (o **Annulla**) la interrompe, così come la modifica di una cella. Un testo molto lungo richiede più tempo in ogni piccolo passaggio rispetto a uno breve, quindi anche con un foglio di testi lunghi la finestra resta utilizzabile.

Le note seguono le celle: dopo un ordinamento o dopo l'inserimento o l'eliminazione di righe o colonne, la ricerca nelle note trova ogni nota nella cella in cui si trova attualmente. Una nota di una cella eliminata non viene più trovata (e ricompare dopo Annulla). Se una cartella di lavoro è stata modificata in così tanti modi che non è possibile seguire le note attraverso tutte le modifiche, viene chiesto di salvarla prima; in una cartella di lavoro aperta da un tipo di file meno recente o da un file di testo, le note sono in un formato in cui JeffCrunch non esegue ricerche, e viene visualizzato un avviso.

## Sostituisci

Premere **CTRL+H**. **Sostituisci** inserisce il nuovo testo nella cella con il cursore, se contiene il testo cercato, e passa a quella successiva. **Sostituisci tutto** esegue la sostituzione in tutte le celle che contengono il testo, in un unico passaggio che è possibile annullare; una sostituzione estesa viene eseguita un po' alla volta. Il nuovo testo viene immesso come se fosse digitato: un numero diventa un numero e una formula modificata resta una formula.

Non viene modificato nulla, e ne viene indicato il motivo, se una cella da modificare è bloccata in un foglio protetto, fa parte di una matrice o di una tabella dati, si trova nella prima riga di una tabella o non sarebbe più una formula.

## Vai a

Premere **F5** (o **CTRL+G**) oppure digitare nella casella Nome. È possibile specificare una cella come B7, un intervallo come B3:D9, colonne o righe intere (C:E, 4:9), una cella di un altro foglio (Foglio2!A1 o 'Il mio foglio'!A1) o il nome di un intervallo.

## Seleziona per tipo

Scegliere **Seleziona per tipo** nel menu **Trova** per selezionare le celle in base al contenuto:

- **Celle vuote**, **Costanti** (digitate), **Formule** e **Celle che mostrano un errore**. Per costanti e formule è possibile escludere numeri, testo, VERO e FALSO o errori.
- **Il blocco intorno alla cella con il cursore** e **L'ultima cella utilizzata**.
- **Celle visibili** (non in righe o colonne nascoste), **Celle con formattazione condizionale** e **Celle con una regola per i valori immettibili** (tutte oppure solo quelle con la stessa regola della cella con il cursore).

Il primo blocco trovato viene selezionato e gli altri vengono evidenziati con una tinta, che scompare quando si fa clic su una cella.
