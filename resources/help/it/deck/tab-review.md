# Scheda Revisione

<!-- keywords: scheda revisione, confronta, riquadro revisioni, accetta, rifiuta, barra multifunzione, correzione, controllo ortografia, ortografia, thesaurus, conteggio parole, commenti, nuovo commento, elimina commento, precedente, successivo, mostra commenti, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, grammatica, controllo grammaticale, stile, linea blu, linea punteggiata, controlla grammatica durante la digitazione, verifica accessibilità, accessibilità, testo alternativo, contrasto, lettore di schermo -->

La scheda **Revisione** controlla le parole, le traduce e gestisce i commenti.

- **Strumenti di correzione** contiene **Controllo ortografia** (F7), **Thesaurus** (Maiusc+F7), **Conteggio parole** e **Verifica accessibilità**. Vedi [Digitare e formattare il testo](text).
- **Lingua** contiene **Traduci**, **Imposta lingua di correzione** e **Non controllare ortografia**. Vedi sotto.
- **Confronta** contiene **Confronta**, **Accetta**, **Rifiuta**, **Precedente**, **Successivo** e **Riquadro revisioni**. Vedi [Confrontare presentazioni](compare).
- **Commenti** contiene **Nuovo commento** (Ctrl+Alt+M), **Elimina** con il suo menu, **Precedente**, **Successivo** e **Mostra commenti**. Vedi [Commenti](comments).

## Controllare la grammatica durante la digitazione

JeffDeck cerca anche errori evidenti di grammatica e di stile nel testo, nelle diapositive e nelle note del relatore. Un problema viene sottolineato con una linea blu punteggiata, mentre una parola scritta male mantiene la sua linea rossa ondulata. Fai clic con il pulsante destro sul problema per vedere cosa non va e scegliere un suggerimento, oppure scegli **Ignora questo problema** o **Ignora tutto**. **Controllo ortografia** (F7) passa ai problemi grammaticali delle diapositive dopo quelli ortografici. Il testo in una lingua non controllata e il testo contrassegnato come "non controllare" viene saltato. Per disattivare la funzione, fai clic su **File > Opzioni > Strumenti di correzione** e deseleziona **Controlla grammatica durante la digitazione**. È una scelta unica per tutte le app di JeffOffice.

**Lingue controllate.** Sono controllate inglese, francese, tedesco e spagnolo, ciascuna con regole proprie. Il francese controlla lo spazio senza interruzione prima di ; : ! ? e all'interno di « », l'apostrofo in parole come "l'ami" e "qu'il", e le confusioni come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole dei nomi comuni. Lo spagnolo cerca un ¿ o un ¡ mancanti, gli accenti nelle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, e il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata e le altre vengono saltate.

**Scegliere che cosa controllare.** Fai clic su **File > Opzioni > Strumenti di correzione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma errata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizioni** e **Stile**. Lo **Stile** è disattivato finché non lo attivi. Non segnala errori veri, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni ridondanti ("in order to", "due to the fact that"). La scelta resta quando fai clic su **OK** nelle Opzioni. Si applica subito alle linee blu e al riquadro Controllo ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice alla loro prossima apertura.

## Impostare la lingua di alcuni testi

Il controllo ortografia verifica ogni testo nella sua lingua. Per cambiare la lingua di alcune parole, selezionale, oppure seleziona una casella di testo o un segnaposto intero, quindi fai clic su **Imposta lingua di correzione** nel gruppo **Lingua**. Scegli la lingua nell'elenco, quindi fai clic su **OK**. Le parole in quella lingua vengono controllate con il dizionario di quella lingua, e il file conserva la scelta quando lo salvi come presentazione o come presentazione OpenDocument. Il testo che non hai contrassegnato viene controllato in inglese americano.

## Non controllare alcuni testi

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Selezionali e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. Le linee rosse ondulate scompaiono. Fai di nuovo clic sul comando per controllare di nuovo il testo. La casella delle note contiene solo testo semplice, quindi non ha una lingua propria.

## Tradurre il testo

1. Seleziona le parole in una casella di testo o nelle note. Per tradurre tutto il testo di una casella, seleziona la casella.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web apre quindi un servizio di traduzione chiamato LibreTranslate, con il tuo testo già inserito. Non viene inviato nulla finché non fai clic su **OK**. Puoi cambiare l'indirizzo del servizio nella casella, ad esempio con uno che gestisci tu. Vengono usati solo indirizzi web sicuri (https) e indirizzi http di questo computer. Vengono inviate al massimo 5.000 lettere.

## Verificare l'accessibilità

**Verifica accessibilità** apre una piccola finestra che elenca ciò che può risultare difficile da usare per alcune persone: un'immagine o una forma senza testo alternativo, una diapositiva senza titolo, una tabella senza riga di intestazione, una tabella con celle unite, un video senza sottotitoli, una diapositiva il cui titolo è uguale a quello di un'altra diapositiva (un suggerimento), un collegamento le cui parole sono il suo indirizzo o non dicono dove porta (come "qui" o "leggi di più"), testo difficile da leggere sullo sfondo, e oggetti che un lettore di schermo legge in un ordine diverso da quello della loro posizione. Il testo deve avere un contrasto di 4,5 a 1 con lo sfondo (3 a 1 per il testo di 18 punti o più, o di 14 punti o più in grassetto). Quando il testo è sopra un'immagine, un suggerimento ti chiede di controllare il contrasto a occhio. Un lettore di schermo legge gli oggetti di una diapositiva dal fondo della pila verso l'alto. Fai clic su **Ordine di lettura** per vederli in quell'ordine, e usa **Letto prima** e **Letto dopo** (oppure trascina) per cambiarlo; ogni spostamento è un passaggio di **Annulla**. Un lettore di schermo usa la prima riga di una tabella per dare un nome alle colonne: seleziona un problema della tabella e fai clic su **Usa la prima riga come intestazione** per attivare la riga di intestazione (un passaggio di **Annulla**). Un video senza sottotitoli viene segnalato come suggerimento, perché i video devono avere i sottotitoli per le persone che non sentono. Non esiste una correzione con un solo clic: seleziona il video e usa **Inserisci sottotitoli** nella scheda **Riproduzione**, e il suggerimento scompare. Fai clic su un problema per andare alla sua diapositiva e selezionare l'oggetto. Assegna un testo alternativo a un'immagine nel riquadro **Formato immagine**, oppure contrassegnala come decorativa, quindi fai clic su **Verifica di nuovo**. Puoi anche avviare la verifica da **File > Informazioni**.

## Collegamenti

- [Confrontare presentazioni](compare)
- [Commenti](comments)
- [Digitare e formattare il testo](text)
