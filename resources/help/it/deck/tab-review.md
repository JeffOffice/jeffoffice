# Scheda Revisione
<!-- keywords: scheda revisione, barra multifunzione, strumenti di correzione, controllo ortografia, ortografia, thesaurus, sinonimi, conteggio parole, commenti, nuovo commento, elimina commento, precedente, successivo, mostra commenti, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, grammatica, controllo grammaticale, stile, linea blu, linea punteggiata, verifica grammatica digitazione -->

La scheda **Revisione** controlla le tue parole, le traduce e gestisce i commenti.

- **Strumenti di correzione** contiene **Controllo ortografia** (F7), **Thesaurus** (Maiusc+F7), **Conteggio parole** e **Verifica accessibilità**. Vedi [Digitare e formattare il testo](text).
- **Lingua** contiene **Traduci**, **Imposta lingua di correzione** e **Non controllare ortografia**. Vedi sotto.
- **Commenti** contiene **Nuovo commento** (Ctrl+Alt+M), **Elimina** con il relativo menu, **Precedente**, **Successivo** e **Mostra commenti**. Vedi [Commenti](comments).

## Verifica la grammatica durante la digitazione

JeffDeck cerca anche chiari errori grammaticali e di stile nel testo delle diapositive e delle note del relatore. Un problema viene sottolineato con una linea blu punteggiata, mentre una parola errata mantiene la sottolineatura rossa ondulata. Fai clic con il pulsante destro del mouse sul problema per vedere di cosa si tratta e scegliere un suggerimento, oppure scegli **Ignora questo problema** o **Ignora tutto**. **Controllo ortografia** (F7) passa ai problemi grammaticali delle diapositive dopo quelli ortografici. Il testo in una lingua non controllata e il testo contrassegnato come "non controllare" vengono saltati. Per disattivare la funzione, fai clic su **File > Opzioni > Strumenti di correzione** e deseleziona **Verifica la grammatica durante la digitazione**. È un'unica impostazione per tutte le app di JeffOffice.

**Lingue controllate.** Inglese, francese, tedesco e spagnolo, ciascuno con regole proprie. Il francese controlla lo spazio senza interruzione prima di ; : ! ? e all'interno di « », l'apostrofo in parole come "l'ami" e "qu'il", e le confusioni come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole dei sostantivi comuni. Lo spagnolo cerca un ¿ o un ¡ mancante, gli accenti sulle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, mentre il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata e le altre vengono saltate.

**Scegli cosa controllare.** Fai clic su **File > Opzioni > Strumenti di correzione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma errata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini superflui o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizioni** e **Stile**. **Stile** è disattivato finché non lo attivi. Non segnala errori veri e propri, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola e locuzioni prolisse ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** in Opzioni. Si applica subito alle linee blu e al riquadro Controllo ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al successivo avvio.

## Imposta la lingua di alcuni testi

Il controllo ortografico verifica ogni porzione di testo nella propria lingua. Per cambiare la lingua di alcune parole, selezionale, oppure seleziona un'intera casella di testo o un segnaposto, quindi fai clic su **Imposta lingua di correzione** nel gruppo **Lingua**. Scegli la lingua nell'elenco, quindi fai clic su **OK**. Le parole in quella lingua vengono controllate con il dizionario di quella lingua, e il file conserva la scelta quando lo salvi come presentazione o come presentazione OpenDocument. Il testo a cui non hai assegnato una lingua viene controllato in inglese americano.

## Non controllare alcuni testi

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Selezionali e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. Le linee rosse ondulate scompaiono. Fai di nuovo clic per controllare di nuovo il testo. La casella delle note contiene solo testo normale, quindi non ha una lingua propria.

## Traduci testo

1. Seleziona le parole in una casella di testo o nelle note. Per tradurre tutto il contenuto di una casella di testo, seleziona la casella.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web si apre quindi con il servizio di traduzione LibreTranslate e il tuo testo. Non viene inviato nulla finché non fai clic su **OK**. Puoi cambiare l'indirizzo del servizio nella casella, ad esempio con uno che gestisci tu. Vengono usati solo indirizzi web sicuri (https) e indirizzi http di questo computer. Vengono inviate al massimo 5.000 lettere.

## Verifica accessibilità

**Verifica accessibilità** apre una piccola finestra che elenca ciò che può essere difficile da usare per alcune persone: un'immagine o una forma senza testo alternativo, una diapositiva senza titolo, una tabella senza riga di intestazione, un video senza sottotitoli, un testo difficile da leggere sullo sfondo e oggetti che uno screen reader legge in un ordine diverso da quello suggerito dalla loro posizione. Il testo deve avere un contrasto di 4,5 a 1 con lo sfondo (3 a 1 per il testo di 18 punti o più, o di 14 punti o più in grassetto). Quando il testo è sopra un'immagine, un suggerimento ti chiede di controllare il contrasto a occhio. Uno screen reader legge gli oggetti di una diapositiva dal fondo della pila verso l'alto. Fai clic su **Ordine di lettura** per vederli in quell'ordine, e usa **Letto prima** e **Letto dopo** (oppure trascina) per cambiarlo; ogni spostamento equivale a un passaggio di Annulla. Uno screen reader usa la prima riga di una tabella per dare un nome alle sue colonne: seleziona un problema della tabella e fai clic su **Usa la prima riga come intestazione** per attivare la riga di intestazione (un passaggio di Annulla). Un video senza sottotitoli viene segnalato come suggerimento: i video necessitano di sottotitoli per chi non sente. Non esiste una correzione con un clic; seleziona il video e usa **Inserisci sottotitoli** nella scheda **Riproduzione**, e il suggerimento scompare. Fai clic su un problema per andare alla sua diapositiva e selezionare l'oggetto. Assegna un testo alternativo a un'immagine nel riquadro **Formato immagine**, oppure contrassegnala come decorativa, quindi fai clic su **Verifica di nuovo**. Puoi avviare il controllo anche da **File > Informazioni**.

## Argomenti correlati

- [Commenti](comments)
- [Digitare e formattare il testo](text)
