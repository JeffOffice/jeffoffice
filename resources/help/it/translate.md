# Traduci testo
<!-- keywords: traduci, traduzione, traduttore, lingua, lingue, spagnolo, francese, tedesco, italiano, portoghese, olandese, russo, ucraino, indonesiano, offline, su questo computer, download, modelli, sostituisci, inserisci sotto -->

**Revisione > Lingua > Traduci** apre il riquadro **Traduci**. Il riquadro traduce il testo direttamente sul tuo computer. Le tue parole non vengono inviate a nessuno, e non serve una connessione a internet una volta che i file di traduzione di una lingua sono sul computer.

## Tradurre del testo

1. Seleziona il testo da tradurre. Se non è selezionato nulla, il riquadro usa il paragrafo in cui si trova il cursore. In una casella di testo selezionata ma non in modifica, usa tutto il testo della casella.
2. Fai clic su **Revisione > Lingua > Traduci**. Il riquadro mostra il tuo testo sotto **Originale**.
3. In **Da**, scegli la lingua del tuo testo. In **A**, scegli la lingua desiderata. Il pulsante **Scambia** inverte le due lingue.
4. Fai clic su **Traduci**. La traduzione compare in **Traduzione**. Le parole in grassetto, corsivo e sottolineate restano in grassetto, corsivo e sottolineate.
5. Fai clic su **Sostituisci** per collocare la traduzione al posto del testo originale, oppure su **Inserisci sotto** per aggiungerla in nuovi paragrafi sotto il testo. Ciascuna delle due operazioni è un unico passaggio, quindi **Annulla** la ripristina.

Se sposti il cursore o modifichi il testo dopo aver fatto clic su **Traduci**, fai clic su **Usa selezione** per rileggere il testo, poi fai di nuovo clic su **Traduci**. Il riquadro non sostituisce una traduzione su un testo che non è più quello tradotto.

**Annulla** interrompe una traduzione che richiede troppo tempo, e lo stesso fa la chiusura del riquadro: la finestra non attende mai il completamento. I testi lunghi richiedono più tempo: una pagina di testo richiede alcuni secondi sulla maggior parte dei computer. Il riquadro traduce al massimo 1.000.000 di lettere alla volta (circa 200 pagine); se ne selezioni di più, lo segnala e ti chiede di selezionarne meno. I paragrafi lunghi vengono divisi in parti di circa 1.500 lettere, possibilmente alla fine di una frase, e poi ricomposti.

## La prima volta che usi una lingua

Una traduzione richiede un insieme di file, chiamato modello, per ciascuna direzione: uno per dall'inglese al tedesco e un altro per dal tedesco all'inglese. Ogni modello occupa circa 37 MB. I file non fanno parte di JeffPub. La prima volta che traduci con una lingua, il riquadro ti dice che cosa serve e quanto pesa, per esempio "Scaricare il tedesco (74 MB)?". Non viene scaricato nulla finché non fai clic su quel pulsante. Il riquadro mostra l'avanzamento del download, e **Annulla** lo interrompe senza lasciare file residui.

Prima di usare ciascun file, JeffPub ne verifica l'impronta (un checksum SHA-256). Un file che non corrisponde viene eliminato e la lingua non viene installata.

I file sono conservati in un'unica cartella condivisa da tutte le app di JeffOffice: una cartella chiamata JeffOffice nella cartella dei dati utente, in Translation/models. Così una lingua scaricata in un'app è pronta anche nelle altre, e viene scaricata una sola volta. Due lingue che non hanno un modello proprio, come tedesco e francese, passano attraverso l'inglese, quindi il riquadro richiede entrambi i modelli.

## Quando una lingua non ha un modello

Se una coppia di lingue non ha un modello, il riquadro lo segnala e propone **Apri nel tuo browser invece**. Il pulsante apre un sito di traduzione gratuito, LibreTranslate (oppure il servizio il cui indirizzo hai scelto nelle impostazioni), con il tuo testo. Solo allora il testo lascia il tuo computer, perché viene inviato a quel sito, e il riquadro non dichiara più che resta sul computer. Nulla viene inviato finché non fai clic su quel pulsante.

## Sulla qualità

I modelli sono piccoli, quindi girano rapidamente su un computer normale. Traducono bene le frasi e meno bene le etichette brevi: una singola parola o il nome di un pulsante può risultare sbagliato. Leggi la traduzione prima di usarla. Nomi, numeri e tabulazioni vengono mantenuti così come sono.

Immagini, tabulazioni e interruzioni di riga all'interno di un paragrafo non vengono modificate. La traduzione riempie le parole tra di esse.

## Vedi anche

- [Controllo ortografia, thesaurus, ricerca e sillabazione](proofing)
