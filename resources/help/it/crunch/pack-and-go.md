# Pack and Go: invia una cartella di lavoro con i file necessari
<!-- keywords: pack and go, pacchetto, invia, condividi, altro computer, zip, cartella, tipi di carattere, font, file collegati, immagini collegate, immagini mancanti, copia, porta con te, chiavetta USB, licenza, collegamento esterno, cartella di lavoro collegata, celle, grafici -->

Quando invii una cartella di lavoro a qualcuno, le immagini e le altre cartelle di lavoro a cui è collegata possono andare perse per strada. **Pack and Go** salva la cartella di lavoro insieme a quei file e ai suoi tipi di carattere, in un unico file ZIP o in un'unica cartella, così appare uguale su un altro computer.

## Come usarlo

1. Fai clic su **File**, quindi su **Esporta**.
2. Fai clic su **Pack and Go: Salva per un altro computer** per creare un unico file ZIP, oppure su **Pack and Go: Salva in una cartella** per creare una nuova cartella.
3. Scegli dove salvarlo. Per un file ZIP, digita un nome. Per una cartella, scegli la cartella in cui creare quella nuova.
4. Leggi il messaggio che compare. Indica dove si trova il pacchetto, quanti file collegati e quanti file di tipi di carattere sono stati inseriti e che cosa non è stato possibile impacchettare.

Un **file ZIP** è un unico file che contiene altri file, quindi è facile da inviare. L'altra persona lo apre per estrarne i file. Una cartella è più comoda quando vuoi copiare tutto su un'unità di memorizzazione.

## Che cosa va nel pacchetto

- **La cartella di lavoro stessa**, con lo stesso nome che ha ora. È una copia: il tuo file non cambia.
- **Immagini collegate.** Un'immagine della cartella di lavoro può essere collegata al suo file. Il pacchetto contiene una copia di ciascuna. Le immagini inserite direttamente nella cartella di lavoro sono già al suo interno.
- **Cartelle di lavoro collegate.** Una formula può prendere numeri da un'altra cartella di lavoro. Il pacchetto contiene anche una copia di quella cartella di lavoro e il collegamento ora rimanda alla copia. I tipi di carattere sono quelli usati nelle celle, nei grafici e nelle forme.
- **Tipi di carattere.** Un tipo di carattere è l'aspetto delle lettere. Il pacchetto contiene i file dei tipi di carattere che vengono forniti con il programma e che la cartella di lavoro usa, in una cartella chiamata **fonts**. Pack and Go non impacchetta mai un tipo di carattere la cui licenza ne vieta la distribuzione. Il messaggio indica ciascun tipo di carattere che è stato escluso. I tipi di carattere installati sul computer da te non vengono impacchettati, quindi l'altra persona potrebbe averne bisogno.

## Come cambiano i collegamenti

Nella copia, ogni collegamento impacchettato punta al file all'interno del pacchetto, tramite un nome breve, ad esempio **linked/photo.png**. I collegamenti non usano più il percorso sul tuo computer, quindi funzionano ovunque venga aperto il pacchetto. Se due file hanno lo stesso nome, al secondo viene aggiunto un numero.

Un collegamento che Pack and Go non riesce a seguire resta invariato, e il messaggio lo elenca. Pack and Go non impacchetta:

- Un file che non può essere trovato.
- Un indirizzo web. Questi restano invariati.
- Un file su un altro computer della tua rete.
- Un collegamento che esce dalla cartella della cartella di lavoro risalendo, ad esempio **../../notes.png**.
- Un documento, una cartella di lavoro o un altro oggetto che non si trova nella cartella della cartella di lavoro (o in una cartella al suo interno), anche quando il collegamento ne indica il percorso completo. Solo immagini, filmati e suoni vengono impacchettati da qualsiasi posizione.
- Un collegamento che esce dalla cartella della cartella di lavoro attraverso una scorciatoia verso un'altra cartella (un collegamento simbolico).
- Un collegamento alla cartella di lavoro stessa.
- Un file che non è un'immagine, un filmato, un suono o un documento. Un programma non viene mai impacchettato.

Pack and Go non cerca altri collegamenti all'interno dei file collegati, quindi un file che rimanda a questo non può creare un ciclo.

## Limiti

Per evitare che un pacchetto diventi troppo grande, Pack and Go ti chiede conferma quando i file collegati sono più di 200 o occupano più di 256 MB. Scegli **No** e non viene salvato nulla. Pack and Go non impacchetta mai più di 2.000 file o più di 1 GB, qualunque sia la tua risposta. I file oltre questo limite vengono esclusi ed elencati nel messaggio.

Se per la cartella di lavoro usi una password, la copia nel pacchetto ha la stessa password.
