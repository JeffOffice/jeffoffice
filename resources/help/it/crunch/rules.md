# Regole per il contenuto delle celle
<!-- keywords: convalida, regola, convalida dati, convalida dei dati, elenco, elenco a discesa, menu a discesa, numero intero, decimale, data, ora, lunghezza, personalizzata, interruzione, avviso, informazioni, messaggio di errore, non consentito, scelte, opzioni, freccia, Alt+Giù -->

Una cartella di lavoro può avere delle **regole** che stabiliscono cosa è possibile inserire in alcune celle: un numero intero da 1 a 10, una data successiva a un determinato giorno, un testo di massimo cinque lettere o una voce scelta da un elenco. Le regole sono definite nella cartella di lavoro; JeffCrunch le applica quando si **digita** un valore in una cella (o lo si sceglie dal relativo elenco). I valori incollati, inseriti con il riempimento o spostati non vengono controllati, come avviene nei fogli di calcolo più diffusi: una regola stabilisce quindi cosa si può digitare, non cosa contiene la cella.

## Quando un valore non è conforme

Ogni regola ha uno **stile** e, se la cartella di lavoro ne prevede uno, il messaggio visualizzato è quello della regola:

- **Interruzione**: viene visualizzato il messaggio e il valore immesso rimane nella cella per poterlo modificare. Premere **Esc** per rinunciare.
- **Avviso**: viene chiesto se si vuole mantenere comunque il valore.
- **Informazioni**: viene segnalato che il valore non è conforme ed è possibile proseguire o rinunciare.

Un valore incollato non è soggetto alla regola: viene inserito così com'è, senza alcuna richiesta. È sempre possibile cancellare il contenuto di una cella.

## Scegliere da un elenco

Quando contiene il cursore, una cella con un elenco mostra sul bordo destro un piccolo pulsante con una freccia. Fare clic sul pulsante o premere **Alt+Giù** per visualizzare le voci, quindi fare clic su una voce per inserirla.

## Funzioni non disponibili

JeffCrunch mantiene le regole presenti nella cartella di lavoro, ma non dispone ancora di un comando per crearne di nuove. Le regole che leggono le voci da un altro foglio con la sintassi più recente vengono conservate nel file, ma non applicate. I messaggi che compaiono quando si seleziona una cella non vengono visualizzati.
