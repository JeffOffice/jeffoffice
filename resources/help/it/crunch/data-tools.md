# Strumenti dati
<!-- keywords: dati, scheda dati, testo in colonne, dividi, separatore, virgola, tabulazione, larghezza fissa, rimuovi duplicati, righe ripetute, valori univoci, file di testo, csv, da testo, colonne, delimitatore, virgolette, origine dati, ordina, filtra -->

La scheda **Dati** contiene gli strumenti che lavorano su un elenco di valori: recuperare testo da un file, ordinare un elenco, dividere il testo in colonne ed eliminare le righe ripetute.

## Da testo/CSV

**Da testo/CSV** apre un file di testo (.csv, .tsv o .txt). Funziona come **Apri file di testo** nella scheda File. JeffCrunch chiede come è suddiviso il file in colonne e che tipo di testo contiene, quindi lo mostra in una nuova cartella di lavoro.

## Ordina e filtra

Il gruppo **Ordina e filtra** contiene gli stessi comandi del menu **Ordina e filtra** nella scheda Home. Vedi [Ordinare e filtrare](sort-filter).

## Testo in colonne

Usa questo strumento quando una colonna contiene più parti di testo che vuoi inserire in colonne separate, ad esempio "Rossi, Anna, 31".

1. Seleziona le celle di **una** colonna.
2. Scegli **Testo in colonne** nella scheda **Dati**.
3. Scegli come dividere il testo:
   - **Dividi in corrispondenza di un separatore**: seleziona i caratteri che separano le parti: **Tabulazione**, **Punto e virgola**, **Virgola**, **Spazio** oppure **Altro**, dove digiti il carattere desiderato. **Considera separatori consecutivi come uno solo** evita che due virgole di seguito creino una parte vuota. **Testo tra virgolette** indica il carattere che racchiude una parte da non dividere, ad esempio "Lee, Bo". All'interno delle virgolette, due caratteri uguali di seguito valgono come uno solo.
   - **Dividi in posizioni fisse**: digita le posizioni in cui inizia una nuova colonna, contando i caratteri da 1. Ad esempio, 5, 12 fa iniziare una nuova colonna al quinto e al dodicesimo carattere.
4. Scegli il tipo di ogni parte: **Generale** legge numeri e date come se li avessi digitati, mentre **Testo** mantiene ogni parte così com'è, quindi "007" resta "007".
5. In **Inserisci le parti a partire da**, digita la prima cella per le parti. Inizialmente è la prima cella selezionata.
6. La casella sotto le opzioni mostra le prime righe divise come hai scelto. Premi **OK**.

Le parti vengono inserite nelle celle a destra della prima cella e sostituiscono il contenuto che c'era. Testo in colonne è un'unica operazione che puoi annullare.

## Rimuovi duplicati

Usa questo strumento per conservare solo la prima di ogni gruppo di righe ripetute.

1. Seleziona le celle dell'elenco, oppure una cella al suo interno. JeffCrunch considera l'intero blocco di dati attorno a una singola cella.
2. Scegli **Rimuovi duplicati**.
3. Seleziona **I miei dati hanno intestazioni** se la prima riga contiene i nomi. Quella riga resta e non viene confrontata.
4. Seleziona le colonne che devono coincidere perché due righe siano considerate ripetute. Due righe sono ripetute solo se tutte le colonne selezionate sono uguali. Maiuscole e minuscole sono considerate uguali, e una formula conta come il valore che restituisce.
5. Premi **OK**. La finestra di dialogo indica quante righe sono state rimosse e quante ne restano.

Le righe sotto una riga rimossa salgono all'interno delle celle selezionate, con i rispettivi formati e formule. Le celle sotto la selezione non si spostano. Le righe vuote rimaste in fondo alla selezione vengono cancellate. Rimuovi duplicati è un'unica operazione che puoi annullare.
