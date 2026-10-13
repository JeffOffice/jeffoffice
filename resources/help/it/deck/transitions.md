# Transizioni tra le diapositive
<!-- keywords: transizione, transizioni, morphing, dissolvenza, spinta, apparizione, divisione, rivela, copri, scopri, zoom, taglio, opzioni effetto, oggetti, parole, caratteri, corrispondenza, coppia, nome, direzione, durata, avanzamento diapositiva, al clic del mouse, dopo, automatico, intervallo, applica a tutte, anteprima, stella -->

Una **transizione** è l'effetto riprodotto quando la presentazione passa da una diapositiva alla successiva. Una transizione appartiene alla diapositiva in cui entra. La presentazione la riproduce quando avanza su quella diapositiva.

## Aggiungere una transizione

1. Fare clic sulla diapositiva nel riquadro delle diapositive.
2. Fare clic sulla scheda **Transizioni**. Nella raccolta **Transizione a questa diapositiva** fare clic su un effetto: **Nessuna**, **Morphing**, **Dissolvenza**, **Spinta**, **Apparizione**, **Divisione**, **Rivela**, **Copri**, **Scopri**, **Zoom** o **Taglia**. Nel riquadro delle diapositive, l'anteprima della diapositiva riceve una piccola stella per indicare che ha una transizione.
3. Fare clic su **Transizioni > Anteprima > Anteprima** per vederla riprodotta in entrata nella diapositiva, sopra l'area della diapositiva. Premere ESC per interromperla.

Scegliere **Nessuna** per rimuovere una transizione.

## Scegliere come si svolge

Fare clic su **Opzioni effetto** per visualizzare le scelte dell'effetto:

- **Spinta**, **Apparizione**, **Rivela**, **Copri** e **Scopri** hanno una direzione: **Da destra**, **Da sinistra**, **Dal basso** o **Dall'alto**.
- **Divisione** e **Zoom** procedono **Verso l'esterno** o **Verso l'interno**.
- **Morphing** sposta **Oggetti**, **Parole** o **Caratteri**. Vedere più avanti.
- **Dissolvenza** e **Taglia** non hanno opzioni. Un taglio passa attraverso il nero.

## Morphing

Il **Morphing** sposta ciò che due diapositive hanno in comune. Un **oggetto** è un singolo elemento di una diapositiva: una forma, un'immagine, una casella di testo, una linea o una tabella. La presentazione passa da una diapositiva alla successiva. Ogni oggetto presente in entrambe le diapositive scorre dalla posizione che ha nella prima diapositiva a quella che ha nella seconda. Durante il percorso si ingrandisce o si riduce, ruota e cambia colore. Un oggetto presente solo nella prima diapositiva scompare in dissolvenza. Un oggetto presente solo nella seconda diapositiva compare in dissolvenza.

Per creare un morphing:

1. Fare clic con il pulsante destro del mouse su una diapositiva nel riquadro delle diapositive e scegliere **Duplica diapositiva**. La copia contiene gli stessi oggetti.
2. Nella copia, spostare un oggetto, ingrandirlo, ruotarlo o cambiarne il colore. È anche possibile modificarne il testo oppure aggiungere ed eliminare oggetti.
3. Fare clic sulla copia, quindi su **Transizioni > Transizione a questa diapositiva > Morphing**. Un morphing dura 2 secondi. È possibile modificare questo valore con **Durata**.
4. Fare clic su **Anteprima** per guardarlo oppure avviare la presentazione.

### Quali oggetti vanno insieme

JeffDeck abbina un oggetto della prima diapositiva a un oggetto della seconda. Prova questi criteri, in questo ordine. Ogni oggetto può far parte al massimo di una coppia.

1. **Un nome che inizia con !!.** Due oggetti con lo stesso nome, che inizia con due punti esclamativi, formano sempre una coppia. Possono essere due oggetti qualsiasi, anche un cerchio e una stella.
2. **Lo stesso nome.** Due oggetti dello stesso tipo con lo stesso nome formano una coppia. Una casella di testo e una forma sono di tipo diverso.
3. **Lo stesso testo.** Due caselle di testo con le stesse parole formano una coppia, indipendentemente dal nome. Lo stesso vale per due forme con le stesse parole.
4. **La stessa immagine.** Due immagini della stessa immagine formano una coppia, anche se hanno un nome diverso o dimensioni diverse.
5. **Lo stesso tipo di forma.** Due cerchi formano una coppia. Lo stesso vale per due linee o due tabelle.

Quando più oggetti potrebbero formare una coppia, vengono abbinati per primi quelli più vicini tra loro.

I nomi degli oggetti provengono dal file della presentazione. Una presentazione creata altrove mantiene i nomi assegnati ai propri oggetti e JeffDeck li utilizza.

### Come si sposta una coppia

- Due oggetti con la stessa forma diventano un unico oggetto che si sposta. Posizione, dimensioni, rotazione, colore di riempimento, colore della linea e trasparenza cambiano gradualmente. Cambia anche il ritaglio di un'immagine.
- Due oggetti con forme diverse, ad esempio un cerchio e un quadrato, si spostano nelle stesse posizioni mentre uno si dissolve nell'altro.
- Se il testo di una coppia non è uguale, il vecchio testo si dissolve nel nuovo.
- Gli oggetti nascosti da un'animazione vengono esclusi. Per un oggetto spostato da un'animazione si considera la posizione in cui si trova alla fine.

### Cosa si sposta: Oggetti, Parole o Caratteri

Fare clic su **Opzioni effetto** per scegliere:

- **Oggetti.** Il testo di una coppia rimane nel suo oggetto. Il vecchio testo si dissolve nel nuovo.
- **Parole.** Una parola presente in entrambi i testi si sposta dalla vecchia alla nuova posizione. Le altre parole scompaiono e compaiono in dissolvenza. Un segno come la virgola conta come una parola a sé.
- **Caratteri.** Come sopra, una lettera o una cifra alla volta.

**Parole** e **Caratteri** funzionano con caselle di testo e forme. Un testo più lungo di 16.000 caratteri, un testo disposto in verticale e il testo in una forma capovolta vengono riprodotti come **Oggetti**. Lo stesso vale per una coppia che non ha parole o caratteri in comune.

Il morphing è una transizione disponibile anche in altri programmi di presentazione (versioni 2019 e successive). JeffDeck lo legge e lo salva nello stesso modo. I programmi meno recenti riproducono il morphing come una dissolvenza.

## Impostare l'intervallo

Il gruppo **Intervallo** imposta la durata e il modo in cui la presentazione lascia la diapositiva.

- **Durata** indica quanti secondi dura l'effetto. Una nuova transizione dura 0,7 secondi. È possibile digitare un valore da 0,01 a 59.
- **Al clic del mouse** è attivato per impostazione predefinita. La presentazione lascia questa diapositiva quando il relatore fa clic. Disattivarlo per avanzare invece con un tasto come BARRA SPAZIATRICE o FRECCIA DESTRA. Le animazioni della diapositiva vengono comunque riprodotte al clic.
- **Dopo** fa avanzare la presentazione automaticamente dopo il numero di secondi digitato. È possibile usare **Dopo** insieme ad **Al clic del mouse**: la presentazione avanza alla prima delle due condizioni che si verifica.

**Applica a tutte** assegna a tutte le diapositive la transizione e l'intervallo della diapositiva corrente.

Ognuna di queste operazioni è un singolo passaggio che CTRL+Z annulla.

## Transizioni da altri file

Una presentazione creata altrove può contenere una transizione per cui JeffDeck non ha un effetto. Viene riprodotta come dissolvenza e salvata così com'era, quindi funziona ancora nel programma di origine.

## Argomenti correlati

- [Scheda Transizioni](tab-transitions)
- [Animazioni](animations)
- [Riprodurre la presentazione e usare la Visualizzazione Relatore](show)
