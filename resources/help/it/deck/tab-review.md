# Scheda Revisione

<!-- keywords: scheda revisione, confronta, riquadro revisione, accetta, rifiuta, barra multifunzione, correzione, ortografia, thesaurus, conteggio parole, commenti, nuovo commento, elimina commento, precedente, successivo, mostra commenti, lingua, lingua di correzione, imposta lingua, non controllare ortografia, traduci, traduzione, libretranslate, grammatica, controllo grammatica, stile, linea blu, linea puntinata, controlla grammatica durante la digitazione, revisioni -->

La scheda **Revisione** controlla le parole, le traduce e gestisce i commenti.

- **Strumenti di correzione** comprende **Controllo ortografia** (F7), **Thesaurus** (Maiusc+F7), **Conteggio parole** e **Verifica accessibilità**. Vedi [Digitare e formattare il testo](text).
- **Lingua** comprende **Traduci**, **Imposta lingua di correzione** e **Non controllare ortografia**. Vedi più avanti.
- **Confronta** comprende **Confronta**, **Accetta**, **Rifiuta**, **Precedente**, **Successivo** e **Riquadro revisione**. Vedi [Confrontare presentazioni](compare).
- **Commenti** comprende **Nuovo commento** (Ctrl+Alt+M), **Elimina** con il suo menu, **Precedente**, **Successivo** e **Mostra commenti**. Vedi [Commenti](comments).

## Controllare la grammatica durante la digitazione

JeffDeck cerca anche chiari errori di grammatica e di stile nel testo, sulle diapositive e nelle note del relatore. Un problema viene sottolineato con una linea blu puntinata, mentre una parola scritta in modo errato mantiene la sua linea rossa ondulata. Fai clic con il pulsante destro sul problema per vedere cosa non va e scegliere un suggerimento, oppure scegli **Ignora questo problema** o **Ignora tutto**. **Controllo ortografia** (F7) passa ai problemi di grammatica delle diapositive dopo quelli di ortografia. Il testo in una lingua non controllata e il testo contrassegnato come "non controllare" vengono saltati. Per disattivare la funzione, fai clic su **File > Opzioni > Strumenti di correzione** e deseleziona **Controlla grammatica durante la digitazione**. È una scelta unica per tutte le app di JeffOffice.

**Lingue controllate.** Inglese, francese, tedesco e spagnolo, ciascuno con regole proprie. Il francese controlla lo spazio unificatore prima di ; : ! ? e all'interno di « », l'apostrofo in parole come "l'ami" e "qu'il", e le confusioni come "a" e "à". Il tedesco controlla "das" e "dass", "seid" e "seit", "wider" e "wieder", la virgola prima di "dass" e "weil", e le maiuscole nei sostantivi comuni. Lo spagnolo controlla la mancanza di ¿ o ¡, gli accenti nelle parole interrogative, "a ver" e "haber", e "del" e "al". Un paragrafo viene controllato nella lingua con cui è contrassegnato, mentre il testo senza lingua viene controllato come inglese. In un paragrafo con più lingue si usa la prima lingua controllata e le altre vengono saltate.

**Scegliere che cosa controllare.** Fai clic su **File > Opzioni > Strumenti di correzione > Impostazioni grammatica** per scegliere i tipi di problemi: **Grammatica** (verbi che non concordano, forma errata di un verbo, "un" e "una"), **Punteggiatura** (spazi, virgole e trattini in eccesso o mancanti), **Maiuscole**, **Parole confuse** (parole che suonano simili), **Ripetizione** e **Stile**. Lo **Stile** è disattivato finché non lo attivi. Non segnala errori veri, ma suggerisce: frasi di più di 40 parole, la forma passiva, tre frasi di seguito che iniziano con la stessa parola, e locuzioni prolisse ("al fine di", "a causa del fatto che"). La scelta viene mantenuta quando fai clic su **OK** nelle Opzioni. Si applica subito alle linee blu e al riquadro Controllo ortografia (F7) nelle finestre aperte di questa app, e alle altre app di JeffOffice al loro successivo avvio.

## Impostare la lingua di alcune parti di testo

Il controllo ortografia verifica ogni porzione di testo nella propria lingua. Per cambiare la lingua di alcune parole, selezionale, oppure seleziona un'intera casella di testo o un segnaposto, quindi fai clic su **Imposta lingua di correzione** nel gruppo **Lingua**. Scegli la lingua nell'elenco, quindi fai clic su **OK**. Le parole in quella lingua vengono controllate con il dizionario di quella lingua, e il file mantiene la scelta quando lo salvi come presentazione o come presentazione OpenDocument. Il testo che non hai contrassegnato viene controllato in inglese americano.

## Non controllare alcuni testi

Alcuni testi non devono essere controllati: un codice, un nome o una riga in una lingua inventata. Selezionali e fai clic su **Non controllare ortografia** nel gruppo **Lingua**. Le linee rosse ondulate scompaiono. Fai di nuovo clic per controllare nuovamente il testo. La casella delle note contiene solo testo semplice, quindi non ha una lingua propria.

## Tradurre il testo

1. Seleziona le parole in una casella di testo o nelle note. Per tradurre tutto il testo di una casella, seleziona la casella.
2. Fai clic su **Traduci** nel gruppo **Lingua**.
3. Scegli la lingua in cui tradurre.
4. Fai clic su **OK**.

Il browser web apre quindi un servizio di traduzione chiamato LibreTranslate, con il testo già inserito. Nulla viene inviato finché non fai clic su **OK**. Puoi modificare l'indirizzo del servizio nella casella, ad esempio scegliendone uno che gestisci tu stesso. Vengono usati solo indirizzi web sicuri (https) e indirizzi http di questo computer. Vengono inviate al massimo 5.000 lettere.

## Verificare l'accessibilità

**Verifica accessibilità** apre una piccola finestra che elenca ciò che può essere difficile da usare per alcune persone: un'immagine o una forma senza testo alternativo, una diapositiva senza titolo, una tabella senza riga di intestazione, un video senza sottotitoli, testo difficile da leggere sullo sfondo, e oggetti che uno screen reader legge in un ordine diverso da quello suggerito dalla loro posizione. Il testo deve avere un contrasto di 4,5 a 1 con lo sfondo (3 a 1 per il testo di 18 punti o più, o di 14 punti o più in grassetto); quando il testo è sopra un'immagine, un suggerimento invita a verificare il contrasto a occhio. Uno screen reader legge gli oggetti di una diapositiva dal fondo della pila verso il primo piano. Fai clic su **Ordine di lettura** per vederli in quell'ordine, e usa **Letto prima** e **Letto dopo** (oppure trascina) per cambiarlo; ogni spostamento equivale a un passo di **Annulla**. Uno screen reader usa la prima riga di una tabella per dare un nome alle sue colonne: seleziona un problema della tabella e fai clic su **Usa la prima riga come intestazione** per attivare la riga di intestazione (un passo di Annulla). Un video senza sottotitoli viene segnalato come suggerimento: i video richiedono sottotitoli per le persone che non possono sentire. Non esiste una correzione con un solo clic; seleziona il video e usa **Inserisci sottotitoli** nella scheda **Riproduzione**, e il suggerimento scompare. Fai clic su un problema per passare alla sua diapositiva e selezionare l'oggetto. Assegna un testo alternativo a un'immagine nel riquadro **Formato immagine**, oppure contrassegnala come decorativa, quindi fai clic su **Verifica di nuovo**. Puoi anche avviare il controllo da **File > Informazioni**.

## Vedi anche

- [Confrontare presentazioni](compare)
- [Commenti](comments)
- [Digitare e formattare il testo](text)
