# Nomi per celle e valori
<!-- keywords: nome, nomi, nome definito, nuovo nome, intervallo denominato, intervallo con nome, ambito, commento, nome da celle, inserisci nome, Ctrl+F3, riferito a, riferimento -->

Un **nome** è una parola che rappresenta una cella, un blocco di celle, un numero o una formula, in modo da poter scrivere =Prezzo*Quantità al posto di =B2*C2. Un nome appartiene all'intera cartella di lavoro oppure a un solo foglio. Su quel foglio, un nome del foglio nasconde un nome della cartella di lavoro scritto allo stesso modo.

## Le regole per un nome

- Inizia con una lettera, un carattere di sottolineatura o una barra rovesciata, e prosegue solo con lettere, cifre, caratteri di sottolineatura e punti. Non contiene spazi.
- Contiene al massimo 255 caratteri.
- Non è l'indirizzo di una cella, come **A1** o **R2C3**.
- Nessun altro nome dello stesso foglio (o della cartella di lavoro) è scritto allo stesso modo, indipendentemente da maiuscole e minuscole.

Durante la digitazione, la finestra di dialogo indica cosa non va e il pulsante **OK** resta disattivato.

## Creare un nome

1. Selezionare le celle e fare clic su **Nuovo nome** nella scheda **Formule**.
2. Digitare il nome, scegliere l'ambito di validità e indicare a cosa si riferisce. Le celle selezionate vengono inserite automaticamente.

**Nome da celle** crea più nomi contemporaneamente dalle etichette intorno a un blocco: con le intestazioni nella riga superiore e i numeri sotto, ogni intestazione assegna il nome alle celle sottostanti. Scegliere dove si trovano le etichette (riga superiore, colonna sinistra, riga inferiore o colonna destra). Gli spazi e i segni che un nome non può contenere vengono sostituiti da caratteri di sottolineatura. Le etichette vuote, ripetute o corrispondenti a un nome già presente nella cartella di lavoro vengono escluse e la barra di stato ne indica il numero.

## Visualizzare tutti i nomi

Fare clic su **Nomi** nella scheda **Formule** oppure premere **Ctrl+F3**. L'elenco mostra per ogni nome il valore attuale, l'elemento a cui fa riferimento, l'ambito di validità e il commento. Digitare nel filtro per mostrare solo i nomi corrispondenti. **Nuovo**, **Modifica** ed **Elimina** consentono di creare, modificare ed eliminare un nome. Per una cartella di lavoro con più di 20.000 nomi vengono elencati i primi 20.000.

Se si cambia un nome, il nome viene aggiornato in un unico passaggio in tutte le formule che lo usano, in tutti i fogli. Se si elimina un nome, le formule che lo usavano mostrano **#NOME?**. Ogni modifica può essere annullata.

## Inserire un nome in una formula

Durante la digitazione di una formula, i nomi corrispondenti vengono visualizzati nell'elenco sotto la cella. **Inserisci nome** nella scheda **Formule** elenca i nomi tra cui scegliere.
