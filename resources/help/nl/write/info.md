# Info: eigenschappen, wachtwoorden en Document controleren
<!-- keywords: controleren op toegankelijkheid, info, eigenschappen, titel, auteur, onderwerp, trefwoorden, categorieën, opmerkingen, bedrijf, manager, aangepaste eigenschappen, docproperty, markeren als definitief, definitief, alleen-lezen, wachtwoord, versleutelen, beveiligen, wachtwoord voor openen, document controleren, persoonlijke gegevens, verborgen tekst, aangepaste XML, compatibiliteit controleren, versiegeschiedenis, versies, versie bewaren, teruggaan, herstellen, versies vergelijken, in map tonen, pad kopiëren, map, pad, gegevens, metagegevens -->

Klik op **Bestand > Info** om de feiten over uw document te zien en om het te beveiligen voordat u het deelt. Druk op **Esc** om terug te gaan naar uw document.

## Waar het bestand staat

Bovenaan de pagina staat waar het bestand is opgeslagen. Klik op **In map tonen** om die map te openen. Klik op **Pad kopiëren** om het pad te kopiëren, zodat u het in een e-mail of chat kunt plakken.

## Eigenschappen

Eigenschappen zijn feiten over het document, zoals de titel en wie het heeft geschreven. De pagina heeft een vak voor elk ervan: **Titel**, **Auteur**, **Onderwerp**, **Trefwoorden**, **Categorieën**, **Opmerkingen**, **Bedrijf** en **Manager**. Typ in een vak en druk op **Tab** of **Enter**. JeffWrite bewaart de wijziging en slaat die op de volgende keer dat u het document opslaat.

Onder de vakken toont de pagina wat JeffWrite berekent: wanneer het document is gemaakt, opgeslagen en afgedrukt, en hoeveel pagina's, woorden, tekens en alinea's het heeft.

### Uw eigen eigenschappen

Klik op **Aangepaste eigenschappen** om eigen eigenschappen te maken, zoals **Project** of **Klant**. Klik op **Toevoegen**, typ een naam, kies welk soort waarde het is (tekst, een geheel getal, een getal, ja of nee, of een datum) en typ de waarde.

Een veld kan een aangepaste eigenschap tonen. Klik op **Invoegen > Veld** en kies een veld. Wijzig de code in `DOCPROPERTY "Klant"` en klik op **OK**. De tekst toont de waarde. Druk op **F9** om het bij te werken nadat u de eigenschap hebt gewijzigd. Zie [Velden](fields).

## Markeren als definitief

Klik op **Markeren als definitief** om lezers te laten weten dat het document klaar is. JeffWrite slaat het document op en houdt het daarna alleen voor lezen beschikbaar. Wanneer iemand het opent, vermeldt de titelbalk dat het alleen-lezen is, en een opmerking laat zien dat het als definitief is gemarkeerd. Dit is een vriendelijke manier om te zeggen: "wijzig dit alstublieft niet". Het is geen slot, want iedereen kan **Als definitief gemarkeerd** opnieuw kiezen om het te bewerken.

## Altijd alleen-lezen openen

Klik op **Altijd alleen-lezen openen** om het document elke keer alleen voor lezen te openen, zodat niemand het per ongeluk wijzigt. De keuze wordt bij het opslaan in het bestand bewaard. De volgende keer dat iemand het bestand opent, vermeldt de statusbalk dat het alleen voor lezen is geopend. Om het te bewerken, opent u **Bestand > Info** en klikt u op **Altijd alleen-lezen openen** om dit uit te schakelen. Dat is één klik, dus dit is een herinnering en geen slot. Andere programma's lezen deze keuze niet, en een .odt-bestand kan haar niet bevatten. Sla het document daarom eerst op als .docx-bestand. Compatibiliteit controleren vermeldt dit voor een .odt-bestand.

## Versleutelen met wachtwoord

Klik op **Versleutelen met wachtwoord** om bij elke keer openen om een wachtwoord te vragen. Typ het wachtwoord twee keer en klik op **Beveiligen**. Het bestand wordt bij het opslaan versleuteld, zodat niemand het zonder wachtwoord kan lezen. JeffWrite gebruikt de sterke beveiliging die andere tekstverwerkers ook gebruiken, zodat zij het bestand met het wachtwoord ook kunnen openen.

- Een wachtwoord **kan niet opnieuw worden opgehaald** als u het vergeet. Bewaar het op een veilige plek.
- De versies die u hebt bewaard en de reservekopie volgen het wachtwoord: ze worden ermee beveiligd, en veranderen wanneer u het wachtwoord wijzigt of verwijdert.
- Om het wachtwoord te wijzigen, klikt u op dezelfde kaart. Klik op **Wachtwoord verwijderen** om het weg te halen. De wijziging wordt opgeslagen de volgende keer dat u opslaat.
- Wachtwoorden werken voor **.docx**-bestanden en voor **.odt**-bestanden. Een .odt-bestand wordt beveiligd op de manier die de OpenDocument-standaard beschrijft, zodat andere programma's die de standaard volgen het ook met het wachtwoord kunnen openen. Een platte tekst- of .rtf-bestand kan geen wachtwoord bevatten, daarom slaat JeffWrite een beveiligd document niet in dat formaat op. In plaats daarvan krijgt u een melding, en wordt het niet zonder wachtwoord opgeslagen.

Om een beveiligd bestand te openen, opent u het zoals gewoonlijk. JeffWrite vraagt om het wachtwoord. Als het niet klopt, kunt u het opnieuw proberen of op **Annuleren** klikken.

JeffWrite opent ook een oud **.doc**-bestand dat om een wachtwoord vraagt. Het vraagt op dezelfde manier. JeffWrite slaat nooit een .doc-bestand op, dus **Opslaan als** vraagt om een nieuwe naam, en de .docx-kopie houdt hetzelfde wachtwoord.

## Document controleren

Klik op **Document controleren** voordat u een document naar iemand stuurt. Het toont wat het bestand bevat dat u misschien niet wilt delen, en hoeveel van elk het heeft gevonden:

- **Opmerkingen** zijn notities die mensen in de kantlijn hebben achtergelaten.
- **Bijgehouden wijzigingen** zijn bewerkingen die nog niet zijn geaccepteerd of geweigerd. **Alle wijzigingen accepteren** maakt van elke wijziging onderdeel van de tekst.
- **Documenteigenschappen en persoonlijke gegevens** zijn de auteur, de persoon die het laatst heeft opgeslagen, het bedrijf en de manager.
- **Verborgen tekst** is tekst die als verborgen is ingesteld. Die wordt niet getoond en niet afgedrukt, maar staat wel in het bestand.
- **Koptekst en voettekst** is de tekst bovenaan en onderaan de pagina's.
- **Aangepaste XML-gegevens** zijn gegevens die andere programma's aan het bestand hebben toegevoegd. U ziet ze niet op een pagina.
- **Externe koppelingen** zijn koppelingen naar andere bestanden, geen webkoppelingen: een afbeelding of object dat alleen is gekoppeld (het toont een bestand dat niet in het document zit), en een veld dat zijn tekst of afbeelding uit een ander bestand haalt (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE en DDEAUTO). **Alles verwijderen** zet elk veld om in de tekst die het nu toont, haalt elke gekoppelde afbeelding of elk gekoppeld object weg, en laat de koppeling weg uit het bestand bij het opslaan. De andere bestanden worden niet aangeraakt. Koppelingen in koptekst, voettekst, voetnoten en eindnoten worden ook geteld en verwijderd. Webkoppelingen blijven staan. Koppelingen binnen opmerkingen, tekstvakken en de onderdelen die het programma ongewijzigd bewaart (zoals de tekst van tekeningen) worden ook opgespoord. Een gekoppelde afbeelding die ook een eigen kopie van de afbeelding bevat, blijft staan als afbeelding zonder koppeling, en de koppeling wordt bij het opslaan weggelaten. Het verwijderen van koppelingen binnen opmerkingen en de andere ongewijzigd bewaarde onderdelen kan niet ongedaan worden gemaakt; de tekstvakken zijn één stap van Ongedaan maken. De sjabloon waaraan het document is gekoppeld (het pad bevat vaak een gebruikersnaam) telt ook mee als externe koppeling: **Alles verwijderen** koppelt hem los, en het document behoudt zijn eigen stijlen. Andere koppelingen naar bestanden worden niet verwijderd, en een regel onder het aantal noemt ze: de gegevensbron voor afdruk samenvoegen, frames, subdocumenten binnen andere onderdelen, externe gegevens van een grafiek, gekoppelde films en geluiden, en gekoppelde stijlbladen. Dezelfde regel telt elke andere koppeling in een veld, een opmerking, een tekstvak of de lijst met koppelingen van het bestand die niets in het document gebruikt, en laat zien wanneer het adres van een gekoppelde afbeelding niet in het bestand staat of wanneer een onderdeel in een codering staat die de controle niet kan lezen. De regel zegt dan "Niet volledig gecontroleerd" in plaats van "Niets gevonden". Eén keer **Alles verwijderen** haalt maximaal 100.000 koppelingen in bewaarde onderdelen weg; als er meer overblijven, toont de regel hoeveel, en kunt u **Alles verwijderen** opnieuw uitvoeren.
- **Originele afbeeldingen bewaard onder afbeeldingseffecten** zijn de afbeeldingen waarvan een effect, zoals Vervagen of Mozaïek, is gemaakt. Het bestand bewaart het origineel naast de bewerkte afbeelding, en iedereen die het bestand krijgt, kan het eruit halen. **Originelen verwijderen** houdt alleen de bewerkte afbeelding over, en **Oorspronkelijke afbeelding herstellen** kan het origineel daarna niet meer terugbrengen.
- **Bijgesneden delen van afbeeldingen** zijn de delen die u buiten beeld hebt gesneden. Ze zitten nog in de afbeelding, en iedereen die het bestand krijgt, kan ze zien. **Bijgesneden delen verwijderen** wist ze, en de pagina ziet er precies hetzelfde uit als eerder. Een afbeelding met een effect wordt eerst de bewerkte afbeelding.

Klik op **Alles verwijderen** naast een soort om het weg te halen. U kunt wijzigingen aan de tekst ongedaan maken met **Ctrl+Z**. Koppelingen die uit opmerkingen en andere bewaarde onderdelen zijn gehaald, en aangepaste XML-gegevens, kunnen niet ongedaan worden gemaakt. Sla het document daarna op om de schone versie te bewaren.

**Persoonlijke gegevens uit dit bestand verwijderen bij het opslaan**, onder de kaarten, doet dit elke keer dat u opslaat, voor elk bestand, zonder het geopende document te wijzigen. Het is hetzelfde vakje als in **Opties**, en hetzelfde in elke JeffOffice-app.

## Compatibiliteit controleren

Klik op **Compatibiliteit controleren** en kies daarna een bestandssoort om te zien wat het niet kan bevatten. JeffWrite slaat **.docx**-, **.odt**-, **.rtf**- en **.txt**-bestanden op. Een .docx-bestand bevat alles in uw document. De andere soorten kunnen sommige dingen niet bevatten, en deze lijst laat zien welke. Ook worden de keuzes genoemd die in het bestand zelf worden bewaard, Markeren als definitief en Altijd alleen-lezen openen, wanneer ze aan staan, en het wachtwoord voor een soort die er geen kan bevatten. Als er niets wordt vermeld, gaat er niets verloren.

Wanneer u opslaat als .odt-bestand, toont JeffWrite welke dingen het bestand niet kan bevatten **voordat** het iets schrijft. U kunt kiezen voor **Toch opslaan**, **Opslaan als .docx** (het .docx-bestand wordt ernaast geplaatst, en er gaat niets verloren) of **Annuleren**. Vink **Niet opnieuw vragen voor dit bestand** aan en JeffWrite vraagt niet meer naar dat bestand totdat u het programma sluit.

## Toegankelijkheid controleren

Klik op **Toegankelijkheid controleren** om de lijst te openen met zaken die voor sommige mensen lastig te lezen of te gebruiken kunnen zijn, zoals een afbeelding zonder beschrijving. Het is dezelfde controle als **Controleren > Toegankelijkheid controleren**. Zie [Woorden tellen, taal instellen, toegankelijkheid controleren en woorden opzoeken](proofing-tools).

## Versiegeschiedenis

Klik op **Versiegeschiedenis** om benoemde kopieën van het document op uw computer te bewaren. Niets wordt ergens naartoe verzonden. Een versie is een volledige kopie van het document zoals het op dat moment is. Bewaar er een voor een grote wijziging, zodat u er later naar terug kunt. Het document moet eerst worden opgeslagen, omdat versies onder de bestandsnaam worden bewaard.

- **Versie bewaren** vraagt om een naam, zoals "Voor de bewerking", en bewaart een kopie.
- **Kopie openen** opent de gekozen versie in een nieuw venster. De versie zelf verandert niet.
- **Vergelijken** zet het document en de gekozen versie naast elkaar als één document, met de verschillen als bijgehouden wijzigingen. Zie [Wijzigingen bijhouden](track-changes).
- **Terug naar deze versie** maakt het document gelijk aan de gekozen versie. JeffWrite bewaart eerst het document zoals het was als nieuwe versie, zodat er niets verloren gaat. Sla het document op om de wijziging te bewaren.
- **Verwijderen** haalt de gekozen versie weg.

JeffWrite bewaart maximaal 50 versies van een bestand en verwijdert de oudste wanneer er meer zijn.
