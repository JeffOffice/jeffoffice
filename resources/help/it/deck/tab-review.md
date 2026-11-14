# Scheda Revisione
<!-- keywords: scheda revisione, barra multifunzione, correzione bozze, controllo ortografia, ortografia, sinonimi, thesaurus, conteggio parole, commenti, nuovo commento, elimina commento, precedente, successivo, mostra commenti, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, grammatica, controllo grammatica, stile, linea blu, linea punteggiata, controlla grammatica durante la digitazione -->

La scheda **Revisione** controlla le tue parole, le traduce e gestisce i commenti.

- **Strumenti di correzione** contiene **Controllo ortografia** (F7), **Thesaurus** (Maiusc+F7), **Conteggio parole** e **Verifica accessibilità**. Vedi [Digitare e formattare il testo](text).
- **Lingua** contiene **Traduci**, **Imposta lingua di correzione** e **Non controllare ortografia**. Vedi sotto.
- **Commenti** contiene **Nuovo commento** (CTRL+ALT+M), **Elimina** con il relativo menu, **Precedente**, **Successivo** e **Mostra commenti**. Vedi [Commenti](comments).

## Controllare la grammatica durante la digitazione

JeffDeck cerca anche errori chiari di grammatica e di stile nel testo, nelle diapositive e nelle note del relatore. Un problema viene evidenziato con una linea blu punteggiata, mentre una parola errata mantiene la sua linea rossa ondulata. Fai clic con il pulsante destro sul problema per vedere che cosa non va e scegliere un suggerimento, oppure scegli **Ignora questo problema** o **Ignora tutto**. **Controllo ortografia** (F7) passa poi ai problemi di grammatica nelle diapositive, dopo quelli di ortografia. Il testo in una lingua non controllata e il testo contrassegnato come "non controllare" vengono saltati. Per disattivarlo, fai clic su **File > Opzioni > Strumenti di correzione** e deseleziona **Controlla grammatica durante la digitazione**. La scelta vale per tutte le app di JeffOffice.

**Lingue controllate.** Inglese, francese, tedesco e spagnolo, ciascuno con regole proprie. Il francese controlla lo spazio senza interruzione prima di ; : ! ? e dentro « », l'apostrofo in parole come "l'ami" e "qu'il", e le confusioni come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole dei nomi comuni. Lo spagnolo cerca una ¿ o una ¡ mancante, gli accenti nelle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, mentre il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata e le altre vengono saltate.

**Scegliere che cosa controllare.** Fai clic su **File > Opzioni > Strumenti di correzione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi non concordati, forma errata di un verbo, "a" e "an"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizione** e **Stile**. **Stile** è disattivato finché non lo attivi. Non indica errori veri e propri, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni prolisse ("in order to", "due to the fact that"). La scelta viene mantenuta quando fai clic su **OK** in Opzioni. Si applica subito alle linee blu e al riquadro Controllo ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al successivo avvio.

## Verifica la grammatica durante la digitazione

Il controllo ortografia verifica ogni testo nella sua lingua. Per cambiare la lingua di alcune parole, selezionale oppure seleziona un'intera casella di testo o un segnaposto, quindi fai clic su **Imposta lingua di correzione** nel gruppo **Lingua**. Scegli la lingua nell'elenco e fai clic su **OK**. Le parole in quella lingua vengono controllate con il dizionario di quella lingua, e il file conserva la scelta quando lo salvi come presentazione o come presentazione OpenDocument. Il testo non contrassegnato viene controllato come inglese americano.

## Non controllare alcuni testi

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Selezionali e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. Le linee rosse ondulate scompaiono. Fai di nuovo clic sul pulsante per controllare di nuovo il testo. La casella delle note contiene solo testo normale, quindi non ha una lingua propria.

## Traduci testo

1. Seleziona le parole in una casella di testo o nelle note. Per tradurre tutto il contenuto di una casella di testo, selezionala.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web apre quindi un servizio di traduzione chiamato LibreTranslate, con il tuo testo già inserito. Nulla viene inviato finché non fai clic su **OK**. Puoi cambiare l'indirizzo del servizio nella casella, ad esempio per usarne uno gestito da te. Vengono usati solo indirizzi web sicuri (https) e indirizzi http per questo computer. Vengono inviate al massimo 5.000 lettere.

## Verifica accessibilità

**Verifica accessibilità** apre una piccola finestra che elenca ciò che può essere difficile da usare per alcune persone: un'immagine o una forma senza testo alternativo, una diapositiva senza titolo, una tabella senza riga di intestazione, un video senza sottotitoli, testo difficile da leggere sullo sfondo e oggetti che un lettore di schermo legge in un ordine diverso da quello in cui appaiono. Il testo deve avere un contrasto di 4,5 a 1 con lo sfondo (3 a 1 per il testo di 18 punti o più, oppure di 14 punti o più in grassetto); quando il testo è sopra un'immagine, un suggerimento ti chiede di verificare il contrasto a occhio. Un lettore di schermo legge gli oggetti di una diapositiva dal fondo della pila verso la parte superiore. Fai clic su **Ordine di lettura** per vederli in quell'ordine e usa **Letto prima** e **Letto dopo** (oppure trascina) per modificarlo; ogni spostamento è un passaggio di Annulla. Un lettore di schermo usa la prima riga di una tabella per dare un nome alle colonne: seleziona un problema di tabella e fai clic su **Usa la prima riga come intestazione** per attivare la riga di intestazione (un passaggio di Annulla). Un video senza sottotitoli compare come suggerimento, perché i video necessitano di sottotitoli per le persone che non sentono. Non esiste una correzione con un solo clic; seleziona il video e usa **Inserisci sottotitoli** nella scheda **Riproduzione**, e il suggerimento scompare. Fai clic su un problema per passare alla sua diapositiva e selezionare l'oggetto. Assegna all'immagine un testo alternativo nel riquadro **Formato immagine**, oppure contrassegnala come decorativa, quindi fai clic su **Verifica di nuovo**. Puoi avviare la verifica anche da **File > Informazioni**.

## Argomenti correlati

- [Commenti](comments)
- [Digitare e formattare il testo](text)
