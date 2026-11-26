# Controllo ortografia
<!-- keywords: ortografia, controllo ortografico, correttore ortografico, parola errata, sottolineatura rossa, linea ondulata, linea rossa ondulata, suggerimenti, ignora, ignora tutto, aggiungi al dizionario, dizionario personalizzato, dizionario, lingua, correzione, F7, revisione, verifica ortografia durante la digitazione, maiuscole, numeri, grammatica, controllo grammaticale, stile, linea blu, linea puntinata, verifica grammatica durante la digitazione -->

JeffWrite controlla l'ortografia mentre scrivi. Una parola che non è nel dizionario riceve una sottolineatura rossa ondulata. Il controllo viene eseguito sul tuo computer, senza connessione a internet, e conosce 31 lingue e varianti regionali.

## Mentre scrivi

- Una sottolineatura rossa ondulata indica che la parola non è presente nel dizionario. La parola potrebbe comunque essere corretta. Nomi e parole nuove spesso mancano dal dizionario.
- I nomi delle app di JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC non vengono mai segnalati, in nessuna lingua, con o senza la "s" finale.
- La parola che stai scrivendo non viene segnalata finché non ti sposti altrove, così una parola ancora in corso non lampeggia.
- Le linee compaiono solo sullo schermo. Non vengono mai stampate e non finiscono in un PDF.

Per correggere una parola, fai **clic destro** su di essa. Il menu mostra queste scelte, sopra quelle abituali:

- Fino a sei **suggerimenti**, il migliore per primo, in grassetto. Fai clic su uno di essi per sostituire la parola. È un solo passaggio, quindi **Ctrl+Z** lo annulla.
- **Ignora tutto** lascia invariata quella parola, ovunque compaia, finché non chiudi JeffWrite.
- **Aggiungi al dizionario** insegna a JeffWrite la parola in modo permanente. La parola viene salvata in un piccolo file nella tua cartella di JeffOffice, e tutte le app di JeffOffice la usano. Per vedere o modificare il tuo elenco di parole, vedi Le tue parole, più avanti.

Se il dizionario non ha alcun suggerimento, il menu lo indica.

## Scorrere gli errori di ortografia

Premi **F7**, oppure fai clic su **Revisione > Ortografia**. Il riquadro Ortografia si apre a destra. Mostra la prima parola errata dopo il cursore, selezionata nel testo, con la riga in cui si trova e un elenco di suggerimenti.

- **Cambia** sostituisce la parola con il suggerimento scelto. Fai doppio clic su un suggerimento per ottenere lo stesso risultato.
- **Cambia tutto** lo fa per ogni occorrenza della parola nei punti che il riquadro attraversa, elencati qui sotto. È un solo passaggio, quindi **Ctrl+Z** annulla tutte le modifiche, ovunque fossero. Al termine, il riquadro indica quante parole ha cambiato e in quanti punti. Il testo, ogni casella di testo, ogni nota e ogni intestazione o piè di pagina contano come un punto ciascuno.
- **Ignora** salta questa parola. La sua sottolineatura rossa resta.
- **Ignora tutto** e **Aggiungi al dizionario** funzionano come nel menu del clic destro.

Il riquadro inizia dalla posizione del cursore: nel testo, oppure nella casella di testo, nella nota a piè di pagina, nell'intestazione o nel piè di pagina che stai modificando. Scorre prima il testo del documento, tabelle comprese, poi le caselle di testo, poi le note a piè di pagina, infine le intestazioni e i piè di pagina. Quando una parola si trova in una casella di testo, in una nota a piè di pagina, in un'intestazione o in un piè di pagina, JeffWrite lo apre per la modifica, come fa Trova, così vedi la parola nel punto in cui si trova. Un'intestazione o un piè di pagina che non compare in nessuna pagina viene saltato, e **Cambia tutto** non lo modifica. Dal cursore il riquadro arriva alla fine di quel punto, prosegue poi con gli altri nell'ordine indicato, e ricomincia dall'inizio fino al punto da cui era partito. **Cambia** è un solo passaggio annullabile, e **Cambia tutto** cerca in ogni punto attraversato dal riquadro ed è anch'esso un solo passaggio. Una parola in una nota di chiusura può essere ignorata o aggiunta al dizionario, ma non cambiata. Quando non ne restano altre, il riquadro indica che il controllo ortografico è completato. Al termine dell'ortografia passa agli errori di grammatica e stile (vedi Controllo grammaticale, più avanti).

## Contare gli errori nella barra di stato

La barra di stato mostra quanti errori di ortografia e grammatica ha il documento, ad esempio **3 errori di correzione**. Fai clic su di essa per selezionare l'errore successivo dopo il cursore. Quando raggiungi l'ultimo, un clic riporta al primo.

JeffWrite conta mentre lavori, un po' alla volta, e ricontrolla solo i paragrafi che modifichi. In un documento lungo il conteggio mostra un segno più, ad esempio **120+ errori di correzione**, finché ogni paragrafo non è stato controllato. Una parola segnalata sia dal controllo ortografico sia da quello grammaticale viene contata due volte. Gli errori di grammatica sono contati per i paragrafi già esaminati dal controllo grammaticale, quindi questo numero può aumentare mentre scorri. Il conteggio è nascosto quando il controllo ortografico e quello grammaticale sono entrambi disattivati.

## Le tue parole

Fai clic su **File > Opzioni > Correzione > Dizionario personalizzato** per vedere e modificare le tue parole. La finestra di dialogo ha due elenchi, ciascuno con una casella **Trova** per restringerlo:

- **Parole aggiunte** sono le parole che hai insegnato a JeffWrite. Vengono accettate in tutte le lingue.
- **Parole escluse** sono parole che vuoi vengano segnalate come errate, anche se un dizionario le accetta, come una parola che digiti sempre sbagliata. Sono segnalate ovunque compaiano, con qualsiasi maiuscola, e non vengono mai proposte come suggerimento. Una parola esclusa è esattamente quella parola, non le sue altre forme: escludere "casa" non segnala "case". Aggiungere una parola al dizionario, dal riquadro, dal menu del clic destro o dalla finestra di dialogo, la toglie dall'elenco delle esclusioni. **Ignora tutto** funziona ancora su una parola esclusa finché non chiudi il programma.

Digita una parola e fai clic su **Aggiungi**, oppure seleziona delle parole e fai clic su **Rimuovi**. Una parola sta in un solo elenco alla volta: aggiungerla a uno la toglie dall'altro. **Importa** prende le parole da un file di testo semplice con una parola per riga, e **Esporta** scrive l'elenco nello stesso tipo di file (UTF-8). Una riga che contiene uno spazio o un carattere di controllo, o che è più lunga di 100 lettere, viene esclusa, e la finestra di dialogo ti dice quante ne sono state escluse. Un elenco può arrivare a 1 MB di testo. Nulla cambia finché non fai clic su **OK**. Gli elenchi sono tuoi, sono gli stessi in tutte le app di JeffOffice e non vengono mai salvati in un documento.

## Quando la correzione automatica cambia una parola

Quando la **Correzione automatica** cambia una parola mentre scrivi, sotto il cursore compare un piccolo pulsante **Opzioni correzione automatica**. Offre tre scelte. Uno screen reader annuncia che ci sono delle scelte quando il pulsante compare, e la descrizione del pulsante indica come aprirle. Puoi anche premere **Alt+Maiusc+F10** per aprirle. È il comando **Menu Opzioni correzione automatica**, quindi si trova nella pagina Tastiera delle opzioni insieme agli altri comandi, e lì puoi assegnarlo a un altro tasto.

- **Annulla correzione automatica** ripristina la parola come l'hai digitata. È un solo passaggio, quindi **Ripeti** applica di nuovo la modifica.
- Una scelta che blocca la modifica. Dipende da cosa ha cambiato la parola: **Interrompi sostituzione** toglie la voce dal tuo elenco di sostituzioni, **Mantieni come digitato** aggiunge una parola con due maiuscole ("THursday") a quelle che mantengono le maiuscole, e **Interrompi maiuscola iniziale** disattiva in tutto il documento la maiuscola per i nomi dei giorni, la prima lettera delle frasi o la prima lettera delle celle di tabella, non solo per questa parola, e lo segnala (è lo stesso interruttore della finestra di dialogo Correzione automatica). Inoltre ripristina la parola come l'hai digitata.
- **Opzioni correzione automatica...** apre la finestra di dialogo Correzione automatica.

Nella scheda **Simboli matematici** della finestra di dialogo Correzione automatica c'è un interruttore, **Sostituisci i nomi dei simboli matematici mentre digiti**, disattivato finché non lo attivi. Con l'opzione attiva, una barra rovesciata seguita da un nome, come \alpha, diventa il simbolo (α) quando digiti uno spazio o un segno di punteggiatura dopo di esso. Il nome deve iniziare una parola, quindi un percorso di cartella come C:\temp\pi resta come l'hai digitato. La scheda elenca tutti i nomi. Quando un nome è stato sostituito, la scelta del pulsante è **Interrompi sostituzione nomi matematici**, che disattiva l'opzione. JeffDeck e JeffPub modificano anch'essi questi nomi; JeffCrunch non modifica nulla in una cella.

Il pulsante scompare quando digiti, modifichi o sposti il cursore.

## Attivare o disattivare le sottolineature rosse

Fai clic su **Revisione > Verifica ortografia durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Correzione**, ed è l'unica scelta che usano tutte le app di JeffOffice. Lì puoi anche scegliere di saltare le parole in MAIUSCOLO e le parole con numeri, come 3a. Entrambe sono saltate per impostazione predefinita.

## Verificare la grammatica mentre scrivi

JeffWrite cerca anche errori di grammatica e di stile chiari nel testo. Un problema riceve una sottolineatura blu puntinata. L'ortografia mantiene la sottolineatura rossa ondulata, e una parola errata non viene mai segnalata come problema di grammatica. Questi sono gli errori che cerca:

- una parola ripetuta ("the the")
- "a" e "an" davanti al suono sbagliato ("an test", "a hour")
- una frase che inizia con una lettera minuscola dopo un punto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando le parole intorno rendono chiaro l'errore
- "could of" al posto di "could have"
- due spazi di seguito all'interno di una frase
- uno spazio prima di una virgola, di un punto o di un altro segno che appartiene alla parola precedente
- nessuno spazio dopo la fine di una frase
- "alot", e "everyday" al posto di "every day" ("almost everyday")
- verbi che non concordano con il soggetto nei casi chiari ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- la forma sbagliata di un verbo ("did went", "has went", "can't hardly", "don't have no")
- parole confuse ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" al posto di "whether", "chose" al posto di "choose", "advise" al posto di "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- una virgola dopo l'anno di una data ("On June 5, 2020, we left"), una virgola dopo città e stato ("in Austin, Texas, the heat"), e un trattino in un composto prima di un sostantivo ("a well-known author")
- una "i" minuscola al posto della "I", e i giorni e i mesi scritti in minuscolo

Preferisce perdere un errore piuttosto che segnalare un testo corretto, quindi una frase senza sottolineatura blu non è una garanzia che sia corretta.

Fai clic destro sul problema. Il menu mostra che cosa non va, poi uno o più suggerimenti. Fai clic su un suggerimento per inserirlo nel testo, e premi Ctrl+Z per annullarlo. **Ignora questo problema** rimuove la sottolineatura solo da questo punto, finché non chiudi JeffWrite. Le stesse parole in un altro punto mantengono la sottolineatura, e modificare altrove nel paragrafo non la fa ricomparire. **Ignora tutto** rimuove la sottolineatura da ogni problema con questo messaggio su queste parole.

Il controllo viene eseguito sul tuo computer, senza connessione a internet, in background: la digitazione non viene mai rallentata, e un paragrafo viene controllato poco dopo che smetti di scrivere in esso. Un paragrafo viene controllato fino ai suoi primi 100.000 caratteri. Un documento che richiede più controlli del limite smette di essere controllato, e la barra di stato te lo segnala una volta; l'ortografia continua a funzionare.

**Lingue controllate.** Inglese, francese, tedesco e spagnolo, ciascuno con regole proprie. Il francese controlla lo spazio senza interruzione prima di ; : ! ? e all'interno di « », l'apostrofo in parole come "l'ami" e "qu'il", e gli scambi come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole sui sostantivi comuni. Lo spagnolo cerca un ¿ o ¡ mancante, gli accenti sulle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, e il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata e le altre vengono saltate. Il testo che un file contrassegna come "non controllare" viene saltato. La grammatica viene controllata nel testo del documento, ma non ancora in tabelle, intestazioni, piè di pagina o note.

Per disattivarlo, fai clic su **Revisione > Verifica grammatica durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Correzione**. È un'unica scelta per tutte le app di JeffOffice, ed è la stessa in ogni finestra. Quando è attiva, il riquadro Ortografia (F7) passa agli errori di grammatica dopo quelli di ortografia.

**Scegliere che cosa cercare.** Fai clic su **File > Opzioni > Correzione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma sbagliata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizioni** e **Stile**. **Stile** è disattivato finché non lo attivi. Non segnala errori, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni ridondanti ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** nelle Opzioni. Si applica subito alle sottolineature blu e al riquadro Ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al loro prossimo avvio.

## Lingue

JeffWrite controlla ogni porzione di testo nella propria lingua. Un file può indicare in che lingua è il suo testo, fino a una singola parola, e JeffWrite lo legge. Il testo che non indica nulla viene controllato nella lingua del documento. Il file indica questa lingua per l'intero documento, e un nuovo documento usa l'inglese americano.

Il testo in una lingua per cui JeffWrite non ha un dizionario non viene controllato. Non viene controllato neanche il testo che un file contrassegna come "non controllare ortografia". Al momento non puoi cambiare la lingua di un testo in JeffWrite, ma viene mantenuta quella indicata dal file.

I 31 dizionari sono: inglese (americano, britannico, canadese e australiano), spagnolo (21 regioni, tra cui Messico e Spagna), francese, tedesco, italiano, olandese e portoghese (Brasile e Portogallo).

## Cosa non viene ancora controllato

Le linee rosse compaiono nel testo, nelle celle delle tabelle e in un'intestazione, un piè di pagina o una nota mentre la modifichi. Non compaiono ancora nelle caselle di testo, ma il riquadro Ortografia le controlla.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
