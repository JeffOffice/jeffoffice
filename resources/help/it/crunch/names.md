# Nomi per celle e valori
<!-- keywords: nome, nomi, nome definito, nuovo nome, intervallo denominato, ambito, commento, nome da celle, inserisci nome, Ctrl+F3, si riferisce a -->

Un **nome** è una parola che rappresenta una cella, un intervallo di celle, un numero o una formula, in modo da poter scrivere =Prezzo*Quantità al posto di =B2*C2. Un nome appartiene all'intera cartella di lavoro oppure a un singolo foglio. Un nome presente in un foglio nasconde, su quel foglio, un nome della cartella di lavoro scritto allo stesso modo.

## Le regole per un nome

- Inizia con una lettera, un carattere di sottolineatura o una barra rovesciata, e dopo di essi contiene solo lettere, cifre, caratteri di sottolineatura e punti. Non contiene spazi.
- Può essere lungo al massimo 255 caratteri.
- Non è l'indirizzo di una cella, come **A1** o **R2C3**.
- Nello stesso foglio (o nella cartella di lavoro) non esiste già un altro nome scritto allo stesso modo, con o senza maiuscole.

Mentre digiti, la finestra di dialogo indica cosa non va, e il pulsante **OK** resta disattivato.

## Creare un nome

1. Seleziona le celle e fai clic su **Nuovo nome** nella scheda **Formule**.
2. Digita il nome, scegli dove funziona e indica a cosa si riferisce. Le celle selezionate vengono inserite automaticamente.

Esiste un modo più rapido per un nome dell'intera cartella di lavoro. Seleziona le celle, fai clic sulla **Casella Nome** (a sinistra della barra della formula), digita il nome e premi **Invio**. Se non esiste ancora un nome scritto in quel modo, viene creato per le celle selezionate. Se esiste, **Invio** porta alle sue celle.

**Crea da celle** crea più nomi in una sola volta a partire dalle etichette di un blocco: con le intestazioni nella riga superiore e i numeri sotto di esse, ogni intestazione dà un nome alle celle sottostanti. Scegli dove si trovano le etichette (riga superiore, colonna sinistra, riga inferiore o colonna destra). Un'etichetta che contiene spazi o caratteri non ammessi nei nomi viene modificata sostituendo tali caratteri con caratteri di sottolineatura. Un'etichetta vuota, duplicata o che corrisponde a un nome già presente nella cartella di lavoro viene ignorata, e la barra di stato indica quante ne sono state ignorate.

## Visualizzare tutti i nomi

Fai clic su **Nomi** nella scheda **Formule**, oppure premi **Ctrl+F3**. L'elenco mostra ogni nome, il valore che ha attualmente, a cosa si riferisce, dove funziona e il suo commento. Digita nel filtro per visualizzare solo i nomi corrispondenti. **Nuovo**, **Modifica** ed **Elimina** creano, modificano o rimuovono un nome. Una cartella di lavoro con più di 20.000 nomi mostra solo i primi 20.000.

Modificare il nome di un nome lo cambia in ogni formula che lo usa, in tutti i fogli, con un unico passaggio. Eliminare un nome fa sì che le formule che lo usavano mostrino **#NOME?**. Ogni modifica può essere annullata.

## Inserire un nome in una formula

Mentre digiti una formula, i nomi corrispondenti vengono visualizzati nell'elenco sotto la cella. **Inserisci nome** nella scheda **Formule** mostra l'elenco dei nomi tra cui scegliere.
