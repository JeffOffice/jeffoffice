# Aprire cartelle di lavoro protette da password
<!-- keywords: password, protetto, cifrato, cifratura, apri, bloccato, password errata, annulla, xlsx, xls, ods, protezione file, password predefinita, privato -->

Alcune cartelle di lavoro sono protette da una password al momento del salvataggio: senza di essa, nessun programma può leggere ciò che si trova nel file. Questo è diverso da **Proteggi foglio** e **Proteggi cartella di lavoro** (vedi [Proteggere un foglio o la cartella di lavoro](protect)), che impediscono soltanto le modifiche. Una cartella di lavoro protetta da password non può essere aperta affatto finché non viene digitata la password. Questa pagina spiega come aprirne una e come crearne una.

## Aprire una cartella protetta

Apri il file come apri qualsiasi altro file: premi **{key:Open}**, scegli il file oppure trascinalo nella finestra. JeffCrunch riconosce che il file è protetto e chiede la password prima di leggere qualsiasi contenuto:

1. Digita la password nella casella che riporta **Questo file è protetto con una password.** I caratteri vengono visualizzati come puntini.
2. Fai clic su **Apri**.

Se la password non è quella corretta, JeffCrunch lo segnala e chiede di nuovo, quante volte vuoi. Fai clic su **Annulla** per interrompere: non viene aperto nulla e le cartelle di lavoro già aperte restano come sono. Le password sono esatte: le lettere maiuscole, gli spazi e gli accenti contano.

JeffCrunch non scrive la password da nessuna parte. La conserva in memoria mentre la cartella di lavoro è aperta, in modo che **Salva** possa proteggere di nuovo il file, e la cancella quando chiudi la cartella di lavoro. La volta successiva che apri il file, ti chiede di nuovo la password.

## Quali file si possono aprire

- **Cartelle di lavoro .xlsx e .xlsm** protette in uno dei due modi usati dal programma originale a partire dal 2007: il metodo più recente (la chiave è a 128, 192 o 256 bit, e JeffCrunch verifica anche che nessuno abbia modificato il file dopo la protezione) e il metodo precedente (AES con chiave a 128, 192 o 256 bit).
- **Cartelle di lavoro .xls** protette in uno dei tre modi previsti da quel formato: la semplice cifratura dei file più antichi, quella a 40 bit e quella più robusta che usa il servizio di chiavi del computer.
- **Fogli di calcolo .ods** protetti da password, in tutti e tre i modi con cui LibreOffice e OpenOffice li hanno scritti: il più recente (LibreOffice 25 e versioni successive bloccano l'intero file con una chiave a 256 bit e ricavano la chiave dalla password con un metodo volutamente lento e che usa molta memoria, quindi l'apertura di tali file richiede qualche secondo in più), quello a 256 bit delle versioni precedenti e quello più vecchio.

Per i file .xls meno recenti ci sono limiti sulla password stessa. La semplice cifratura dei file più antichi usa solo i primi 15 caratteri di una password e può contenere soltanto lettere, cifre e segni del set di caratteri dell'Europa occidentale (Latin-1): una password con altre lettere (come quelle greche o cinesi) non viene mai riconosciuta come corretta in questo caso. Il tipo a 40 bit accetta qualsiasi lettera, ma usa solo i primi 16 caratteri. JeffCrunch scrive una copia .xls con password nel tipo più robusto (quello che usa il servizio di chiavi del computer, con chiave a 128 bit), perché gli altri due sono più deboli. Questo tipo usa i primi 255 caratteri della password, che possono essere lettere di qualsiasi alfabeto.

Alcuni file sono bloccati con una password che tutti conoscono, in modo che possano essere aperti da chiunque ma non modificati per errore. JeffCrunch apre questi file senza chiedere nulla.

## Quando un file non si apre

- **"La password non è corretta."** La password digitata non è quella con cui è stato protetto il file. Riprova prestando attenzione alle maiuscole e agli spazi; JeffCrunch non può recuperare una password smarrita.
- **"Questo file è protetto in un modo che questo programma non può aprire."** Il file è bloccato da qualcosa di diverso da una password (ad esempio una chiave contenuta in un certificato), oppure in una forma che JeffCrunch non legge.
- **"La protezione con password di questo file richiede più memoria o tempo di quanto questo programma consenta, quindi il file non viene aperto."** Un file creato da LibreOffice 25 o versioni successive stabilisce quanta memoria e quanto tempo occorrono per verificare la password. JeffCrunch accetta fino a dieci volte quanto impostato da LibreOffice stesso (640 MiB di memoria), in modo che un file costruito apposta per occupare il computer non possa bloccarlo. Un file scritto da LibreOffice rientra ampiamente in questo limite.
- **"Il file è danneggiato oppure è stato modificato dopo la protezione."** La password era corretta, ma il contenuto del file non è quello che era stato bloccato: è stato troncato, danneggiato o modificato da un altro programma. Copialo di nuovo dall'originale.

## Salvare con una password

Fai clic su **File**, poi su **Proteggi con password** (oppure **Revisione > Proteggi con password**), digita due volte la password e fai clic su **Proteggi**. La cartella di lavoro viene scritta protetta al successivo salvataggio: chiunque apra il file deve digitare la password. Per cambiare la password, scegli lo stesso comando e digitane una nuova; per eliminarla, fai clic su **Rimuovi password**, quindi salva.

Una cartella di lavoro aperta con una password la mantiene finché resta aperta, quindi **Salva** scrive di nuovo il file protetto con la stessa password. JeffCrunch conserva la password in memoria solo finché chiudi la cartella di lavoro, poi la cancella. Il file aperto con la password non resta non protetto per sbaglio: una copia salvata come file .xls è protetta con la stessa password nel modo più robusto previsto per .xls, e una copia salvata come file .ods o .ots è protetta con la password nel modo descritto dallo standard OpenDocument, così altri programmi che seguono tale standard possono aprire la copia con la password. Una copia in testo non può essere protetta, quindi JeffCrunch ti avvisa in anticipo che la copia non sarà protetta, e puoi annullare.

Usa una password lunga e conservala in un luogo sicuro: nessuno, nemmeno noi, può aprire il file senza di essa.
