# Cambiare il modello di una pubblicazione
<!-- keywords: cambia modello, applica modello, altra struttura, nuova struttura, nuovo aspetto, cambiare grafica, sostituire modello, contenuto aggiuntivo, testo rimasto, immagini rimaste, segnaposto, ruoli, creare una nuova pubblicazione, spostare testo e immagini -->

**Cambia modello** assegna alla pubblicazione aperta l'aspetto di una struttura diversa e vi sposta il testo e le immagini. Non è necessario digitare nuovamente nulla. L'intera operazione è un unico passaggio che è possibile annullare.

## Cambiare il modello

1. Fare clic su **Progettazione pagina > Modello > Cambia modello**. La visualizzazione File si apre su una raccolta di strutture, intitolata **Cambia modello**.
2. Scegliere una struttura come per una nuova pubblicazione: selezionare un gruppo a sinistra oppure digitare nella casella **Cerca modelli**, quindi fare clic su una struttura. Vedere [Iniziare una nuova pubblicazione](create).
3. Se si desidera, usare il pannello a destra. **Combinazione colori** e **Combinazione tipi di carattere** sono impostate inizialmente su **(mantieni corrente)**, che mantiene i colori e i tipi di carattere attuali. Scegliere una combinazione solo se si vuole che la nuova struttura li modifichi. **Informazioni aziendali** seleziona il set di recapiti con cui la struttura viene compilata. **Includi logo** e **Includi indirizzo postale** sono visualizzate solo per le strutture che li utilizzano.
4. Fare clic su **Cambia modello**. In alternativa, fare doppio clic sulla struttura.
5. Scegliere dove applicare la nuova struttura, quindi fare clic su **OK**:
   - **Applica il modello alla pubblicazione corrente** modifica la pubblicazione aperta.
   - **Crea una nuova pubblicazione con testo e immagini personali** apre una seconda finestra con il risultato. La pubblicazione di partenza non viene modificata.

La raccolta mostra solo strutture. Per modificare le dimensioni del foglio di una pubblicazione senza cambiarne la struttura, usare [Imposta pagina](page-setup).

## Dove vengono collocati testo e immagini

Un **articolo** è il testo contenuto in una casella di testo o in una catena di caselle di testo collegate. Ogni casella di testo di una struttura ha un **ruolo**, che ne indica la funzione. I ruoli sono:

- Titolo (il titolo principale) e sottotitolo
- Intestazione, per il testo in corpo grande nelle parti successive della pubblicazione
- Corpo del testo
- Data, che contiene anche un luogo o un orario
- Indirizzo e recapiti: indirizzo stradale, telefono, posta elettronica e indirizzo Web
- Organizzazione, ovvero il nome e lo slogan dell'organizzazione
- Persona, ovvero il nome e la qualifica di una persona
- Didascalia e citazione in evidenza
- Indirizzo postale, il blocco compilato dalla stampa unione
- Testo breve, per piccole etichette

Le strutture fornite con JeffPub includono già questi ruoli. Per una casella disegnata dall'utente, o proveniente da un file .pub, JeffPub determina il ruolo in base al testo (una data, un numero di telefono), allo stile (Didascalia o Citazione) e alle dimensioni.

Ogni articolo viene inserito nella casella della nuova struttura con lo stesso ruolo. Ad esempio, l'articolo scritto nel corpo di un volantino viene inserito nella casella del corpo di una newsletter e il nome dell'organizzazione nella testata. Se la nuova struttura contiene più caselle con lo stesso ruolo, gli articoli le riempiono in ordine di lettura: pagina per pagina, dall'alto in basso, da sinistra a destra.

L'articolo mantiene le parole, il grassetto, il corsivo, i collegamenti e i campi. Assume invece il carattere, il colore e la spaziatura della nuova struttura, in modo da armonizzarsi con il resto. Il titolo di una struttura che usa il testo artistico riceve il primo paragrafo dell'articolo del titolo. I paragrafi successivi vengono spostati in Contenuto aggiuntivo.

Le immagini vengono inserite nei **segnaposto immagine** della struttura, ovvero le immagini provvisorie visualizzate finché non si aggiungono le proprie. La prima immagine occupa il primo segnaposto in ordine di lettura, la successiva il segnaposto successivo e così via. Ogni immagine viene ritagliata per riempire la cornice e mantiene luminosità, contrasto e ricolorazione. I segnaposto per cui non è disponibile un'immagine mantengono l'immagine provvisoria della struttura.

Alcuni elementi rimangono invariati:

- Il testo mai sovrascritto è il testo di esempio della struttura precedente. Non viene trasferito e la nuova struttura mostra al suo posto il proprio testo di esempio.
- Le dimensioni della pagina, i margini, le pagine e le pagine master provengono dalla nuova struttura. Se la pubblicazione ha più pagine della struttura, vengono mantenute le pagine della struttura e il resto del contenuto viene spostato in Contenuto aggiuntivo. JeffPub non aggiunge pagine per accoglierlo.
- Tabelle, forme, linee, testo artistico e gruppi di oggetti creati dalla struttura precedente e non modificati rimangono con essa. Tutto ciò che è stato disegnato o modificato dall'utente (una tabella con una cella compilata, una forma con il testo modificato, una linea spostata) viene spostato per intero in Contenuto aggiuntivo.
- Informazioni aziendali, proprietà del documento, stili, elenco della stampa unione e oggetti nell'area di lavoro rimangono nella pubblicazione.

## Contenuto aggiuntivo

Nessun elemento creato dall'utente viene eliminato. Il contenuto che non trova posto nella nuova struttura resta in attesa nel riquadro **Contenuto aggiuntivo**, che si apre accanto alla pagina al termine della modifica. Il riquadro elenca ogni articolo, immagine, tabella, forma, linea e gruppo rimasto.

- Fare clic su un elemento, quindi su **Posiziona nella pagina** per inserirlo nella pagina visualizzata. Un articolo diventa una casella di testo adattata al testo, una tabella o un gruppo viene ripristinato per intero e un'immagine o un oggetto troppo grande viene ridotto per adattarlo alla pagina. In alternativa, fare doppio clic su un elemento o trascinarlo nella pagina.
- Fare clic su un elemento, quindi su **Elimina** per rimuoverlo definitivamente. (È comunque possibile annullare l'operazione.)

Il riquadro resta associato alla pubblicazione finché questa non viene chiusa e viene salvato nel file della pubblicazione. (Un file .pub non può contenerlo, quindi salvando in formato .pub il riquadro non viene incluso.) Per riaprirlo, fare clic su **Progettazione pagina > Modello > Contenuto aggiuntivo**. Il pulsante è disattivato quando non rimane nulla.

## Annullare

**Annulla** (CTRL+Z) ripristina la struttura precedente e rimuove di nuovo gli elementi rimasti da Contenuto aggiuntivo. Il posizionamento o l'eliminazione di un elemento costituisce un passaggio a sé.

## Argomenti correlati

- [Iniziare una nuova pubblicazione](create)
- [Combinazioni colori e tipi di carattere](color-schemes)
- [Dimensioni pagina, margini e layout](page-setup)
- [La scheda Progettazione pagina](tab-page-design)
