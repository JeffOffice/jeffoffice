# Formati numero e lingua

<!-- keywords: formato numero, numeri, decimale, separatore decimale, posizioni decimali, migliaia, separatore, raggruppamento, virgola, punto, lingua, impostazioni internazionali, valuta, percentuale, data, ora, scientifico, frazione, testo, contabilità, generale, personalizzato, codice, esempio, Formato celle, Ctrl+1, Ctrl+Maiusc+4, Ctrl+Maiusc+5, aumenta decimali, riduci decimali, stile separatore, separatore di formula, punto e virgola -->

Una cella contiene un numero, e il suo **formato numero** stabilisce come quel numero viene scritto sullo schermo. Il formato cambia ciò che vedi, mai ciò che la cella contiene. Esempio: la cella contiene 0,256. Con il formato Percentuale mostra **25,6%**, con Valuta mostra **0,26 €**, e con Generale mostra **0,256**. Fai clic sulla cella e la barra della formula continua a indicare 0,256.

## I numeri sono scritti come il computer li scrive nella tua lingua

JeffCrunch usa i segni della lingua impostata sul computer. In italiano, millecentotrentaquattro e mezzo si scrive **1.234,5**. In inglese (Stati Uniti) è **1,234.5**, e in francese **1 234,5**. La cartella di lavoro non cambia: conserva una forma neutra, quindi lo stesso file mostra 1.234,5 a una persona e 1,234.5 a un'altra.

Questo vale ovunque un numero viene mostrato o digitato:

- Una cella, la copia di una cella e la larghezza che **Adatta larghezza colonne** assegna a una colonna seguono i segni della tua lingua.
- La **barra della formula** e una cella modificata con **F2** mostrano un numero come lo digiti. Se la tua lingua scrive i decimali con la virgola, digita **2,5**, non 2.5.
- In una lingua che scrive i decimali con la virgola, una formula usa il **punto e virgola** tra gli argomenti: **=SOMMA(A1;A2)**. I nomi delle funzioni restano in inglese.
- Date e orari seguono il codice del formato: un formato che dice "mese, giorno, anno" le mostra in quell'ordine, mentre un formato pensato per la tua lingua (quelli predefiniti nell'elenco) usa l'ordine e i nomi dell'italiano.

Il testo che il formato racchiude tra virgolette, come "unità", viene mostrato così com'è.

## Scegliere un formato

1. Seleziona le celle.
2. Usa il gruppo **Numero** nella scheda **Home**, oppure premi **Ctrl+1** (su Mac, **Command+1**) per aprire **Formato celle**, che si apre sulla scheda **Numero**.

La casella del gruppo **Numero** elenca i formati più comuni: Generale, Numero, Valuta, Contabilità, Data breve, Data estesa, Ora, Percentuale, Frazione, Scientifico, Testo e Personalizzato. **Altri formati numero** in fondo apre la finestra Formato celle. I pulsanti sotto la casella sono:

| Pulsante | Che cosa fa |
| --- | --- |
| Valuta | Mostra il numero come denaro, con il simbolo di valuta della tua lingua (**Ctrl+Maiusc+4**). La freccia accanto offre **Formato numero contabile** con una valuta da scegliere (dollaro statunitense, sterlina britannica, euro, yen giapponese, yuan cinese o franco svizzero), Generale e **Altri formati numero**. |
| Stile percentuale | Moltiplica per 100 e aggiunge il segno di percentuale, senza decimali (**Ctrl+Maiusc+5**). Fai clic di nuovo per eliminarlo. |
| Stile separatore | Raggruppa le cifre per migliaia, mostra due decimali e allinea i separatori decimali in una colonna, senza simbolo. |
| Aumenta decimali, Riduci decimali | Aggiunge o toglie una posizione decimale. |
| Generale (nella casella, e nella freccia accanto a Valuta) | Elimina il formato: il numero viene mostrato così com'è (**Ctrl+Maiusc+~**). |

## La scheda Numero di Formato celle

Scegli una **Categoria** a sinistra; a destra compaiono le opzioni relative, e **Esempio** mostra come apparirà la prima cella selezionata, nella tua lingua.

- **Numero**: posizioni decimali, raggruppamento delle cifre per migliaia e aspetto dei numeri negativi (segno meno, rosso o tra parentesi).
- **Valuta** e **Contabilità**: posizioni decimali e un simbolo. Contabilità allinea i simboli e i separatori decimali in una colonna.
- **Data** e **Ora**: un elenco di formati predefiniti, con l'ordine e i nomi dell'italiano.
- **Percentuale**, **Scientifico**: posizioni decimali.
- **Frazione**: formati predefiniti come "fino a una cifra" (1/4), "fino a due cifre" (21/25), "in quarti" e "in sedicesimi".
- **Speciale**: formati predefiniti per un codice postale, un codice postale più 4, un numero di telefono e un numero di previdenza sociale.
- **Testo**: la cella contiene ciò che digiti come testo, quindi un numero come 00123 conserva gli zeri.
- **Personalizzato**: digita un codice tuo, oppure parti da uno dell'elenco (i codici più comuni e quelli già usati nella cartella di lavoro).

Il **codice** è il breve testo che indica a JeffCrunch come scrivere un numero. Esempio: **#.##0,00** significa "raggruppa le cifre per migliaia e mostra sempre due decimali", quindi 1234,5 viene mostrato come 1.234,50. Ogni **0** è una cifra che viene sempre mostrata, ogni **#** una cifra che viene mostrata solo se necessaria, e **%** moltiplica per 100. Un codice può avere fino a quattro sezioni separate da punti e virgola, per numeri positivi, numeri negativi, zero e testo.

## Che cosa viene conservato

Il codice viene salvato nella cartella di lavoro nella sua forma neutra, così la cartella si legge correttamente su qualsiasi computer. La formattazione è un'operazione che puoi annullare, e una cartella di lavoro che già usava un formato lo conserva esattamente: un formato presente nell'elenco viene ritrovato, e nessuno viene creato due volte.
