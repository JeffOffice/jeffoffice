# Nomi per celle e valori
<!-- keywords: nome, nomi, nome definito, nuovo nome, intervallo con nome, ambito, commento, nome da celle, inserisci nome, Ctrl+F3, si riferisce a, ordina nomi, nome di errore, avviso -->

Un **nome** è una parola che rappresenta una cella, un intervallo di celle, un numero o una formula, così da poter scrivere =Prezzo*Quantità al posto di =B2*C2. Un nome appartiene all'intera cartella di lavoro oppure a un singolo foglio. Un nome definito in un foglio nasconde, in quel foglio, un nome della cartella di lavoro scritto allo stesso modo.

## Le regole per un nome

- Inizia con una lettera, un carattere di sottolineatura o una barra rovesciata, e dopo di esso può contenere solo lettere, cifre, caratteri di sottolineatura e punti. Non contiene spazi.
- Può essere lungo al massimo 255 caratteri.
- Non è l'indirizzo di una cella, come **A1** o **R2C3**.
- Nello stesso foglio (o nella cartella di lavoro) non esiste già un altro nome scritto allo stesso modo, con o senza maiuscole.

Mentre si digita, la finestra di dialogo indica che cosa non va, e il pulsante **OK** resta disattivato.

## Creare un nome

1. Selezionare le celle e fare clic su **Nuovo nome** nella scheda **Formule**.
2. Digitare il nome, scegliere dove deve funzionare e indicare che cosa rappresenta. Le celle selezionate vengono inserite automaticamente.

Esiste un modo più rapido per un nome dell'intera cartella di lavoro. Selezionare le celle, fare clic sulla **Casella Nome** (a sinistra della barra della formula), digitare il nome e premere **Invio**. Se non esiste ancora un nome scritto in quel modo, viene creato per le celle selezionate. Se esiste, **Invio** porta alle sue celle.

**Nome da celle** crea molti nomi contemporaneamente a partire dalle etichette che circondano un intervallo: con le intestazioni nella riga superiore e i numeri sotto di esse, ogni intestazione dà un nome alle celle sottostanti. Scegliere dove si trovano le etichette (riga superiore, colonna sinistra, riga inferiore o colonna destra). Un'etichetta con spazi o con simboli non ammessi nei nomi riceve caratteri di sottolineatura al loro posto. Un'etichetta vuota, ripetuta o già usata come nome nella cartella di lavoro viene esclusa, e la barra di stato indica quante ne sono state escluse.

## Vedere tutti i nomi

Fare clic su **Nomi** nella scheda **Formule**, oppure premere **Ctrl+F3**. L'elenco mostra ogni nome, il valore che ha ora, ciò che rappresenta, dove funziona e il relativo commento. Digitare nel filtro per mostrare solo i nomi corrispondenti. **Nuovo**, **Modifica** ed **Elimina** creano un nome, ne modificano uno o ne rimuovono uno. Fare clic su un'intestazione per ordinare l'elenco in base a essa, e fare di nuovo clic per invertire l'ordine. Un nome che punta a un errore, come una posizione eliminata, ha accanto un piccolo segnale di avviso. In una cartella di lavoro con più di 20.000 nomi, l'elenco mostra i primi 20.000.

Cambiare il nome di un nome lo modifica in tutte le formule che lo usano, in tutti i fogli, con un solo passaggio. Eliminare un nome fa sì che le formule che lo usavano mostrino **#NOME?**. Ogni modifica può essere annullata.

## Inserire un nome in una formula

Mentre si digita una formula, sotto la cella compaiono i nomi corrispondenti in un elenco. **Inserisci nome** nella scheda **Formule** mostra l'elenco dei nomi tra cui scegliere.
