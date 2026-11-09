# Salvare e aprire pubblicazioni
<!-- keywords: apri, salva, salva con nome, salva una copia, chiudi, file recenti, aggiunti, tipi di file, jpub, rinomina, backup, proprietà, informazioni, contrassegna come finale, ispeziona, cronologia versioni, nuovo nome, copia, Ctrl+S, sola lettura, apri come copia, nuova finestra, i miei modelli, salva come modello, ripara, apri e ripara, danneggiato -->

JeffPub salva il lavoro in file .jpub. Apre e salva anche i file .pub. Vedi [Lavorare con i file .pub](pub-files).

## Salvare

Premi CTRL+S, fai clic sul pulsante **Salva** nella barra degli strumenti rapida oppure fai clic su **File > Salva**. La prima volta, JeffPub chiede un nome e una posizione. Dopo, **Salva** sostituisce il file con la versione più recente del lavoro.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffPub si interrompe all'improvviso, vedi [Recuperare il lavoro non salvato](recover).

Per conservare l'ultima copia salvata di un file, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffPub copia prima il file precedente in un file chiamato "Backup di" seguito dal nome, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare comunque. La risposta predefinita è No, a meno che tu non risponda Sì, e il file resta com'era. Questa opzione si trova in **File > Opzioni**, nella scheda **Generale**. È presente in tutte le app di JeffOffice.

## Salvare con un nuovo nome

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli **Salva come tipo** e fai clic su **Salva**. Le scelte sono:

- **Pubblicazione JeffPub (*.jpub)** è la scelta normale.
- **File di pubblicazione .pub (*.pub)** crea un file .pub.
- **PDF (*.pdf)** crea un PDF e lascia la pubblicazione com'era.
- **Modello JeffPub (*.jpub)** aggiunge la pubblicazione a **I miei modelli**.

Dopo un salvataggio in .jpub o .pub, la barra del titolo mostra il nuovo nome e tu continui a lavorare nel nuovo file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffPub scrive la pubblicazione in quel file e tu continui a lavorare in quello di partenza. Il suo nome e le sue modifiche non salvate restano invariati.

## Aprire

1. Premi CTRL+O, oppure fai clic su **File > Apri**.
2. Fai clic su **Sfoglia** per scegliere un file. Oppure fai clic su un nome in **Aggiunti** o **Pubblicazioni recenti**.

Puoi anche trascinare un file .jpub o .pub sulla pagina. Se la pubblicazione corrente ha modifiche non salvate, JeffPub chiede prima come procedere.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima) o **Rimuovi dall'elenco**. Un nome in grigio indica che JeffPub non riesce a trovare il file. Per cambiare quanti nomi recenti vengono mostrati, vedi [Modificare le impostazioni di JeffPub](options).

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la pubblicazione solo per la lettura. Puoi guardarla e scorrerla, ma non cambiarla. La barra del titolo indica "Sola lettura". Se premi **Salva**, JeffPub chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre la pubblicazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in una nuova finestra** apre il file in una finestra separata.
- **Apri e ripara** serve per un file .jpub danneggiato, ad esempio uno che è stato troncato durante la copia. JeffPub legge tutte le parti del file che riesce a leggere e tralascia quelle che non può leggere. La pubblicazione si apre come copia chiamata "Riparata" seguito dal nome, e un messaggio ti indica cosa è stato tralasciato. Il file danneggiato non viene modificato.

## Chiudere

Fai clic su **File > Chiudi**, oppure premi CTRL+F4. JeffPub chiede se salvare le modifiche, poi mostra una pagina vuota.

## Informazioni: proprietà, Contrassegna come finale, Ispeziona e versioni

**File > Informazioni** mostra dove si trova il file, con **Mostra nella cartella** e **Copia percorso**. Mostra dati sulla pubblicazione: pagine, dimensioni della pagina, immagini, parole e date. Scrivi nelle caselle per impostare **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**, e fai clic su **Proprietà personalizzate** per proprietà tue. La modifica viene salvata con la pubblicazione, e CTRL+Z la annulla. La pagina apre anche le tue informazioni aziendali, le **Impostazioni di stampa commerciale** (vedi [File PDF e tipografie](pdf)) e la [Verifica struttura](design-checker).

- **Contrassegna come finale** salva la pubblicazione e poi la rende disponibile solo per la lettura. Quando qualcuno la apre, la barra del titolo indica che è di sola lettura, e una nota avvisa che è stata contrassegnata come finale. Non è un blocco: per modificarla scegli di nuovo **Contrassegnato come finale**. Un file .pub non può contenere questo contrassegno, quindi salva prima come file .jpub.
- **Ispeziona pubblicazione** elenca ciò che il file contiene e che potresti non voler condividere, e lo rimuove: il nome dell'autore e altri nomi nelle proprietà, le informazioni aziendali, l'elenco della stampa unione e gli oggetti collocati nell'area di lavoro accanto alle pagine.
- **Cronologia versioni** conserva sul computer copie con nome della pubblicazione. Salva prima la pubblicazione. Fai clic su **Conserva una versione…**, poi, in seguito, scegli una versione per **Apri una copia**, **Torna a questa versione** (la pubblicazione così com'è ora viene prima conservata come versione) oppure **Elimina**.

Un file .jpub non prevede un modo per la password, quindi JeffPub non ha una scheda password. JeffPub non dispone ancora di una verifica dell'accessibilità, quindi la pagina non ha una scheda per questa funzione.

## Se Windows non permette a JeffPub di salvare

Windows Sicurezza può bloccare i programmi che salvano in cartelle come Documenti. JeffPub mostra allora un messaggio con i passaggi per consentirlo. Oppure salva in un'altra cartella.

## Vedi anche

- [Recuperare il lavoro non salvato](recover)
- [Lavorare con i file .pub](pub-files)
- [Creare una nuova pubblicazione](create)
- [Modificare le impostazioni di JeffPub](options)
