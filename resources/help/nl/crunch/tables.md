# Tabellen
<!-- keywords: tabel, tabellen, dubbele waarden verwijderen, duplicaten, samenvatten, stijl wissen, gestructureerde verwijzing, koprij, totaalrij, berekende kolom, gebande, gestreept, stijl, tabeluiterlijk, filterknoppen, Ctrl+T, converteren, gewone cellen, formaat wijzigen, kolomnaam, [@Kolom], Tabel1[Kolom] -->

Een tabel is een blok cellen met een rij namen boven de kolommen, die als één geheel wordt behandeld: de tabel heeft een naam, een uiterlijk, knoppen om de rijen te filteren en te sorteren, en ze groeit wanneer u er gegevens aan toevoegt. Formules kunnen de kolommen van een tabel bij naam aanroepen in plaats van bij celadres.

## Een tabel maken

Selecteer de cellen, of klik op een cel in de gegevens, en kies **Tabel** op het tabblad **Invoegen** (**Ctrl+T**). JeffCrunch vraagt welke cellen de tabel omvat en of de eerste rij de namen van de kolommen bevat. Namen die getallen, datums of formules zijn, worden de tekst die ze tonen, lege namen worden Kolom1, Kolom2, enzovoort, en twee kolommen met dezelfde naam worden genummerd. Als er geen namen zijn, heeft de tabel geen rij met namen, en heten de kolommen in formules Kolom1, Kolom2, enzovoort.

De galerie **Tabeluiterlijk** (tabblad **Start**, of het tabblad **Tabel**) toont de uiterlijken: licht, middel en donker, elk in zwart en in de zes kleuren van het thema van de werkmap. (**Wissen**, onder de galerie op het tabblad **Tabel**, haalt het uiterlijk van de tabel af.) Als u er een kiest voor cellen die geen tabel zijn, worden het wel een tabel. Het uiterlijk wordt getekend onder wat een cel zelf heeft: een opvulling, randen, een tekstkleur of vet die u op een cel hebt ingesteld, blijven behouden.

## Het tabblad Tabel

Het tabblad **Tabel** wordt getoond zolang de cursor in een tabel staat.

- **Naam** is de naam waarmee formules de tabel aanroepen. Elke formule die de oude naam gebruikt, volgt. Een naam bestaat uit letters, cijfers, underscores en punten, begint met een letter of een underscore, en is niet de naam van een andere tabel, een andere naam in de werkmap of een celadres zoals A1.
- **Rij met namen**, **Totaalrij**, **Gestreepte rijen**, **Gestreepte kolommen**, **Eerste kolom**, **Laatste kolom** en **Filterknoppen** worden voor de tabel aan- of uitgevinkt.
- **Tabelgrootte wijzigen…** vraagt om de cellen die de tabel omvat. De eerste cel blijft behouden; rijen en kolommen kunnen onderaan en rechts worden toegevoegd of verwijderd.
- **Samenvatten met draaitabel** maakt een draaitabel van de cellen van de tabel (zie [Draaitabellen](pivot-tables)). De draaitabel leest de tabel bij de naam, en volgt de tabel wanneer die groeit.
- **Dubbele waarden verwijderen…** haalt de rijen weg die zich herhalen. Een rij herhaalt zich wanneer hij in alle kolommen die u aanvinkt dezelfde waarden toont als een rij erboven. Hoofdletters en kleine letters tellen als gelijk. De eerste van de rijen blijft staan, en de rijen onder een verwijderde rij schuiven omhoog. De tabel wordt korter, en de totaalrij schuift mee omhoog. Eén keer **Ongedaan maken** zet ze allemaal terug. Cellen onder de tabel blijven waar ze zijn. Formules die naar de cellen van de tabel verwijzen, op elk blad, volgen de cellen zoals ze dat doen bij het verwijderen van rijen; een formule die naar een verwijderde rij verwees, toont #VERW!. Dit werkt niet op een tabel met een filter erop.
- **Terug naar gewone cellen** zet de tabel om in gewone cellen. Ze behouden wat ze bevatten. Een formule die de tabel of zijn kolommen bij naam noemde, noemt daarna de cellen.

## Het menu van de cellen

Klik met de rechtermuisknop op een cel van een tabel en kies **Tabel**: **Rij boven invoegen**, **Rij onder invoegen**, **Kolom links invoegen**, **Kolom rechts invoegen**, **Tabelrijen verwijderen**, **Tabelkolommen verwijderen**, **Totaalrij** en **Terug naar gewone cellen**. Ze voegen hele rijen en kolommen van het blad in of verwijderen ze, zoals Invoegen en Verwijderen dat doen; een rij die net na het einde van de tabel wordt ingevoegd, wordt door de tabel opgenomen.

## Totaalrij

Zet **Totaalrij** aan en de rij onder de tabel bevat in de eerste kolom het woord Totaal en in de laatste kolom een totaal. Elke cel van de rij heeft een knop: kies wat het totaal van de kolom toont (het gemiddelde, een aantal, hoeveel getallen er zijn, de grootste, de kleinste, de som, de standaarddeviatie, de variantie, of **Eigen formule**). Een totaal is een SUBTOTAAL van zijn kolom, dus het laat rijen weg die een filter verbergt.

## Iets aan een tabel toevoegen

- Typ in de rij onder een tabel, of in de kolom ernaast, en de tabel groeit om de cel op te nemen. **Ongedaan maken** haalt de groei weg en laat wat u typte staan.
- Een formule die u typt in een cel van een kolom die verder leeg is, wordt in elke cel van die kolom gezet: een berekende kolom. **Ongedaan maken** direct daarna, en de formule blijft in de ene cel staan. Een formule die u typt over een berekende kolom verandert de hele kolom, wanneer de andere cellen de oude formule bevatten; als sommige dat niet doen, staat de cel op zichzelf.
- Rijen die binnen een tabel worden ingevoegd, krijgen de formules van de berekende kolommen. Kolommen die binnen de tabel worden ingevoegd, krijgen een naam in de rij met namen. Rijen of kolommen verwijderen binnen de tabel maakt hem kleiner; de rij met namen en elke kolom kunnen niet allemaal worden verwijderd.

## Een tabel filteren en sorteren

De knoppen op de namen filteren en sorteren alleen de rijen van de tabel. Ze werken zoals de knoppen van het filter van een blad werken (zie [Sorteren en filteren](sort-filter)). Een tabel kan niet worden gefilterd terwijl een filter van het blad of van een andere tabel dezelfde rijen omvat, omdat elk zou moeten weten welke rijen van hem zijn.

## Formules die een tabel noemen

- `Verkoop[Bedrag]` zijn de cellen van de kolom Bedrag, zonder de naam en het totaal.
- `[@Bedrag]` is de cel in de eigen rij van de formule, en wordt in het bestand geschreven als `Verkoop[[#Deze rij],[Bedrag]]`.
- `Verkoop[#Koppen]`, `Verkoop[#Gegevens]`, `Verkoop[#Totalen]` en `Verkoop[#Alles]` zijn de namen, de gegevens, de totalen en de hele tabel, en `Verkoop[[#Koppen],[Bedrag]]` is één kolom daarvan.
- `Verkoop[[Regio]:[Bedrag]]` zijn de kolommen van Regio tot en met Bedrag.

Typ `[` na de naam van een tabel, dan verschijnt een lijst van de kolommen. Terwijl u een formule typt, schrijft een klik op een cel van een tabel een verwijzing zoals `Verkoop[@Bedrag]` of `Verkoop[Bedrag]` (een klik op een hele kolom met gegevens geeft de laatste). Als u een kolom hernoemt in de koptekst, verandert dat in elke formule; kolommen invoegen en verwijderen houdt formules gericht op de kolommen die ze noemden, en een verwijderde kolom laat #VERW! achter in de formules die haar noemden.
