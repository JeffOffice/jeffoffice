# Beveiliging: waar een bestand naartoe mag verwijzen

<!-- keywords: vertrouwenscentrum, beveiliging, macro's, gekoppelde bestanden, koppelingen, gekoppelde afbeeldingen, gekoppelde media, internet, gedownload, alleen-lezen, beveiligd, externe koppelingen, programma's, veilig -->

Een bestand kan verwijzen naar andere dingen: bestanden, webpagina's, video's en zelfs programma's. Op de pagina **Beveiliging** bepaalt u wat JeffPub daarmee doet. Klik op **Bestand > Opties** en vervolgens op het tabblad **Beveiliging**. Klik op **OK** om uw wijzigingen te bewaren, of op **Annuleren** om ze weg te gooien.

Deze keuzes gelden voor alle JeffOffice-apps: wijzigt u er één in één app, dan verandert deze in alle apps. Een rij voor iets wat een app niet heeft, zegt **Dit is niet van toepassing** en is grijs weergegeven.

## Macro's

Macro's zijn kleine programma's die in een bestand zijn opgeslagen. JeffPub **voert ze nooit uit**, en er is geen instelling die ze wel uitvoert. JeffPub bewaart ook de macro's van een publicatie niet: wanneer u een geopend `.pub`-bestand opslaat, heeft de kopie geen macro's. Bewaar het origineel als u de macro's nog nodig hebt.

## Gekoppelde bestanden en werkmappkoppelingen

Dit gaat over afbeeldingen die **gekoppeld** zijn aan een bestand in plaats van in de publicatie zelf zijn opgeslagen. De keuze bepaalt wat JeffPub doet wanneer het een publicatie opent met zulke afbeeldingen.

- **Bijwerken** is de gebruikelijke keuze. Wanneer een publicatie wordt geopend, leest JeffPub het bestand van elke gekoppelde afbeelding opnieuw in als het bestand in de eigen map van de publicatie staat, zodat de afbeelding up-to-date is. JeffPub vraagt daar nooit naar.
- **Nooit bijwerken** laat elke afbeelding staan zoals ze is opgeslagen. Elke afbeelding toont **Niet bijgewerkt** totdat u erop **Koppeling bijwerken** gebruikt. **Exporteren naar PDF** en **Afdrukken** melden dan in de statusbalk dat voorbeelden zijn gebruikt voor afbeeldingen waarvan geen volledige kopie in de publicatie staat.

JeffPub leest alleen een bestand dat in de eigen map van de publicatie staat (of in een map die u als veilig hebt aangewezen). Elk ander gekoppeld bestand wacht totdat u erom vraagt.

## Gekoppelde video's en geluiden

Dit is niet van toepassing in JeffPub: het heeft geen gekoppelde video's of geluiden om af te spelen. De rij is grijs weergegeven.

## Bestanden van internet of van andere computers

Een bestand dat u hebt gedownload, of dat op een andere computer in uw netwerk staat, is mogelijk niet veilig. Deze keuze bepaalt hoe JeffPub het opent.

- **Eerst als alleen-lezen openen** is de gebruikelijke keuze. Het bestand opent zodat u het kunt lezen, maar u kunt het niet wijzigen. Een balk onder het bestand laat dat zien. Klik op de balk op **Bewerken inschakelen** om alleen dat bestand te wijzigen (tot u het programma afsluit). Wanneer u het daarna op dezelfde plek opslaat, verwijdert JeffPub de downloadmarkering van het bestand. Of gebruik **Opslaan als** om een kopie te maken, of schakel deze instelling naar de volgende keuze om.
- **Normaal openen** opent elk bestand op dezelfde manier.

Een bestand dat u opent met **Kopie openen**, **Openen en herstellen** of **Openen als alleen-lezen** op het scherm **Bestand**, wordt altijd op die manier geopend, wat deze instelling ook zegt.

## Koppelingen die een programma starten

Een koppeling die een programma of script zou starten (een bestand met de extensie .exe, .bat, .sh, enzovoort) wordt **altijd geweigerd**. JeffPub meldt dat, en er wordt niets uitgevoerd. Een koppeling naar een bestand op een andere computer wordt nooit gevolgd zonder dat u eerst wordt gevraagd. Geen instelling verandert dit.

## Zie ook

- [JeffPub-instellingen wijzigen](options)
- [Afbeeldingen](pictures)
