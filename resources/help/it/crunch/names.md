# Nomi per celle e valori
<!-- keywords: nome, nomi, nome definito, nuovo nome, intervallo con nome, ambito, commento, nome da celle, inserisci nome, applica nomi, Ctrl+F3, si riferisce a, ordina nomi, nome di errore, avviso -->

Un **nome** è una parola che rappresenta una cella, un intervallo di celle, un numero o una formula, in modo che =Prezzo*Quantità si possa scrivere al posto di =B2*C2. Un nome appartiene all'intera cartella di lavoro oppure a un singolo foglio. Un nome su un foglio nasconde, su quel foglio, un nome della cartella di lavoro scritto allo stesso modo.

## Le regole per un nome

- Inizia con una lettera, un carattere di sottolineatura o una barra rovesciata, e dopo di essi contiene solo lettere, cifre, caratteri di sottolineatura e punti. Non contiene spazi.
- Ha al massimo 255 caratteri.
- Non è l'indirizzo di una cella, come **A1** o **R2C3**.
- Nessun altro nome dello stesso foglio (o della cartella di lavoro) è scritto allo stesso modo, con maiuscole o minuscole.

Mentre si digita, la finestra di dialogo indica che cosa non va, e il pulsante **OK** resta disattivato.

## Creare un nome

1. Selezionare le celle e fare clic su **Nuovo nome** nella scheda **Formule**.
2. Digitare il nome, scegliere dove deve funzionare e indicare che cosa rappresenta. Le celle selezionate vengono inserite automaticamente.

Esiste un modo più rapido per un nome dell'intera cartella di lavoro. Selezionare le celle, fare clic sulla **Casella Nome** (a sinistra della barra della formula), digitare il nome e premere **Invio**. Se non esiste ancora un nome scritto in quel modo, viene creato per le celle selezionate. Se esiste, **Invio** porta alle sue celle.

**Nome da celle** crea molti nomi in una sola volta dalle etichette che circondano un intervallo: con le intestazioni nella riga superiore e i numeri sotto di esse, ogni intestazione assegna un nome alle celle sottostanti. Scegliere dove si trovano le etichette (riga superiore, colonna sinistra, riga inferiore o colonna destra). Un'etichetta con spazi o caratteri non ammessi nei nomi riceve al loro posto dei caratteri di sottolineatura. Un'etichetta vuota, ripetuta o che corrisponde a un nome già presente nella cartella di lavoro viene esclusa, e la barra di stato indica quante ne sono state escluse.

## Visualizzare tutti i nomi

Fare clic su **Nomi** nella scheda **Formule**, oppure premere **Ctrl+F3**. L'elenco mostra ogni nome, il valore che ha in questo momento, ciò che rappresenta, dove funziona e il suo commento. Digitare nel filtro per mostrare solo i nomi corrispondenti. **Nuovo**, **Modifica** ed **Elimina** creano un nome, ne modificano uno o ne rimuovono uno. Fare clic su un'intestazione per ordinare l'elenco in base a essa, e fare di nuovo clic per invertire l'ordine. Un nome che punta a un errore, come una posizione eliminata, è contrassegnato da un piccolo segnale di avviso accanto a sé. Una cartella di lavoro con più di 20.000 nomi mostra i primi 20.000.

Modificare il nome di un nome lo cambia in ogni formula che lo usa, in tutti i fogli, in un solo passaggio. Eliminare un nome lascia le formule che lo usavano con **#NOME?**. Ogni modifica può essere annullata.

## Inserire un nome in una formula

Mentre si digita una formula, sotto la cella compaiono i nomi corrispondenti nell'elenco. **Inserisci nome** nella scheda **Formule** mostra l'elenco dei nomi tra cui scegliere.

## Inserire nomi in formule già scritte

Supponiamo di aver creato il nome Tasso per $A$1 quando molte formule facevano già riferimento a $A$1. Fare clic su **Applica nomi** nella scheda **Formule** per sostituire $A$1 con Tasso in quelle formule. Selezionare prima le celle da esaminare, oppure selezionare una sola cella per esaminare l'intero foglio. Scegliere i nomi da inserire. Un riferimento viene modificato solo quando corrisponde esattamente alle celle del nome, con o senza il segno del dollaro: A1:B2 non viene sostituito con un nome per A1. Vengono proposti solo i nomi di un intervallo di celle su un foglio, e solo i nomi dell'intera cartella di lavoro. La modifica avviene in un unico passaggio, quindi **Annulla** la ripristina.
