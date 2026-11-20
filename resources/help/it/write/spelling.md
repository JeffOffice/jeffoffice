# Verifica ortografia
<!-- keywords: ortografia, controllo ortografia, correttore ortografico, parola errata, linea rossa, sottolineatura ondulata, suggerimenti, ignora, ignora tutto, aggiungi al dizionario, dizionario personalizzato, dizionario, lingua, correzione di bozze, F7, revisione, controlla ortografia durante la digitazione, maiuscole, numeri, grammatica, controllo grammatica, stile, linea blu, linea puntinata, controlla grammatica durante la digitazione -->

JeffWrite controlla l'ortografia mentre digiti. Una parola che non è nel dizionario viene sottolineata con una linea rossa ondulata. Il controllo viene eseguito sul computer, senza connessione a internet, e conosce 31 lingue e varianti regionali.

## Mentre digiti

- Una linea rossa ondulata indica che la parola non è nel dizionario. La parola potrebbe essere comunque corretta. Nomi e termini nuovi spesso mancano. I nomi delle app di JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC non vengono mai segnalati, in nessuna lingua, con o senza la "s" finale.
- La parola che stai digitando non viene controllata finché non passi oltre, così una parola non ancora completa non lampeggia.
- Le linee compaiono solo sullo schermo. Non vengono mai stampate e non finiscono in un PDF.

Per correggere una parola, fai **clic destro** su di essa. Il menu mostra queste scelte sopra le solite:

- Fino a sei **suggerimenti**, il migliore per primo, in grassetto. Fai clic su uno per sostituire la parola. È un'unica operazione, quindi **Ctrl+Z** la annulla.
- **Ignora tutto** lascia invariata quella parola, ovunque compaia, finché non chiudi JeffWrite.
- **Aggiungi al dizionario** insegna in modo permanente a JeffWrite la parola. La parola viene salvata in un piccolo file nella cartella di JeffOffice dell'utente, e tutte le app di JeffOffice la usano. Per vedere o modificare l'elenco delle parole, vedi Le tue parole più avanti.

Se il dizionario non ha alcun suggerimento, il menu lo indica.

## Scorrere gli errori di ortografia

Premi **F7** oppure fai clic su **Revisione > Ortografia**. Sulla destra si apre il riquadro Controllo ortografia. Mostra la prima parola errata dopo il cursore, selezionata nel testo, con la riga in cui si trova e un elenco di suggerimenti.

- **Cambia** sostituisce la parola con il suggerimento scelto. Fai doppio clic su un suggerimento per fare lo stesso.
- **Cambia tutto** lo fa per ogni copia della parola nei punti che il riquadro attraversa, elencati sotto. È un'unica operazione, quindi **Ctrl+Z** annulla tutte le modifiche, ovunque si trovino. Al termine, il riquadro indica quante parole sono state cambiate e in quanti punti. Il testo, ciascuna casella di testo, ciascuna nota e ciascuna intestazione o piè di pagina contano come un punto ciascuno.
- **Ignora** salta questa parola. La sua linea rossa resta.
- **Ignora tutto** e **Aggiungi al dizionario** funzionano come nel menu del clic destro.

Il riquadro inizia dalla posizione del cursore: nel testo, oppure nella casella di testo, nella nota, nell'intestazione o nel piè di pagina che stai modificando. Scorre prima il testo del documento, tabelle comprese, poi le caselle di testo, poi le note a piè di pagina, infine le intestazioni e i piè di pagina. Quando una parola si trova in una casella di testo, in una nota, in un'intestazione o in un piè di pagina, JeffWrite la apre per la modifica, come fa Trova, così vedi la parola dove si trova. Un'intestazione o un piè di pagina che non compare in nessuna pagina viene saltata, e **Cambia tutto** non la modifica. Dal cursore il riquadro arriva alla fine di quel punto, poi prosegue con gli altri nell'ordine indicato, quindi ricomincia dall'inizio fino al punto da cui era partito. **Cambia** è un'unica operazione annullabile, e anche **Cambia tutto** lo è: cerca in tutti i punti attraversati dal riquadro. Una parola in una nota di chiusura può essere ignorata o aggiunta al dizionario, ma non modificata. Quando non restano altre parole, il riquadro indica che il controllo ortografico è completato. Quando l'ortografia è terminata, passa ai problemi di grammatica e stile (vedi Controlla la grammatica, più avanti).

## Le tue parole

Fai clic su **File > Opzioni > Correzione di bozze > Dizionario personalizzato** per vedere e modificare le tue parole. La finestra ha due elenchi, ciascuno con una casella **Trova** per restringerlo:

- **Parole aggiunte** sono le parole che hai insegnato a JeffWrite. Sono accettate in tutte le lingue.
- **Parole escluse** sono parole che vuoi segnalare come errate anche se un dizionario le accetta, ad esempio una parola che sbagli sempre. Vengono segnalate ovunque compaiano, con qualsiasi uso di maiuscole, e non vengono mai proposte come suggerimento. Una parola esclusa vale solo per quella forma esatta, non per le altre: escludere "casa" non segnala "case". Aggiungere una parola al dizionario, dal riquadro, dal menu del clic destro o dalla finestra, la toglie dall'elenco delle parole escluse. **Ignora tutto** funziona ancora su una parola esclusa fino alla chiusura del programma.

Digita una parola e fai clic su **Aggiungi**, oppure seleziona delle parole e fai clic su **Rimuovi**. Una parola può stare in un solo elenco alla volta: aggiungerla a uno la toglie dall'altro. **Importa** prende le parole da un file di testo semplice con una parola per riga, e **Esporta** scrive l'elenco nello stesso tipo di file (UTF-8). Una riga con uno spazio o un carattere di controllo, oppure più lunga di 100 lettere, viene esclusa, e la finestra indica quante ne sono state escluse. Un elenco può arrivare a 1 MB di testo. Nulla cambia finché non fai clic su **OK**. Gli elenchi sono tuoi, sono gli stessi in tutte le app di JeffOffice e non vengono mai salvati in un documento.

## Quando la correzione automatica cambia una parola

Quando la Correzione automatica cambia una parola mentre digiti, sotto il cursore compare un piccolo pulsante **Opzioni correzione automatica**. Offre tre scelte. Un lettore di schermo annuncia che ci sono delle scelte quando il pulsante compare, e la descrizione del pulsante indica come aprirle. Puoi anche premere **Alt+Maiusc+F10** per aprirle. È il comando **Menu Opzioni correzione automatica**, quindi si trova nella pagina Tastiera delle opzioni insieme agli altri comandi, e lì puoi assegnargli un altro tasto.

- **Annulla correzione automatica** ripristina la parola come l'avevi digitata. È un'unica operazione, quindi **Ripeti** applica di nuovo la modifica.
- Una scelta che blocca la modifica. Dipende da ciò che ha cambiato la parola: **Interrompi sostituzione** toglie la voce dal tuo elenco di sostituzioni, **Mantieni come digitato** aggiunge una parola con due maiuscole ("THursday") alle parole che le conservano, e **Interrompi maiuscole** disattiva la maiuscola per i nomi dei giorni, la prima lettera delle frasi o la prima lettera delle celle di tabella in tutto il documento, non solo per questa parola, e lo indica. È lo stesso interruttore della finestra Correzione automatica. Inoltre rimette la parola come l'avevi digitata.
- **Opzioni correzione automatica...** apre la finestra Correzione automatica.

Il pulsante scompare quando digiti, modifichi o sposti il cursore.

## Attivare o disattivare le linee rosse

Fai clic su **Revisione > Controlla ortografia durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Correzione di bozze**, ed è l'unica scelta condivisa da tutte le app di JeffOffice. Lì puoi anche scegliere di saltare le parole in MAIUSCOLO e le parole con numeri, come 3°. Entrambe sono saltate all'inizio.

## Controlla la grammatica durante la digitazione

JeffWrite cerca anche errori di grammatica e di stile chiari nel testo. Un problema riceve una linea blu puntinata. L'ortografia mantiene la sua linea rossa ondulata, e una parola errata non viene mai segnalata come problema di grammatica. Ecco gli errori che cerca:

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
- una virgola dopo l'anno di una data ("On June 5, 2020, we left"), una virgola dopo una città e uno stato ("in Austin, Texas, the heat"), e un trattino in un composto prima di un sostantivo ("a well-known author")
- una "i" minuscola al posto di "I", e i giorni e i mesi scritti in minuscolo

JeffWrite preferisce non segnalare un errore piuttosto che segnalare un testo corretto, quindi una frase senza linea blu non è la garanzia che sia corretta.

Fai clic destro sul problema. Il menu mostra che cosa non va, poi uno o più suggerimenti. Fai clic su un suggerimento per inserirlo nel testo, e premi Ctrl+Z per annullarlo. **Ignora questo problema** rimuove la linea da questo solo punto, fino alla chiusura di JeffWrite. Le stesse parole in un altro punto mantengono la loro linea, e modificare altro nel paragrafo non la fa riapparire. **Ignora tutto** rimuove la linea da ogni problema con questo messaggio su queste parole.

Il controllo viene eseguito sul computer, senza connessione a internet, in secondo piano: la digitazione non viene mai rallentata, e un paragrafo viene controllato poco dopo che smetti di digitarvi. Un paragrafo viene controllato fino ai primi 100.000 caratteri. Un documento che richiede più controlli del limite smette di essere controllato, e la barra di stato te lo indica una volta; l'ortografia continua a funzionare.

**Quali lingue vengono controllate.** Inglese, francese, tedesco e spagnolo, ciascuno con regole proprie. Il francese controlla lo spazio senza interruzione prima di ; : ! ? e dentro « », l'apostrofo in parole come "l'ami" e "qu'il", e le confusioni come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole dei sostantivi comuni. Lo spagnolo cerca un ¿ o un ¡ mancanti, gli accenti sulle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, e il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata, e il resto viene saltato. Il testo che un file contrassegna come "non controllare" viene saltato. La grammatica viene controllata nel testo del documento, ma non ancora nelle tabelle, nelle intestazioni, nei piè di pagina o nelle note.

Per disattivarlo, fai clic su **Revisione > Controlla grammatica durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Correzione di bozze**. È un'unica scelta per tutte le app di JeffOffice, ed è uguale in ogni finestra. Quando è attiva, il riquadro Controllo ortografia (F7) passa dopo l'ortografia anche ai problemi di grammatica.

**Scegliere che cosa controllare.** Fai clic su **File > Opzioni > Correzione di bozze > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma sbagliata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizioni** e **Stile**. **Stile** è disattivato finché non lo attivi. Non riguarda gli errori, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni prolisse ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** nelle Opzioni. Si applica subito alle linee blu e al riquadro Controllo ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al loro prossimo avvio.

## Lingue

JeffWrite controlla ogni porzione di testo nella propria lingua. Un file può indicare la lingua del testo fino a una singola parola, e JeffWrite la legge. Il testo che non indica nulla viene controllato nella lingua del documento. Il file indica quella lingua per l'intero documento, e un nuovo documento usa l'inglese americano.

Il testo in una lingua per cui JeffWrite non ha un dizionario non viene controllato. Non viene controllato nemmeno il testo che un file contrassegna "non controllare ortografia". Non è ancora possibile cambiare la lingua del testo in JeffWrite, ma il programma conserva ciò che indica il file.

I 31 dizionari sono: inglese (americano, britannico, canadese e australiano), spagnolo (21 regioni, tra cui Messico e Spagna), francese, tedesco, italiano, olandese e portoghese (Brasile e Portogallo).

## Che cosa non è ancora controllato

Le linee rosse compaiono nel testo, nelle celle delle tabelle e in un'intestazione, un piè di pagina o una nota mentre la modifichi. Non compaiono ancora nelle caselle di testo, ma il riquadro Controllo ortografia le attraversa.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
