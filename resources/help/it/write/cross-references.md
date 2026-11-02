# Riferimenti incrociati
<!-- keywords: riferimento incrociato, riferimenti incrociati, rimando, rimandi, REF, PAGEREF, NOTEREF, vedere pagina, vedi pagina, vedere figura, sopra, sotto, numero del titolo, collegamento -->

Un **riferimento incrociato** è un insieme di parole nel testo che rimanda a un altro punto, ad esempio "vedere la Figura 2 a pagina 7". È un campo, quindi quando la figura o la pagina cambiano basta aggiornare il campo e le parole si adeguano.

## Inserire un riferimento incrociato

1. Fai clic nel punto in cui devono comparire le parole.
2. Fai clic su **Riferimenti > Riferimento incrociato**.
3. Scegli il **Tipo di riferimento**: un titolo, un segnalibro, una nota a piè di pagina, una nota di chiusura, un elemento numerato o una didascalia come Figura o Tabella.
4. Scegli cosa indicare in **Inserisci riferimento a**: il testo, il numero, il numero di pagina oppure "sopra" e "sotto".
5. Scegli l'elemento dall'elenco, lascia selezionato **Inserisci come collegamento ipertestuale** se vuoi poterlo raggiungere con un clic e fai clic su **Inserisci**.

Ad esempio, per scrivere "Vedere il titolo Risultati a pagina 4": digita `Vedere il titolo `, inserisci un riferimento al titolo con **Testo del titolo**, digita ` a pagina ` e inserisci un secondo riferimento allo stesso titolo con **Numero di pagina**.

## Seguire un riferimento

Tieni premuto **Ctrl** e fai clic su un riferimento che è un collegamento ipertestuale. Il cursore si sposta nel punto indicato.

## Mantenere le parole corrette

Dopo aver aggiunto pagine, modificato titoli o spostato figure, seleziona il testo e premi **F9**. Ogni riferimento ricalcola le proprie parole. Se il punto a cui rimanda un riferimento è stato eliminato, il campo mostra `Errore: il segnalibro a cui si fa riferimento è mancante.` e puoi scegliere una nuova destinazione.

## Cosa succede dietro le quinte

JeffWrite contrassegna la destinazione con un segnalibro nascosto (il cui nome inizia con `_Ref`) e crea un campo che lo nomina: `REF` per testo e numeri, `PAGEREF` per i numeri di pagina e `NOTEREF` per le note. Anche aprendo il file con un altro programma, i riferimenti continuano a funzionare.
