# Creare moduli con controlli del contenuto
<!-- keywords: controllo del contenuto, controlli del contenuto, controllo di contenuto, modulo, moduli, compilare, testo normale, elenco a discesa, menu a discesa, selezione data, calendario, casella di controllo, titolo, tag, segnaposto, blocco, non può essere eliminato, proprietà, campo, campo di testo, maschera -->

Un **controllo del contenuto** è una casella nel testo che una persona compila: una riga di testo, una scelta da un elenco, una data dal calendario o una casella di controllo. Usalo per creare un modulo che altri possono compilare senza modificare il resto del documento.

## Inserire un controllo

1. Posiziona il punto di inserimento dove deve comparire il controllo.
2. Scegli **Inserisci > Controllo di contenuto**, quindi seleziona il tipo:
   - **Testo normale:** una casella per le parole. Mostra il segnaposto "Inserisci testo" finché non viene digitato qualcosa. Se sono selezionate parole, il controllo viene creato attorno a esse.
   - **Elenco a discesa:** un elenco di scelte. Mostra "Scegli un elemento" finché non ne viene selezionato uno.
   - **Selezione data:** una data scelta da un calendario. Mostra "Scegli una data" finché non ne viene selezionata una.
   - **Casella di controllo:** una piccola casella che può essere selezionata o deselezionata.

Un controllo ha uno sfondo grigio chiaro, così puoi vedere dove inizia e dove finisce. Quando il punto di inserimento si trova al suo interno, il controllo ha un contorno e il suo titolo compare in una scheda sopra di esso. Inserire un controllo è un'operazione annullabile con un solo passaggio.

## Compilare un controllo

- **Testo normale:** fai clic sul segnaposto e digita. Il testo digitato prende il posto del segnaposto. Premi MAIUSC+Invio per andare a capo; Invio non crea un nuovo paragrafo all'interno di un controllo.
- **Elenco a discesa:** fai clic sul controllo (oppure premi Invio o ALT+Freccia giù con il punto di inserimento al suo interno) e fai clic su una voce. La voce scelta viene contrassegnata la volta successiva.
- **Selezione data:** fai clic sul controllo, quindi fai clic su un giorno nel calendario. **Oggi** inserisce la data odierna. **Cancella** ripristina il segnaposto.
- **Casella di controllo:** fai clic sulla casella, oppure premi la BARRA SPAZIATRICE con il punto di inserimento al suo interno, per selezionarla o deselezionarla.

Un elenco a discesa, una selezione data e una casella di controllo contengono solo ciò che viene scelto per essi: non puoi digitarvi testo.

## Modificare le proprietà di un controllo

Posiziona il punto di inserimento nel controllo e scegli **Inserisci > Controllo di contenuto > Proprietà**. Puoi impostare:

- **Titolo** e **Tag:** il titolo compare nella scheda sopra il controllo. Il tag è un nome breve che altri programmi possono usare per trovare il controllo.
- **Voci dell'elenco** (elenco a discesa): Aggiungi, rinomina (fai doppio clic su una voce), Rimuovi, Sposta su e Sposta giù. Un elenco può contenere fino a 1.000 voci.
- **Formato data** (selezione data): scegli un formato oppure digitane uno personalizzato, ad esempio MMMM g, aaaa. La casella mostra come appare la data odierna in quel formato.
- **Il controllo non può essere eliminato:** il controllo resta, ma il suo contenuto può essere modificato.
- **Il contenuto non può essere modificato:** il controllo può essere eliminato, ma il suo contenuto non può essere modificato.

Scegli **OK** per applicare le modifiche come un'unica operazione annullabile.

## Utile da sapere

- Un documento può contenere fino a 10.000 controlli, e i controlli possono essere annidati l'uno nell'altro fino a 8 livelli di profondità.
- I controlli presenti in file di altri programmi vengono visualizzati e mantenuti così come sono. Un tipo che JeffWrite non è in grado di creare (ad esempio testo RTF, un'immagine, una casella combinata o una sezione ripetuta) viene mantenuto nella sua posizione: è possibile modificarne il titolo, il tag e i blocchi, ma non l'elenco dei contenuti.
- I controlli vengono salvati nei file `.docx`. In altri tipi di file il testo rimane, ma il controllo non viene conservato.
