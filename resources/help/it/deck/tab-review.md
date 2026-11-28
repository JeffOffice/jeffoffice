# Scheda Revisione
<!-- keywords: scheda revisione, confronta, riquadro revisioni, accetta, rifiuta, barra multifunzione, strumenti di correzione, controllo ortografia, thesaurus, conteggio parole, commenti, nuovo commento, elimina commento, precedente, successivo, mostra commenti, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, grammatica, controllo grammaticale, stile, linea blu, linea punteggiata, controlla grammatica durante la digitazione, verifica accessibilità, accessibilità -->

La scheda **Revisione** controlla le parole, le traduce e gestisce i commenti.

- **Strumenti di correzione** contiene **Controllo ortografia** (F7), **Thesaurus** (Maiusc+F7), **Conteggio parole** e **Verifica accessibilità**. Vedi [Digitare e formattare il testo](text).
- **Lingua** contiene **Traduci**, **Imposta lingua di correzione** e **Non controllare ortografia**. Vedi sotto.
- **Confronta** contiene **Confronta**, **Accetta**, **Rifiuta**, **Precedente**, **Successivo** e **Riquadro revisioni**. Vedi [Confrontare presentazioni](compare).
- **Commenti** contiene **Nuovo commento** (Ctrl+Alt+M), **Elimina** con il relativo menu, **Precedente**, **Successivo** e **Mostra commenti**. Vedi [Commenti](comments).

## Controllare la grammatica durante la digitazione

JeffDeck cerca anche errori evidenti di grammatica e di stile nel testo, nelle diapositive e nelle note del relatore. Un problema viene sottolineato con una linea blu punteggiata, mentre una parola errata mantiene la sua linea rossa ondulata. Fai clic con il pulsante destro del mouse sul problema per vedere che cosa non va e per scegliere un suggerimento, oppure scegli **Ignora questo problema** o **Ignora tutto**. **Controllo ortografia** (F7) passa ai problemi grammaticali delle diapositive dopo quelli ortografici. Il testo in una lingua non controllata e il testo contrassegnato come "non controllare" viene saltato. Per disattivarlo, fai clic su **File > Opzioni > Strumenti di correzione** e deseleziona **Controlla grammatica durante la digitazione**. La scelta vale per tutte le app di JeffOffice.

**Lingue controllate.** La grammatica viene controllata solo in inglese, in tutte le varianti regionali (americano, britannico, canadese, australiano e le altre). Il testo in qualsiasi altra lingua viene controllato solo per l'ortografia e non riceve segni grammaticali. Un paragrafo viene controllato nella lingua con cui è contrassegnato, mentre il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue viene controllato solo l'inglese e il resto viene saltato. Anche il testo che un file contrassegna come "non controllare" viene saltato.

**Scegliere che cosa cercare.** Fai clic su **File > Opzioni > Strumenti di correzione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma errata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizione** e **Stile**. **Stile** è disattivato finché non lo attivi. Non segnala errori, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola e locuzioni prolisse ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** nelle Opzioni. Si applica subito alle linee blu e al riquadro Controllo ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al loro prossimo avvio.

## Impostare la lingua di alcuni testi

Il controllo ortografia verifica ogni testo nella sua lingua. Per cambiare la lingua di alcune parole, selezionale, oppure seleziona un'intera casella di testo o un segnaposto, quindi fai clic su **Imposta lingua di correzione** nel gruppo **Lingua**. Scegli la lingua nell'elenco, quindi fai clic su **OK**. Le parole in quella lingua vengono controllate con il dizionario di quella lingua, e il file conserva la scelta quando lo salvi come presentazione o come presentazione OpenDocument. Il testo a cui non hai assegnato una lingua viene controllato come inglese americano.

## Non controllare alcuni testi

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Selezionali e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. Le linee rosse ondulate scompaiono. Fai di nuovo clic per controllare di nuovo il testo. La casella delle note contiene solo testo semplice, quindi non ha una lingua propria.

## Tradurre il testo

1. Seleziona le parole in una casella di testo o nelle note. Per tradurre tutto il contenuto di una casella di testo, seleziona la casella.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web apre quindi un servizio di traduzione chiamato LibreTranslate, con il tuo testo già inserito. Nulla viene inviato finché non fai clic su **OK**. Puoi cambiare l'indirizzo del servizio nella casella, ad esempio con uno che gestisci tu stesso. Vengono usati solo indirizzi web sicuri (https) e, per questo computer, indirizzi http. Vengono inviate al massimo 5.000 lettere.

## Verificare l'accessibilità

**Verifica accessibilità** apre una piccola finestra che elenca ciò che potrebbe essere difficile da usare per alcune persone: un'immagine o una forma senza testo alternativo, una diapositiva senza titolo, una tabella senza riga di intestazione, una tabella con celle unite, un video senza sottotitoli, una diapositiva con lo stesso titolo di un'altra (un suggerimento), un collegamento il cui testo è il suo stesso indirizzo o non dice nulla sulla destinazione (come "qui" o "leggi di più"), testo difficile da leggere sullo sfondo e oggetti che un lettore dello schermo legge in un ordine diverso da quello in cui appaiono. Il testo deve avere un contrasto di 4,5 a 1 con lo sfondo (3 a 1 per il testo di 18 punti o più, oppure di 14 punti o più in grassetto); quando il testo è sopra un'immagine, un suggerimento ti chiede di verificare il contrasto a occhio. Un lettore dello schermo legge gli oggetti di una diapositiva dal fondo della pila verso l'alto. Fai clic su **Ordine di lettura** per vederli in quell'ordine, e usa **Letto prima** e **Letto dopo** (oppure trascina) per cambiarlo; ogni spostamento è un passo di Annulla. Un lettore dello schermo usa la prima riga di una tabella per dare un nome alle colonne: seleziona un problema di una tabella e fai clic su **Usa la prima riga come intestazione** per attivare la riga di intestazione (un passo di Annulla). Un video senza sottotitoli viene segnalato come suggerimento: i video hanno bisogno dei sottotitoli per chi non sente. Non esiste una correzione con un solo clic; seleziona il video e usa **Inserisci sottotitoli** nella scheda **Riproduzione**, e il suggerimento scompare. Fai clic su un problema per passare alla sua diapositiva e selezionare l'oggetto. Assegna un testo alternativo a un'immagine nel riquadro **Formato immagine**, oppure contrassegnala come decorativa, quindi fai clic su **Verifica di nuovo**. Puoi anche avviare il controllo da **File > Informazioni**.

## Vedi anche

- [Confrontare presentazioni](compare)
- [Commenti](comments)
- [Digitare e formattare il testo](text)
