# Traduci testo
<!-- keywords: traduci, traduzione, traduttore, lingua, lingue, spagnolo, francese, tedesco, italiano, portoghese, olandese, russo, ucraino, indonesiano, offline, su questo computer, scarica, modelli, sostituisci, inserisci sotto -->

**Revisione > Lingua > Traduci** apre il riquadro **Traduci**. Il riquadro traduce il testo direttamente sul computer. Le tue parole non vengono inviate da nessuna parte e non serve una connessione internet, una volta che i file di traduzione per una lingua sono presenti nel computer.

## Tradurre del testo

1. Seleziona il testo da tradurre. Se non c'è nulla di selezionato, il riquadro usa il paragrafo in cui si trova il cursore. In una casella di testo selezionata ma non in modifica, usa tutto il testo della casella.
2. Fai clic su **Revisione > Lingua > Traduci**. Il riquadro mostra il tuo testo sotto **Originale**.
3. In **Da**, scegli la lingua del tuo testo. In **A**, scegli la lingua desiderata. Il pulsante **Inverti** scambia le due lingue.
4. Fai clic su **Traduci**. La traduzione compare sotto **Traduzione**. Le parole in grassetto, corsivo e sottolineato restano in grassetto, corsivo e sottolineato.
5. Fai clic su **Sostituisci** per collocare la traduzione al posto del testo originale, oppure su **Inserisci sotto** per aggiungerla in nuovi paragrafi sotto il testo. Ognuna delle due operazioni è un unico passaggio, quindi **Annulla** la può annullare.

Se sposti il cursore o modifichi il testo dopo aver fatto clic su **Traduci**, fai clic su **Usa selezione** per rileggere il testo, poi fai di nuovo clic su **Traduci**. Il riquadro non sostituirà con una traduzione un testo che non è più quello tradotto.

**Annulla** interrompe una traduzione che richiede troppo tempo, e lo stesso fa la chiusura del riquadro: la finestra non attende mai il completamento. I testi lunghi richiedono più tempo: una pagina di testo richiede alcuni secondi sulla maggior parte dei computer. Il riquadro traduce al massimo 1.000.000 di lettere alla volta (circa 200 pagine); se la selezione è più lunga, lo segnala e ti chiede di selezionare meno testo. Divide i paragrafi lunghi in parti di circa 1.500 lettere, alla fine di una frase quando possibile, e poi le riunisce.

## La prima volta che usi una lingua

Una traduzione richiede un insieme di file, chiamato modello, per ogni direzione: uno per dall'inglese al tedesco, un altro per dal tedesco all'inglese. Ogni modello occupa circa 37 MB. I file non fanno parte di JeffPub. La prima volta che traduci con una lingua, il riquadro ti dice che cosa serve e quanto è grande, per esempio "Scaricare il tedesco (74 MB)?". Non viene scaricato nulla finché non fai clic su quel pulsante. Il riquadro mostra a che punto è il download, e **Annulla** lo interrompe senza lasciare file residui.

JeffPub verifica ogni file con un'impronta (una somma di controllo SHA-256) prima di usarlo. Un file che non corrisponde viene eliminato e la lingua non viene installata.

I file sono conservati in un'unica cartella condivisa da tutte le app di JeffOffice (una cartella chiamata JeffOffice nella cartella dei dati utente, in Translation/models), quindi una lingua scaricata in un'app è pronta anche nelle altre, e viene scaricata una sola volta. Due lingue che non hanno un modello proprio, come tedesco e francese, passano attraverso l'inglese, quindi il riquadro ha bisogno di entrambi i modelli.

## Quando una lingua non ha un modello

Se una coppia di lingue non ha un modello, il riquadro lo segnala e propone il pulsante **Apri nel browser**. Questo apre con il testo un sito gratuito di traduzione, LibreTranslate (o il servizio il cui indirizzo hai scelto nelle impostazioni), e il testo viene inviato lì. Solo allora il testo esce dal computer, perché viene inviato a quel sito, e il riquadro non dice più che resta sul computer. Non viene inviato nulla finché non fai clic su quel pulsante.

## Qualità della traduzione

I modelli sono piccoli, quindi funzionano rapidamente su un computer normale. Traducono bene le frasi e meno bene le etichette brevi: una singola parola o il nome di un pulsante può risultare sbagliato. Leggi la traduzione prima di usarla. Nomi, numeri e tabulazioni vengono mantenuti così come sono.

Le immagini, le tabulazioni e le interruzioni di riga all'interno di un paragrafo non vengono modificate. La traduzione riempie le parole tra di esse.

## Vedi anche

- [Controllo ortografia, thesaurus, ricerche e sillabazione](proofing)
