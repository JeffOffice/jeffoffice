# Aprire cartelle di lavoro protette da password
<!-- keywords: password, protetto, cifrato, crittografia, apri, bloccato, password errata, annulla, xlsx, xls, ods, protezione file, password predefinita, privato -->

Alcune cartelle di lavoro sono protette con una password al momento del salvataggio: senza di essa, nessun programma può leggere il contenuto del file. Questo è diverso da **Proteggi foglio** e **Proteggi cartella di lavoro** (vedi [Proteggere un foglio o la cartella di lavoro](protect)), che impediscono solo le modifiche. Una cartella di lavoro protetta con una password non può essere aperta finché non si digita la password. Questa pagina spiega come aprirne una e come crearne una.

## Aprire una cartella protetta

Apri il file come faresti con un altro: premi **{key:Open}**, scegli il file oppure trascinalo sulla finestra. JeffCrunch riconosce che il file è protetto e chiede la password prima di leggere qualsiasi cosa:

1. Digita la password nella casella che riporta **Questo file è protetto da password.** I caratteri appaiono come puntini.
2. Fai clic su **Apri**.

Se la password non è quella corretta, JeffCrunch lo segnala e chiede di nuovo, quante volte vuoi. Fai clic su **Annulla** per interrompere: non viene aperto nulla e le cartelle di lavoro già aperte restano invariate. Le password sono esatte: maiuscole, spazi e accenti contano.

JeffCrunch non scrive la password da nessuna parte. La conserva in memoria mentre la cartella di lavoro è aperta, in modo che **Salva** possa proteggere di nuovo il file, e la cancella quando chiudi la cartella di lavoro. La volta successiva che apri il file, ti chiederà di nuovo la password.

## Che cosa si può aprire

- **Cartelle di lavoro .xlsx e .xlsm** protette in uno dei due modi usati dal programma originale a partire dal 2007: il metodo più recente (la chiave è di 128, 192 o 256 bit, e JeffCrunch verifica anche che nessuno abbia modificato il file dopo la protezione) e il metodo precedente (AES con una chiave di 128, 192 o 256 bit).
- **Cartelle di lavoro .xls** protette in uno dei tre modi previsti da quel formato: la semplice offuscazione dei file più vecchi, quella a 40 bit e quella più robusta che usa il servizio di chiavi del computer.
- **Fogli di calcolo .ods** protetti con una password, in tutti e tre i modi in cui LibreOffice e OpenOffice li hanno scritti: il più recente (LibreOffice 25 e versioni successive bloccano l'intero file con una chiave a 256 bit e ricavano la chiave dalla password con un metodo volutamente lento e che richiede molta memoria, perciò aprire questi file richiede un po' più di tempo), quello a 256 bit delle versioni precedenti e quello più vecchio.

Per i file .xls meno recenti ci sono limiti sulla password stessa. La semplice offuscazione dei file più vecchi usa solo i primi 15 caratteri di una password e può contenere solo lettere, cifre e segni del set di caratteri dell'Europa occidentale (Latin-1): una password con altre lettere (come greche o cinesi) non viene mai riconosciuta come corretta. Il metodo a 40 bit accetta qualsiasi lettera, ma solo i primi 16 caratteri. JeffCrunch non scrive file .xls con password.

Alcuni file sono bloccati con una password che tutti conoscono, in modo che possano essere aperti da chiunque ma non modificati per errore. JeffCrunch apre questi file senza chiedere nulla.

## Quando un file non si può aprire

- **"La password non è corretta."** La password digitata non è quella con cui il file è stato protetto. Riprova prestando attenzione a maiuscole e spazi; JeffCrunch non può recuperare una password smarrita.
- **"Questo file è protetto in un modo che questo programma non può aprire."** Il file è bloccato con qualcosa di diverso da una password (per esempio una chiave contenuta in un certificato), oppure in una forma che JeffCrunch non legge.
- **"La protezione con password di questo file richiede più memoria o tempo di quanto questo programma consenta, quindi il file non viene aperto."** Un file creato con LibreOffice 25 o versioni successive imposta quanta memoria e quanto tempo servono per verificare la password. JeffCrunch accetta fino a dieci volte il valore impostato da LibreOffice stesso (640 MiB di memoria), così che un file costruito per occupare il computer non possa farlo. Un file scritto da LibreOffice rientra ampiamente in questo limite.
- **"Il file è danneggiato, oppure è stato modificato dopo la protezione."** La password era corretta, ma il contenuto del file non è quello che era stato bloccato: è stato troncato, danneggiato o modificato da un altro programma. Copialo di nuovo dall'originale.

## Salvare con una password

Fai clic su **File**, poi su **Proteggi con password** (oppure **Revisione > Proteggi con password**), digita la password due volte e fai clic su **Proteggi**. La cartella di lavoro viene scritta protetta al successivo salvataggio: chiunque apra il file deve digitare la password. Per cambiare la password, scegli lo stesso comando e digita una nuova password; per rimuoverla, fai clic su **Rimuovi password** e salva.

Una cartella di lavoro aperta con una password la conserva finché resta aperta, quindi **Salva** scrive di nuovo il file protetto con la stessa password. JeffCrunch tiene la password in memoria solo fino alla chiusura della cartella di lavoro, poi la cancella. Il file aperto con la password non resta senza protezione per errore: una copia salvata come file .ods o .ots è protetta con la stessa password, secondo quanto descrive lo standard OpenDocument, così altri programmi che seguono lo standard possono aprirla con la password. Una copia in formato .xls o testo non può essere protetta, quindi JeffCrunch ti avvisa prima che la copia non sarà protetta e puoi annullare.

Usa una password lunga e conservala in un posto sicuro: nessuno, nemmeno noi, può aprire il file senza di essa.
