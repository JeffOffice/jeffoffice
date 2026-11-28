# Beveiliging: waar een bestand naar kan verwijzen

<!-- keywords: vertrouwenscentrum, beveiliging, macro's, gekoppelde bestanden, koppelingen, gekoppelde afbeeldingen, gekoppelde media, internet, gedownload, alleen-lezen, beveiligd, externe koppelingen, programma's, veilig, toegankelijkheid, veilige instellingen -->

Een bestand kan naar andere dingen verwijzen: bestanden, webpagina's, video's en zelfs programma's. Op de pagina **Beveiliging** bepaalt u wat JeffCrunch daarmee doet. Klik op **Bestand > Opties** en daarna op het tabblad **Beveiliging**. Klik op **OK** om uw wijzigingen te bewaren, of op **Annuleren** om ze weg te gooien.

Deze keuzes gelden voor alle JeffOffice-apps: wijzigt u er een in één app, dan verandert deze in alle apps. Een rij voor iets wat een app niet heeft, staat op **Dit is niet van toepassing** en is grijs.

## Macro's

Macro's zijn kleine programma's die in een bestand zijn opgeslagen. JeffCrunch bewaart ze in het bestand, zodat ze er nog zijn wanneer u opslaat, en het **voert ze nooit uit**. Er is geen instelling die ze uitvoert.

## Gekoppelde bestanden en werkmapkoppelingen

Dit gaat over de andere bestanden die een werkmap leest, zoals de werkmappen waaruit hij getallen haalt, die **koppelingen** heten, de bestanden die zijn query's lezen, en de bestanden waarop zijn draaitabellen zijn gebaseerd. De keuze bepaalt wat JeffCrunch doet wanneer het een werkmap opent die er een aantal leest, en wanneer u op **Vernieuwen** of **Alles vernieuwen** klikt bij de query's en draaitabellen.

- **Vragen** is de gebruikelijke keuze. Voordat JeffCrunch een van die bestanden leest, toont het het volledige pad van het bestand en vraagt het of het mag. Klik alleen op **Bestand lezen** als u de bron van de werkmap vertrouwt. **Niet lezen**, de standaardknop, leest niets en laat de werkmap ongewijzigd. Deze keuze vraagt elke keer dat u een werkmap opent, en daarna één keer per bestand. Nadat u voor een bestand op **Bestand lezen** hebt geklikt, vraagt JeffCrunch in dat venster niet opnieuw naar datzelfde bestand, totdat u het venster sluit.
- **Bijwerken** leest een bestand zonder te vragen, alleen als u al hebt aangegeven dat dezelfde werkmap dat specifieke bestand mag lezen. Voor elk ander bestand, ook een bestand dat genoemd wordt in een werkmap die u zojuist hebt ontvangen, vraagt het eerst op dezelfde manier.
- **Nooit bijwerken** leest niets wanneer een werkmap wordt geopend en toont geen vraag. De getallen die het bestand heeft opgeslagen, blijven zoals ze zijn. Wanneer u zelf op **Vernieuwen** klikt, is dat een verzoek, en dan toont JeffCrunch de vraag wel. Nadat u op **Bestand lezen** hebt geklikt, vraagt het niet opnieuw naar datzelfde bestand in dat venster.

## Wat wordt onthouden

Wanneer u op **Bestand lezen** of **Bestanden lezen** klikt, onthoudt JeffCrunch dat koppel: deze werkmap en dat bestand, elk met zijn volledige pad. Alleen het koppel wordt onthouden. Als de werkmap wordt verplaatst of hernoemd, of zo wordt bewerkt dat hij een ander bestand leest, of als het bestand wordt vervangen door een snelkoppeling naar een ander bestand, vraagt JeffCrunch opnieuw. Een werkmap die u nog nooit hebt opgeslagen, heeft geen pad, dus over zo'n werkmap wordt na het sluiten niets onthouden.

De koppels worden alleen bewaard in uw eigen instellingen op deze computer, nooit in een werkmap. Daardoor kan een werkmap er geen toevoegen, en krijgt niemand die een werkmap van u ontvangt de koppels mee. De instellingen bewaren een versleutelde vorm van elk pad, niet het pad zelf. Er worden maximaal 2.000 koppels bewaard; als er meer zijn, wordt het koppel dat het langst geleden is gebruikt, het eerst vergeten.

Op **Opties > Beveiliging** ziet u hoeveel koppels er worden onthouden (**Onthouden goedkeuringen**). Klik op **Alles vergeten** om ze allemaal in één keer te vergeten. Er wordt geen werkmap gewijzigd, en elke werkmap vraagt de volgende keer opnieuw.

Een werkmap die in de eigen **Opstartvraag** (**Gegevens > Koppelingen bewerken**) "nooit" aangeeft, wordt altijd met rust gelaten. Een werkmap op een andere computer wordt nooit gelezen wanneer een bestand wordt geopend, wat u ook kiest. U kunt zelf altijd **Waarden bijwerken** gebruiken in **Koppelingen bewerken**.

## Gekoppelde video's en geluiden

Dit geldt niet voor JeffCrunch: het heeft geen gekoppelde video's of geluiden om af te spelen. De rij is grijs.

## Bestanden van internet of van andere computers

Een bestand dat u hebt gedownload, of dat op een andere computer in uw netwerk staat, is misschien niet veilig. Deze keuze bepaalt hoe JeffCrunch het opent.

- **Eerst als alleen-lezen openen** is de gebruikelijke keuze. Het bestand opent zodat u het kunt lezen, maar u kunt het niet wijzigen. Een balk onder het bestand geeft dat aan. Klik op de balk op **Bewerken inschakelen** om alleen dat bestand te wijzigen (tot u het programma sluit). Wanneer u het daarna op dezelfde plek opslaat, wordt de downloadmarkering van het bestand verwijderd. Of gebruik **Opslaan als** om een kopie te maken, of wijzig deze instelling naar de volgende keuze.
- **Normaal openen** opent elk bestand op dezelfde manier.

Een bestand dat u opent met **Kopie openen**, **Openen en herstellen** of **Openen als alleen-lezen** vanuit het scherm **Bestand**, wordt zo geopend, wat deze instelling ook zegt.

## Koppelingen die een programma starten

Een koppeling die een programma of script zou starten (een bestand dat eindigt op .exe, .bat, .sh enzovoort) wordt **altijd geweigerd**. JeffCrunch laat het u weten, en er wordt niets uitgevoerd. Een koppeling naar een bestand op een andere computer wordt nooit gevolgd zonder dat u eerst wordt gevraagd. Geen instelling verandert dit.

## Zie ook

- [JeffCrunch-instellingen wijzigen](options)
- [Notities en koppelingen](notes-links)
