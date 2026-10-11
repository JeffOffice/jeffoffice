# Gestione grafica
<!-- keywords: gestione grafica, immagini, foto, immagine collegata, collega al file, aggiorna collegamento, cambia collegamento, incorpora immagine, incorporata, mancante, modificata, non aggiornata, fuori dalla cartella, ricollega, trova file immagine, stato immagine, risoluzione -->

**Gestione grafica** elenca tutte le immagini della pubblicazione e indica la provenienza di ciascuna. È particolarmente utile per le immagini collegate ai rispettivi file, perché un'immagine collegata dipende da un file esterno alla pubblicazione. Per informazioni su come collegare un'immagine, vedere [Aggiungere e regolare immagini](pictures).

## Aprire il riquadro

Fare clic su **Visualizza > Mostra > Gestione grafica**. Il riquadro si apre sulla destra. Per chiuderlo, fare di nuovo clic sullo stesso pulsante oppure sulla **X** nella parte superiore del riquadro.

## Leggere l'elenco

Ogni riga mostra un'anteprima, il nome del file, la pagina, il tipo di file, le dimensioni in KB e lo **stato**. La casella sopra l'elenco ordina le righe per pagina, nome, dimensioni o tipo. Sotto l'elenco, i dettagli dell'immagine selezionata ne indicano lo stato, il file, se la pubblicazione conserva l'immagine completa, le dimensioni in pixel e la **risoluzione effettiva**, cioè quanti pixel dell'immagine cadono in ogni pollice della pagina. Per la stampa è consigliabile un valore di circa 300.

| Stato | Significato |
|---|---|
| Incorporata | L'immagine è memorizzata all'interno della pubblicazione. Non serve nient'altro. |
| Collegata | L'immagine è collegata a un file, che si trova dove JeffPub se lo aspetta ed è invariato. |
| Mancante | L'immagine è collegata a un file che non esiste più. Se la pubblicazione non ne conserva una copia, la pagina mostra l'anteprima ridotta, che appare sfocata, ed è questa che viene stampata. |
| Modificata | Il file è cambiato dalla creazione o dall'ultimo aggiornamento del collegamento. Fare clic su **Aggiorna collegamento** per applicare le modifiche. |
| Non aggiornata | L'immagine è collegata a un file esterno alla cartella della pubblicazione, quindi JeffPub non ha aperto il file. La pagina mostra ciò che è memorizzato nella pubblicazione. Fare clic su **Aggiorna collegamento** per usare il file. Vedere "Immagini esterne alla cartella della pubblicazione" più avanti. |

## Funzione dei pulsanti

- **Vai a** seleziona l'immagine nella relativa pagina.
- **Salva con nome…** salva una copia dell'immagine in un file.
- **Sostituisci…** inserisce un file diverso nella stessa cornice.
- **Aggiorna collegamento** rilegge il file. Usarlo per un'immagine **Modificata**, dopo aver rimesso al suo posto un file **Mancante** o per un'immagine **Non aggiornata**. Vengono aggiornate tutte le immagini che condividono il collegamento e l'immagine mantiene posizione e larghezza.
- **Cambia collegamento…** collega l'immagine a un altro file, ad esempio se i file sono stati spostati in un'altra cartella.
- **Incorpora immagine** trasforma il collegamento in una copia memorizzata nella pubblicazione. L'immagine viene quindi indicata come **Incorporata**. Se il file è mancante, è possibile incorporare solo l'anteprima ridotta, come segnalato dalla barra di stato.

I tre pulsanti dei collegamenti sono disattivati se l'immagine selezionata non è collegata. Ognuno esegue un'unica operazione, quindi è possibile annullarla con **CTRL+Z**.

## Immagini esterne alla cartella della pubblicazione

Quando si apre una pubblicazione, JeffPub legge i file delle immagini collegate che si trovano nella stessa cartella della pubblicazione o in una sua sottocartella. Non accede a file in altre posizioni, né nel computer né in rete.

Il motivo è che una pubblicazione ricevuta da altri può indicare qualsiasi file. Se JeffPub aprisse qualunque file indicato, la pubblicazione di uno sconosciuto potrebbe inserire nelle sue pagine una delle vostre immagini private, che potreste poi salvare o rispedire senza accorgervene. In rete, anche solo verificare l'esistenza di un file può inviare le credenziali di accesso a Windows a un altro computer.

Per questo un'immagine collegata a un file in un'altra posizione mostra ciò che è memorizzato nella pubblicazione: l'immagine completa, se la pubblicazione la conserva, altrimenti l'anteprima ridotta, che appare sfocata. Il suo stato è **Non aggiornata** e i dettagli indicano che il file è esterno alla cartella.

Per usare il file, selezionare l'immagine e fare clic su **Aggiorna collegamento**, oppure fare clic su **Cambia collegamento…** e scegliere il file. Questa operazione autorizza solo quell'immagine e vale fino alla chiusura della pubblicazione. Le altre immagini restano **Non aggiornate** finché non si ripete l'operazione per ciascuna.

Il salvataggio mantiene il collegamento invariato, quindi alla successiva apertura della pubblicazione l'immagine sarà di nuovo **Non aggiornata**. Per caricare le immagini automaticamente, conservarle nella cartella della pubblicazione o in una sua sottocartella.

La stampa e il salvataggio come PDF o come e-book usano ciò che viene mostrato nella pagina. Se un'immagine è **Non aggiornata**, fare clic su **Aggiorna collegamento** prima di stampare. Queste immagini sono elencate in [Verifica struttura](design-checker).

## Correggere un'immagine mancante

1. Selezionare l'immagine **Mancante** nell'elenco.
2. Fare clic su **Cambia collegamento…** e scegliere il file. In alternativa, rimettere il file nella posizione originale e fare clic su **Aggiorna collegamento**.
3. Se il file non è più disponibile, fare clic su **Incorpora immagine** per conservare l'anteprima oppure su **Sostituisci…** per usare un'altra immagine.

Anche [Verifica struttura](design-checker) elenca le immagini collegate mancanti.

## Argomenti correlati

- [Aggiungere e regolare immagini](pictures)
- [Verificare la struttura](design-checker)
- [Salvare come immagine, pagina Web o e-book](export)
