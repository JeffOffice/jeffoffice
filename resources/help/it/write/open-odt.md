# Aprire e salvare file OpenDocument
<!-- keywords: odt, fodt, ott, opendocument, open document, aprire, apri, salvare, salva, salva con nome, formato, tipo di file, altri programmi, scambiare documenti, condividere, compatibilità, revisioni, modifiche rilevate, omessi, elementi persi, modello -->

JeffWrite apre e salva testi OpenDocument, il tipo di file il cui nome termina con **.odt**. Molti programmi gratuiti sono in grado di leggerlo e scriverlo, quindi è un buon modo per scambiare un documento con chi non usa JeffWrite.

## Aprire un file OpenDocument

Premi **Ctrl+O** oppure fai clic su **File > Apri** e scegli un file:

- **.odt** è un documento.
- **.ott** è un modello. Si apre come nuovo documento basato sul modello, che quindi rimane invariato.
- **.fodt** è lo stesso tipo di documento, contenuto in un unico file XML. JeffWrite non scrive file .fodt: se usi **Salva** su un file di questo tipo, ti viene chiesto un nuovo nome e il file viene salvato in formato .odt.

JeffWrite riconosce il tipo di file dal contenuto e non dal nome, quindi si apre anche un file con l'estensione sbagliata. Puoi anche indicare il nome di un file all'avvio di JeffWrite oppure trascinarlo sul programma dalla gestione file, come per un file .docx.

JeffWrite legge il testo e la sua formattazione, i titoli, gli stili e gli stili su cui sono basati, gli elenchi puntati e numerati, le tabelle (con celle unite, larghezza delle colonne, bordi, sfondo e tabelle annidate), le sezioni e le colonne, le dimensioni, i margini e l'orientamento della pagina, le intestazioni e i piè di pagina (prima pagina, pagine sinistre e destre), le note a piè di pagina e le note di chiusura, i campi (numeri di pagina, numero di pagine, titolo, autore, nome file, riferimenti incrociati e numerazioni), i segnalibri, i collegamenti, il sommario, le immagini, le caselle di testo, le tabulazioni, i commenti e le revisioni, che vengono mostrate come modifiche che puoi accettare o rifiutare.

Alcuni elementi non possono ancora essere visualizzati in JeffWrite: forme disegnate che non contengono testo, formule, grafici e altri oggetti inseriti nel testo. Se un file ne contiene, JeffWrite te lo indica all'apertura. In questo caso non salva sopra il file: **Salva** chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso. I documenti protetti da password non vengono aperti. Aprili nel programma con cui sono stati creati e salvane una copia senza password.

## Salvare un file OpenDocument

Fai clic su **File > Salva con nome** e scegli **Testo OpenDocument (*.odt)** nell'elenco dei tipi di file. Da quel momento, **Ctrl+S** salva di nuovo nello stesso formato.

Il file contiene gli elementi elencati sopra, con i commenti e le revisioni: testo aggiunto ed eliminato, formattazione modificata e interruzioni di paragrafo aggiunte o eliminate. Per alcuni elementi il formato non prevede uno spazio e JeffWrite li indica dopo il salvataggio: vengono salvati nel loro stato attuale oppure omessi. Si tratta delle revisioni di tabelle, colonne e impostazione della pagina e delle revisioni in intestazioni, piè di pagina e note (salvate nel loro stato attuale); dei campi modulo (salvati come il testo visualizzato); delle equazioni (salvate come testo normale); e di disegni, forme, grafici e oggetti incorporati che non è possibile mantenere. Se salvi di nuovo dopo una modifica che comporta l'omissione di un nuovo elemento, JeffWrite ti avvisa anche di questo, senza ripetere quanto ti ha già segnalato.

Gli altri programmi che leggono il formato OpenDocument aprono il file senza chiedere di ripararlo.

## Argomenti correlati

- [Aprire, salvare, stampare ed esportare](open-save)
- [Rilevare le modifiche e accettarle o rifiutarle](track-changes)
- [Benvenuto in JeffWrite](start)
