# Usare gli stili

<!-- keywords: gestire stili, stile bloccato, bloccare uno stile, limitare la formattazione, limitare formattazione, esportare stili, file modello, dotx, nascondere uno stile, stili consigliati, stili, stile, riquadro stili, titolo, normale, creare stile, nuovo stile, modificare stile, cambiare stile, basato su, stile successivo, aggiorna automaticamente, raccolta, stile carattere, stile paragrafo, ctrl+alt+maiusc+s -->

Uno **stile** è un insieme salvato di aspetti: un carattere, una dimensione, la spaziatura e altro. Quando assegni uno stile a un paragrafo, il paragrafo acquisisce tutti i suoi aspetti in una sola volta. Quando cambi lo stile, cambiano anche tutti i paragrafi che lo usano. Per questo gli stili mantengono ordinati i documenti lunghi.

JeffWrite ha due tipi di stili:

- Uno **stile paragrafo** imposta l'aspetto di interi paragrafi. **Titolo 1** e **Normale** sono stili paragrafo.
- Uno **stile carattere** imposta l'aspetto delle parole selezionate. **Forte** è uno stile carattere.

## Assegnare uno stile al testo

1. Fai clic in un paragrafo oppure seleziona le parole che vuoi modificare.
2. Fai clic su uno stile nella raccolta **Stili** della scheda **Home**. Oppure premi **Ctrl+Maiusc+N** per Normale, oppure **Ctrl+Alt+1**, **Ctrl+Alt+2** o **Ctrl+Alt+3** per Titolo 1, 2 o 3.

Uno stile paragrafo si applica a ogni paragrafo che tocchi. Uno stile carattere si applica alle parole selezionate.

## Aprire il riquadro Stili

Fai clic sulla piccola freccia nell'angolo del gruppo **Stili**, oppure premi **Ctrl+Alt+Maiusc+S**. Il riquadro **Stili** si apre a destra. Elenca gli stili del documento, ciascuno con il proprio aspetto.

- Lo stile nella posizione del cursore è in grassetto, con una freccia prima del nome.
- Fai clic su uno stile per assegnarlo alla selezione.
- Usa l'elenco sotto gli stili per scegliere quali mostrare:
  - **In uso** mostra solo gli stili usati nel testo.
  - **Tutti gli stili** mostra ogni stile, anche quelli che il documento non ha ancora. Uno stile viene aggiunto al documento la prima volta che lo usi.
  - **Consigliati** mostra gli stili pensati per essere visti. Uno stile nascosto finché non viene usato compare qui una volta che lo usi.
- Fai clic su **Gestisci…** sotto l'elenco per scegliere, per ogni stile, se è **Consigliato** (mostrato nella raccolta), se è **Nascosto** (escluso da tutti gli elenchi) e il suo **Ordine negli elenchi**. Un numero più basso compare per primo. Fai clic su **OK**: tutte le modifiche avvengono in un unico passaggio, quindi un solo **Annulla** le ripristina.
- In **Gestisci…**, fai clic su **Elimina** per rimuovere uno stile creato da te. I paragrafi che lo avevano tornano a Normale, e gli stili basati su di esso vengono basati su ciò su cui era basato. Gli stili forniti con JeffWrite non possono essere eliminati. L'eliminazione è un unico passaggio, quindi **Annulla** ripristina lo stile.
- In **Gestisci…**, fai clic su **Importa da file…** per copiare stili da un altro documento. Scegli il file, seleziona gli stili che ti servono e fai clic su **OK**. Uno stile che ne richiede un altro porta con sé anche quest'ultimo. Gli stili già presenti restano invariati, a meno che tu non selezioni **Sostituisci gli stili con lo stesso nome**. L'importazione si annulla con un solo passaggio.
- In **Gestisci…**, fai clic su **Esporta in file…** per salvare alcuni stili in un file modello (un file **.dotx**). Seleziona gli stili che ti servono, fai clic su **OK** e scegli dove salvarlo. Un altro documento può prenderli da quel file con **Importa da file…**. Uno stile che ne richiede un altro porta con sé anche quest'ultimo.
- Seleziona **Bloccato** per uno stile per bloccarlo. Poi seleziona **Limita la formattazione agli stili non bloccati** per attivare il limite. Mentre il limite è attivo, nessuno può formattare il testo a mano, ad esempio con **Grassetto** o **Centra**, e nessuno può usare uno stile bloccato. Possono comunque usare tutti gli altri stili. La barra di stato lo segnala quando un comando viene rifiutato. Deseleziona la casella per disattivare il limite. Se il documento è protetto con una password, interrompi prima quella protezione, in **Revisione > Limita modifiche**. Il limite e i blocchi vengono salvati nel file.
- Sotto l'elenco, la casella **Ordina** dispone gli stili in ordine. **Alfabetico** ordina per nome. **Come consigliato** mette per primi gli stili più utili. **Per tipo** elenca prima gli stili paragrafo, poi gli stili collegati, poi gli stili carattere. **In base a** raggruppa ogni stile con lo stile da cui deriva.

## Creare uno stile dal testo

1. Formatta un paragrafo come vuoi. Ad esempio, rendilo in grassetto, centralo e imposta la dimensione.
2. Fai clic su **Nuovo stile** in fondo al riquadro, oppure fai clic su **Crea stile** nella scheda **Home**.
3. Digita un **Nome**. Un nome può contenere fino a 255 caratteri e non può essere il nome di un altro stile.
4. Controlla le altre opzioni e fai clic su **OK**.

Il nuovo stile viene salvato con il documento, e il paragrafo lo acquisisce. Un solo **Annulla** (Ctrl+Z) rimuove lo stile.

## Modificare uno stile

1. Fai clic in un paragrafo che ha lo stile. Oppure fai clic sullo stile nel riquadro.
2. Fai clic su **Modifica** in fondo al riquadro, oppure fai clic su **Modifica stile** nella scheda **Home**.
3. Cambia ciò che ti serve e fai clic su **OK**.

La finestra di dialogo offre queste opzioni:

- **Nome** è il nome dello stile. Uno stile fornito con JeffWrite mantiene il suo nome.
- **Basato su** è lo stile da cui parte questo. Il nuovo stile somiglia a quello, tranne per ciò che cambi. Se in seguito modifichi lo stile di base, anche questo lo seguirà.
- **Stile per il paragrafo seguente** è lo stile che riceve il paragrafo successivo quando premi **Invio** alla fine di questo. Di solito a un titolo segue Normale.
- **Carattere**, **dimensione**, **Grassetto**, **Corsivo**, **Sottolineato** e **colore** impostano l'aspetto delle parole.
- **Allineamento**, **rientri**, **spaziatura** e **interlinea** impostano l'aspetto dei paragrafi.
- **Aggiungi alla raccolta stili** inserisce lo stile nella raccolta della scheda **Home**.
- **Aggiorna automaticamente** fa seguire allo stile la tua formattazione. Quando modifichi un paragrafo che ha questo stile, lo stile si adegua, e con lui tutti gli altri paragrafi che lo usano.

Quando modifichi uno stile, cambiano i paragrafi che lo usano. Se hai dato a un paragrafo un aspetto proprio, come una parola in rosso, quell'aspetto resta.

Se fai clic su **OK** senza aver cambiato nulla, il documento non viene modificato.

## Aggiornare uno stile in base al testo

Formatta un paragrafo come vuoi. Poi fai clic su **Aggiorna stile in base alla selezione** nella scheda **Home**. Lo stile di quel paragrafo acquisisce le modifiche fatte, e ogni paragrafo con quello stile le segue.

## Vedi anche

- [Cambiare l'aspetto del testo](font)
- [Copiare un aspetto con Copia formato](format-painter)
- [Usare JeffWrite con la tastiera](keyboard)
