# Pack and Go: invia un documento con tutto ciò che gli serve
<!-- keywords: pack and go, pacchetto, invia, condividi, altro computer, zip, cartella, tipi di carattere, collegamenti a file, immagini collegate, immagini mancanti, copia, porta con te, unità USB, licenza, INCLUDEPICTURE, INCLUDETEXT, campo, crea pacchetto, impacchetta -->

Quando invii un documento a qualcuno, le immagini e i tipi di carattere che usa possono andare persi lungo il percorso. **Pack and Go** salva il documento insieme a ciò che gli serve, in un unico file ZIP o in un'unica cartella, in modo che abbia lo stesso aspetto su un altro computer.

## Come si usa

1. Fai clic su **File**, quindi su **Esporta**.
2. Fai clic su **Pack and Go: Salva per un altro computer** per creare un unico file ZIP, oppure su **Pack and Go: Salva in una cartella** per creare una nuova cartella.
3. Scegli dove salvarlo. Per un file ZIP, digita un nome. Per una cartella, scegli la cartella in cui inserire quella nuova.
4. Leggi il messaggio che compare. Indica dove si trova il pacchetto, quanti file collegati e file di tipi di carattere sono stati inseriti e quali elementi non è stato possibile includere.

Un **file ZIP** è un unico file che contiene altri file, quindi è facile da inviare. Il destinatario lo apre per estrarre i file. Una cartella è più adatta quando vuoi copiare tutto su un'unità.

## Cosa entra nel pacchetto

- **Il documento stesso**, con lo stesso nome che ha ora. È una copia. Il tuo file originale non cambia.
- **Immagini collegate.** Un'immagine collegata mostra un'immagine presa da un file, invece di contenere l'immagine stessa. Il pacchetto contiene una copia di ciascuna.
- **File indicati dai campi.** Un campo è un codice che genera da solo il proprio risultato. Un campo **INCLUDEPICTURE** mostra un'immagine presa da un file, mentre un campo **INCLUDETEXT** mostra un testo preso da un altro documento. Il pacchetto contiene una copia di ciascun file, e il campo ora indica quella copia.
- **Tipi di carattere.** Un tipo di carattere è l'aspetto delle lettere. Il pacchetto contiene i file dei tipi di carattere forniti con il programma e usati dal documento, in una cartella denominata **fonts**. Pack and Go non include mai un tipo di carattere la cui licenza ne vieta la distribuzione. Il messaggio indica ciascun tipo di carattere escluso. I tipi di carattere installati da te sul computer non vengono inclusi, quindi il destinatario potrebbe doverli installare a parte.

## Come cambiano i collegamenti

Nella copia, ogni collegamento incluso nel pacchetto punta al file all'interno del pacchetto, tramite un nome breve, ad esempio **linked/photo.png**. I collegamenti non usano più il percorso del tuo computer, quindi funzionano ovunque venga aperto il pacchetto. Se due file hanno lo stesso nome, al secondo viene aggiunto un numero.

Un collegamento che Pack and Go non riesce a seguire resta invariato, e il messaggio lo elenca. Pack and Go non include:

- Un file che non si trova.
- Un indirizzo web. Questi restano invariati.
- Un file presente su un altro computer della tua rete.
- Un collegamento che risale e esce dalla cartella del documento, ad esempio **../../notes.png**.
- Un documento, una cartella di lavoro o un altro oggetto che non si trova nella cartella del documento (o in una cartella al suo interno), anche quando il collegamento ne indica il percorso completo. Solo immagini, video e suoni vengono inclusi da qualsiasi posizione.
- Un collegamento che esce dalla cartella del documento attraverso una scorciatoia verso un'altra cartella (collegamento simbolico).
- Un collegamento al documento stesso.
- Un file che non è un'immagine, un video, un suono o un documento. Un programma non viene mai incluso.

Pack and Go non cerca altri collegamenti all'interno dei file collegati, quindi un file che rimanda a questo non può creare un ciclo.

## Limiti

Per evitare che un pacchetto diventi troppo grande, Pack and Go chiede conferma quando i file collegati sono più di 200 o occupano più di 256 MB. Scegli **No** e non viene salvato nulla. Pack and Go non include mai più di 2.000 file o più di 1 GB, qualunque sia la risposta. I file oltre tale limite vengono esclusi ed elencati nel messaggio.

Se il documento è protetto da password, la copia nel pacchetto ha la stessa password.
