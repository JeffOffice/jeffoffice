# Controllo ortografia

<!-- keywords: ortografia, controllo ortografia, correttore ortografico, parola errata, sottolineatura rossa, linea ondulata, sottolineatura ondulata, suggerimenti, ignora, ignora tutto, aggiungi al dizionario, dizionario personalizzato, dizionario, lingua, correzione di bozze, strumenti di correzione, F7, revisione, controlla ortografia durante la digitazione, maiuscole, numeri, grammatica, controllo grammaticale, stile, linea blu, linea punteggiata, controlla grammatica durante la digitazione -->

JeffWrite controlla l'ortografia mentre digiti. Una parola che non è nel dizionario viene sottolineata con una linea rossa ondulata. Il controllo viene eseguito sul tuo computer, senza connessione a internet, e riconosce 31 lingue e varianti regionali.

## Mentre digiti

- Una linea rossa ondulata indica che il dizionario non contiene la parola. La parola potrebbe comunque essere corretta. Nomi e parole nuove spesso non sono presenti nel dizionario. I nomi delle app di JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC non vengono mai contrassegnati, in nessuna lingua, con o senza la "s" finale.
- La parola che stai digitando non viene controllata finché non passi oltre, così una parola non ancora completata non lampeggia.
- Le linee compaiono solo sullo schermo. Non vengono mai stampate e non finiscono mai in un PDF.

Per correggere una parola, fai **clic con il pulsante destro** su di essa. Il menu mostra queste opzioni sopra quelle usuali:

- Fino a sei **suggerimenti**, il migliore per primo, in grassetto. Fai clic su uno di essi per sostituire la parola. È un'unica operazione, quindi **Ctrl+Z** la annulla.
- **Ignora tutto** lascia invariata quella parola, ovunque compaia, fino alla chiusura di JeffWrite.
- **Aggiungi al dizionario** insegna a JeffWrite la parola in modo permanente. La parola viene salvata in un piccolo file nella cartella di JeffWrite, così puoi leggere o modificare lì l'elenco delle parole aggiunte.

Se il dizionario non ha alcun suggerimento, il menu lo indica.

## Scorrere gli errori ortografici

Premi **F7** oppure fai clic su **Revisione > Controllo ortografia**. Sulla destra si apre il riquadro Controllo ortografia. Mostra la prima parola errata dopo il cursore, selezionata nel testo, con la riga in cui si trova e un elenco di suggerimenti.

- **Cambia** sostituisce la parola con il suggerimento scelto. Fai doppio clic su un suggerimento per ottenere lo stesso risultato.
- **Cambia tutto** fa lo stesso per ogni occorrenza della parola nel documento. È un'unica operazione, quindi **Ctrl+Z** annulla tutte le modifiche.
- **Ignora** salta questa parola. La sua linea rossa resta.
- **Ignora tutto** e **Aggiungi al dizionario** funzionano come nel menu del clic destro.

Il riquadro arriva alla fine del documento, poi continua dall'inizio, fino al punto da cui è partito. Quando non ne restano altri, indica che il controllo ortografico è completato. Al termine del controllo ortografico, il riquadro passa ai problemi di grammatica e stile (vedi Controllare la grammatica, più avanti).

## Attivare o disattivare le linee rosse

Fai clic su **Revisione > Controlla ortografia durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Strumenti di correzione**, ed è l'unica impostazione condivisa da tutte le app di JeffOffice. Lì puoi anche scegliere di saltare le parole in MAIUSCOLO e quelle che contengono numeri, come 3°. Inizialmente entrambe vengono saltate.

## Controllare la grammatica mentre digiti

JeffWrite cerca anche errori grammaticali e di stile evidenti nei testi in inglese. Un problema viene sottolineato con una linea blu punteggiata. L'ortografia mantiene la sua linea rossa ondulata, e una parola errata non viene mai segnalata come problema grammaticale. Questi sono gli errori che cerca:

- una parola ripetuta ("the the")
- "a" e "an" davanti al suono sbagliato ("an test", "a hour")
- una frase che inizia con una lettera minuscola dopo un punto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando le parole intorno rendono chiaro l'errore
- "could of" al posto di "could have"
- due spazi di seguito all'interno di una frase
- uno spazio prima di una virgola, di un punto o di un altro segno che appartiene alla parola precedente
- nessuno spazio dopo la fine di una frase
- "alot", e "everyday" al posto di "every day" ("almost everyday")

Il programma preferisce non segnalare un errore piuttosto che contrassegnare un testo corretto, quindi una frase senza linea blu non è una garanzia che sia corretta.

Fai clic con il pulsante destro sul problema. Il menu indica cosa non va, seguito da uno o più suggerimenti. Fai clic su un suggerimento per inserirlo nel testo, e premi Ctrl+Z per annullarlo. **Ignora questo problema** rimuove la linea solo in questo punto, fino alla chiusura di JeffWrite. Le stesse parole in un altro punto conservano la loro linea, e modificare altro nel paragrafo non la fa ricomparire. **Ignora tutto** rimuove la linea da ogni problema con questo messaggio su queste parole.

Il controllo viene eseguito sul tuo computer, senza connessione a internet, in background: la digitazione non viene mai rallentata, e un paragrafo viene controllato poco dopo che hai smesso di scrivere in esso. Un paragrafo viene controllato fino ai suoi primi 100.000 caratteri. Un documento che richiede un controllo superiore al limite smette di essere controllato, e la barra di stato lo segnala una sola volta; il controllo ortografico continua a funzionare.

Viene controllato solo l'inglese. In un paragrafo con più lingue, viene controllato il testo inglese e il resto viene saltato, così come il testo che un file contrassegna come "non controllare". La grammatica viene controllata nel testo del documento, e non ancora nelle tabelle, nelle intestazioni, nei piè di pagina o nelle note.

Per disattivarlo, fai clic su **Revisione > Controlla grammatica durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Strumenti di correzione**. Solo JeffWrite la offre, ed è la stessa in tutte le finestre di JeffWrite. Quando l'opzione è attiva, il riquadro Controllo ortografico (F7) scorre i problemi di grammatica dopo quelli di ortografia.

## Lingue

JeffWrite controlla ogni porzione di testo nella propria lingua. Un file può indicare la lingua del testo, fino a una singola parola, e JeffWrite la legge. Il testo che non indica nulla viene controllato nella lingua del documento. Il file assegna una lingua a tutto il documento, e un nuovo documento usa l'inglese americano.

Il testo in una lingua per cui JeffWrite non ha un dizionario non viene controllato. Non viene controllato neppure il testo che un file contrassegna come "non controllare ortografia". Non è ancora possibile cambiare la lingua del testo in JeffWrite, ma il programma mantiene quella indicata dal file.

I 31 dizionari sono: inglese (americano, britannico, canadese e australiano), spagnolo (21 varianti regionali, tra cui messicano e spagnolo di Spagna), francese, tedesco, italiano, olandese e portoghese (brasiliano e portoghese europeo).

## Cosa non viene ancora controllato

Le linee rosse compaiono nel testo e nelle celle delle tabelle. Non compaiono ancora nelle intestazioni, nei piè di pagina, nelle note o nelle caselle di testo, e il riquadro Controllo ortografico non le controlla.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
