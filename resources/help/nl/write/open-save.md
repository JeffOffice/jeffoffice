# Openen, opslaan, afdrukken en exporteren
<!-- keywords: lettertypen in het bestand, ingesloten lettertypen, ontbrekend lettertype, e-book, ebook, epub, e-reader, hoofdstukken, inhoud, webpagina, html, opslaan als webpagina, gefilterde webpagina, e-mail, eml, bladwijzers, pdf-bladwijzers, shift+f5, laatste bewerking, waar u was gebleven, ingebouwde sjablonen, galerie, brief, rapport, betoog, manuscript, nieuwsbrief, openen, opslaan, opslaan als, docx, doc, dot, odt, opendocument, rtf, oudere bestanden, sjabloon, dotx, recent, vastzetten, nieuw, sluiten, afdrukken, pdf, exporteren, herstellen, automatisch herstel, niet-opgeslagen, vastlopen, bestandsscherm, bestandsindelingen, kopie opslaan, alleen-lezen, alleen lezen, als kopie openen, nieuw venster, reservekopie, mijn sjablonen, opslaan als sjabloon, ott, repareren, openen en repareren, beschadigd, wachtwoord, versleutelen, beveiligen met een wachtwoord -->

Klik linksboven op **Bestand** om het bestandsscherm te zien. Het heeft pagina's voor Nieuw, Openen, Afdrukken en Exporteren, en opdrachten voor Opslaan en Opslaan als. Druk op **Esc** om terug te gaan naar uw document.

## Een nieuw document starten

Druk op **Ctrl+N**, of klik op **Bestand > Nieuw** en daarna op **Leeg document**. Een nieuw document wordt geopend in een eigen venster. Als het venster waarin u zich bevindt leeg is en nog nooit is opgeslagen, neemt het nieuwe document de plaats ervan in.

## Een document openen

Druk op **Ctrl+O**, of klik op **Bestand > Openen**. Klik op **Bladeren** om een bestand te kiezen. JeffWrite opent deze soorten bestanden:

- **.docx** is een document.
- **.dotx** is een sjabloon. Een sjabloon wordt geopend als een nieuw document dat ervan is gemaakt, zodat de sjabloon zelf ongewijzigd blijft.
- **.docm** en **.dotm** zijn een document en een sjabloon die **macro's** kunnen bevatten, kleine programma's die bij een bestand horen. JeffWrite voert ze nooit uit. Ze worden bewaard zoals ze waren wanneer u opslaat als .docm of .dotm. Als u opslaat als .docx of .odt, worden de macro's weggelaten, en JeffWrite meldt dat.
- **.txt** is platte tekst, alleen de woorden. Bij het openen leest JeffWrite het bestand meteen als het gewone ASCII- of Unicode-tekst is. Is dat niet het geval, dan vraagt JeffWrite in welke **codering** het moet worden gelezen, en toont het de eerste regels, zodat u kunt zien of de letters goed zijn weergegeven. Een **codering** is de manier waarop een bestand letters opslaat, zoals UTF-8 of Windows 1252. Bij opslaan als .txt kiest u de codering en de regeleinden, en het bestand bewaart alleen de woorden: geen lettertypen, afbeeldingen of tabellijnen.
- **.html** en **.htm** zijn webpagina's. JeffWrite leest de woorden, koppen, vet, cursief, onderstreping, kleuren, lijsten en tabellen. Opsommingstekens en nummers van lijsten komen binnen als getypte tekens, en afbeeldingen en koppelingen worden weggelaten. JeffWrite haalt nooit iets van internet en negeert scripts. Een webpagina wordt nooit overschreven: **Opslaan** vraagt om een nieuwe naam en slaat een .docx-bestand op. Zie **Opslaan als webpagina** hieronder om een webpagina te maken.
- **.doc** en **.dot** zijn de oudere soorten document en sjabloon. JeffWrite leest ze, en slaat wat u ermee doet op als een nieuw .docx-bestand.
- **.odt**, **.ott** en **.fodt** zijn OpenDocument-bestanden. JeffWrite leest alle drie. Het slaat .odt-bestanden op, en wat u met een .ott- of .fodt-bestand doet, slaat het op als een nieuw .odt-bestand. Zie [OpenDocument-bestanden openen en opslaan](open-odt).
- **.rtf** is Rich Text Format. JeffWrite leest het en slaat het op. Zie [RTF-bestanden openen](open-rtf).

De pagina **Openen** toont ook de documenten die u onlangs hebt geopend of opgeslagen. Klik op een document om het te openen. Klik met de rechtermuisknop op een document om het bovenaan de lijst vast te zetten, of het van de lijst te halen.

Als een bestand onderdelen bevat die JeffWrite nog niet kan bewerken, zoals de indeling van een tabel of een koptekst, dan behoudt het die. Zie [Welkom bij JeffWrite](start). Als een deel van een bestand helemaal niet kan worden gelezen, meldt JeffWrite wat het heeft weggelaten. Daarna wordt dat bestand niet overschreven: Opslaan vraagt om een nieuwe naam, zodat het origineel behoudt wat de kopie zou verliezen.

## Een document opslaan

Druk op **Ctrl+S** om op te slaan. De eerste keer vraagt JeffWrite om een naam. **Opslaan als** (F12) slaat een kopie op onder een nieuwe naam. Kies **Sjabloon (*.dotx)** om een sjabloon op te slaan, **Document met macro's (*.docm)** om macro's te bewaren, **Platte tekst (*.txt)** voor alleen de woorden, of **Rich Text Format (*.rtf)** voor een bestand dat andere programma's kunnen lezen. Een bestand dat uit een oudere bestandssoort komt, wordt altijd onder een nieuwe naam opgeslagen, zodat het oude bestand nooit wordt overschreven.

## Een kopie opslaan

Klik op **Bestand > Kopie opslaan**. Kies een naam. JeffWrite schrijft het document naar dat bestand, en u werkt verder in het document dat u al had. De naam en de niet-opgeslagen wijzigingen blijven zoals ze waren. Dit is handig wanneer u een versie naar iemand wilt sturen en daarna door wilt schrijven.

## Een reservekopie bewaren

Klik op **Bestand > Opties**, en zet op de pagina **Algemeen** het selectievakje **Altijd een reservekopie maken** aan. Elke keer dat u opslaat, kopieert JeffWrite eerst het oude bestand naar een bestand met de naam "Reservekopie van" plus de naam van het bestand, in dezelfde map. Als de kopie niet kan worden gemaakt, vraagt JeffWrite of u wilt opslaan zonder die kopie. Het antwoord is Nee, tenzij u Ja kiest, en het bestand blijft zoals het was. Zo kunt u teruggaan naar de laatst opgeslagen versie.

## Opslaan als sjabloon

Een sjabloon is een document dat u als uitgangspunt gebruikt. Klik op **Bestand > Opslaan als sjabloon** en typ een naam. De sjabloon komt in **Mijn sjablonen**, en het document waarin u schrijft, wordt niet gewijzigd. In **Opslaan als** kunt u ook **Sjabloon (*.dotx)** kiezen, of **OpenDocument-tekstsjabloon (*.ott)**, om een sjabloonbestand op te slaan waar u wilt.

Om een nieuw document te starten op basis van uw sjabloon, klikt u op **Bestand > Nieuw**. Uw sjablonen staan onder **Mijn sjablonen**. Klik op een sjabloon. Er wordt een nieuw document geopend met de tekst en stijlen van de sjabloon. Om een sjabloon uit de lijst te verwijderen, klikt u er met de rechtermuisknop op en kiest u **Verwijderen uit Mijn sjablonen**.

## Andere manieren om een bestand te openen

Klik op de pagina **Openen** op **Meer manieren om te openen**. U kunt ook met de rechtermuisknop op een naam in de lijst Recent klikken. Elke manier vraagt om een bestand, of gebruikt het bestand waarop u hebt geklikt:

- **Alleen-lezen openen** opent het bestand alleen om te lezen. U kunt bekijken, scrollen, zoeken en kopiëren, maar niets wijzigen. De titelbalk toont "Alleen-lezen". Als u op **Opslaan** drukt, vraagt JeffWrite om een nieuwe naam, zodat het geopende bestand niet wordt gewijzigd.
- **Als kopie openen** opent een nieuw document met de tekst van het bestand. Het heeft geen eigen bestand, dus **Opslaan** vraagt waar het moet worden opgeslagen. Het geopende bestand wordt nooit gewijzigd.
- **In nieuw venster openen** opent het bestand in een eigen venster, ook wanneer het venster waarin u zich bevindt leeg is.
- **Openen en repareren** is bedoeld voor een beschadigd .docx- of .doc-bestand, bijvoorbeeld een bestand dat is afgebroken bij het kopiëren. Zie hieronder.

## Openen en repareren

Soms is een .docx- of .doc-bestand beschadigd. Een kopieeractie is halverwege gestopt, of een schijf heeft fouten gemaakt. Een .docx-bestand is een bundel kleinere onderdelen, zoals de tekst, de stijlen en de afbeeldingen. **Openen en repareren** leest elk onderdeel dat het kan lezen. Een onderdeel dat midden in de tekst is gestopt, wordt behouden tot de plek waar het stopte. Een onderdeel dat niet kan worden gelezen, wordt weggelaten. Als de lijst van onderdelen of de koppelingen ertussen verloren zijn, maakt JeffWrite die opnieuw.

Een oud .doc-bestand is één grote omhulling met een paar gegevensstromen erin. Eén stroom bevat de tekst, en een andere bevat de tabel die aangeeft waar elk stuk tekst staat en hoe het eruitziet. Als de omhulling is afgebroken of de lijsten zijn beschadigd, zoekt **Openen en repareren** die stromen op basis van hun inhoud en herstelt zoveel mogelijk. Waar de lijsten die aangeven hoe de stukken op elkaar aansluiten zijn beschadigd, neemt het programma elk stuk als opvolger van het vorige, en toont het wat het daar vindt. Die tekst kan bij een oudere kopie van het document horen of bij een ander deel van het bestand, daarom noemt het bericht de alinea waar die begint, en vraagt het u om die te vergelijken met een reservekopie als u die hebt. Als de gok vóór de tekst ligt, meldt het bericht dat het hele document is samengesteld, en als de gok ná de tekst ligt, meldt het bericht dat de opmaak is samengesteld. Als afbeeldingen niet volledig konden worden hersteld, zegt het bericht dat ook. Als de tabel verloren is, wordt de tekst bewaard als gewone alinea's, zonder stijlen, lijsten, tabellen of afbeeldingen. Een .doc-bestand met een wachtwoord kan niet worden hersteld.

Het gerepareerde document wordt geopend als kopie met een naam als "Gerepareerde brief". Een bericht vertelt wat tot een onderbreking is bewaard en wat is weggelaten. Het beschadigde bestand wordt niet gewijzigd. Sla de gerepareerde kopie op onder een eigen naam, en controleer die voordat u erop vertrouwt.

## Niet-opgeslagen werk terugkrijgen

JeffWrite slaat om de paar minuten een reservekopie op van niet-opgeslagen werk, als bestand in een eigen map. Als het programma onverwacht sluit, biedt het die kopieën aan de volgende keer dat u het start. Klik op **Bestand > Openen > Niet-opgeslagen werk herstellen** om ze later te bekijken. U kunt kiezen welke u opent, en elke kopie opslaan waar u wilt.

## Starten vanuit een sjabloon

**Bestand > Nieuw** toont het **Leeg document** en een reeks sjablonen die bij JeffWrite horen, gegroepeerd per soort: een brief, een rapport, een betoog, een manuscript en een nieuwsbrief. Elke afbeelding is de eerste pagina van het document dat u krijgt. Een **sjabloon** is een document dat al is ingesteld, met de stijlen, marges en voorbeeldtekst. Klik op een sjabloon en vervang de voorbeeldtekst door uw eigen tekst. Zelf opgeslagen sjablonen staan onder **Mijn sjablonen**.

## Verdergaan waar u was gebleven

JeffWrite onthoudt waar de cursor stond toen u een bestand opsloeg of sloot. De volgende keer dat u het bestand opent, staat de cursor daar. Druk op **Shift+F5** om naar de plaatsen te gaan die u het laatst hebt bewerkt. Elke keer dat u drukt, gaat u naar de vorige: de laatste drie plaatsen, en dan weer terug naar de eerste.

## Opslaan als .odt

Een .odt-bestand kan niet alles bevatten wat een .docx-bestand kan. Bij opslaan als .odt toont JeffWrite eerst wat er verloren zou gaan, en laat u een .docx-bestand opslaan in plaats daarvan. Zie [Info](info).

## Afdrukken

Druk op **Ctrl+P**, of klik op **Bestand > Afdrukken**. Kies een printer, het aantal exemplaren en de pagina's, bekijk elke pagina in het voorbeeld, en klik op **Afdrukken**. Zie [Afdrukken](print).

## Een pdf maken

Klik op **Bestand > Exporteren > Exporteren naar pdf**. De pdf neemt de titel, auteur, onderwerp en trefwoorden over uit **Bestand > Info** (met **Documenteigenschappen opnemen** aan), de datum waarop hij is gemaakt, en JeffWrite als het programma dat hem heeft gemaakt. De auteur en een titel die is afgeleid van de bestandsnaam, blijven weg wanneer persoonlijke gegevens worden verwijderd. Elk van deze gegevens wordt volledig bewaard tot 4.000 tekens. Vink **PDF openen na opslaan** aan om het bestand bij het opslaan te openen in het pdf-programma van uw computer. Deze optie staat uit tot u hem aanvinkt. Als uw computer het bestand niet kan openen, meldt de statusbalk dat (het bestand is dan wel opgeslagen). Kies welke pagina's u wilt opnemen: alle, de huidige pagina, of een bereik zoals 1-3, 5 (p2s3 en s3 werken hier ook). Kies daarna hoe de pdf wordt gemaakt:

- **Optimaliseren voor** heeft **Standaard**, die uw afbeeldingen bewaart zoals ze zijn, en **Minimale grootte**, die elke afbeelding niet fijner maakt dan 150 pixels per inch (ongeveer 6 pixels per cm), zodat het bestand kleiner wordt. Gebruik Minimale grootte voor een pdf die u op een webpagina zet of per e-mail verstuurt.
- **Documenteigenschappen opnemen** zet de titel, auteur, onderwerp en trefwoorden van het document in de pdf.
- **Bladwijzers maken op basis van koppen** maakt van elke kop een **bladwijzer**, een item in de lijst die een pdf-lezer naast de pagina's toont. Klik op een bladwijzer om naar de bijbehorende pagina te springen.
- **Documentstructuurtags voor toegankelijkheid** tagt de koppen, alinea's, lijsten, tabellen (met hun kopregels) en afbeeldingen (met hun alternatieve tekst), en noemt de taal, zodat een schermlezer de pdf in de juiste volgorde voorleest.

Kies daarna een naam. De pdf ziet er op elke computer hetzelfde uit, omdat de lettertypen erin zijn opgenomen. De pdf die **Afdrukken** maakt wanneer u een pdf-bestand kiest, heeft ook de bladwijzers en tags. Wanneer er meerdere pagina's op één vel komen, gaat elke bladwijzer en elke koppeling naar het vel dat zijn pagina bevat, en worden de pagina's op een vel gelezen in de volgorde waarin ze erop liggen: van links naar rechts, en dan van boven naar beneden.

## Opslaan als webpagina

Klik op **Bestand > Exporteren** en kies een van de twee soorten webpagina. Een **webpagina** is een bestand dat u in elke webbrowser kunt openen.

- **Opslaan als webpagina** maakt één .html-bestand dat uw woorden, hun opmaak en uw afbeeldingen bevat. U kunt dat ene bestand op zichzelf versturen. De opmaak die het bewaart, is het lettertype, de grootte, de kleur, de markering, vet, cursief, onderstreping, uitlijning en de ruimte rond alinea's.
- **Opslaan als gefilterde webpagina** maakt een klein .html-bestand met alleen de structuur: koppen, vet en cursief, lijsten, tabellen, koppelingen en afbeeldingen. Het heeft geen lettertypen, groottes of kleuren, zodat u er later zelf een opmaak aan kunt geven.

Beide bewaren koppen, lijsten met opsommingstekens en nummers, tabellen met samengevoegde cellen, koppelingen en afbeeldingen. Verborgen tekst, de woorden die een bijgehouden wijziging heeft verwijderd, opmerkingen, koptekst, voettekst en notities worden weggelaten. Een koppeling die een programma zou starten of een script zou uitvoeren, wordt opgeslagen als gewone tekst.

## Opslaan als e-book

Een **e-book** is een boek dat u leest op een e-reader, een telefoon of een tablet. Het bestand is een **EPUB**. De woorden passen zich aan elk scherm aan, zodat de lezer de letters groter of kleiner kan maken.

1. Klik op **Bestand > Exporteren > Opslaan als e-book (EPUB)**.
2. Controleer de **Titel** en de **Auteur**. Ze komen overeen met de titel en auteur in de eigenschappen van uw document (**Bestand > Info**). U kunt ze alleen voor dit boek wijzigen.
3. Zet **De eerste afbeelding als omslag gebruiken** aan als u wilt dat de eerste afbeelding in uw document de omslag wordt. Zonder deze optie heeft het boek geen omslag.
4. Klik op **OK** en kies daarna waar u het bestand wilt opslaan.

Zo wordt het boek gemaakt:

- Elke **Kop 1** begint een nieuw hoofdstuk. De woorden vóór de eerste Kop 1 vormen een eigen hoofdstuk.
- De **inhoud** toont elke Kop 1, Kop 2 en Kop 3, en een lezer kan naar elk ervan springen. Kleinere koppen staan wel in het boek, maar niet in de inhoud.
- Afbeeldingen en tabellen blijven staan waar u ze hebt geplaatst. De alternatieve tekst van een afbeelding gaat mee, zodat een schermlezer kan zeggen wat die toont.
- Voetnoten en eindnoten worden aan het eind van het hoofdstuk geplaatst waarin ze staan. Elke noot heeft een koppeling terug naar de plek in de tekst, en elk nummer in de tekst is een koppeling naar de bijbehorende noot.

Een boek is aan grenzen gebonden, zodat een beschadigd bestand geen enorm boek kan maken: maximaal 500 hoofdstukken, 4.000 regels in de inhoud, 1.000 afbeeldingen (geen daarvan groter dan 32 MB), en 20.000 noten. Wat de grens overschrijdt, blijft in het laatste hoofdstuk, of wordt gewone tekst, en uw woorden gaan nooit verloren: een afbeelding boven de grens wordt haar alternatieve tekst, en een noot boven de grens krijgt geen opmaak. Een afbeelding die een lezer niet kan tonen (een metabestand, een bitmap of een TIFF) wordt omgezet in een PNG. Als u hetzelfde opgeslagen document opnieuw exporteert, ontstaat hetzelfde boek, zodat een lezer zijn plaats behoudt.

## Een pdf met een wachtwoord beveiligen

Klik in het vak **Exporteren naar pdf** op **Versleutelen met een wachtwoord…**. Typ het wachtwoord twee keer. Wie de pdf opent, moet dan het wachtwoord intypen. Het wachtwoord beschermt het bestand echt: de pagina's worden met sterke versleuteling (AES-256) door elkaar gehaald, en niemand kan ze zonder wachtwoord lezen.

U kunt ook **Afdrukken, kopiëren en bewerken beperken** aanvinken, een tweede wachtwoord opgeven en kiezen wat is toegestaan. Die beperkingen zijn alleen een verzoek. De meeste pdf-lezers houden zich eraan, maar ze houden niemand tegen die ze wil omzeilen, dus reken er niet op om een bestand privé te houden.

Een vergeten wachtwoord kan niet worden hersteld, dus bewaar het op een veilige plek. Het wachtwoord geldt voor die ene pdf en wordt nooit opgeslagen. Het vak schakelt **PDF/A** uit zolang een wachtwoord is ingesteld, omdat een PDF/A-bestand geen wachtwoord kan hebben.

## Een PDF/A maken

Vink in het vak **Exporteren naar pdf** **PDF/A, om lang te bewaren** aan. Een PDF/A-bestand (PDF/A-1b) bevat elk lettertype dat het gebruikt, benoemt de kleuren en volgt een norm die is bedoeld voor bestanden die vele jaren moeten worden bewaard. Het kan voor schermlezers getagd worden, en het kan geen wachtwoord hebben.

## Een e-mail maken

Klik op **Bestand > Exporteren > E-mail maken** om een berichtbestand (.eml) te maken. Er wordt niets verzonden. U kiest wat in het bericht komt: het document, het document als pdf, of de getoonde pagina's als afbeeldingen in het bericht (de eerste 30 pagina's). Open het bestand in uw e-mailprogramma, voeg toe voor wie het is, en verstuur het. Voor een pdf kunt u in het vak op **Versleutelen met een wachtwoord…** klikken; geef de ontvanger het wachtwoord op een andere manier dan in het bericht zelf.

## Lettertypen in het bestand

Een .docx-bestand kan de lettertypen meenemen waarin de tekst is gezet, zodat het document eruitziet zoals de auteur het heeft gemaakt, ook op een computer die die lettertypen niet heeft. JeffWrite leest ze wanneer het bestand wordt geopend. Ze worden alleen voor dat document gebruikt, terwijl het open is. Ze worden nooit op uw computer geïnstalleerd, geen ander document of programma ziet ze, en ze verdwijnen wanneer u het document sluit. Een lettertype dat het bestand meeneemt, telt niet als een ontbrekend lettertype.

JeffWrite gebruikt geen lettertype waarvan de licentie van de maker het insluiten verbiedt, geen beschadigd lettertype, en geen lettertype na de eerste 64 in één bestand. Als dat gebeurt, zegt het bericht dat u ziet bij het openen van het bestand dat ook, en wordt de tekst in een vergelijkbaar lettertype getoond. Het bestand bewaart zijn lettertypen zoals ze waren wanneer u het opslaat.

Om de lettertypen die uw tekst gebruikt in de bestanden die u opslaat op te nemen, klikt u op **Bestand > Opties**, opent u de pagina **Opslaan** en zet u **Lettertypen in het bestand opslaan** aan. Deze optie staat in het begin uit. JeffWrite neemt dan de lettertypen op die bij het programma horen en die de tekst gebruikt, in elke stijl die het document gebruikt, wanneer het een .docx-bestand opslaat. Het neemt nooit een lettertype op waarvan de licentie dat niet toestaat, en wanneer het er één weglaat, noemt het bericht na het opslaan welk lettertype dat is. Het bestand wordt hierdoor groter, en er gaan maximaal 64 lettertypen en 256 MB in één bestand.

## Gerelateerd

- [Welkom bij JeffWrite](start)
- [Afdrukken](print)
- [JeffWrite met het toetsenbord gebruiken](keyboard)
