# Creare diapositive da una struttura
<!-- keywords: diapositive da struttura, importa struttura, markdown, struttura testo, rtf, docx, titoli in diapositive, elenco puntato in diapositive, avvia presentazione da testo, salva struttura, esporta struttura, diapositive da testo, creare presentazione da file di testo, struttura, contorno -->

Puoi scrivere una presentazione come testo semplice e lasciare che JeffDeck la trasformi in diapositive. Ogni diapositiva ha un titolo e un corpo, con il layout Titolo e contenuto, quindi la **Visualizzazione struttura**, **Reimposta diapositiva** e i temi funzionano su di essa come di consueto.

## Aggiungere le diapositive alla presentazione aperta

1. Fai clic su **Home > Diapositive**, poi sulla piccola freccia sotto **Nuova diapositiva**, e scegli **Diapositive da struttura…**.
2. Scegli un file. Può essere un file di testo (.txt), un file Markdown (.md), un file di testo formattato (.rtf) o un file di documento (.docx).
3. Le diapositive vengono aggiunte dopo quella in cui ti trovi. Costituiscono un'unica operazione, quindi un solo Ctrl+Z le annulla tutte.

## Oppure avviare una nuova presentazione da una struttura

Scegli **File > Apri** e seleziona il file della struttura. JeffDeck crea una nuova presentazione a partire da esso. Il file non viene mai modificato, e **Salva** chiede un nome.

## Come viene letto il file

**File di testo.** Una riga che inizia al margine sinistro è il titolo di una diapositiva. Le righe sotto di essa sono il testo della diapositiva. Una riga rientrata più di quella precedente passa a un livello più in basso (usa tabulazioni o spazi). Un punto elenco (-, * o +) o un numero all'inizio di una riga viene eliminato, perché la diapositiva ha già i suoi punti elenco.

**File Markdown.** Il livello di titolo più alto usato nel file crea le diapositive: # se il file ne contiene, altrimenti ##. I titoli di livello inferiore diventano righe del testo della diapositiva, un livello più in basso per ogni passo. Una riga di tre trattini (---) avvia una nuova diapositiva. Gli elenchi diventano livelli. **Grassetto**, *corsivo*, `codice` e collegamenti mantengono il loro aspetto. Un blocco di codice compreso tra righe di tre apici inversi diventa una casella di testo in un carattere a larghezza fissa. Un'immagine su una riga propria, scritta nella forma Markdown per le immagini (un punto esclamativo, la descrizione tra parentesi quadre e il nome del file tra parentesi tonde), viene collocata nella diapositiva, ma solo se il file dell'immagine si trova nella stessa cartella della struttura, o in una cartella al suo interno. Le immagini provenienti da Internet o da altre cartelle non vengono mai caricate.

**File di testo formattato e .docx.** I paragrafi in uno stile Titolo creano le diapositive: lo stile di titolo più alto usato nel file diventa il titolo, i titoli di livello inferiore diventano righe del testo, e ogni altro paragrafo è una riga sotto il titolo che lo precede.

## Limiti

Questi limiti valgono per ogni tipo di file di struttura, testo, Markdown, testo formattato e .docx:

- Fino a 5.000 diapositive. Quelle oltre questo numero vengono omesse.
- Fino a 4.000 caratteri per riga. Una riga più lunga viene troncata, così come un paragrafo Markdown le cui righe, unite, superano tale lunghezza.
- Fino a 500 righe di testo su una diapositiva e 100.000 righe in totale.
- Il testo di una diapositiva può avere fino a 9 livelli; una riga rientrata più in profondità viene mostrata al livello più basso.
- Fino a 200 righe di codice su una diapositiva.
- Fino a 100 immagini in totale e 4 su una diapositiva. Un file immagine può arrivare a 32 MB, e tutti i file immagine insieme a 64 MB. Un'immagine deve essere un file PNG, JPEG, GIF, BMP, WebP o TIFF, indipendentemente dal nome, con fino a 16 milioni di pixel (4.000 per 4.000), e tutte le immagini insieme fino a 64 milioni di pixel.

Anche le dimensioni del file stesso sono limitate. Un file di testo o Markdown può arrivare a 10 MB, in UTF-8 o UTF-16. Un file di testo formattato o .docx può arrivare a 96 MB. Un file che non è di testo viene rifiutato. Le immagini contenute in un file di testo formattato o .docx non vengono inserite nelle diapositive, e JeffDeck indica quante ne sono state omesse.

Quando qualcosa viene omesso, come un'immagine che non è stato possibile utilizzare o un testo oltre un limite, JeffDeck ti dice che cosa è successo. Mentre legge un file grande, JeffDeck continua a funzionare, e chiudere la finestra interrompe la lettura.

## Salvare una copia come struttura

Scegli **File > Salva una copia** e seleziona **Struttura Markdown (*.md)**. Il file contiene il titolo e il testo di ciascuna diapositiva, con grassetto, corsivo e collegamenti. Non include immagini, forme, tabelle, grafici, altre caselle di testo, note del relatore né l'aspetto delle diapositive. JeffDeck te lo segnala al momento del salvataggio, e **File > Informazioni > Verifica compatibilità** lo elenca a sua volta.

## Vedi anche

- [Aggiungere, modificare ed eliminare diapositive](slides)
- [Riutilizzare diapositive da un'altra presentazione](reuse)
- [Visualizzazioni](views)
