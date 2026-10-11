# Elenchi puntati, numerazione e tabulazioni
<!-- keywords: elenco puntato, punti elenco, elenco numerato, numerazione, elenchi puntati, elenco, tabulazioni, tabulatori, righello, carattere di riempimento, puntini di riempimento, tabulazione decimale, rientro sporgente -->

Un elenco dispone ogni voce su una riga a sé, contrassegnata da un punto elenco o da un numero. Le tabulazioni allineano il testo in colonne ordinate, come i prezzi di un menu.

## Creare un elenco

1. Fare clic in un paragrafo oppure selezionarne più di uno.
2. Fare clic su **Home > Paragrafo > Elenchi puntati** o su **Home > Paragrafo > Numerazione**.

Fare di nuovo clic sullo stesso pulsante per disattivare l'elenco. Ogni pulsante ha anche una freccia. La freccia accanto a **Elenchi puntati** offre otto simboli di punto elenco. La freccia accanto a **Numerazione** offre sette stili, ad esempio 1. 2. 3., a. b. c., I. II. III. e (1) (2) (3). Scegliere **Nessuno** per rimuovere un elenco.

Per altre opzioni, scegliere **Elenchi puntati e numerati…** da uno dei due menu. La scheda **Elenchi puntati** contiene 12 simboli e un pulsante **Carattere…** per sceglierne uno personalizzato. La scheda **Numerazione** contiene una casella **Inizia da**, che consente ad esempio di far iniziare un elenco da 5.

**Aumenta rientro** e **Riduci rientro** spostano le voci dell'elenco di un livello verso l'interno o verso l'esterno, di 0,6 cm alla volta. Una voce spostata verso l'interno inizia un elenco a sé sotto quella precedente: i numeri diventano a. b. c. (e poi i. ii. iii. al livello successivo) e i punti elenco diventano cerchi (e poi quadrati). Ad esempio, selezionando le voci 2 e 3 di un elenco numerato e facendo clic su **Aumenta rientro**, queste diventano a. e b. sotto la voce 1., mentre la voce successiva rimane 2.

## Digitare un elenco

- Digitare `*` o `-` seguito da uno spazio all'inizio di un paragrafo per iniziare un elenco puntato.
- Digitare `1.` o `1)` seguito da uno spazio per iniziare un elenco numerato.
- Premere INVIO per passare alla voce successiva. Premere INVIO su una voce vuota per terminare l'elenco.
- Premere BACKSPACE all'inizio di una voce per rimuoverne il punto elenco.

Per disattivare questa funzione, deselezionare **Formattazione automatica durante la digitazione** nella scheda **Strumenti di correzione** di **File > Opzioni** (la finestra **Opzioni di JeffPub**).

## Cosa sono le tabulazioni

Una tabulazione è una posizione della riga in cui il testo si sposta quando si preme il tasto TAB. Se non se ne imposta nessuna, TAB passa alla tabulazione predefinita successiva. Le tabulazioni predefinite sono distanziate di 1,25 cm, a partire dal bordo sinistro del testo, a meno che non vengano modificate (vedere **Modificare le tabulazioni predefinite** più avanti). Se il testo che segue una tabulazione non entra nella riga, la tabulazione e il testo passano insieme all'inizio della riga successiva, purché la prima parola vi entri dopo la posizione della tabulazione. In caso contrario, la tabulazione rimane alla fine della riga e il testo inizia la riga successiva dal bordo sinistro del paragrafo. Esistono quattro tipi di tabulazione:

- **A sinistra:** il testo inizia in corrispondenza della tabulazione.
- **Al centro:** il testo è centrato sulla tabulazione.
- **A destra:** il testo termina in corrispondenza della tabulazione.
- **Decimale:** i separatori decimali si allineano sulla tabulazione.

Una tabulazione può avere anche un carattere di riempimento, ovvero una serie di punti, trattini, una linea o punti elenco che riempie lo spazio prima della tabulazione. Una riga di menu come "Zuppa ....... 4,50 €" è una tabulazione a destra con carattere di riempimento a punti.

Le tabulazioni appartengono ai paragrafi. Vengono misurate dal bordo sinistro del testo, all'interno del margine della casella. Fanno eccezione le tabulazioni predefinite, che appartengono all'intera pubblicazione.

## Impostare le tabulazioni sul righello

1. Fare clic su **Visualizza > Mostra > Righelli** se i righelli sono nascosti.
2. Fare clic nel testo. Il righello mostra in bianco la larghezza del testo.
3. Fare clic sul righello nel punto in cui si vuole inserire una tabulazione. Viene visualizzata una tabulazione a sinistra.
4. Trascinare una tabulazione per spostarla. Trascinarla in basso, fuori dal righello, per rimuoverla.

## Impostare le tabulazioni in una finestra di dialogo

1. Fare clic nel paragrafo e fare doppio clic sul righello. Si apre la finestra di dialogo **Paragrafo** sulla scheda **Tabulazioni**. È anche possibile aprirla da **Home > Paragrafo > Interlinea > Paragrafo…** e fare clic sulla scheda **Tabulazioni**.
2. Fare clic su **Aggiungi**, quindi fare doppio clic sulla nuova **Posizione** per digitare un valore.
3. Scegliere un **Allineamento** (a sinistra, al centro, a destra o decimale) e un **Carattere di riempimento** per la riga.
4. Fare clic su **OK**.

**Elimina** rimuove la riga selezionata. **Cancella tutto** rimuove tutte le tabulazioni.

In una cella di tabella, il tasto TAB passa invece alla cella successiva. Vedere [Creare e modificare tabelle](tables).

## Modificare le tabulazioni predefinite

Se un paragrafo non ha tabulazioni proprie, il tasto TAB passa alla tabulazione predefinita successiva. Inizialmente le tabulazioni predefinite sono distanziate di 1,25 cm e un'unica impostazione ne definisce la spaziatura per tutti i paragrafi della pubblicazione.

1. Aprire la finestra di dialogo **Paragrafo** e fare clic sulla scheda **Tabulazioni**, come descritto sopra. Non è necessario selezionare prima del testo.
2. Sotto l'elenco delle tabulazioni, digitare una distanza nella casella **Tabulazioni predefinite**. Il valore può essere compreso tra 1 punto e 1.584 punti (circa 56 cm). La casella usa le unità di misura scelte in **File > Opzioni** ed è possibile digitare un'unità, ad esempio 1 cm.
3. Fare clic su **OK**.

Tutto il testo che usa le tabulazioni predefinite si adegua immediatamente alla nuova spaziatura e **Annulla** ripristina quella precedente. L'impostazione viene mantenuta quando si salva la pubblicazione, sia come file JeffPub sia come file .pub. Le tabulazioni impostate manualmente, sul righello o con **Aggiungi**, rimangono dove sono state posizionate.

## Argomenti correlati

- [Formattare il testo](formatting-text)
- [Usare gli stili](styles)
- [La scheda Home](tab-home)
