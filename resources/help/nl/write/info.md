# Info: eigenschappen, wachtwoorden en Inspecteren
<!-- keywords: toegankelijkheid controleren, info, eigenschappen, titel, auteur, onderwerp, trefwoorden, categorieën, opmerkingen, bedrijf, manager, aangepaste eigenschappen, docproperty, als definitief markeren, definitief, alleen-lezen, wachtwoord, versleutelen, beveiligen, wachtwoord voor openen, document inspecteren, persoonlijke gegevens, verborgen tekst, aangepaste XML, compatibiliteit controleren, versiegeschiedenis, versies, versie bewaren, teruggaan, herstellen, versies vergelijken, in map weergeven, pad kopiëren, map, pad -->

Klik op **Bestand > Info** om de feiten over uw document te zien en om het te beveiligen voordat u het deelt. Druk op **Esc** om terug te gaan naar uw document.

## Waar het bestand staat

Bovenaan de pagina staat waar het bestand is opgeslagen. Klik op **Map weergeven** om die map te openen. Klik op **Pad kopiëren** om het pad te kopiëren, zodat u het in een e-mail of chat kunt plakken.

## Eigenschappen

Eigenschappen zijn gegevens over het document, zoals de titel en wie het heeft geschreven. De pagina heeft een vak voor elk van deze: **Titel**, **Auteur**, **Onderwerp**, **Trefwoorden**, **Categorieën**, **Opmerkingen**, **Bedrijf** en **Manager**. Typ in een vak en druk op **Tab** of **Enter**. JeffWrite bewaart de wijziging en slaat die op de volgende keer dat u het document opslaat.

Onder de vakken toont de pagina wat JeffWrite berekent: wanneer het document is gemaakt, opgeslagen en afgedrukt, en hoeveel pagina's, woorden, tekens en alinea's het heeft.

### Eigen eigenschappen

Klik op **Aangepaste eigenschappen** om eigen eigenschappen te maken, zoals **Project** of **Klant**. Klik op **Toevoegen**, typ een naam, kies het soort waarde (tekst, een geheel getal, een getal, ja of nee, of een datum) en typ de waarde.

Een veld kan een aangepaste eigenschap tonen. Klik op **Invoegen > Veld** en kies een veld. Wijzig de code in `DOCPROPERTY "Klant"` en klik op **OK**. De tekst toont de waarde. Druk op **F9** om het bij te werken nadat u de eigenschap hebt gewijzigd. Zie [Velden](fields).

## Als definitief markeren

Klik op **Als definitief markeren** om lezers te laten weten dat het document af is. JeffWrite slaat het document op en houdt het daarna alleen beschikbaar om te lezen. Wanneer iemand het opent, staat in de titelbalk dat het alleen-lezen is, en een melding laat zien dat het als definitief is gemarkeerd. Dit is een vriendelijke manier om te zeggen: "wijzig dit alstublieft niet". Het is geen slot, want iedereen kan **Als definitief gemarkeerd** opnieuw kiezen om het te bewerken.

## Altijd alleen-lezen openen

Klik op **Altijd alleen-lezen openen** om het document elke keer alleen om te lezen te openen, zodat niemand het per ongeluk wijzigt. De keuze wordt bij het opslaan met het bestand opgeslagen. De volgende keer dat iemand het bestand opent, meldt de statusbalk dat het alleen om te lezen is geopend. Om het te bewerken, opent u **Bestand > Info** en klikt u op **Altijd alleen-lezen openen**. Dat is één klik, dus dit is een herinnering en geen slot. Andere programma's lezen deze keuze niet, en een .odt-bestand kan die niet bewaren. Sla het document daarom eerst op als .docx-bestand. Compatibiliteit controleren vermeldt dit voor een .odt-bestand.

## Versleutelen met wachtwoord

Klik op **Versleutelen met wachtwoord** om elke keer bij het openen van het bestand om een wachtwoord te vragen. Typ het wachtwoord twee keer en klik op **Beveiligen**. Het bestand wordt bij het opslaan onleesbaar gemaakt, zodat niemand het zonder wachtwoord kan lezen. JeffWrite gebruikt de sterke beveiliging die andere tekstverwerkers ook gebruiken, zodat zij het bestand met het wachtwoord ook kunnen openen.

- Een wachtwoord **kan niet opnieuw worden opgehaald** als u het vergeet. Bewaar het op een veilige plek.
- De versies die u hebt bewaard en de reservekopie volgen het wachtwoord: ze worden ermee beveiligd en veranderen wanneer u het wachtwoord wijzigt of verwijdert.
- Om het wachtwoord te wijzigen, klikt u opnieuw op dezelfde kaart. Klik op **Wachtwoord verwijderen** om het weg te halen. De wijziging wordt de volgende keer dat u opslaat bewaard.
- Wachtwoorden werken voor **.docx**-bestanden en voor **.odt**-bestanden. Een .odt-bestand wordt beveiligd op de manier die de OpenDocument-standaard beschrijft, zodat andere programma's die de standaard volgen het met het wachtwoord ook kunnen openen. Een platte tekst- of .rtf-bestand kan geen wachtwoord bevatten. JeffWrite slaat een beveiligd document daarom niet als zodanig op. In plaats van zonder wachtwoord op te slaan, geeft het een melding.

Om een beveiligd bestand te openen, opent u het zoals gewoonlijk. JeffWrite vraagt om het wachtwoord. Als dat niet klopt, kunt u het opnieuw proberen of op **Annuleren** klikken.

JeffWrite opent ook een oud **.doc**-bestand dat om een wachtwoord vraagt. Het vraagt op dezelfde manier. JeffWrite slaat nooit een .doc-bestand op, dus bij **Opslaan als** vraagt het om een nieuwe naam, en de .docx-kopie behoudt hetzelfde wachtwoord.

## Document inspecteren

Klik op **Document inspecteren** voordat u een document naar iemand stuurt. Het toont wat het bestand bevat dat u misschien niet wilt delen, en hoeveel van elk het heeft gevonden:

- **Opmerkingen** zijn notities die mensen in de kantlijn hebben achtergelaten.
- **Bijgehouden wijzigingen** zijn bewerkingen die nog niet zijn geaccepteerd of geweigerd. **Alles accepteren** neemt elke wijziging over als onderdeel van de tekst.
- **Eigenschappen van document en persoonlijke gegevens** zijn de auteur, de persoon die het laatst heeft opgeslagen, het bedrijf en de manager.
- **Verborgen tekst** is tekst die als verborgen is ingesteld. Die wordt niet getoond of afgedrukt, maar staat wel in het bestand.
- **Koptekst en voettekst** is de tekst bovenaan en onderaan de pagina's.
- **Aangepaste XML-gegevens** zijn gegevens die andere programma's aan het bestand hebben toegevoegd. U ziet die niet op een pagina.
- **Externe koppelingen** zijn koppelingen naar andere bestanden, geen weblinks: een afbeelding of object dat alleen gekoppeld is (het toont een bestand dat niet in het document zit), en een veld dat zijn tekst of afbeelding uit een ander bestand haalt (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE en DDEAUTO). **Alles verwijderen** zet elk veld om in de tekst die het nu toont, haalt elke gekoppelde afbeelding of elk gekoppeld object weg en laat de koppeling bij het opslaan uit het bestand. De andere bestanden blijven ongewijzigd. Koppelingen in koptekst, voettekst en voetnoten tellen niet mee, en weblinks blijven staan.

Klik naast een soort op **Alles verwijderen** om die te verwijderen. Wijzigingen aan de tekst kunt u ongedaan maken met **Ctrl+Z**. Sla het document daarna op om de schone versie te bewaren.

Het vak **Persoonlijke gegevens uit dit bestand verwijderen bij het opslaan**, onder de kaarten, doet dit elke keer dat u opslaat, voor elk bestand, zonder het document dat u open hebt te wijzigen. Het is hetzelfde vak als in **Opties**, en hetzelfde in elke JeffOffice-app.

## Compatibiliteit controleren

Klik op **Compatibiliteit controleren** en kies daarna een bestandssoort om te zien wat die niet kan bewaren. JeffWrite slaat **.docx**-, **.odt**-, **.rtf**- en **.txt**-bestanden op. Een .docx-bestand bewaart alles in uw document. De andere soorten kunnen sommige dingen niet bewaren, en deze lijst geeft aan welke. Ook worden de keuzes genoemd die in het bestand zelf worden bewaard, Als definitief markeren en Altijd alleen-lezen openen, wanneer ze aan staan, en het wachtwoord voor een soort die er geen kan bewaren. Staat er niets in de lijst, dan gaat er niets verloren.

Wanneer u als .odt-bestand opslaat, toont JeffWrite wat het bestand niet kan bewaren **voordat** het iets schrijft. U kunt dan kiezen voor **Toch opslaan**, **Opslaan als .docx** (het .docx-bestand wordt naast het .odt-bestand gezet en er gaat niets verloren) of **Annuleren**. Vink **Niet opnieuw vragen voor dit bestand** aan en JeffWrite stopt met vragen over dat bestand tot u het programma sluit.

## Toegankelijkheid controleren

Klik op **Toegankelijkheid controleren** om de lijst te openen met zaken die voor sommige mensen moeilijk te lezen of te gebruiken kunnen zijn, zoals een afbeelding zonder beschrijving. Het is dezelfde controle als **Controleren > Toegankelijkheid controleren**. Zie [Woorden tellen, taal instellen, toegankelijkheid controleren en woorden opzoeken](proofing-tools).

## Versiegeschiedenis

Klik op **Versiegeschiedenis** om benoemde kopieën van het document op uw computer te bewaren. Er wordt niets ergens naartoe verzonden. Een versie is een volledige kopie van het document zoals het op dat moment is. Bewaar er een vóór een grote wijziging, zodat u er later naar kunt teruggaan. Het document moet eerst worden opgeslagen, omdat versies worden bewaard onder de bestandsnaam.

- **Versie bewaren** vraagt om een naam, zoals "Voor de bewerking", en bewaart een kopie.
- **Kopie openen** opent de gekozen versie in een nieuw venster. De versie zelf verandert niet.
- **Vergelijken** zet het document en de gekozen versie naast elkaar als één document, met de verschillen weergegeven als bijgehouden wijzigingen. Zie [Wijzigingen bijhouden](track-changes).
- **Teruggaan naar deze versie** maakt het document gelijk aan de gekozen versie. JeffWrite bewaart eerst het document zoals het was, als nieuwe versie, zodat er niets verloren gaat. Sla het document op om de wijziging te behouden.
- **Verwijderen** haalt de gekozen versie weg.

JeffWrite bewaart maximaal 50 versies van een bestand en verwijdert de oudste als er meer zijn.
