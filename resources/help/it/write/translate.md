# Traduci testo
<!-- keywords: traduci, traduzione, traduttore, lingua, lingue, spagnolo, francese, tedesco, italiano, portoghese, olandese, russo, ucraino, indonesiano, offline, su questo computer, scarica, modelli, sostituisci, inserisci sotto, revisioni -->

**Revisione > Lingua > Traduci** apre il riquadro **Traduci** a destra. Traduce il testo sul proprio computer. Le parole non vengono inviate a nessuno e non serve una connessione internet una volta che i file di traduzione di una lingua sono presenti sul computer.

## Tradurre del testo

1. Seleziona il testo da tradurre. Se non c'è nulla di selezionato, il riquadro usa il paragrafo in cui si trova il cursore.
2. Fai clic su **Traduci** nel gruppo **Lingua**. Il riquadro mostra il testo sotto **Originale**.
3. In **Da**, scegli la lingua in cui è scritto il testo. JeffWrite parte dalla lingua indicata per il testo. In **A**, scegli la lingua desiderata. Il pulsante **Scambia** inverte le due lingue.
4. Fai clic su **Traduci**. La traduzione compare sotto **Traduzione**. Le parole in grassetto, corsivo e sottolineato restano in grassetto, corsivo e sottolineato, e un colore o una dimensione vengono mantenuti dove le parole avevano già lo stesso aspetto.
5. Fai clic su **Sostituisci** per mettere la traduzione al posto del testo originale, oppure su **Inserisci sotto** per aggiungerla in nuovi paragrafi sotto il testo. Ciascuna delle due operazioni è un unico passaggio, quindi **Annulla** la ripristina. Le parole tradotte sono contrassegnate con la loro lingua, così il controllo ortografia le verifica in quella lingua.

Se sposti il cursore o modifichi il testo dopo aver fatto clic su **Traduci**, fai clic su **Usa selezione** per rileggere il testo, quindi fai di nuovo clic su **Traduci**. Il riquadro non sostituirà con una traduzione un testo che non è più quello tradotto.

Quando **Revisioni** è attivo, **Sostituisci** e **Inserisci sotto** sono modifiche tracciate come la digitazione, così un revisore può vedere che cosa c'era prima. Le parole tradotte assumono quindi l'aspetto che avrebbe la tua digitazione. Le parole eliminate da una modifica tracciata restano invariate: non vengono tradotte né copiate da **Inserisci sotto**.

**Annulla** interrompe una traduzione che richiede troppo tempo, e lo stesso fa la chiusura del riquadro: la finestra non attende mai che la traduzione termini. I testi lunghi richiedono più tempo: una pagina di testo richiede alcuni secondi sulla maggior parte dei computer. Il riquadro traduce al massimo 1.000.000 di lettere alla volta (circa 200 pagine); se ne sono selezionate di più, lo segnala e chiede di selezionarne meno. I paragrafi lunghi vengono suddivisi in parti di circa 1.500 lettere, alla fine di una frase quando possibile, e poi ricomposti.

## La prima volta che usi una lingua

Una traduzione richiede un insieme di file, chiamato modello, per ciascuna direzione: uno per dall'inglese al tedesco, un altro per dal tedesco all'inglese. Ogni modello occupa circa 37 MB. I file non fanno parte di JeffWrite. La prima volta che traduci con una lingua, il riquadro indica di cosa ha bisogno e le sue dimensioni, ad esempio "Scaricare il tedesco (74 MB)?". Nulla viene scaricato finché non fai clic su quel pulsante. Il riquadro mostra a che punto è il download, e **Annulla** lo interrompe senza lasciare file residui.

JeffWrite verifica ogni file con un'impronta (un checksum SHA-256) prima di usarlo. Un file che non corrisponde viene eliminato e la lingua non viene installata.

I file sono conservati in un'unica cartella condivisa da tutte le app di JeffOffice: una cartella chiamata JeffOffice nella cartella dati utente, in Translation/models. Così una lingua scaricata in un'app è subito disponibile nelle altre, e viene scaricata una sola volta. Due lingue che non hanno un modello proprio, come tedesco e francese, passano per l'inglese, quindi il riquadro ha bisogno di entrambi i modelli.

## Quando una lingua non ha un modello

Se una coppia di lingue non ha un modello, il riquadro lo segnala e offre **Open in Your Browser Instead**, cioè di aprire un sito di traduzione gratuito, LibreTranslate (o il servizio il cui indirizzo hai scelto nelle impostazioni), con il tuo testo. Solo a quel punto il testo lascia il tuo computer, perché viene inviato a quel sito, e il riquadro non dice più che il testo resta sul computer. Nulla viene inviato finché non fai clic su quel pulsante.

## Sulla qualità

I modelli sono piccoli, quindi funzionano rapidamente su un computer normale. Traducono bene le frasi e meno bene le etichette brevi: una singola parola o il nome di un pulsante può risultare errato. Leggi la traduzione prima di usarla. Nomi, numeri e tabulazioni vengono mantenuti come erano.

Immagini, tabulazioni e interruzioni di riga all'interno di un paragrafo non vengono modificate. La traduzione riempie solo le parole tra di esse.

## Vedi anche

- [Conta le parole, imposta la lingua, verifica l'accessibilità e cerca parole](proofing-tools)
