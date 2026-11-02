# Formati numerici e lingua
<!-- keywords: formato numero, formati numerici, numeri, decimali, separatore decimale, virgola decimale, posizioni decimali, migliaia, separatore delle migliaia, raggruppamento cifre, virgola, punto, lingua, impostazioni internazionali, valuta, euro, percentuale, data, ora, scientifico, notazione scientifica, frazione, testo, contabilità, generale, personalizzato, codice formato, esempio, Formato celle, Ctrl+1, Ctrl+Maiusc+4, Ctrl+Maiusc+5, aumenta decimali, diminuisci decimali, stile separatore, separatore formule, punto e virgola -->

Una cella contiene un numero e il suo **formato numerico** stabilisce come il numero viene scritto sullo schermo. Il formato cambia ciò che vedi, mai ciò che la cella contiene. Esempio: la cella contiene 0,256. Con il formato Percentuale mostra **25,6%**, con Valuta mostra **0,26 €** e con Generale mostra **0,256**. Se fai clic sulla cella, la barra della formula indica sempre 0,256.

## I numeri vengono scritti come li scrive il computer

JeffCrunch usa i segni della lingua impostata nel computer. In inglese (Stati Uniti), milleduecentotrentaquattro e mezzo si scrive **1,234.5**. In tedesco si scrive **1.234,5** e in francese **1 234,5**. La cartella di lavoro non cambia: conserva un'unica forma neutra, quindi lo stesso file mostra 1,234.5 a una persona e 1.234,5 a un'altra.

Questo vale ovunque un numero venga visualizzato o digitato:

- Una cella, la copia di una cella e la larghezza che **Adatta larghezza colonne** assegna a una colonna seguono i segni della tua lingua.
- La **barra della formula** e una cella che modifichi con **F2** riportano il numero come lo digiti. Se la tua lingua scrive i decimali con la virgola, digita **2,5**, non 2.5.
- In una lingua che scrive i decimali con la virgola, gli argomenti di una formula sono separati da **punti e virgola**: **=SUM(A1;A2)**. I nomi delle funzioni restano in inglese.
- Date e ore seguono il codice del formato: un formato che indica "mese, giorno, anno" le mostra in quest'ordine, mentre un formato pensato per la tua lingua (quelli predefiniti dell'elenco) usa l'ordine e i nomi della tua lingua.

Il testo che il formato racchiude tra virgolette, ad esempio "unità", viene mostrato così com'è.

## Scegliere un formato

1. Seleziona le celle.
2. Usa il gruppo **Numeri** della scheda **Home** oppure premi **Ctrl+1** (su Mac, **Comando+1**) per aprire **Formato celle**, che si apre sulla scheda **Numeri**.

La casella del gruppo **Numeri** elenca i formati più comuni: Generale, Numero, Valuta, Contabilità, Data breve, Data estesa, Ora, Percentuale, Frazione, Scientifico, Testo e Personalizzato. **Altri formati numero…** in fondo all'elenco apre la finestra Formato celle. I pulsanti sotto la casella sono:

| Pulsante | Funzione |
| --- | --- |
| Valuta | Mostra il numero come importo in denaro, con il simbolo di valuta della tua lingua (**Ctrl+Maiusc+4**). La freccia accanto offre Generale e **Altri formati numero…**. |
| Stile percentuale | Moltiplica per 100 e aggiunge il simbolo di percentuale, senza decimali (**Ctrl+Maiusc+5**). Fai di nuovo clic per rimuoverlo. |
| Stile separatore | Raggruppa le cifre per migliaia, mostra due decimali e allinea i separatori decimali lungo la colonna, senza simbolo. |
| Aumenta decimali, Diminuisci decimali | Aggiunge o toglie una posizione decimale. |
| Generale (nella casella e nella freccia accanto a Valuta) | Rimuove il formato: il numero viene mostrato così com'è (**Ctrl+Maiusc+~**). |

## La scheda Numeri di Formato celle

Scegli una **Categoria** a sinistra; le relative opzioni compaiono a destra e l'**Esempio** mostra l'aspetto della prima cella selezionata, nella tua lingua.

- **Numero**: posizioni decimali, raggruppamento delle cifre per migliaia e aspetto dei numeri negativi (segno meno, rosso o tra parentesi).
- **Valuta** e **Contabilità**: posizioni decimali e simbolo. Contabilità allinea i simboli e i separatori decimali lungo la colonna.
- **Data** e **Ora**: un elenco di formati predefiniti, con l'ordine e i nomi della tua lingua.
- **Percentuale**, **Scientifico**: posizioni decimali.
- **Frazione**: formati predefiniti come "Fino a una cifra" (1/4), "Fino a due cifre" (21/25), "In quarti" e "In sedicesimi".
- **Speciale**: formati predefiniti per codice postale, codice postale + 4, numero di telefono e numero di previdenza sociale.
- **Testo**: la cella conserva ciò che digiti come testo, quindi un numero come 00123 mantiene gli zeri.
- **Personalizzato**: digita un codice personalizzato oppure parti da uno dell'elenco (i codici più comuni e quelli già usati nella cartella di lavoro).

Il **codice** è il breve testo che indica a JeffCrunch come scrivere un numero. Esempio: **#,##0.00** significa "raggruppa le cifre per migliaia e mostra sempre due decimali", quindi 1234,5 viene mostrato come 1.234,50. Ogni **0** è una cifra sempre visualizzata, ogni **#** una cifra visualizzata solo se necessaria e **%** moltiplica per 100. Un codice può avere fino a quattro sezioni separate da punti e virgola, per numeri positivi, numeri negativi, zero e testo.

## Cosa viene conservato

Il codice viene salvato nella cartella di lavoro nella sua forma neutra, quindi la cartella di lavoro viene letta correttamente su qualsiasi computer. La formattazione è un'unica operazione che puoi annullare e una cartella di lavoro che usava già un formato lo conserva esattamente: un formato presente nell'elenco viene ritrovato e nessuno viene creato due volte.
