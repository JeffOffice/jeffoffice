# Traduci testo
<!-- keywords: traduci, traduzione, traduttore, lingua, lingue, spagnolo, francese, tedesco, italiano, portoghese, olandese, russo, ucraino, indonesiano, offline, su questo computer, download, scaricare, modelli, sostituisci, inserisci sotto, revisioni -->

**Revisione > Lingua > Traduci** apre il riquadro **Traduci** a destra. Il programma traduce il testo sul proprio computer. Le parole non vengono inviate da nessuna parte e non serve una connessione internet quando i file di traduzione per una lingua sono già sul computer.

## Tradurre un testo

1. Seleziona il testo da tradurre. Se non è selezionato nulla, il riquadro usa il paragrafo in cui si trova il cursore.
2. Fai clic su **Traduci** nel gruppo **Lingua**. Il riquadro mostra il testo in **Originale**.
3. In **Da**, scegli la lingua del testo. JeffWrite parte dalla lingua indicata per il testo. In **A**, scegli la lingua desiderata. Il pulsante **Scambia** inverte le due lingue.
4. Fai clic su **Traduci**. La traduzione compare in **Traduzione**. Le parole in grassetto, corsivo e sottolineato restano in grassetto, corsivo e sottolineato, e un colore o una dimensione vengono mantenuti dove le parole avevano già lo stesso aspetto.
5. Fai clic su **Sostituisci** per mettere la traduzione al posto del testo originale, oppure su **Inserisci sotto** per aggiungerla in nuovi paragrafi sotto il testo. Ciascuna delle due operazioni è un unico passaggio, quindi **Annulla** la ripristina. Le parole tradotte sono contrassegnate con la loro lingua, così il controllo ortografico le verifica in quella lingua.

Se sposti il cursore o modifichi il testo dopo aver fatto clic su **Traduci**, fai clic su **Usa selezione** per rileggere il testo, poi fai di nuovo clic su **Traduci**. Il riquadro non sostituisce una traduzione su un testo che non è più quello tradotto.

Quando **Revisioni** è attivo, **Sostituisci** e **Inserisci sotto** sono modifiche registrate come la tua digitazione, così un revisore può vedere che cosa c'era prima. Le parole tradotte assumono allora l'aspetto che avrebbe la tua digitazione. Le parole che una modifica registrata ha eliminato non vengono toccate: non vengono tradotte e non vengono copiate da **Inserisci sotto**.

**Annulla** interrompe una traduzione che richiede troppo tempo, e così fa la chiusura del riquadro: la finestra non attende mai la traduzione. I testi lunghi richiedono più tempo: una pagina di testo richiede alcuni secondi sulla maggior parte dei computer. Il riquadro traduce al massimo 1.000.000 di lettere alla volta (circa 200 pagine); se se ne selezionano di più, lo segnala e chiede di selezionare meno testo. I paragrafi lunghi vengono divisi in parti di circa 1.500 lettere, alla fine di una frase quando possibile, e poi riuniti.

## La prima volta che usi una lingua

Una traduzione richiede un insieme di file, detto modello, per ciascuna direzione: uno per dall'inglese al tedesco, un altro per dal tedesco all'inglese. Ogni modello occupa circa 37 MB. I file non fanno parte di JeffWrite. La prima volta che traduci con una lingua, il riquadro indica che cosa serve e quanto spazio occupa, per esempio «Scaricare il tedesco (74 MB)?». Non viene scaricato nulla finché non fai clic su quel pulsante. Il riquadro mostra l'avanzamento del download, e **Annulla** lo interrompe senza lasciare nulla sul computer.

JeffWrite verifica ogni file con un'impronta (un checksum SHA-256) prima di usarlo. Un file che non corrisponde viene eliminato e la lingua non viene installata.

I file sono conservati in un'unica cartella condivisa da tutte le app di JeffOffice (una cartella chiamata JeffOffice nella cartella dei dati utente, in Translation/models), quindi una lingua scaricata in un'app è disponibile anche nelle altre, e viene scaricata una sola volta. Due lingue che non hanno un modello proprio, come tedesco e francese, passano attraverso l'inglese, quindi il riquadro ha bisogno di entrambi i modelli.

## Quando una lingua non ha un modello

Se una coppia di lingue non ha un modello, il riquadro lo segnala e propone **Apri invece nel browser**. Questo apre con il testo un sito di traduzione gratuito, LibreTranslate (o il servizio il cui indirizzo hai scelto nelle impostazioni per questo scopo). Solo a questo punto il testo esce dal computer, perché viene inviato a quel sito, e il riquadro non dichiara più che resta sul computer. Non viene inviato nulla finché non fai clic su quel pulsante.

## Sulla qualità

I modelli sono piccoli, quindi funzionano rapidamente su un computer comune. Traducono bene le frasi e meno bene le etichette brevi: una singola parola o il nome di un pulsante possono risultare sbagliati. Leggi la traduzione prima di usarla. Nomi, numeri e tabulazioni vengono mantenuti come erano.

Immagini, tabulazioni e interruzioni di riga all'interno di un paragrafo non vengono modificate. La traduzione riempie le parole tra di esse.

## Vedi anche

- [Contare le parole, impostare la lingua, verificare l'accessibilità e cercare parole](proofing-tools)
