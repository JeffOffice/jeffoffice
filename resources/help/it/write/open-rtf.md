# Aprire file RTF
<!-- keywords: rtf, rich text, rich text format, aprire rtf, .rtf, documenti meno recenti, vecchi documenti, importare, aprire, convertire, formato RTF -->

**RTF** (Rich Text Format) è un tipo di file di documento che quasi tutti i programmi di videoscrittura sono in grado di leggere e scrivere. JeffWrite apre i file **.rtf** e ne mantiene il testo e l'aspetto.

## Aprire un file RTF

Premi **CTRL+O** oppure fai clic su **File > Apri**, quindi fai clic su **Sfoglia**. Scegli **Rich Text Format (*.rtf)** nell'elenco dei tipi di file per visualizzare solo questi file, oppure seleziona il file nell'elenco di tutti i documenti. Puoi anche trascinare un file .rtf in una finestra di JeffWrite oppure indicarne il nome quando avvii JeffWrite dalla riga di comando.

JeffWrite riconosce un file RTF dal contenuto, non dal nome: un file con un'estensione diversa viene quindi aperto comunque, se è davvero un file RTF.

## Cosa viene importato

JeffWrite legge queste parti di un file RTF:

- Testo in qualsiasi lingua, con tipi di carattere, dimensioni, colori, grassetto, corsivo, sottolineato, barrato, apice, pedice ed evidenziazione.
- Impostazioni dei paragrafi: allineamento, rientri, spaziatura, tabulazioni, bordi, sfondo e le impostazioni Mantieni con il successivo e Mantieni righe assieme.
- Stili, elenchi numerati ed elenchi puntati.
- Tabelle, incluse le tabelle all'interno di altre tabelle, i bordi, lo sfondo, la larghezza delle colonne e le celle unite.
- Dimensioni pagina, margini, colonne, orientamento della pagina, sezioni, intestazioni e piè di pagina.
- Note a piè di pagina, note di chiusura, campi numero di pagina, altri campi e segnalibri.
- Revisioni: testo aggiunto, testo eliminato, interruzioni di paragrafo aggiunte o eliminate e modifiche di formattazione. Una **revisione** è una modifica contrassegnata da un revisore. Vedi [Tenere traccia delle modifiche e accettarle o rifiutarle](track-changes).
- Immagini in formato PNG, JPEG, EMF e WMF, interruzioni di pagina e interruzioni di colonna.

Se una parte del file non può essere letta, JeffWrite indica cosa è stato omesso. Alcuni file RTF contengono elementi che un documento di JeffWrite non supporta ancora, ad esempio disegni o equazioni. Questi elementi vengono omessi, mentre il testo circostante viene mantenuto.

## Salvare le modifiche

JeffWrite non scrive file .rtf. Quando salvi un documento proveniente da un file .rtf, JeffWrite chiede un nuovo nome e lo salva come file .docx. Il file .rtf rimane invariato.

## Argomenti correlati

- [Aprire, salvare, stampare ed esportare](open-save)
- [Benvenuto in JeffWrite](start)
