# Tabellen
<!-- keywords: tabel, tabellen, gestructureerde verwijzing, tabelverwijzing, koprij, rij met namen, kolomnamen, totaalrij, totalen, berekende kolom, gestreept, gestreepte rijen, stijl, tabelstijl, tabeluiterlijk, filterknoppen, Ctrl+T, converteren, gewone cellen, bereik, grootte wijzigen, kolomnaam, [@Column], Table1[Column] -->

Een tabel is een blok cellen met een rij namen boven de kolommen, dat als één geheel wordt behandeld. Een tabel heeft een naam en een uiterlijk, heeft knoppen om de rijen te filteren en te sorteren, en wordt groter als u er iets aan toevoegt. In formules kunt u de kolommen bij hun naam noemen in plaats van met een celadres.

## Een tabel maken

Selecteer de cellen, of klik in een cel van de gegevens, en kies **Tabel** op het tabblad **Invoegen** (**Ctrl+T**). JeffCrunch vraagt welke cellen de tabel beslaat en of de eerste rij de namen van de kolommen bevat. Namen die getallen, datums of formules zijn, worden de tekst die ze weergeven. Lege namen worden Column1, Column2 enzovoort, en als twee kolommen dezelfde naam hebben, worden ze genummerd. Als er geen namen zijn, heeft de tabel geen rij met namen en heten de kolommen in formules Column1, Column2 enzovoort.

De galerie **Tabeluiterlijk** (op het tabblad **Start** of het tabblad **Tabel**) toont de beschikbare stijlen: licht, gemiddeld en donker, telkens in zwart en in de zes kleuren van het thema van de werkmap. Als u een stijl kiest voor cellen die nog geen tabel zijn, worden ze een tabel. De stijl ligt onder de eigen opmaak van een cel: een opvulling, randen, een tekstkleur of vet die u zelf op een cel hebt ingesteld, blijft behouden.

## Het tabblad Tabel

Het tabblad **Tabel** wordt weergegeven zolang de cursor in een tabel staat.

- **Naam** is de naam waarmee formules naar de tabel verwijzen. Alle formules die de oude naam gebruiken, worden aangepast. Een naam bestaat uit letters, cijfers, onderstrepingstekens en punten, begint met een letter of een onderstrepingsteken, en mag niet gelijk zijn aan de naam van een andere tabel, een naam in de werkmap of een celadres zoals A1.
- **Rij met namen**, **Totaalrij**, **Gestreepte rijen**, **Gestreepte kolommen**, **Eerste kolom**, **Laatste kolom** en **Filterknoppen** schakelt u voor de tabel in of uit.
- **Tabelgrootte wijzigen…** vraagt welke cellen de tabel beslaat. De eerste cel blijft hetzelfde; u kunt onderaan en rechts rijen en kolommen toevoegen of weghalen.
- **Terug naar gewone cellen** maakt van de tabel weer gewone cellen. De inhoud blijft behouden. Een formule die naar de tabel of de kolommen ervan verwees, verwijst voortaan naar de cellen.

## Het snelmenu van de cellen

Klik met de rechtermuisknop op een cel van een tabel en kies **Tabel**: **Rij boven invoegen**, **Rij onder invoegen**, **Kolom links invoegen**, **Kolom rechts invoegen**, **Tabelrijen verwijderen**, **Tabelkolommen verwijderen**, **Totaalrij** en **Terug naar gewone cellen**. Hiermee voegt u hele rijen en kolommen van het blad in of verwijdert u ze, net als met Invoegen en Verwijderen. Een rij of kolom die direct na het einde van de tabel wordt ingevoegd, wordt in de tabel opgenomen.

## Totaalrij

Als u **Totaalrij** inschakelt, staat in de rij onder de tabel het woord Totaal in de eerste kolom en een totaal in de laatste. Elke cel van die rij heeft een knop waarmee u kiest wat het totaal van de kolom weergeeft: het gemiddelde, een aantal, het aantal getallen, het grootste getal, het kleinste getal, de som, de standaarddeviatie, de variantie of **Eigen formule…**. Een totaal is een SUBTOTAL van de kolom en laat dus rijen weg die door een filter verborgen zijn.

## Iets aan een tabel toevoegen

- Typ in de rij onder een tabel of in de kolom ernaast, en de tabel wordt groter zodat de cel erbij hoort. Met **Ongedaan maken** maakt u de uitbreiding ongedaan, terwijl wat u hebt getypt blijft staan.
- Een formule die u typt in een cel van een verder lege kolom, wordt in elke cel van die kolom geplaatst: een berekende kolom. Kiest u direct daarna **Ongedaan maken**, dan blijft de formule alleen in die ene cel staan. Een formule die u over een berekende kolom typt, wijzigt de hele kolom als de andere cellen de oude formule bevatten; is dat bij sommige niet zo, dan staat de cel op zichzelf.
- Rijen die binnen een tabel worden ingevoegd, krijgen de formules van de berekende kolommen. Kolommen die binnen de tabel worden ingevoegd, krijgen een naam in de rij met namen. Als u rijen of kolommen binnen de tabel verwijdert, wordt de tabel kleiner; de rij met namen en alle kolommen kunnen niet allemaal worden verwijderd.

## Een tabel filteren en sorteren

Met de knoppen bij de namen filtert en sorteert u alleen de rijen van de tabel. Ze werken net als de knoppen van een filter op een blad (zie [Sorteren en filteren](sort-filter)). Een tabel kan niet worden gefilterd als een filter van het blad of van een andere tabel dezelfde rijen beslaat, omdat elk filter moet weten welke rijen bij hem horen.

## Formules die naar een tabel verwijzen

- `Sales[Amount]` zijn de cellen van de kolom Amount, zonder de naam en het totaal.
- `[@Amount]` is de cel in dezelfde rij als de formule; in het bestand wordt dit geschreven als `Sales[[#This Row],[Amount]]`.
- `Sales[#Headers]`, `Sales[#Data]`, `Sales[#Totals]` en `Sales[#All]` zijn de namen, de gegevens, de totalen en de hele tabel, en `Sales[[#Headers],[Amount]]` is één kolom daarvan.
- `Sales[[Region]:[Amount]]` zijn de kolommen van Region tot en met Amount.

Typ `[` na de naam van een tabel en er verschijnt een lijst met de kolommen. Als u tijdens het typen van een formule op een cel van een tabel klikt, wordt een verwijzing zoals `Sales[@Amount]` of `Sales[Amount]` (als u een hele kolom met gegevens aanklikt) geschreven. Als u een kolom een andere naam geeft in de rij met namen, wordt die naam in elke formule aangepast. Bij het invoegen en verwijderen van kolommen blijven formules verwijzen naar de kolommen die ze noemden; een verwijderde kolom laat #REF! achter in de formules die ernaar verwezen.
