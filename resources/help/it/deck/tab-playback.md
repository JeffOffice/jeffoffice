# Scheda Riproduzione

<!-- keywords: scheda riproduzione, barra multifunzione, filmato, video, suono, audio, riproduci, pausa, anteprima, taglia, didascalie, inserisci didascalie, rimuovi didascalie, sottotitoli, webvtt, vtt, volume, disattiva audio, avvia, in sequenza di clic, automaticamente, al clic, riproduci a schermo intero, nascondi se non in riproduzione, riproduci a ciclo continuo, riavvolgi dopo la riproduzione, strumenti elementi multimediali -->

La scheda **Riproduzione** imposta la modalità di riproduzione di un filmato o di un suono e aggiunge didascalie a un filmato. Compare solo quando è selezionato un filmato o un suono, e scompare quando si seleziona altro.

- **Anteprima** contiene **Riproduci**. Riproduce il filmato o il suono nella diapositiva, così da poterlo vedere e ascoltare. Mentre è in riproduzione, il pulsante riporta **Pausa**.
- **Modifica** contiene **Taglia**. Apre una finestra con **Ora di inizio** e **Ora di fine**, in secondi. Il filmato o il suono viene riprodotto solo dall'ora di inizio all'ora di fine. Il resto del file rimane all'interno della presentazione. Per riprodurre di nuovo fino alla fine del file, imposta l'ora di fine sulla durata completa.
  - **Inserisci didascalie** aggiunge didascalie a un filmato da un file WebVTT (un file di testo con estensione `.vtt`). Chiede prima il file e poi la lingua delle didascalie, come codice breve, ad esempio `en`, `es` o `pt-BR`. Le didascalie si possono aggiungere a un filmato incorporato nella presentazione; per un filmato soltanto collegato dall'esterno il pulsante è disattivato. Un filmato conserva un solo set di didascalie per ogni lingua: inserendo un secondo file per la stessa lingua, il primo viene sostituito. Un file che non è in formato WebVTT, non contiene didascalie o è troppo grande (il massimo è 4 MB, 20.000 didascalie e 2.000 caratteri in una singola didascalia) viene rifiutato con un messaggio, e non cambia nulla.
  - **Rimuovi didascalie** elimina le didascalie dal filmato. Se il filmato ha didascalie in più lingue, chiede quale rimuovere oppure se rimuoverle tutte.
- **Opzioni elementi multimediali** stabilisce come viene riprodotto nella presentazione:
  - **Volume** può essere **Basso**, **Medio**, **Alto** oppure **Disattiva audio**.
  - **Avvio** indica che cosa lo avvia. **In sequenza di clic** lo riproduce con un clic, insieme alle animazioni della diapositiva. **Automaticamente** lo riproduce da solo quando arriva il suo turno. **Al clic** lo riproduce quando si fa clic sul filmato o sul suono.
  - **Riproduci a schermo intero** fa occupare tutto lo schermo al filmato durante la riproduzione. Funziona solo per i filmati.
  - **Nascondi se non in riproduzione** nasconde l'immagine finché il filmato o il suono non viene riprodotto, e di nuovo dopo che si è fermato. L'icona dell'altoparlante di un suono resta nascosta per tutto il tempo.
  - **Riproduci a ciclo continuo** riproduce di nuovo il filmato o il suono ogni volta che termina, finché non viene arrestato o la diapositiva non cambia.
  - **Riavvolgi dopo la riproduzione** riporta un filmato alla sua prima immagine quando termina. Senza questa opzione, il filmato resta sull'ultima immagine.

Le didascalie compaiono sulla parte inferiore del filmato, in testo bianco su una fascia scura, mentre il filmato viene riprodotto nella presentazione e nell'editor. Sono sincronizzate con il filmato e la loro dimensione segue quella del filmato. Se un filmato ha più lingue, viene mostrata la prima. Le didascalie vengono salvate con la presentazione, come parte del file `.pptx`, quindi si ritrovano quando riapri il file. Un file di didascalie in un altro formato (non WebVTT) viene conservato così com'è e salvato di nuovo invariato, ma non viene mostrato. Quando un filmato ha delle didascalie, **Verifica accessibilità** non lo segnala più.

Ogni modifica è un'unica operazione. **CTRL+Z** la annulla.

Un suono o un filmato che viene riprodotto su più diapositive, come impostato in alcuni file, continua a essere riprodotto per il numero di diapositive indicato nel file. JeffDeck lo riproduce in questo modo nella presentazione e mantiene l'impostazione al salvataggio.

Vedi [Forme, immagini, tabelle e Testo artistico](objects) e [Avviare la presentazione e usare la Visualizzazione Relatore](show).

## Argomenti correlati

- [Forme, immagini, tabelle e Testo artistico](objects)
- [Scheda Inserisci](tab-insert)
- [Avviare la presentazione e usare la Visualizzazione Relatore](show)
