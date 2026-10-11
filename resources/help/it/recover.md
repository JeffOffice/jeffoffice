# Recuperare il lavoro non salvato
<!-- keywords: recuperare, recupero, ripristino, ripristinare, salvataggio automatico, autosalvataggio, arresto anomalo, blocco, crash, lavoro perso, non salvato, interruzione di corrente, programma bloccato, chiuso per errore, backup, copia di sicurezza -->

Se JeffPub si chiude improvvisamente o va via la corrente, il lavoro più recente potrebbe essere ancora al sicuro. Mentre si lavora, JeffPub conserva una copia di riserva di ogni pubblicazione con modifiche non salvate. Questa funzionalità si chiama **Salvataggio automatico**.

## Come funziona il Salvataggio automatico

- A intervalli di pochi minuti, JeffPub salva una copia di riserva di ogni pubblicazione con modifiche non salvate. L'intervallo predefinito è di 10 minuti.
- JeffPub conserva le copie in una cartella di Salvataggio automatico, all'interno della cartella in cui tiene i propri dati. Non è mai necessario aprirla.
- La copia di riserva non sostituisce mai il file vero e proprio.
- La copia viene eliminata quando si salva la pubblicazione, la si chiude o si esce da JeffPub normalmente.

Il Salvataggio automatico non sostituisce il salvataggio. Se JeffPub si interrompe, si può perdere il lavoro degli ultimi minuti. Premere spesso CTRL+S.

## Recuperare il lavoro

Al successivo avvio di JeffPub dopo un arresto anomalo, la finestra **Recupera lavoro non salvato** si apre automaticamente. Ogni riga mostra il nome della pubblicazione, la posizione del file (oppure "mai salvato") e quando è stata creata la copia.

1. Lasciare il segno di spunta accanto a ogni pubblicazione da recuperare. Togliere il segno dalle altre.
2. Fare clic su **Apri**.
3. Controllare il lavoro, quindi premere CTRL+S.

La pubblicazione recuperata mostra "(Recuperata)" nella barra del titolo finché non viene salvata. CTRL+S chiede un nome e propone la cartella del file originale, con il nome originale. Scegliere il nome originale per sostituire il file oppure un nuovo nome per conservarli entrambi.

Le immagini collegate a file vengono cercate come se la pubblicazione fosse aperta nella cartella del file originale: quelle che si trovano in tale cartella, o in una sua sottocartella, vengono caricate normalmente. Un'immagine collegata a un file in qualsiasi altra posizione mostra l'anteprima salvata ed è contrassegnata come **Non aggiornata**; selezionarla e fare clic su **Aggiorna collegamento** in [Gestione grafica](graphics-manager). Una pubblicazione mai salvata non ha una cartella, quindi per tutte le sue immagini collegate è necessario usare **Aggiorna collegamento**.

Gli altri pulsanti della finestra sono:

- **Elimina** rimuove definitivamente le copie selezionate, dopo aver chiesto conferma.
- **Non ora** chiude la finestra e conserva le copie. JeffPub le propone di nuovo al successivo avvio.

JeffPub propone solo le copie di una sessione terminata in modo imprevisto. Non propone le copie di una finestra di JeffPub ancora aperta.

## Recuperare il lavoro in un secondo momento

Se la finestra è stata chiusa e si desidera recuperare le copie, fare clic su **File > Apri**, quindi su **Recupera lavoro non salvato**. Se non c'è nulla da recuperare, JeffPub lo segnala.

## Modificare la frequenza di salvataggio

Fare clic su **File > Opzioni**, quindi sulla scheda **Salvataggio**. Impostare **Salva informazioni di Salvataggio automatico ogni** su un valore compreso tra 1 e 120 minuti. Un valore più basso riduce il lavoro che si rischia di perdere. Il nuovo intervallo diventa pienamente effettivo al successivo avvio di JeffPub.

## Argomenti correlati

- [Aprire e salvare pubblicazioni](open-save)
- [Modificare le impostazioni di JeffPub](options)
- [Ottenere assistenza e segnalare problemi](support)
