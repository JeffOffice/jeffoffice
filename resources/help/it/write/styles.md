# Usare gli stili

<!-- keywords: stile elenco, salvare uno stile elenco, stile elenco multilivello, stili elenco, gestire stili, stile bloccato, bloccare uno stile, limitare la formattazione, limitare formattazione, esportare stili, file modello, dotx, nascondere uno stile, stili consigliati, stili, stile, riquadro stili, titolo, normale, creare stile, nuovo stile, modificare stile, cambiare stile, basato su, stile successivo, aggiorna automaticamente, raccolta, stile carattere, stile paragrafo, ctrl+alt+maiusc+s -->

Uno **stile** è un insieme salvato di aspetti: un carattere, una dimensione, la spaziatura e altro. Quando assegni uno stile a un paragrafo, il paragrafo assume tutti i suoi aspetti in una sola volta. Quando modifichi lo stile, cambiano anche tutti i paragrafi che lo usano. Per questo gli stili mantengono ordinati i documenti lunghi.

JeffWrite ha due tipi di stili:

- Uno **stile paragrafo** imposta l'aspetto di interi paragrafi. **Titolo 1** e **Normale** sono stili paragrafo.
- Uno **stile carattere** imposta l'aspetto delle parole selezionate. **Forte** è uno stile carattere.

## Assegnare uno stile al testo

1. Fai clic in un paragrafo, oppure seleziona le parole da modificare.
2. Fai clic su uno stile nella raccolta **Stili** nella scheda **Home**. Oppure premi **Ctrl+Maiusc+N** per Normale, oppure **Ctrl+Alt+1**, **Ctrl+Alt+2** o **Ctrl+Alt+3** per Titolo 1, 2 o 3.

Uno stile paragrafo viene applicato a ogni paragrafo che tocchi. Uno stile carattere viene applicato alle parole selezionate.

## Aprire il riquadro Stili

Fai clic sulla piccola freccia nell'angolo del gruppo **Stili**, oppure premi **Ctrl+Alt+Maiusc+S**. Il riquadro **Stili** si apre a destra e mostra gli stili del documento, ciascuno con il proprio aspetto.

- Lo stile sotto il cursore è in grassetto, con una freccia prima del nome.
- Fai clic su uno stile per assegnarlo alla selezione.
- Usa l'elenco sotto gli stili per scegliere quali mostrare:
  - **In uso** mostra solo gli stili usati dal testo.
  - **Tutti gli stili** mostra ogni stile, anche quelli che il documento non ha ancora. Uno stile viene aggiunto al documento la prima volta che lo usi.
  - **Consigliati** mostra gli stili pensati per essere visti. Uno stile nascosto finché non viene usato compare qui dopo il primo utilizzo.
- Fai clic su **Gestisci…** sotto l'elenco per scegliere, per ogni stile, se è **Consigliato** (mostrato nella raccolta), se è **Nascosto** (escluso da tutti gli elenchi) e il suo **Ordine negli elenchi**. Un numero più basso compare per primo. Fai clic su **OK**: tutte le modifiche avvengono in un solo passaggio, quindi un solo **Annulla** le ripristina.
- In **Gestisci…**, fai clic su **Elimina** per rimuovere uno stile creato da te. I paragrafi che lo usavano tornano a Normale, e gli stili basati su di esso passano a quello su cui era basato. Gli stili forniti con JeffWrite non possono essere eliminati. L'eliminazione è un solo passaggio, quindi **Annulla** ripristina lo stile.
- In **Gestisci…**, fai clic su **Importa da file…** per copiare stili da un altro documento. Scegli il file, seleziona gli stili che ti servono e fai clic su **OK**. Uno stile che ne richiede un altro porta con sé anche quello. Gli stili già presenti restano invariati, a meno che tu non selezioni **Sostituisci gli stili con lo stesso nome**. L'importazione si annulla con un solo passaggio.
- In **Gestisci…**, fai clic su **Esporta in file…** per salvare alcuni stili in un file modello (un file **.dotx**). Seleziona gli stili che ti servono, fai clic su **OK** e scegli dove salvarlo. Un altro documento può prenderli da quel file con **Importa da file…**. Uno stile che ne richiede un altro porta con sé anche quello.
- Seleziona **Bloccato** per bloccare uno stile. Poi seleziona **Limita la formattazione agli stili non bloccati** per attivare il limite. Mentre il limite è attivo, nessuno può formattare il testo a mano, ad esempio con **Grassetto** o **Centra**, e nessuno può usare uno stile bloccato. Possono comunque usare tutti gli altri stili. La barra di stato lo indica quando un comando non è disponibile. Deseleziona la casella per disattivare il limite. Se il documento è protetto con una password, interrompi prima la protezione, in **Revisione > Limita modifiche**. Il limite e i blocchi vengono salvati nel file.
- Sotto l'elenco, la casella **Ordina** dispone gli stili in ordine. **Alfabetico** li ordina per nome. **Come consigliato** mette per primi gli stili più utili. **Per tipo** elenca prima gli stili paragrafo, poi quelli collegati, poi gli stili carattere. **Basato su** raggruppa ogni stile con quello su cui è basato.

## Creare uno stile dal testo

1. Formatta un paragrafo come desideri. Per esempio, rendilo in grassetto, centralo e imposta la dimensione.
2. Fai clic su **Nuovo stile** in fondo al riquadro, oppure fai clic su **Crea stile** nella scheda **Home**.
3. Digita un **Nome**. Un nome può contenere fino a 255 caratteri e non può essere il nome di un altro stile.
4. Controlla le altre opzioni e fai clic su **OK**.

Il **Tipo di stile** può essere **Paragrafo**, **Carattere** o **Collegato**. Uno stile collegato è uno stile paragrafo accompagnato da uno stile carattere con lo stesso aspetto (chiamato come lo stile paragrafo, con "Car" aggiunto), così l'aspetto può essere assegnato a un intero paragrafo oppure solo ad alcune parole. Quando in seguito modifichi lo stile paragrafo, anche il suo stile carattere assume il nuovo aspetto.

Uno stile **Tabella** gestisce i bordi, la riga di intestazione e le righe a bande di una tabella, mentre uno stile **Elenco** gestisce i numeri o i punti elenco di un elenco. Per questi due tipi, digita il **Nome** e fai clic su **OK**: la finestra di quel tipo si apre con il nome già inserito. Un elenco creato in questo modo viene sempre salvato come stile elenco. Gli stili tabella o elenco non vengono mantenuti quando salvi il documento come file .odt (stili tabella) o come file .rtf (entrambi); **Verifica compatibilità** (File > Informazioni) ti avvisa.

**Disponibile in** permette di scegliere **Solo in questo documento** oppure **Nuovi documenti basati su questo modello**. Con la seconda scelta lo stile viene salvato anche nel modello da cui partono i nuovi documenti.

Il nuovo stile viene salvato con il documento, e il paragrafo lo assume. Un solo **Annulla** (Ctrl+Z) elimina lo stile.

## Modificare uno stile

1. Fai clic in un paragrafo che ha lo stile. Oppure fai clic sullo stile nel riquadro.
2. Fai clic su **Modifica** in fondo al riquadro, oppure fai clic su **Modifica stile** nella scheda **Home**.
3. Cambia ciò che ti serve e fai clic su **OK**.

La finestra di dialogo offre queste opzioni:

- **Nome** è il nome dello stile. Uno stile fornito con JeffWrite mantiene il proprio nome.
- **Basato su** è lo stile da cui questo parte. Il nuovo stile ha l'aspetto di quello, tranne per ciò che modifichi. Se in seguito cambi lo stile di base, anche questo si aggiorna.
- **Stile per il paragrafo successivo** è lo stile che riceve il paragrafo seguente quando premi **Invio** alla fine di questo. Di solito a un titolo segue Normale.
- **Carattere**, **dimensione**, **Grassetto**, **Corsivo**, **Sottolineato** e **colore** impostano l'aspetto delle parole.
- **Allineamento**, **rientro**, **spaziatura** e **interlinea** impostano l'aspetto dei paragrafi.
- **Aggiungi alla raccolta stili** inserisce lo stile nella raccolta della scheda **Home**.
- **Aggiorna automaticamente** fa seguire allo stile la tua formattazione. Quando modifichi un paragrafo che ha questo stile, lo stile cambia di conseguenza, e cambiano anche tutti gli altri paragrafi che lo usano.

Quando modifichi uno stile, cambiano i paragrafi che lo usano. Se a un paragrafo hai dato un aspetto proprio, per esempio una parola in rosso, quell'aspetto resta.

Se fai clic su **OK** senza aver cambiato nulla, il documento non viene modificato.

## Vedere da dove deriva l'aspetto del testo

Fai clic su **Ispettore stili** in fondo al riquadro Stili. Il riquadro segue il cursore, nel testo, in un'intestazione, in un piè di pagina o in una nota, e mostra tre elementi:

- Lo **Stile paragrafo** del paragrafo. Se il paragrafo si trova in una tabella, sotto il nome compare lo stile della tabella. Sotto il nome, "Aggiunto" elenca ciò che il paragrafo imposta da solo oltre al proprio stile, come allineamento, rientri, spaziatura, interlinea, tabulazioni, bordi, sfondo, interruzione di pagina prima, livello struttura, numerazione o punti elenco e direzione del testo. **Cancella formattazione paragrafo** rimuove questi elementi, così il paragrafo appare come indica il suo stile. Ciò che lo stile di una tabella assegna ai suoi paragrafi non viene elencato, perché non è un'impostazione propria del paragrafo.
- Lo **Stile carattere** del testo, se ne ha uno. **Cancella stile carattere** lo rimuove.
- La **Formattazione del testo** che le lettere hanno di per sé, come carattere, dimensione, grassetto, colore, testo nascosto o bordo. **Cancella formattazione carattere** la rimuove, così le lettere appaiono come indicano i loro stili. La lingua del testo e il suo stile carattere restano invariati.

Seleziona prima il testo per cancellare solo quello. Senza testo selezionato, il riquadro descrive la lettera prima del cursore, e **Cancella stile carattere** e **Cancella formattazione carattere** agiscono su tutta la sequenza di lettere intorno al cursore che hanno lo stesso aspetto di quella lettera. Se selezioni più paragrafi o lettere diverse, il riquadro indica "Più stili nella selezione" oppure "Più aspetti nella selezione", elenca ciò che la selezione contiene, e i pulsanti agiscono su tutto.

Ogni pulsante è un solo passaggio: **Annulla** (Ctrl+Z) ripristina l'aspetto, e il cursore resta dov'era. Cancellare uno stile non fa mai sì che i suoi paragrafi abbiano un aspetto proprio, quindi un paragrafo che segue il suo stile non mostra nulla sotto "Aggiunto". Maiusc+F1 apre comunque Mostra formattazione, che visualizza l'aspetto vero e proprio.

## Aggiornare uno stile in base al testo

Formatta un paragrafo come desideri. Poi fai clic su **Aggiorna stile in base alla selezione** nella scheda **Home**. Lo stile di quel paragrafo assume le modifiche che hai fatto, e ogni paragrafo con quello stile le segue.

## Salvare un elenco come stile elenco

Uno **stile elenco** è un elenco numerato o puntato con un nome. Conserva tutti i nove livelli: il numero o il punto di ogni livello, il suo testo, il punto di partenza e la posizione del testo.

1. Fai clic su **Elenco multilivello** nella scheda **Home**, poi su **Definisci nuovo elenco multilivello…**.
2. Imposta i livelli come preferisci.
3. Seleziona **Salva come stile elenco** e digita un nome. Se il nome è vuoto o già usato, la finestra lo segnala e rimane aperta.
4. Fai clic su **OK**. I paragrafi selezionati assumono lo stile elenco.

Lo stile elenco ora compare in due posti: sotto **Stili elenco** nella raccolta **Elenco multilivello** (i primi 100) e nel riquadro **Stili** (tutti). Seleziona dei paragrafi e fai clic sul suo nome per assegnarglielo.

- **Modificarlo.** Fai clic su **Elenco multilivello**, **Modifica stile elenco** e sul nome, oppure scegli lo stile elenco nel riquadro **Stili** e fai clic su **Modifica…**. Ogni paragrafo che ha lo stile elenco cambia di conseguenza, nel testo e nelle intestazioni, nei piè di pagina e nelle note. Nel testo è un solo passaggio, quindi un solo **Annulla** lo ripristina.
- **Eliminarlo.** Fai clic su **Elenco multilivello**, **Elimina stile elenco** e sul nome. I paragrafi mantengono i numeri e i punti elenco esattamente come appaiono, ma non seguono più uno stile.
- **Salvarlo in un file.** Uno stile elenco viene conservato in un file .docx come stile di numerazione, e in un file .odt come stile elenco. Viene ripristinato come stile elenco quando riapri il file. Un file può contenere fino a 1.000 stili elenco; gli altri vengono usati come elenchi semplici.

Un livello può ancora essere collegato a uno stile paragrafo, come in qualsiasi elenco multilivello. Lo stile paragrafo viene quindi numerato dallo stile elenco, e **Modifica stile elenco** mostra il collegamento così puoi modificarlo.

Se nella selezione non c'è un elenco a cui assegnare lo stile (per esempio perché il testo selezionato è bloccato), lo stile elenco non viene salvato e un messaggio lo indica.

## Argomenti correlati

- [Cambiare l'aspetto del testo](font)
- [Copiare un aspetto con Copia formato](format-painter)
- [Usare JeffWrite con la tastiera](keyboard)
