# Apri, salva, stampa ed esporta
<!-- keywords: apri, salva, salva con nome, docx, doc, dot, odt, opendocument, rtf, file meno recenti, modello, dotx, recenti, fissa, nuovo, chiudi, stampa, pdf, esporta, recupera, recupero automatico, non salvato, arresto anomalo, schermata file, formati file, salva una copia, sola lettura, apri in sola lettura, apri come copia, nuova finestra, copia di backup, i miei modelli, salva come modello, ott, ripara, apri e ripara, danneggiato -->

Fai clic su **File** in alto a sinistra per visualizzare la schermata File. Contiene le schede Nuovo, Apri, Stampa ed Esporta, e i comandi per Salva e Salva con nome. Premi **Esc** per tornare al documento.

## Creare un nuovo documento

Premi **Ctrl+N**, oppure fai clic su **File > Nuovo** e poi su **Documento vuoto**. Il nuovo documento si apre in una finestra propria. Se la finestra in cui ti trovi è vuota e non è mai stata salvata, il nuovo documento la sostituisce.

## Aprire un documento

Premi **Ctrl+O**, oppure fai clic su **File > Apri**. Fai clic su **Sfoglia** per scegliere un file. JeffWrite apre questi tipi di file:

- **.docx** è un documento.
- **.dotx** è un modello. Un modello si apre come nuovo documento creato a partire da esso, così il modello resta invariato.
- **.docm** e **.dotm** si aprono anch'essi. JeffWrite non conserva le loro macro. Te lo segnala la prima volta che salvi un documento di questo tipo, e salva un file .docx.
- **.doc** e **.dot** sono i formati meno recenti di documento e modello. JeffWrite li legge e salva ciò che ci fai come nuovo file .docx.
- **.odt**, **.ott** e **.fodt** sono file OpenDocument. JeffWrite li legge tutti e tre. Salva i file .odt, e salva ciò che fai con un file .ott o .fodt come nuovo file .odt. Vedi [Aprire e salvare file OpenDocument](open-odt).
- **.rtf** è Rich Text Format. JeffWrite lo legge e salva ciò che ci fai come nuovo file .docx. Vedi [Aprire file RTF](open-rtf).

La scheda **Apri** elenca anche i documenti aperti o salvati di recente. Fai clic su uno di essi per aprirlo. Fai clic con il pulsante destro su un elemento per fissarlo in cima all'elenco oppure per rimuoverlo dall'elenco.

Se un file contiene parti che JeffWrite non sa ancora modificare, come il layout di una tabella o un'intestazione, le conserva. Vedi [Benvenuto in JeffWrite](start). Se una parte del file non può essere letta affatto, JeffWrite ti dice che cosa ha lasciato fuori. In questo caso non sovrascrive quel file: Salva chiede un nuovo nome, così l'originale conserva ciò che la copia ha perso.

## Salvare un documento

Premi **Ctrl+S** per salvare. La prima volta, JeffWrite chiede un nome. **Salva con nome** (F12) salva una copia con un nuovo nome. Scegli **Modello (*.dotx)** per salvare un modello. Un file derivato da un formato meno recente viene sempre salvato con un nuovo nome, così il file originale non viene mai sovrascritto.

## Salvare una copia

Fai clic su **File > Salva una copia**. Scegli un nome. JeffWrite scrive il documento in quel file, e tu continui a lavorare nel documento che avevi aperto. Il suo nome e le modifiche non salvate restano come erano. È utile quando vuoi inviare una versione a qualcuno e continuare a scrivere.

## Conservare una copia di backup

Fai clic su **File > Opzioni**, e nella scheda **Generali** attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffWrite copia prima il file precedente in uno chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, JeffWrite chiede se salvare senza di essa. La risposta predefinita è No, a meno che tu non dica Sì, e in quel caso il file resta com'era. Così puoi tornare all'ultima versione salvata.

## Salvare come modello

Un modello è un documento che usi come punto di partenza. Fai clic su **File > Salva come modello** e digita un nome. Il modello finisce in **I miei modelli**, e il documento su cui stai lavorando non viene modificato. In **Salva con nome** puoi anche scegliere **Modello (*.dotx)** oppure **Modello di testo OpenDocument (*.ott)** per salvare un file modello dove preferisci.

Per iniziare un nuovo documento dal tuo modello, fai clic su **File > Nuovo**. I tuoi modelli sono elencati sotto **I miei modelli**. Fai clic su uno di essi. Si apre un nuovo documento con il testo e gli stili del modello. Per rimuovere un modello dall'elenco, fai clic con il pulsante destro su di esso e scegli **Rimuovi da I miei modelli**.

## Altri modi per aprire un file

Nella scheda **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti. Ogni modo chiede un file, oppure usa quello su cui hai fatto clic:

- **Apri in sola lettura** apre il file solo per la lettura. Puoi guardarlo, scorrerlo, cercare e copiare, ma non puoi modificare nulla. La barra del titolo riporta "Sola lettura". Se premi **Salva**, JeffWrite chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre un nuovo documento con il testo del file. Non ha un file proprio, quindi **Salva** chiede dove metterlo. Il file aperto non viene mai modificato.
- **Apri in nuova finestra** apre il file in una finestra propria, anche quando la finestra in cui ti trovi è vuota.
- **Apri e ripara** serve per un file .docx danneggiato, ad esempio uno interrotto durante la copia. Vedi sotto.

## Apri e ripara

A volte un file .docx è danneggiato. Una copia si è interrotta a metà, oppure un disco ha commesso errori. Un file .docx è un insieme di parti più piccole, come il testo, gli stili e le immagini. **Apri e ripara** legge ogni parte che riesce a leggere. Una parte interrotta a metà viene conservata fino al punto in cui si è interrotta. Una parte che non può essere letta viene lasciata fuori. Se l'elenco delle parti o i collegamenti tra di esse sono andati persi, JeffWrite li ricostruisce.

Il documento riparato si apre come copia con un nome del tipo "Lettera riparata". Un messaggio ti dice che cosa è stato conservato fino a un'interruzione e che cosa è stato lasciato fuori. Il file danneggiato non viene modificato. Salva la copia riparata con un nome proprio e controllala prima di fidarti.

## Recuperare lavoro non salvato

JeffWrite salva una copia di riserva del lavoro non salvato ogni pochi minuti, come file in una propria cartella. Se il programma si chiude in modo imprevisto, alla riapertura successiva le propone. Fai clic su **File > Apri > Recupera lavoro non salvato** per vederle in seguito. Puoi scegliere quali aprire e salvare ciascuna dove preferisci.

## Stampare

Premi **Ctrl+P**, oppure fai clic su **File > Stampa**. Scegli una stampante, il numero di copie e le pagine, controlla ogni pagina nell'anteprima e fai clic su **Stampa**. Vedi [Stampa](print).

## Creare un PDF

Fai clic su **File > Esporta > Esporta in PDF**. Scegli un nome. Il PDF appare uguale su ogni computer, perché i tipi di carattere vi sono incorporati.

## Vedi anche

- [Benvenuto in JeffWrite](start)
- [Stampa](print)
- [Usare JeffWrite da tastiera](keyboard)
