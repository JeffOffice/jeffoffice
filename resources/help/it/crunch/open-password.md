# Aprire cartelle di lavoro protette da password
<!-- keywords: password, protetta, protezione, crittografata, crittografia, cifrata, aprire, bloccata, password errata, password sbagliata, annulla, xlsx, xls, ods, protezione file, password predefinita, privata, riservata -->

Alcune cartelle di lavoro vengono protette con una password al momento del salvataggio: senza password, nessun programma può leggere il contenuto del file. Questa protezione è diversa da **Proteggi foglio** e **Proteggi cartella di lavoro** (vedere [Proteggere un foglio o la cartella di lavoro](protect)), che impediscono solo le modifiche. Una cartella di lavoro protetta da password non può essere aperta finché non si digita la password. Questa pagina spiega come aprirne una e come crearne una.

## Aprire una cartella di lavoro protetta

Aprire il file come qualsiasi altro: premere **{key:Open}** e sceglierlo, oppure trascinarlo nella finestra. JeffCrunch rileva che il file è protetto e chiede la password prima di leggerne il contenuto:

1. Digitare la password nella casella accanto a **Questo file è protetto da password.** I caratteri vengono visualizzati come punti.
2. Fare clic su **Apri**.

Se la password non è corretta, JeffCrunch lo segnala e la richiede di nuovo, tutte le volte che si vuole. Fare clic su **Annulla** per interrompere: non viene aperto nulla e le cartelle di lavoro già aperte restano invariate. Le password devono essere esatte: maiuscole, spazi e lettere accentate contano.

JeffCrunch non registra la password da nessuna parte. La conserva in memoria mentre la cartella di lavoro è aperta, in modo che **Salva** possa proteggere di nuovo il file, e la cancella quando la cartella di lavoro viene chiusa. Alla successiva apertura del file, la password viene richiesta di nuovo.

## Cosa è possibile aprire

- **Cartelle di lavoro .xlsx e .xlsm** protette in uno dei due modi usati dal programma originale dal 2007: quello più recente (con chiave a 128, 192 o 256 bit; JeffCrunch verifica anche che nessuno abbia modificato il file dopo la protezione) e quello meno recente (AES con chiave a 128, 192 o 256 bit).
- **Cartelle di lavoro .xls** protette in uno dei tre modi previsti da quel formato: la semplice codifica dei file più vecchi, la crittografia a 40 bit e quella più robusta che usa il servizio di chiavi del computer.
- **Fogli di calcolo .ods** protetti da password, in tutti e tre i modi in cui li hanno scritti LibreOffice e OpenOffice: il più recente (LibreOffice 25 e versioni successive bloccano l'intero file con una chiave a 256 bit e ricavano la chiave dalla password con un metodo volutamente lento e che usa molta memoria, quindi l'apertura di questi file richiede qualche istante in più), il tipo a 256 bit delle versioni precedenti e il tipo più vecchio.

I file .xls meno recenti hanno dei limiti sulla password stessa. La semplice codifica dei file più vecchi usa solo i primi 15 caratteri della password e accetta solo lettere, cifre e simboli del set di caratteri occidentale (Latin-1): una password con altre lettere (ad esempio greche o cinesi) non viene mai riconosciuta come corretta. La crittografia a 40 bit accetta qualsiasi lettera, ma solo i primi 16 caratteri. JeffCrunch non salva file .xls con password.

Alcuni file sono bloccati con una password nota a tutti, in modo che chiunque possa aprirli ma nessuno li modifichi per errore. JeffCrunch li apre senza chiedere nulla.

## Quando un file non si apre

- **"La password non è corretta."** La password digitata non è quella con cui è stato protetto il file. Riprovare facendo attenzione a maiuscole e spazi; JeffCrunch non è in grado di recuperare una password persa.
- **"Questo file è protetto in un modo che questo programma non è in grado di aprire."** Il file è bloccato con un sistema diverso da una password (ad esempio una chiave legata a un certificato) oppure in un formato che JeffCrunch non legge.
- **"La protezione con password di questo file richiede più memoria o tempo di quanto consentito da questo programma, quindi il file non viene aperto."** Un file creato con LibreOffice 25 o versioni successive stabilisce quanta memoria e quanto tempo servono per verificare la password. JeffCrunch accetta fino a dieci volte i valori impostati da LibreOffice (640 MiB di memoria), così un file creato apposta per bloccare il computer non può riuscirci. Un file scritto da LibreOffice rientra ampiamente in questo limite.
- **"Il file è danneggiato oppure è stato modificato dopo la protezione."** La password era corretta, ma il contenuto del file non corrisponde a quello protetto: il file è troncato, danneggiato o modificato da un altro programma. Copiarlo di nuovo dall'originale.

## Salvare con una password

Fare clic su **File**, quindi su **Proteggi con password** (oppure **Revisione > Proteggi con password**), digitare la password due volte e fare clic su **Proteggi**. La cartella di lavoro viene salvata protetta al salvataggio successivo: chiunque apra il file dovrà digitare la password. Per cambiare la password, scegliere lo stesso comando e digitarne una nuova; per rimuoverla, fare clic su **Rimuovi password** e salvare.

Una cartella di lavoro aperta con una password la conserva finché resta aperta, quindi **Salva** scrive di nuovo il file protetto con la stessa password. JeffCrunch conserva la password in memoria solo fino alla chiusura della cartella di lavoro, poi la cancella. Il file aperto con la password non resta mai senza protezione per errore: se se ne salva una copia come file .xls o .ods, formati che JeffCrunch non è in grado di proteggere, il programma avvisa prima che la copia non sarà protetta ed è possibile annullare.

Usare una password lunga e conservarla in un luogo sicuro: nessuno, nemmeno noi, può aprire il file senza di essa.
