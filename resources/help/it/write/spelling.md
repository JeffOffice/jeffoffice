# Verifica ortografia

<!-- keywords: ortografia, controllo ortografia, correzione ortografica, parole errate, errore ortografico, linea rossa, linea ondulata, sottolineatura ondulata, suggerimenti, ignora, ignora tutto, aggiungi al dizionario, dizionario personalizzato, dizionario, lingua, revisione, verifica ortografia durante la digitazione, maiuscole, numeri, grammatica, controllo grammaticale, stile, linea blu, linea tratteggiata, verifica grammatica durante la digitazione -->

JeffWrite controlla l'ortografia mentre scrivi. Una parola che non è nel dizionario viene sottolineata con una linea rossa ondulata. Il controllo viene eseguito sul tuo computer, senza connessione a internet, e conosce 31 lingue e varianti regionali.

## Mentre scrivi

- Una linea rossa ondulata indica che il dizionario non contiene la parola. La parola potrebbe comunque essere corretta. Nomi e termini nuovi spesso mancano nel dizionario. I nomi delle app di JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e LLC non vengono mai segnalati, in nessuna lingua, con o senza la "s" finale.
- La parola che stai scrivendo non viene controllata finché non passi oltre, così una parola a metà non lampeggia.
- Le linee compaiono solo sullo schermo. Non vengono mai stampate e non finiscono in un PDF.

Per correggere una parola, fai **clic con il pulsante destro** su di essa. Il menu mostra queste scelte sopra quelle usuali:

- Fino a sei **suggerimenti**, il migliore per primo, in grassetto. Fai clic su uno di essi per sostituire la parola. È un'unica operazione, quindi **Ctrl+Z** la annulla.
- **Ignora tutto** lascia invariata quella parola, ovunque compaia, fino alla chiusura di JeffWrite.
- **Aggiungi al dizionario** insegna a JeffWrite la parola in modo permanente. La parola viene salvata in un piccolo file nella tua cartella di JeffOffice, e tutte le app di JeffOffice la usano. Per vedere o modificare l'elenco delle tue parole, consulta Le tue parole, più avanti.

Se il dizionario non ha alcun suggerimento, il menu lo indica.

## Scorrere gli errori di ortografia

Premi **F7**, oppure fai clic su **Revisione > Ortografia**. Il riquadro Ortografia si apre a destra. Mostra la prima parola errata dopo il punto di inserimento, selezionata nel testo, con la riga in cui si trova e un elenco di suggerimenti.

- **Cambia** sostituisce la parola con il suggerimento scelto. Fai doppio clic su un suggerimento per ottenere lo stesso risultato.
- **Cambia tutto** fa la stessa cosa per ogni copia della parola nei punti che il riquadro attraversa, elencati più sotto. È un'unica operazione, quindi **Ctrl+Z** annulla tutte le sostituzioni, ovunque si trovino. Al termine, il riquadro indica quante parole ha cambiato e in quanti punti. Il testo, ogni casella di testo, ogni nota e ogni intestazione o piè di pagina contano come un punto ciascuno.
- **Ignora** salta questa parola. La sua linea rossa resta.
- **Ignora tutto** e **Aggiungi al dizionario** funzionano come nel menu del pulsante destro.

Il riquadro inizia dal punto di inserimento: nel testo, oppure nella casella di testo, nella nota a piè di pagina, nell'intestazione o nel piè di pagina che stai modificando. Scorre prima il testo del documento, tabelle comprese, poi le caselle di testo, poi le note a piè di pagina, infine le intestazioni e i piè di pagina. Quando una parola si trova in una casella di testo, in una nota, in un'intestazione o in un piè di pagina, JeffWrite la apre per la modifica, come fa Trova, così vedi la parola nel punto in cui si trova. Un'intestazione o un piè di pagina che nessuna pagina mostra viene saltato/a, e anche **Cambia tutto** lo lascia com'è. Dal punto di inserimento il riquadro va fino alla fine di quel punto, poi prosegue con gli altri nell'ordine indicato, quindi ricomincia dall'inizio fino al punto da cui era partito. **Cambia** è un'unica operazione annullabile, e **Cambia tutto** cerca in tutti i punti attraversati dal riquadro ed è anch'esso un'unica operazione. Una parola in una nota di chiusura può essere ignorata o aggiunta, ma non modificata. Quando non ne restano altre, il riquadro indica che il controllo ortografico è completato. Una volta terminata l'ortografia, passa agli errori di grammatica e di stile (vedi Verifica grammatica, più sotto).

## Contare gli errori nella barra di stato

La barra di stato mostra quanti errori di ortografia e grammatica contiene il documento, ad esempio **3 errori di revisione**. Fai clic per selezionare l'errore successivo dopo il punto di inserimento. Quando arrivi all'ultimo, un clic ti riporta al primo.

JeffWrite conta gli errori mentre lavori, un po' alla volta, e ricontrolla solo i paragrafi che modifichi. In un documento lungo il conteggio mostra un segno più, ad esempio **120+ errori di revisione**, finché non sono stati controllati tutti i paragrafi. Una parola segnalata sia dal controllo ortografico sia da quello grammaticale viene contata due volte. Gli errori di grammatica vengono contati per i paragrafi già esaminati dal controllo grammaticale, quindi questo numero può aumentare man mano che scorri. Il conteggio è nascosto quando il controllo ortografico e quello grammaticale sono entrambi disattivati.

## Le tue parole

Fai clic su **File > Opzioni > Revisione > Dizionario personalizzato** per vedere e modificare le tue parole. La finestra di dialogo ha due elenchi, ciascuno con una casella **Trova** per restringerlo:

- **Parole aggiunte** sono le parole che hai insegnato a JeffWrite. Vengono accettate in tutte le lingue.
- **Parole escluse** sono le parole che vuoi vedere segnalate come errate anche se un dizionario le accetta, ad esempio una parola che sbagli sempre. Vengono segnalate ovunque compaiano, con qualunque uso di maiuscole, e non vengono mai proposte come suggerimento. Una parola esclusa è solo quella esatta, non le sue altre forme: escludere "casa" non segnala "case". Aggiungere una parola al dizionario, dal riquadro, dal menu del pulsante destro o dalla finestra di dialogo, la toglie dall'elenco delle esclusioni. **Ignora tutto** funziona ancora su una parola esclusa fino alla chiusura del programma.

Digita una parola e fai clic su **Aggiungi**, oppure seleziona alcune parole e fai clic su **Rimuovi**. Una parola può stare in un solo elenco alla volta: aggiungerla a uno la toglie dall'altro. **Importa** prende le parole da un file di testo semplice con una parola per riga, e **Esporta** scrive l'elenco nello stesso tipo di file (UTF-8). Una riga che contiene uno spazio o un carattere di controllo, o che è più lunga di 100 lettere, viene esclusa, e la finestra di dialogo ti dice quante ne sono state escluse. Un elenco può arrivare a 1 MB di testo. Nulla cambia finché non fai clic su **OK**. Gli elenchi sono tuoi, sono uguali in tutte le app di JeffOffice e non vengono mai salvati in un documento.

## Quando la correzione automatica cambia una parola

Quando la Correzione automatica cambia una parola mentre scrivi, sotto il punto di inserimento compare un piccolo pulsante **Opzioni di Correzione automatica**. Offre tre scelte. Uno screen reader annuncia che ci sono delle scelte quando il pulsante compare, e la descrizione del pulsante indica come aprirle. Puoi anche premere **Alt+Maiusc+F10** per aprirle. È il comando **Menu Opzioni correzione automatica**, quindi si trova nella pagina Tastiera delle opzioni insieme agli altri comandi, e lì puoi assegnargli un altro tasto.

- **Annulla correzione automatica** ripristina la parola come l'avevi scritta. È un'unica operazione, quindi **Ripeti** applica di nuovo la modifica.
- Una scelta che blocca la modifica. Dipende da ciò che ha cambiato la parola: **Interrompi sostituzione** toglie la voce dal tuo elenco di sostituzioni, **Mantieni come digitato** aggiunge una parola con due maiuscole ("THursday") a quelle che le mantengono, e **Interrompi maiuscole** disattiva la maiuscola per i nomi dei giorni, la prima lettera delle frasi o la prima lettera delle celle di tabella in tutto il documento, non solo per questa parola, e lo indica (è lo stesso interruttore della finestra di dialogo Correzione automatica). Inoltre ripristina la parola come l'avevi digitata.
- **Opzioni di Correzione automatica...** apre la finestra di dialogo Correzione automatica.

Il pulsante scompare quando scrivi, modifichi o sposti il punto di inserimento.

## Attivare o disattivare le linee rosse

Fai clic su **Revisione > Verifica ortografia durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Revisione**, ed è l'unica scelta che tutte le app di JeffOffice condividono. Lì puoi anche scegliere di saltare le parole in MAIUSCOLO e quelle con numeri, come 3°. Entrambe sono saltate all'inizio.

## Verifica grammatica durante la digitazione

JeffWrite cerca anche chiari errori di grammatica e di stile nel testo. Un problema viene sottolineato con una linea blu tratteggiata. L'ortografia mantiene la sua linea rossa ondulata, e una parola errata non viene mai segnalata come problema di grammatica. Ecco gli errori che cerca:

- una parola ripetuta ("the the")
- "a" e "an" davanti al suono sbagliato ("an test", "a hour")
- una frase che inizia con una lettera minuscola dopo un punto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", "lose" e "loose", quando le parole intorno rendono chiaro l'errore
- "could of" al posto di "could have"
- due spazi di seguito all'interno di una frase
- uno spazio prima di una virgola, di un punto o di un altro segno che appartiene alla parola precedente
- nessuno spazio dopo la fine di una frase
- "alot", e "everyday" al posto di "every day" ("almost everyday")
- verbi che non concordano con il soggetto nei casi chiari ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- la forma sbagliata di un verbo ("did went", "has went", "can't hardly", "don't have no")
- parole confuse ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" al posto di "whether", "chose" al posto di "choose", "advise" al posto di "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- una virgola dopo l'anno di una data ("On June 5, 2020, we left"), una virgola dopo città e stato ("in Austin, Texas, the heat") e un trattino in un composto davanti a un nome ("a well-known author")
- una "i" minuscola al posto di "I", e i giorni e i mesi in minuscolo

Preferisce perdere un errore piuttosto che segnalare un testo corretto, quindi una frase senza linea blu non è una garanzia che sia corretta.

Fai **clic con il pulsante destro** sul problema. Il menu indica cosa non va, poi uno o più suggerimenti. Fai clic su un suggerimento per inserirlo nel testo, e premi Ctrl+Z per annullarlo. **Ignora questo problema** toglie la linea solo da questo punto, fino alla chiusura di JeffWrite. Le stesse parole in un altro punto mantengono la loro linea, e modificare altrove nel paragrafo non la fa ricomparire. **Ignora tutto** toglie la linea da ogni problema con questo messaggio su queste parole.

Il controllo viene eseguito sul tuo computer, senza connessione a internet, in background: la digitazione non viene mai rallentata, e un paragrafo viene controllato poco dopo che smetti di scrivere in esso. Un paragrafo viene controllato fino ai primi 100.000 caratteri. Un documento che richiede più controllo del limite smette di essere controllato, e la barra di stato te lo segnala una volta; l'ortografia continua a funzionare.

**Lingue controllate.** Inglese, francese, tedesco e spagnolo, ciascuna con regole proprie. Il francese controlla lo spazio senza interruzione prima di ; : ! ? e all'interno di « », l'apostrofo in parole come "l'ami" e "qu'il", e le confusioni come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole nei sostantivi comuni. Lo spagnolo cerca un ¿ o un ¡ mancanti, gli accenti nelle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, e il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata e le altre vengono saltate. Il testo che un file contrassegna come "non controllare" viene saltato. La grammatica viene controllata nel testo del documento, ma non ancora nelle tabelle, nelle intestazioni, nei piè di pagina o nelle note.

Per disattivarla, fai clic su **Revisione > Verifica grammatica durante la digitazione**. La stessa opzione si trova in **File > Opzioni > Revisione**. È un'unica scelta per tutte le app di JeffOffice, e vale in ogni finestra. Quando è attiva, il riquadro Ortografia (F7) scorre i problemi di grammatica dopo quelli di ortografia.

**Scegliere cosa controllare.** Fai clic su **File > Opzioni > Revisione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma sbagliata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizioni** e **Stile**. **Stile** è disattivato finché non lo attivi. Suggerisce, e non riguarda errori: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni prolisse ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** nelle Opzioni. Si applica subito alle linee blu e al riquadro Ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al loro prossimo avvio.

## Lingue

JeffWrite controlla ogni porzione di testo nella propria lingua. Un file può indicare la lingua del testo fino a una singola parola, e JeffWrite la legge. Il testo che non indica nulla viene controllato nella lingua del documento. Il file fornisce quella lingua per l'intero documento, e un nuovo documento usa l'inglese americano.

Il testo in una lingua per cui JeffWrite non ha un dizionario non viene controllato. Non viene controllato neppure il testo che un file contrassegna come "non controllare l'ortografia". Non puoi ancora cambiare la lingua del testo in JeffWrite, ma il programma mantiene ciò che indica il file.

I 31 dizionari sono: inglese (americano, britannico, canadese e australiano), spagnolo (21 regioni, tra cui Messico e Spagna), francese, tedesco, italiano, olandese e portoghese (Brasile e Portogallo).

## Cosa non è ancora controllato

Le linee rosse compaiono nel testo, nelle celle delle tabelle, e in un'intestazione, in un piè di pagina o in una nota mentre la modifichi. Non compaiono ancora nelle caselle di testo, ma il riquadro Ortografia le attraversa.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
