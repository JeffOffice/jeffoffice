# Salvare e aprire pubblicazioni
<!-- keywords: apri, salva, salva con nome, salva una copia, chiudi, file recenti, aggiunti, tipi di file, jpub, rinomina, backup, proprietà, informazioni, contrassegna come finale, ispeziona, cronologia versioni, nuovo nome, copia, Ctrl+S, sola lettura, apri in sola lettura, apri come copia, nuova finestra, i miei modelli, salva come modello, ripara, apri e ripara, danneggiato, verifica compatibilità, salva comunque, non chiedere più -->

JeffPub salva il lavoro in file .jpub. Apre e salva anche file .pub. Vedi [Lavorare con i file .pub](pub-files).

## Salvare

Premi Ctrl+S, fai clic sul pulsante **Salva** nella barra degli strumenti rapida oppure fai clic su **File > Salva**. La prima volta, JeffPub chiede un nome e una posizione. Dopo, Salva sostituisce il file con la versione più recente del lavoro.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffPub si chiude all'improvviso, vedi [Recuperare il lavoro non salvato](recover).

Per conservare l'ultima copia salvata di un file, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffPub copia prima il file precedente in un file chiamato "Backup di" seguito dal nome, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare comunque senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì, e in quel caso il file resta com'era. Questa opzione si trova in **File > Opzioni**, nella scheda **Generale**. È presente in tutte le app di JeffOffice.

## Salvare con un nuovo nome

Fai clic su **File > Salva con nome**, oppure premi F12. Digita un nome, scegli **Salva come tipo** e fai clic su **Salva**. Le scelte sono:

- **Pubblicazione JeffPub (*.jpub)** è la scelta normale.
- **File di pubblicazione .pub (*.pub)** crea un file .pub.
- **PDF (*.pdf)** crea un PDF e lascia la pubblicazione com'era.
- **Modello JeffPub (*.jpub)** aggiunge la pubblicazione a **I miei modelli**.

Dopo un salvataggio in .jpub o .pub, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffPub scrive la pubblicazione in quel file e tu continui a lavorare in quello che avevi aperto. Il suo nome e le modifiche non salvate restano come erano.

## Aprire

1. Premi Ctrl+O, oppure fai clic su **File > Apri**.
2. Fai clic su **Sfoglia** per scegliere un file. Oppure fai clic su un nome sotto **Aggiunti** o **Pubblicazioni recenti**.

Puoi anche trascinare un file .jpub o .pub nella pagina. Se la pubblicazione corrente ha modifiche non salvate, JeffPub chiede prima come procedere.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima) oppure **Rimuovi dall'elenco**. Un nome in grigio indica che JeffPub non riesce più a trovare quel file. Per cambiare quanti nomi recenti vengono mostrati, vedi [Modificare le impostazioni di JeffPub](options).

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei file recenti.

- **Apri in sola lettura** apre la pubblicazione solo per la lettura. Puoi visualizzarla e scorrerla, ma non modificarla. La barra del titolo indica "Sola lettura". Se premi **Salva**, JeffPub chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre la pubblicazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in una nuova finestra** apre il file in una finestra separata.
- **Apri e ripara** serve per un file .jpub danneggiato, per esempio uno tagliato durante la copia. JeffPub legge tutte le parti del file che riesce a leggere e tralascia quelle che non può leggere. La pubblicazione si apre come copia chiamata "Riparata" seguito dal nome, e un messaggio indica cosa è stato tralasciato. Il file danneggiato non viene modificato.

## Chiudere

Fai clic su **File > Chiudi**, oppure premi Ctrl+F4. JeffPub chiede se salvare le modifiche, poi mostra una pagina vuota.

## Informazioni: proprietà, Contrassegna come finale, Ispeziona e versioni

**File > Informazioni** mostra dove si trova il file, con **Mostra nella cartella** e **Copia percorso**. Mostra dati sulla pubblicazione: pagine, dimensioni della pagina, immagini, parole e date. Scrivi nelle caselle per impostare **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**, e fai clic su **Proprietà personalizzate** per proprietà tue. La modifica viene salvata con la pubblicazione, e **Ctrl+Z** la annulla. La pagina apre anche le tue informazioni aziendali, le **Impostazioni di stampa commerciale** (vedi [File PDF e tipografie](pdf)) e la [Verifica struttura](design-checker).

- **Verifica compatibilità** elenca ciò che un file .pub non può contenere della tua pubblicazione, senza salvare nulla: oggetti nascosti, alcuni aspetti degli elenchi e tutto ciò che il writer .pub tralascia. Quando salvi come file .pub, JeffPub mostra lo stesso elenco **prima** di scrivere qualsiasi cosa. Scegli **Salva comunque**, **Salva come .jpub** (il file .jpub viene creato accanto, e non si perde nulla) oppure **Annulla**. Seleziona **Non chiedere più per questo file** e JeffPub smette di chiedere per quel file fino alla chiusura del programma.
- **Apri sempre in sola lettura** fa sì che la pubblicazione si apra solo per la lettura ogni volta. La scelta viene salvata con un file .jpub, e la barra di stato lo indica quando qualcuno apre il file. Per modificarla, scegli **Si apre sempre in sola lettura** nella stessa pagina. È un promemoria e non un blocco. Un file .pub non può contenere questa impostazione (il writer .pub non conserva proprietà di questo tipo, e JeffPub non ne legge una da un file .pub), quindi salva prima come file .jpub. Verifica compatibilità la segnala per un file .pub.
- **Contrassegna come finale** salva la pubblicazione e poi la imposta come sola lettura. Quando qualcuno la apre, la barra del titolo indica che è di sola lettura, e una nota avvisa che è stata contrassegnata come finale. Non è un blocco: scegli di nuovo **Contrassegnato come finale** per modificarla. Un file .pub non può contenere il contrassegno, quindi salva prima come file .jpub.
- **Ispeziona pubblicazione** elenca ciò che il file contiene e che potresti non voler condividere, e lo rimuove: i commenti e i nomi di chi li ha scritti, l'autore e gli altri nomi nelle proprietà, le informazioni aziendali, l'elenco della stampa unione e gli oggetti lasciati nell'area di lavoro accanto alle pagine.
- **Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, toglie a ogni salvataggio dal file l'autore, la persona che ha salvato l'ultima volta, la società e il responsabile, per un file .jpub o .pub. È la stessa casella presente in **Opzioni**, e uguale in tutte le app di JeffOffice.
- **Cronologia versioni** conserva sul computer copie con nome della pubblicazione. Salva prima la pubblicazione. Fai clic su **Conserva una versione…**, poi in seguito scegli una versione per **Apri come copia**, **Torna a questa versione** (la pubblicazione così com'è ora viene prima conservata come versione) oppure **Elimina**.

Un file .jpub non ha un modo per una password, quindi JeffPub non ha una scheda per la password. JeffPub non ha ancora una verifica dell'accessibilità, quindi la pagina non ha una scheda per essa.

## Se Windows non permette a JeffPub di salvare

Windows Sicurezza può impedire ai programmi di salvare in cartelle come Documenti. In tal caso JeffPub mostra un messaggio con i passaggi per consentirlo. Oppure salva in un'altra cartella.

## Vedi anche

- [Recuperare il lavoro non salvato](recover)
- [Lavorare con i file .pub](pub-files)
- [Creare una nuova pubblicazione](create)
- [Modificare le impostazioni di JeffPub](options)
