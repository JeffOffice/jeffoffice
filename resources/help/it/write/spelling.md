# Controllo ortografia

<!-- keywords: ortografia, controllo ortografia, correttore ortografico, parola errata, linea rossa, linea ondulata, sottolineatura ondulata, suggerimenti, ignora, ignora tutto, aggiungi al dizionario, dizionario personalizzato, dizionario, lingua, correzione, F7, revisione, controlla ortografia durante la digitazione, maiuscole, numeri, grammatica, controllo grammaticale, stile, linea blu, linea punteggiata, controlla grammatica durante la digitazione -->

JeffWrite controlla l'ortografia mentre scrivi. Una parola che non è nel dizionario viene sottolineata con una linea rossa ondulata. Il controllo viene eseguito sul tuo computer, senza connessione a internet, e conosce 31 lingue e varianti regionali.

## Durante la digitazione

- Una linea rossa ondulata indica che il dizionario non contiene la parola. La parola potrebbe essere corretta comunque. Nomi e parole nuove spesso mancano. I nomi delle app di JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC non vengono mai segnalati, in nessuna lingua, con o senza la "s" finale.
- La parola che stai digitando non viene controllata finché non passi oltre, così una parola non ancora completata non lampeggia.
- Le linee compaiono solo sullo schermo. Non vengono mai stampate e non finiscono mai in un PDF.

Per correggere una parola, fai **clic destro** su di essa. Il menu mostra queste scelte sopra quelle abituali:

- Fino a sei **suggerimenti**, il migliore per primo, in grassetto. Fai clic su uno per sostituire la parola. È un'unica operazione, quindi **Ctrl+Z** la annulla.
- **Ignora tutto** lascia invariata quella parola, ovunque compaia, fino a quando non chiudi JeffWrite.
- **Aggiungi al dizionario** insegna a JeffWrite la parola in modo permanente. La parola viene salvata in un piccolo file nella cartella di JeffOffice dell'utente, e tutte le app di JeffOffice la usano. Per vedere o modificare l'elenco delle parole, vedi Le tue parole, più avanti.

Se il dizionario non ha alcun suggerimento, il menu lo indica.

## Scorrere gli errori di ortografia

Premi **F7**, oppure fai clic su **Revisione > Ortografia**. Sulla destra si apre il riquadro Ortografia. Mostra la prima parola errata dopo il cursore, selezionata nel testo, con la riga in cui si trova e un elenco di suggerimenti.

- **Cambia** sostituisce la parola con il suggerimento scelto. Fai doppio clic su un suggerimento per ottenere lo stesso risultato.
- **Cambia tutto** applica la modifica a ogni occorrenza della parola nei punti che il riquadro attraversa, elencati qui sotto. È un'unica operazione, quindi **Ctrl+Z** le annulla tutte, ovunque si trovassero. Al termine, il riquadro indica quante parole ha cambiato e in quanti punti. Il testo, ogni casella di testo, ogni nota e ogni intestazione o piè di pagina contano ciascuno come un punto.
- **Ignora** salta questa parola. La sua linea rossa resta.
- **Ignora tutto** e **Aggiungi al dizionario** funzionano come nel menu del clic destro.

Il riquadro parte dal cursore: nel testo, oppure nella casella di testo, nella nota o nell'intestazione o piè di pagina che stai modificando. Scorre prima il testo del documento, tabelle comprese, poi le caselle di testo, poi le note a piè di pagina e infine le intestazioni e i piè di pagina. Quando una parola si trova in una casella di testo, in una nota, in un'intestazione o in un piè di pagina, JeffWrite lo apre per la modifica, come fa Trova, così vedi la parola dove si trova. Un'intestazione o un piè di pagina che nessuna pagina mostra viene saltato, e anche Cambia tutto lo lascia invariato. Dal cursore il riquadro va fino alla fine di quel punto, poi prosegue attraverso gli altri in quest'ordine, quindi ricomincia dall'inizio, fino al punto da cui era partito. **Cambia** è un'unica operazione annullabile, e **Cambia tutto** cerca in ogni punto attraversato dal riquadro ed è anch'essa un'unica operazione. Una parola in una nota di chiusura può essere ignorata o aggiunta, ma non cambiata. Quando non ne restano altre, il riquadro indica che il controllo ortografico è completato. Quando l'ortografia è terminata, passa ai problemi di grammatica e stile (vedi Controllo grammaticale, più avanti).

## Contare gli errori nella barra di stato

La barra di stato mostra quanti errori di ortografia e grammatica ha il documento, ad esempio **3 errori di correzione**. Fai clic su di essa per selezionare il prossimo errore dopo il cursore. Quando arrivi all'ultimo, un clic torna al primo.

JeffWrite conta mentre lavori, un po' alla volta, e controlla di nuovo solo i paragrafi che modifichi. In un documento lungo il conteggio mostra un segno più, ad esempio **120+ errori di correzione**, finché non sono stati controllati tutti i paragrafi. Una parola segnalata sia dal controllo ortografico sia da quello grammaticale viene contata due volte. Gli errori di grammatica vengono contati per i paragrafi già esaminati dal controllo grammaticale, quindi questo numero può aumentare mentre scorri. Il conteggio è nascosto quando il controllo ortografico e quello grammaticale sono entrambi disattivati.

## Le tue parole

Fai clic su **File > Opzioni > Correzione > Dizionario personalizzato** per vedere e modificare le tue parole. La finestra di dialogo ha due elenchi, ciascuno con una casella **Trova** per restringerlo:

- **Parole aggiunte** sono le parole che hai insegnato a JeffWrite. Sono accettate in tutte le lingue.
- **Parole escluse** sono le parole che vuoi vedere segnalate come errate anche se un dizionario le accetta, ad esempio una parola che digiti sempre in modo sbagliato. Vengono segnalate ovunque compaiano, con qualsiasi uso di maiuscole, e non vengono mai proposte come suggerimento. Una parola esclusa è esattamente quella parola, non le sue altre forme: escludere "casa" non segnala "case". Aggiungere una parola al dizionario, dal riquadro, dal menu del clic destro o dalla finestra di dialogo, la toglie dall'elenco delle escluse. **Ignora tutto** continua a funzionare su una parola esclusa fino a quando non chiudi il programma.

Digita una parola e fai clic su **Aggiungi**, oppure seleziona delle parole e fai clic su **Rimuovi**. Una parola sta in un solo elenco alla volta: aggiungerla a uno la toglie dall'altro. **Importa** prende le parole da un file di testo semplice con una parola per riga, e **Esporta** scrive l'elenco nello stesso tipo di file (UTF-8). Una riga con uno spazio o un carattere di controllo, o più lunga di 100 lettere, viene esclusa, e la finestra di dialogo ti dice quante ne ha escluse. Un elenco può arrivare a 1 MB di testo. Nulla cambia finché non fai clic su **OK**. Gli elenchi sono tuoi, sono gli stessi in ogni app di JeffOffice e non vengono mai salvati in un documento.

## Quando la correzione automatica cambia una parola

Quando la correzione automatica cambia una parola mentre digiti, sotto il cursore compare un piccolo pulsante **Opzioni correzione automatica**. Offre tre scelte. Uno screen reader annuncia che ci sono delle scelte quando il pulsante compare, e la descrizione del pulsante indica come aprirle. Puoi anche premere **Alt+Maiusc+F10** per aprirle. È il comando **Menu Opzioni correzione automatica**, quindi si trova nella pagina Tastiera delle opzioni insieme agli altri comandi, e lì puoi assegnargli un altro tasto.

- **Annulla correzione automatica** rimette la parola come l'avevi digitata. È un'unica operazione, quindi **Ripeti** applica di nuovo la modifica.
- Una scelta che interrompe la modifica. Dipende da ciò che ha cambiato la parola: **Interrompi sostituzione** toglie la voce dall'elenco delle sostituzioni, **Mantieni come digitato** aggiunge una parola con due maiuscole ("THursday") a quelle che le mantengono, e **Interrompi maiuscola** disattiva la maiuscola per i nomi dei giorni, la prima lettera delle frasi o la prima lettera delle celle di tabella in tutto il documento, non solo per questa parola, e lo indica (è lo stesso interruttore della finestra di dialogo Correzione automatica). Inoltre rimette la parola come l'avevi digitata.
- **Opzioni correzione automatica...** apre la finestra di dialogo Correzione automatica.

La scheda **Simboli matematici** della finestra di dialogo Correzione automatica contiene un interruttore, **Sostituisci i nomi dei simboli matematici mentre digiti**, disattivato finché non lo attivi. Con l'interruttore attivo, una barra rovesciata seguita da un nome, come \alpha, diventa il simbolo (α) quando digiti uno spazio o un segno dopo di esso. Il nome deve iniziare una parola, così un percorso di cartella come C:\temp\pi resta come l'hai digitato. La scheda elenca tutti i nomi. Quando un nome è stato sostituito, la scelta del pulsante è **Interrompi sostituzione nomi matematici**, che disattiva l'interruttore. JeffDeck e JeffPub li modificano anch'essi; JeffCrunch non modifica nulla in una cella.

Il pulsante scompare quando digiti, modifichi o sposti il cursore.

## Attivare o disattivare le linee rosse

Fai clic su **Revisione > Controlla ortografia durante la digitazione**. La stessa scelta si trova in **File > Opzioni > Correzione**, ed è l'unica scelta che usano tutte le app di JeffOffice. Lì puoi anche scegliere di saltare le parole in MAIUSCOLO e le parole con numeri, come 3°. Inizialmente entrambe vengono saltate.

## Controllare la grammatica mentre digiti

JeffWrite cerca anche errori di grammatica e di stile evidenti nel testo. Un problema riceve una linea blu punteggiata. L'ortografia mantiene la sua linea rossa ondulata, e una parola errata non viene mai segnalata come problema di grammatica. Questi sono gli errori che cerca:

- una parola ripetuta ("the the")
- "a" e "an" prima del suono sbagliato ("an test", "a hour")
- una frase che inizia con una lettera minuscola dopo un punto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando le parole intorno rendono chiaro l'errore
- "could of" al posto di "could have"
- due spazi di seguito all'interno di una frase
- uno spazio prima di una virgola, di un punto o di un altro segno che appartiene alla parola precedente
- nessuno spazio dopo la fine di una frase
- "alot", e "everyday" al posto di "every day" ("almost everyday")
- verbi che non concordano con il soggetto nei casi evidenti ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- la forma sbagliata di un verbo ("did went", "has went", "can't hardly", "don't have no")
- parole confuse ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" al posto di "whether", "chose" al posto di "choose", "advise" al posto di "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- una virgola dopo l'anno di una data ("On June 5, 2020, we left"), una virgola dopo una città e uno stato ("in Austin, Texas, the heat"), e un trattino in un composto prima di un sostantivo ("a well-known author")
- una "i" minuscola al posto della parola "I", e i giorni e i mesi in minuscolo

JeffWrite preferisce perdere un errore piuttosto che segnalare un testo corretto, quindi una frase senza linea blu non è una garanzia che sia corretta.

Fai clic destro sul problema. Il menu indica l'errore, poi uno o più suggerimenti. Fai clic su un suggerimento per inserirlo nel testo, e premi Ctrl+Z per annullarlo. **Ignora questo problema** rimuove la linea solo da questo punto, fino a quando non chiudi JeffWrite. Le stesse parole in un altro punto mantengono la loro linea, e modificare altrove nel paragrafo non la fa ricomparire. **Ignora tutto** rimuove la linea da ogni problema con questo messaggio su queste parole.

Il controllo viene eseguito sul tuo computer, senza connessione a internet, in background: la digitazione non viene mai rallentata, e un paragrafo viene controllato poco dopo che smetti di scrivere in esso. Un paragrafo viene controllato fino ai primi 100.000 caratteri. Un documento che richiede più controlli del limite smette di essere controllato, e la barra di stato lo segnala una volta; il controllo ortografico continua a funzionare.

**Quali lingue vengono controllate.** La grammatica viene controllata solo in inglese, in tutte le varianti regionali (americano, britannico, canadese, australiano e le altre). Il testo in qualsiasi altra lingua viene controllato solo per l'ortografia e non riceve segnalazioni grammaticali. Un paragrafo viene controllato nella lingua con cui è contrassegnato, e il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue viene controllato solo l'inglese, e il resto viene saltato. Anche il testo che un file contrassegna come "non controllare" viene saltato, così come le parole eliminate con le revisioni, il testo nascosto e i risultati dei campi (come i numeri di pagina). La grammatica viene controllata nel testo del documento, ma non ancora nelle tabelle, nelle intestazioni, nei piè di pagina e nelle note.

Per disattivarlo, fai clic su **Revisione > Controlla grammatica durante la digitazione**. La stessa scelta si trova in **File > Opzioni > Correzione**. È un'unica scelta per tutte le app di JeffOffice, ed è la stessa in ogni finestra. Quando è attivo, il riquadro Ortografia (F7) segnala i problemi di grammatica dopo quelli di ortografia.

**Scegliere cosa controllare.** Fai clic su **File > Opzioni > Correzione > Impostazioni grammaticali** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma sbagliata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizioni** e **Stile**. **Stile** è disattivato finché non lo attivi. Non segnala errori veri e propri, ma suggerimenti: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni ridondanti ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** nelle Opzioni. Si applica subito alle linee blu e al riquadro Ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al loro prossimo avvio.

## Lingue

JeffWrite controlla ogni porzione di testo nella propria lingua. Un file può indicare la lingua del suo testo, fino a una singola parola, e JeffWrite la legge. Il testo che non indica nulla viene controllato nella lingua del documento. Il file fornisce quella lingua per tutto il documento, e un nuovo documento usa l'inglese americano.

Il testo in una lingua per cui JeffWrite non ha un dizionario non viene controllato. Non viene controllato neppure il testo che un file contrassegna "non controllare l'ortografia". Al momento non è possibile cambiare la lingua del testo in JeffWrite, ma viene mantenuta quella indicata dal file.

I 31 dizionari sono: inglese (americano, britannico, canadese e australiano), spagnolo (21 regioni, incluse Messico e Spagna), francese, tedesco, italiano, olandese e portoghese (Brasile e Portogallo).

## Cosa non viene ancora controllato

Le linee rosse compaiono nel testo, nelle celle delle tabelle e in un'intestazione, un piè di pagina o una nota mentre la modifichi. Non compaiono ancora nelle caselle di testo, ma il riquadro Ortografia le attraversa.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
