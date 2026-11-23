# Info: eigenschappen, wachtwoorden en Document controleren
<!-- keywords: toegankelijkheid controleren, info, eigenschappen, titel, auteur, onderwerp, trefwoorden, categorieën, opmerkingen, bedrijf, manager, aangepaste eigenschappen, docproperty, als definitief markeren, definitief, alleen-lezen, wachtwoord, versleutelen, beveiligen, wachtwoord voor openen, document controleren, persoonlijke gegevens, verborgen tekst, aangepaste XML, compatibiliteit controleren, versiegeschiedenis, versies, versie bewaren, teruggaan, herstellen, versies vergelijken, in map tonen, pad kopiëren, map, pad, info, eigenschappen document -->

Klik op **Bestand > Info** om de feiten over uw document te zien, en om het te beveiligen voordat u het deelt. Druk op **Esc** om terug te gaan naar uw document.

## Waar het bestand staat

Bovenaan de pagina staat waar het bestand is opgeslagen. Klik op **In map tonen** om die map te openen. Klik op **Pad kopiëren** om het pad te kopiëren, zodat u het in een e-mail of chat kunt plakken.

## Eigenschappen

Eigenschappen zijn gegevens over het document, zoals de titel en wie het heeft geschreven. De pagina heeft een vak voor elk van deze: **Titel**, **Auteur**, **Onderwerp**, **Trefwoorden**, **Categorieën**, **Opmerkingen**, **Bedrijf** en **Manager**. Typ in een vak en druk op **Tab** of **Enter**. JeffWrite bewaart de wijziging en slaat die op de volgende keer dat u het document opslaat.

Onder de vakken toont de pagina wat JeffWrite berekent: wanneer het document is gemaakt, opgeslagen en afgedrukt, en hoeveel pagina's, woorden, tekens en alinea's het heeft.

### Uw eigen eigenschappen

Klik op **Aangepaste eigenschappen** om eigen eigenschappen te maken, zoals **Project** of **Klant**. Klik op **Toevoegen**, typ een naam, kies het soort waarde (tekst, een geheel getal, een getal, ja of nee, of een datum) en typ de waarde.

Een veld kan een aangepaste eigenschap tonen. Klik op **Invoegen > Veld** en kies een veld. Wijzig de code in `DOCPROPERTY "Klant"` en klik op **OK**. De tekst toont de waarde. Druk op **F9** om het bij te werken nadat u de eigenschap hebt gewijzigd. Zie [Velden](fields).

## Als definitief markeren

Klik op **Als definitief markeren** om lezers te laten weten dat het document klaar is. JeffWrite slaat het document op en houdt het daarna alleen beschikbaar om te lezen. Wanneer iemand het opent, staat in de titelbalk dat het alleen-lezen is, en een melding vertelt dat het als definitief is gemarkeerd. Dit is een zachte manier om te zeggen: "wijzig dit alstublieft niet". Het is geen slot, want iedereen kan **Als definitief gemarkeerd** opnieuw kiezen om het te bewerken.

## Altijd alleen-lezen openen

Klik op **Altijd alleen-lezen openen** om het document elke keer alleen-lezen te openen, zodat niemand het per ongeluk wijzigt. De keuze wordt bij het bestand opgeslagen wanneer u opslaat. De volgende keer dat iemand het bestand opent, meldt de statusbalk dat het alleen-lezen wordt geopend. Om het te bewerken, opent u **Bestand > Info** en klikt u op **Altijd alleen-lezen openen**. Dat is één klik, dus dit is een herinnering en geen slot. Andere programma's lezen deze keuze niet, en een .odt-bestand kan die niet bevatten. Sla het document daarom eerst op als .docx-bestand. **Compatibiliteit controleren** vermeldt dit bij een .odt-bestand.

## Versleutelen met wachtwoord

Klik op **Versleutelen met wachtwoord** om elke keer dat het bestand wordt geopend om een wachtwoord te vragen. Typ het wachtwoord tweemaal en klik op **Beveiligen**. Het bestand wordt bij het opslaan versleuteld, zodat niemand het zonder wachtwoord kan lezen. JeffWrite gebruikt de sterke vorm van beveiliging die andere tekstverwerkers ook gebruiken, zodat die het bestand met het wachtwoord ook kunnen openen.

- Een wachtwoord **kan niet opnieuw worden opgevraagd** als u het vergeet. Bewaar het op een veilige plek.
- De versies die u hebt bewaard en de reservekopie volgen het wachtwoord: ze zijn ermee beveiligd en veranderen wanneer u het wachtwoord wijzigt of verwijdert.
- Om het wachtwoord te wijzigen, klikt u opnieuw op dezelfde kaart. Klik op **Wachtwoord verwijderen** om het weg te halen. De wijziging wordt opgeslagen de volgende keer dat u opslaat.
- Wachtwoorden werken voor **.docx**-bestanden en voor **.odt**-bestanden. Een .odt-bestand wordt beveiligd op de manier die de OpenDocument-standaard beschrijft, zodat andere programma's die de standaard volgen het met het wachtwoord ook kunnen openen. Een platte tekst- of .rtf-bestand kan geen wachtwoord bevatten, dus JeffWrite slaat een beveiligd document niet als zodanig op. Het meldt dit, in plaats van zonder wachtwoord op te slaan.

Om een beveiligd bestand te openen, opent u het zoals gewoonlijk. JeffWrite vraagt om het wachtwoord. Als het niet klopt, kunt u het opnieuw proberen of op **Annuleren** klikken.

JeffWrite opent ook een oud **.doc**-bestand dat om een wachtwoord vraagt. Het vraagt op dezelfde manier. JeffWrite slaat nooit een .doc-bestand op, dus **Opslaan als** vraagt om een nieuwe naam, en de .docx-kopie behoudt hetzelfde wachtwoord.

## Document controleren

Klik op **Document controleren** voordat u een document naar iemand stuurt. Het toont wat het bestand bevat dat u misschien niet wilt delen, en hoeveel van elk het heeft gevonden:

- **Opmerkingen** zijn notities die mensen in de kantlijn hebben achtergelaten.
- **Bijgehouden wijzigingen** zijn bewerkingen die nog niet zijn geaccepteerd of geweigerd. **Alle wijzigingen accepteren** maakt elke wijziging deel van de tekst.
- **Documenteigenschappen en persoonlijke gegevens** zijn de auteur, de persoon die het het laatst heeft opgeslagen, het bedrijf en de manager.
- **Verborgen tekst** is tekst die als verborgen is ingesteld. Die wordt niet getoond of afgedrukt, maar staat wel in het bestand.
- **Koptekst en voettekst** is de tekst bovenaan en onderaan de pagina's.
- **Aangepaste XML-gegevens** zijn gegevens die andere programma's aan het bestand hebben toegevoegd. U ziet ze niet op een pagina.
- **Externe koppelingen** zijn koppelingen naar andere bestanden, geen weblinks: een afbeelding of object dat alleen is gekoppeld (het toont een bestand dat niet in het document zit), en een veld dat zijn tekst of afbeelding uit een ander bestand haalt (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE en DDEAUTO). **Alles verwijderen** zet elk veld om in de tekst die het nu toont, haalt elke gekoppelde afbeelding of elk gekoppeld object weg en laat de koppeling bij het opslaan uit het bestand. De andere bestanden worden niet aangeraakt. Koppelingen in koptekst, voettekst, voetnoten en eindnoten worden ook geteld en verwijderd. Weblinks blijven staan. Koppelingen in opmerkingen, tekstvakken en de onderdelen die het programma zoals ze zijn bewaart (zoals de tekst van tekeningen) worden ook opgezocht. Een gekoppelde afbeelding die ook een eigen kopie van de afbeelding bevat, blijft staan als afbeelding zonder koppeling, en de koppeling wordt bij het opslaan uit het bestand weggelaten. Koppelingen verwijderen in opmerkingen en de andere onderdelen die zoals ze zijn bewaard blijven, kan niet ongedaan worden gemaakt; de tekstvakken zijn één stap van Ongedaan maken. De sjabloon waaraan het document is gekoppeld (het pad bevat vaak een gebruikersnaam) telt ook als externe koppeling: **Alles verwijderen** koppelt die los, en het document behoudt zijn eigen stijlen. Andere koppelingen naar bestanden worden niet verwijderd, en een regel onder het aantal noemt ze: de gegevensbron voor afdruk samenvoegen, frames, subdocumenten in andere onderdelen, de externe gegevens van een grafiek, gekoppelde films en geluiden, en gekoppelde stijlbladen. Dezelfde regel telt elke andere koppeling in een veld, een opmerking, een tekstvak of de lijst met koppelingen van het bestand die niets in het document gebruikt, en zegt dat ook wanneer het adres van een gekoppelde afbeelding niet in het bestand staat of een onderdeel in een codering staat die de controle niet kan lezen. De rij zegt dan "Niet volledig gecontroleerd" in plaats van "Niets gevonden". Eén keer **Alles verwijderen** haalt maximaal 100.000 koppelingen uit bewaarde onderdelen weg; als er meer overblijven, meldt de regel hoeveel, en kunt u **Alles verwijderen** opnieuw uitvoeren.

Klik naast een soort op **Verwijderen** om die te verwijderen. U kunt wijzigingen in de tekst ongedaan maken met **Ctrl+Z**. Koppelingen die uit opmerkingen en andere bewaarde onderdelen zijn verwijderd, en aangepaste XML-gegevens, kunnen niet ongedaan worden gemaakt. Sla het document daarna op om de schone versie te bewaren.

**Persoonlijke gegevens uit dit bestand verwijderen bij het opslaan**, onder de kaarten, doet dit elke keer dat u opslaat, voor elk bestand, zonder het document dat u open hebt te wijzigen. Het is hetzelfde vakje als in **Opties**, en hetzelfde in elke JeffOffice-app.

## Compatibiliteit controleren

Klik op **Compatibiliteit controleren** en kies een bestandssoort om te zien wat die niet kan bevatten. JeffWrite slaat **.docx**-, **.odt**-, **.rtf**- en **.txt**-bestanden op. Een .docx-bestand bevat alles in uw document. De andere soorten kunnen sommige zaken niet bevatten, en deze lijst vertelt welke. Ze noemt ook de keuzes die in het bestand zelf worden bewaard, Als definitief markeren en Altijd alleen-lezen openen, wanneer die aan staan, en het wachtwoord voor een soort die er geen kan bevatten. Staat er niets in de lijst, dan gaat er niets verloren.

Wanneer u als .odt-bestand opslaat, toont JeffWrite wat het bestand niet kan bevatten **voordat** er iets wordt geschreven. U kunt kiezen voor **Toch opslaan**, **Opslaan als .docx** (het .docx-bestand wordt naast het andere gezet en er gaat niets verloren) of **Annuleren**. Vink **Niet opnieuw vragen voor dit bestand** aan, dan vraagt JeffWrite daar niet meer om totdat u het programma sluit.

## Toegankelijkheid controleren

Klik op **Toegankelijkheid controleren** om de lijst te openen met zaken die voor sommige mensen moeilijk te lezen of te gebruiken kunnen zijn, zoals een afbeelding zonder beschrijving. Het is dezelfde controle als **Controleren > Toegankelijkheid controleren**. Zie [Woorden tellen, taal instellen, toegankelijkheid controleren en woorden opzoeken](proofing-tools).

## Versiegeschiedenis

Klik op **Versiegeschiedenis** om benoemde kopieën van het document op uw computer te bewaren. Er wordt niets ergens naartoe verzonden. Een versie is een volledige kopie van het document zoals het op dat moment is. Bewaar er een voordat u een grote wijziging maakt, zodat u erop terug kunt vallen. Het document moet eerst worden opgeslagen, omdat versies onder de bestandsnaam worden bewaard.

- **Versie bewaren** vraagt om een naam, zoals "Voor de bewerking", en bewaart een kopie.
- **Kopie openen** opent de gekozen versie in een nieuw venster. De versie zelf verandert niet.
- **Vergelijken** zet het document en de gekozen versie naast elkaar als één document, met de verschillen weergegeven als bijgehouden wijzigingen. Zie [Wijzigingen bijhouden](track-changes).
- **Teruggaan naar deze versie** maakt het document gelijk aan de gekozen versie. JeffWrite bewaart eerst het document zoals het was als nieuwe versie, zodat er niets verloren gaat. Sla het document op om de wijziging te behouden.
- **Verwijderen** haalt de gekozen versie weg.

JeffWrite bewaart maximaal 50 versies van een bestand en verwijdert de oudste wanneer er meer zijn.
