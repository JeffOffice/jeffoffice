# Scheda Riproduzione
<!-- keywords: scheda riproduzione, nastro, filmato, video, suono, audio, riproduci, pausa, anteprima, taglia, didascalie, inserisci didascalie, rimuovi didascalie, sottotitoli, webvtt, vtt, volume, disattiva audio, avvia, in sequenza di clic, automaticamente, al clic, riproduci a schermo intero, nascondi se non in riproduzione, ciclo continuo, riavvolgi dopo la riproduzione, riproduci tra diapositive, continua a riprodurre, testo alternativo, allinea, altezza, larghezza, dimensioni, strumenti multimediali -->

La scheda **Riproduzione** imposta come viene riprodotto un filmato o un suono, e aggiunge le didascalie a un filmato. Compare solo quando è selezionato un filmato o un suono, e scompare quando si seleziona altro.

- **Anteprima** contiene **Riproduci**. Riproduce il filmato o il suono sulla diapositiva, così puoi vederlo e sentirlo. Mentre è in riproduzione, il pulsante riporta **Pausa**.
- **Modifica** contiene **Taglia**. Apre una finestra con **Ora di inizio** e **Ora di fine**, in secondi. Il filmato o il suono viene riprodotto solo dall'ora di inizio all'ora di fine. Il resto del file resta all'interno della presentazione. Per riprodurre di nuovo fino alla fine del file, imposta come ora di fine la durata completa.
  - **Inserisci didascalie** aggiunge didascalie a un filmato da un file WebVTT (un file di testo con estensione `.vtt`). Chiede prima il file e poi la lingua delle didascalie, come codice breve, ad esempio `it`, `es` o `pt-BR`. Le didascalie vanno con un filmato incorporato nella presentazione; per un filmato solo collegato dall'esterno, il pulsante è disattivato. Un filmato conserva un solo set di didascalie per ogni lingua: inserire un secondo file per la stessa lingua sostituisce il primo. Un file che non è WebVTT, non contiene didascalie o è troppo grande (il massimo è 4 MB, 20.000 didascalie e 2.000 caratteri in una singola didascalia) viene rifiutato con un messaggio, e non cambia nulla.
  - **Rimuovi didascalie** elimina le didascalie dal filmato. Se il filmato ha didascalie in più lingue, chiede quale rimuovere, oppure se rimuoverle tutte.
- **Opzioni elementi multimediali** stabilisce come viene riprodotto nella presentazione:
  - **Volume** è **Basso**, **Medio**, **Alto** oppure **Disattiva audio**.
  - **Avvia** indica che cosa lo avvia. **In sequenza di clic** lo riproduce a un clic, insieme alle animazioni della diapositiva. **Automaticamente** lo riproduce da solo quando arriva il suo turno. **Al clic** lo riproduce quando fai clic sul filmato o sul suono.
  - **Riproduci a schermo intero** fa occupare tutto lo schermo al filmato durante la riproduzione. Funziona solo per i filmati.
  - **Nascondi se non in riproduzione** nasconde l'immagine finché il filmato o il suono non viene riprodotto, e di nuovo dopo che si è fermato. L'icona dell'altoparlante di un suono resta nascosta per tutto il tempo.
  - **Riproduci a ciclo continuo fino all'interruzione** riproduce di nuovo ogni volta che finisce, finché non viene interrotto o non cambia la diapositiva.
  - **Riavvolgi dopo la riproduzione** riporta un filmato alla sua prima immagine quando finisce. Senza questa opzione, il filmato resta sull'ultima immagine.
  - **Riproduci tra diapositive** (per un suono; un filmato viene disegnato solo sulla propria diapositiva) mantiene il suono in riproduzione nella presentazione quando cambia la diapositiva, finché non finisce o non viene interrotto, fino alla fine della presentazione. Un file creato con un altro programma può indicare per quante diapositive il suono viene riprodotto: tale numero resta fissato mentre la casella è selezionata, e il suono si interrompe dopo quel numero di diapositive, contate a partire dalla diapositiva in cui è iniziato (andare avanti e indietro tra esse non ne consuma nessuna). Se la casella non è selezionata, il suono si interrompe con la sua diapositiva; selezionandola di nuovo, prosegue fino alla fine della presentazione. Con **Riproduci a ciclo continuo fino all'interruzione** riparte ogni volta che finisce. Premi CTRL+Z per annullare la modifica; un file .pptx la conserva.

Le didascalie appaiono nella parte inferiore del filmato, con testo bianco su una fascia scura, mentre il filmato viene riprodotto nella presentazione e nell'editor. Sono sincronizzate con il filmato, e la loro dimensione segue quella del filmato. Se un filmato ha più lingue, viene mostrata la prima. Le didascalie vengono salvate con la presentazione, come parte del file `.pptx`, e tornano quando lo apri di nuovo. Un file di didascalie in un altro formato (non WebVTT) viene conservato così com'è e salvato di nuovo senza modifiche, ma non viene mostrato. Quando un filmato ha le didascalie, **Verifica accessibilità** non lo segnala più.

I gruppi **Accessibilità**, **Disponi** e **Dimensioni** sono quelli della scheda Formato immagine: **Testo alternativo**, **Allinea** e le caselle **Altezza** e **Larghezza**, per il filmato o il suono selezionato.

Ogni modifica è un passaggio. **CTRL+Z** la annulla.

Un suono o un filmato che viene riprodotto tra diapositive, come impostano alcuni file, continua a essere riprodotto per tante diapositive quante ne indica il file. JeffDeck lo riproduce in questo modo nella presentazione, e mantiene l'impostazione quando salvi.

Vedi [Forme, immagini, tabelle e Testo artistico](objects) e [Avviare la presentazione e usare la Visualizzazione Relatore](show).

## Correlati

- [Forme, immagini, tabelle e Testo artistico](objects)
- [Scheda Inserisci](tab-insert)
- [Avviare la presentazione e usare la Visualizzazione Relatore](show)
