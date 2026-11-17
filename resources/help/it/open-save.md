# Salvare e aprire pubblicazioni
<!-- keywords: apri, salva, salva con nome, salva una copia, chiudi, file recenti, aggiunti, tipi di file, jpub, rinomina, backup, proprietà, informazioni, contrassegna come finale, ispeziona, cronologia versioni, nuovo nome, copia, Ctrl+S, sola lettura, apri in sola lettura, apri come copia, nuova finestra, i miei modelli, salva come modello, ripara, apri e ripara, danneggiato, verifica compatibilità, salva comunque, non chiedere più -->

JeffPub salva il lavoro in file .jpub. Apre e salva anche file .pub. Vedi [Lavorare con file .pub](pub-files).

## Salvare

Premi CTRL+S, fai clic sul pulsante **Salva** nella barra degli strumenti rapida oppure fai clic su **File > Salva**. La prima volta, JeffPub chiede un nome e una posizione. Dopo, Salva sostituisce il file con il lavoro più recente.

Un asterisco (*) dopo il nome nella barra del titolo indica che ci sono modifiche non salvate. Salva spesso. Se JeffPub si arresta all'improvviso, vedi [Recuperare il lavoro non salvato](recover).

Per conservare l'ultima copia salvata di un file, attiva **Crea sempre una copia di backup**. Ogni volta che salvi, JeffPub copia prima il file precedente in uno chiamato "Backup di" seguito dal nome del file, nella stessa cartella. Se la copia non può essere creata, l'app chiede se salvare senza di essa. La risposta predefinita è No, a meno che tu non risponda Sì, e in quel caso il file resta com'era. Questa opzione si trova in **File > Opzioni**, nella scheda **Generale**. Ogni app di JeffOffice la offre.

## Salvare con un nuovo nome

Fai clic su **File > Salva con nome** oppure premi F12. Digita un nome, scegli **Salva come tipo** e fai clic su **Salva**. Le scelte sono:

- **Pubblicazione JeffPub (*.jpub)** è la scelta normale.
- **File di pubblicazione .pub (*.pub)** crea un file .pub.
- **PDF (*.pdf)** crea un PDF e lascia la pubblicazione com'era.
- **Modello JeffPub (*.jpub)** aggiunge la pubblicazione a **I miei modelli**.

Dopo un salvataggio in .jpub o .pub, la barra del titolo mostra il nuovo nome e continui a lavorare nel nuovo file.

## Salvare una copia

Fai clic su **File > Salva una copia** e scegli un nome. JeffPub scrive la pubblicazione in quel file, e tu continui a lavorare in quella di prima. Il suo nome e le sue modifiche non salvate restano come erano.

## Aprire

1. Premi CTRL+O oppure fai clic su **File > Apri**.
2. Fai clic su **Sfoglia** per scegliere un file. Oppure fai clic su un nome sotto **Aggiunti** o **Pubblicazioni recenti**.

Puoi anche trascinare un file .jpub o .pub nella pagina. Se la pubblicazione corrente ha modifiche non salvate, JeffPub chiede prima come procedere.

Fai clic con il pulsante destro su un nome nell'elenco per **Aggiungi all'elenco** (resta in cima) oppure **Rimuovi dall'elenco**. Un nome in grigio indica che JeffPub non riesce più a trovare il file. Per cambiare quanti nomi recenti vengono mostrati, vedi [Modificare le impostazioni di JeffPub](options).

## Altri modi per aprire un file

Nella pagina **Apri**, fai clic su **Altri modi per aprire**. Puoi anche fare clic con il pulsante destro su un nome nell'elenco dei recenti.

- **Apri in sola lettura** apre la pubblicazione solo per la lettura. Puoi guardarla e scorrerla, ma non modificarla. La barra del titolo mostra "Sola lettura". Se premi **Salva**, JeffPub chiede un nuovo nome, così il file aperto non viene modificato.
- **Apri come copia** apre la pubblicazione senza un file proprio. **Salva** chiede dove metterla.
- **Apri in nuova finestra** apre il file in una finestra separata.
- **Apri e ripara** serve per un file .jpub danneggiato, per esempio uno troncato durante la copia. JeffPub legge ogni parte del file che riesce a leggere e tralascia la parte che non riesce a leggere. La pubblicazione si apre come copia chiamata "Riparata" più il nome, e un messaggio ti dice che cosa è stato tralasciato. Il file danneggiato non viene modificato.

## Chiudere

Fai clic su **File > Chiudi**, oppure premi CTRL+F4. JeffPub chiede se salvare le modifiche, poi mostra una pagina vuota.

## Informazioni: proprietà, Contrassegna come finale, Ispeziona e versioni

**File > Informazioni** mostra dove si trova il file, con **Mostra nella cartella** e **Copia percorso**. Mostra dati sulla pubblicazione: pagine, dimensioni della pagina, immagini, parole e date. Scrivi nelle caselle per impostare **Titolo**, **Autore**, **Oggetto**, **Parole chiave**, **Categorie**, **Commenti**, **Società** e **Responsabile**, oppure fai clic su **Proprietà personalizzate** per proprietà tue. La modifica viene salvata con la pubblicazione, e **CTRL+Z** la annulla. La pagina apre anche le tue informazioni aziendali, le **Impostazioni di stampa commerciale** (vedi [File PDF e tipografie](pdf)) e la [Verifica struttura](design-checker).

- **Verifica compatibilità** elenca ciò che un file .pub non può contenere della tua pubblicazione, senza salvare nulla: oggetti nascosti, alcuni aspetti degli elenchi e tutto ciò che lo scrittore di file .pub tralascia. Quando salvi come file .pub, JeffPub mostra lo stesso elenco **prima** di scrivere qualsiasi cosa. Scegli **Salva comunque**, **Salva come .jpub** (il file .jpub viene creato accanto, e non si perde nulla) oppure **Annulla**. Seleziona **Non chiedere più per questo file** e JeffPub smette di chiedere per quel file fino alla chiusura del programma.
- **Contrassegna come finale** salva la pubblicazione e poi la rende disponibile solo per la lettura. Quando qualcuno la apre, la barra del titolo indica che è di sola lettura, e una nota dice che è stata contrassegnata come finale. Non è un blocco: scegli di nuovo **Contrassegnato come finale** per modificarla. Un file .pub non può contenere il contrassegno, quindi salva prima come file .jpub.
- **Ispeziona pubblicazione** elenca ciò che il file contiene e che potresti non voler condividere, e lo rimuove: i commenti e i nomi di chi li ha scritti, l'autore e gli altri nomi nelle proprietà, le informazioni aziendali, l'elenco della stampa unione e gli oggetti lasciati nell'area di lavoro accanto alle pagine.
- **Rimuovi le informazioni personali da questo file al salvataggio**, sotto le schede, elimina dal file ad ogni salvataggio, in un file .jpub o .pub, l'autore, chi ha salvato il file per ultimo, la società e il responsabile. È la stessa casella che si trova in **Opzioni**, ed è uguale in ogni app di JeffOffice.
- **Cronologia versioni** conserva sul computer copie con nome della pubblicazione. Salva prima la pubblicazione. Fai clic su **Conserva una versione…**, poi, più avanti, scegli una versione per **Apri come copia**, **Torna a questa versione** (la pubblicazione così com'è ora viene prima conservata come versione) oppure **Elimina**.

Un file .jpub non ha un modo per contenere una password, quindi JeffPub non ha una scheda per le password. JeffPub non ha ancora una verifica di accessibilità, quindi la pagina non ha una scheda per questa.

## Se Windows non permette a JeffPub di salvare

Windows Sicurezza può impedire ai programmi di salvare in cartelle come Documenti. JeffPub allora mostra un messaggio con i passaggi per consentirlo. Oppure salva in un'altra cartella.

## Vedi anche

- [Recuperare il lavoro non salvato](recover)
- [Lavorare con file .pub](pub-files)
- [Creare una nuova pubblicazione](create)
- [Modificare le impostazioni di JeffPub](options)
