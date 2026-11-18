# Pack and Go: porta con te una presentazione
<!-- keywords: pack and go, pacchetto, invia, condividi, altro computer, zip, cartella, caratteri, font, collegamenti a file, file collegati, immagini collegate, immagini mancanti, copia, portare con sé, chiavetta USB, licenza, filmato, video, suono, audio, presentazione -->

Una presentazione può riprodurre un filmato o un suono da un file che non si trova al suo interno. Su un altro computer quel file manca e la presentazione presenta una lacuna. **Pack and Go** salva la presentazione insieme ai file collegati e ai suoi caratteri, in un unico file ZIP o in un'unica cartella.

## Come usarlo

1. Fai clic su **File**, quindi su **Esporta**.
2. Fai clic su **Pack and Go: salva per un altro computer** per creare un unico file ZIP, oppure su **Pack and Go: salva in una cartella** per creare una nuova cartella.
3. Scegli dove salvarlo. Per un file ZIP, digita un nome. Per una cartella, scegli la cartella in cui crearne una nuova.
4. Leggi il messaggio che compare. Indica dove si trova il pacchetto, quanti file collegati e quanti file di caratteri sono stati inclusi e che cosa non è stato possibile includere.

Un **file ZIP** è un unico file che contiene altri file, quindi è facile da inviare. La persona che lo riceve lo apre per estrarne i file. Una cartella è più comoda quando vuoi copiare tutto su un'unità USB.

## Che cosa contiene il pacchetto

- **La presentazione stessa**, con lo stesso nome che ha ora. È una copia: il tuo file originale non cambia.
- **Filmati e suoni collegati.** Un filmato o un suono può essere collegato, cioè la presentazione lo riproduce dal suo file invece di contenerlo. Il pacchetto include una copia di ciascuno, così la presentazione funziona allo stesso modo sull'altro computer. Se non hai inserito il file nella presentazione, questo è il modo per portarlo con te.
- **Immagini collegate.** Il pacchetto contiene una copia di ogni immagine collegata al proprio file.
- **Caratteri.** Un carattere è l'aspetto delle lettere. Il pacchetto contiene i file dei caratteri forniti con il programma e utilizzati dalla presentazione, in una cartella chiamata **fonts**. Pack and Go non include mai un carattere la cui licenza ne vieta la distribuzione. Il messaggio indica ciascun carattere escluso. I caratteri installati sul tuo computer da te non vengono inclusi, quindi l'altra persona potrebbe averne bisogno.

## Come cambiano i collegamenti

Nella copia, ogni collegamento incluso nel pacchetto punta al file all'interno del pacchetto, tramite un nome breve, ad esempio **linked/photo.png**. I collegamenti non usano più il percorso del tuo computer, quindi funzionano ovunque venga aperto il pacchetto. Se due file hanno lo stesso nome, al secondo viene aggiunto un numero.

Un collegamento che Pack and Go non riesce a seguire resta invariato, e il messaggio lo elenca. Pack and Go non include:

- Un file che non può essere trovato.
- Un indirizzo web. Questi restano invariati.
- Un file presente su un altro computer della tua rete.
- Un collegamento che risale e esce dalla cartella della presentazione, ad esempio **../../notes.png**.
- Un documento, una cartella di lavoro o un altro oggetto che non si trova nella cartella della presentazione (o in una cartella al suo interno), anche quando il collegamento indica il percorso completo. Solo immagini, filmati e suoni vengono inclusi da qualsiasi posizione.
- Un collegamento che esce dalla cartella della presentazione attraverso un collegamento simbolico (symbolic link) verso un'altra cartella.
- Un collegamento alla presentazione stessa.
- Un file che non è un'immagine, un filmato, un suono o un documento. I programmi non vengono mai inclusi.

Pack and Go non cerca altri collegamenti all'interno dei file collegati, quindi un file che rimanda a questo non può creare un ciclo infinito.

## Limiti

Per evitare che il pacchetto diventi troppo grande, Pack and Go chiede conferma quando i file collegati sono più di 200 o superano 256 MB. Scegli **No** e non viene salvato nulla. Pack and Go non include mai più di 2.000 file né più di 1 GB, qualunque risposta tu dia. I file oltre tale limite vengono esclusi e elencati nel messaggio.

Se la presentazione è protetta da password, la copia nel pacchetto ha la stessa password.
