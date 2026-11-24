# Dia's maken uit een overzicht
<!-- keywords: dia's uit overzicht, overzicht importeren, markdown, tekstoverzicht, rtf, docx, koppen naar dia's, opsommingslijst naar dia's, presentatie starten vanuit tekst, overzicht opslaan, overzicht exporteren -->

U kunt een presentatie als platte tekst schrijven en laten omzetten in dia's door JeffDeck. Elke dia heeft een titel en een tekst, met de indeling Titel en object. Overzichtsweergave, Dia opnieuw instellen en thema's werken dus allemaal zoals gewoonlijk.

## De dia's toevoegen aan de presentatie die u open hebt

1. Klik op **Start > Dia's**, klik op het kleine pijltje onder **Nieuwe dia** en kies **Dia's uit overzicht…**.
2. Kies een bestand. Dit kan een tekstbestand (.txt), een Markdown-bestand (.md), een opgemaakt tekstbestand (.rtf) of een documentbestand (.docx) zijn.
3. De dia's worden toegevoegd na de dia waarop u staat. Ze vormen één stap, dus één keer **Ctrl+Z** haalt ze allemaal weer weg.

## Of begin een nieuwe presentatie met een overzicht

Kies **Bestand > Openen** en selecteer het overzichtsbestand. JeffDeck maakt er een nieuwe presentatie van. Het bestand wordt nooit gewijzigd, en bij **Opslaan** wordt om een naam gevraagd.

## Hoe het bestand wordt gelezen

**Tekstbestanden.** Een regel die aan de linkerkant begint, is de titel van een dia. De regels eronder vormen de tekst van die dia. Een regel die verder is ingesprongen dan de regel ervoor, komt één niveau dieper te staan (gebruik tabs of spaties). Een opsommingsteken (-, * of +) of een getal aan het begin van een regel wordt weggelaten, omdat de dia zijn eigen opsommingstekens heeft.

**Markdown-bestanden.** Het hoogste kopniveau dat het bestand gebruikt, maakt de dia's: # als het bestand dat heeft, anders ##. Kleinere koppen worden regels in de tekst van de dia, en elk niveau dat kleiner is, staat één stap dieper. Een regel met drie streepjes (---) begint een nieuwe dia. Lijsten worden niveaus. **Vet**, *cursief*, `code` en hyperlinks behouden hun opmaak. Een codeblok tussen regels van drie backticks wordt een tekstvak in een schrijfmachinelettertype. Een afbeelding op een eigen regel, geschreven in de Markdown-vorm voor afbeeldingen (een uitroepteken, de beschrijving tussen vierkante haken en de bestandsnaam tussen haakjes), wordt op de dia geplaatst, maar alleen als het afbeeldingsbestand in dezelfde map staat als het overzicht, of in een map binnen die map. Afbeeldingen van internet of uit andere mappen worden nooit geladen.

**Opgemaakte tekst- en .docx-bestanden.** Alinea's in een kopstijl maken de dia's: de hoogste kopstijl die het bestand gebruikt, is de titel, kleinere koppen zijn regels van de tekst, en elke andere alinea is een regel onder de kop erboven.

## Beperkingen

Deze beperkingen gelden voor alle soorten overzichtsbestanden, tekst, Markdown, opgemaakte tekst en .docx:

- Maximaal 5.000 dia's. Wat daarna komt, wordt weggelaten.
- Maximaal 4.000 tekens per regel. Een langere regel wordt afgekapt, net als een Markdown-alinea waarvan de regels samen langer worden dan dat.
- Maximaal 500 regels tekst op een dia en 100.000 regels in totaal.
- De tekst van een dia heeft maximaal 9 niveaus. Een regel die dieper is ingesprongen, wordt op het diepste niveau getoond.
- Maximaal 200 regels code op een dia.
- Maximaal 100 afbeeldingen in totaal en 4 op een dia. Een afbeeldingsbestand mag maximaal 32 MB zijn, en alle afbeeldingsbestanden samen maximaal 64 MB. Een afbeelding moet een PNG-, JPEG-, GIF-, BMP-, WebP- of TIFF-bestand zijn, ongeacht de naam, met maximaal 16 miljoen pixels (4.000 bij 4.000), en alle afbeeldingen samen maximaal 64 miljoen pixels.

Ook de grootte van het bestand zelf is beperkt. Een tekst- of Markdown-bestand mag maximaal 10 MB zijn, in UTF-8 of UTF-16. Een opgemaakt tekst- of .docx-bestand mag maximaal 96 MB zijn. Een bestand dat geen tekst is, wordt geweigerd. Afbeeldingen in een opgemaakt tekst- of .docx-bestand worden niet op de dia's geplaatst, en JeffDeck laat u weten hoeveel er zijn weggelaten.

Wanneer iets wordt weggelaten, zoals een afbeelding die niet kon worden gebruikt of tekst na een limiet, laat JeffDeck u weten wat het was. Terwijl een groot bestand wordt gelezen, blijft JeffDeck doorwerken, en als u het venster sluit, stopt het lezen.

## Een kopie opslaan als overzicht

Kies **Bestand > Kopie opslaan** en selecteer **Markdown-overzicht (*.md)**. Het bestand bevat de titel en tekst van elke dia, met vet, cursief en hyperlinks. Afbeeldingen, vormen, tabellen, grafieken, andere tekstvakken, sprekersnotities en de opmaak van de dia's worden weggelaten. JeffDeck laat u dat weten bij het opslaan, en **Bestand > Info > Compatibiliteit controleren** vermeldt het ook.

## Zie ook

- [Dia's toevoegen, wijzigen en verwijderen](slides)
- [Dia's hergebruiken uit een andere presentatie](reuse)
- [Weergaven](views)
