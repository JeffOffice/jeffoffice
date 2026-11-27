# Bladen werken

<!-- keywords: blad, bladen, tabblad, tabbladen, toevoegen, invoegen, nieuw blad, verwijderen, wissen, hernoemen, naam, verplaatsen, kopiëren, dupliceren, opnieuw ordenen, slepen, verbergen, zichtbaar maken, tonen, tabbladkleur, kleur, meerdere bladen selecteren, Shift+F11, #VERW!, 3D-verwijzing, Verplaatsen of kopiëren -->

Een werkmap bestaat uit **bladen**, en de **tabbladen** onderaan het venster zijn hun namen. Klik op een tabblad om dat blad te tonen. Elke wijziging hieronder is één stap die u ongedaan kunt maken en wordt in de werkmap opgeslagen.

## Een blad toevoegen

- Klik op de knop **+** na het laatste tabblad om een blad aan het eind toe te voegen.
- Druk op **Shift+F11** of gebruik **Start > Invoegen > Blad invoegen** om een blad vóór het blad te voegen waarnaar u kijkt.

Het nieuwe blad is leeg, krijgt de eerstvolgende vrije naam (Blad4, Blad5, enzovoort) en wordt meteen geopend. Een werkmap kan maximaal 1.000 bladen hebben. Daarboven meldt JeffCrunch dat en voegt het er geen toe, omdat een bestand met meer bladen niet door JeffCrunch wordt geopend.

## Een blad hernoemen

**Dubbelklik** op het tabblad, typ de nieuwe naam en druk op **Enter** (**Esc** annuleert). U kunt ook **Start > Opmaak > Blad hernoemen** gebruiken.

Een naam mag maximaal 31 tekens bevatten. Hij mag niet leeg zijn, geen van de tekens **: \ / ? * [ ]** bevatten, niet beginnen of eindigen met een apostrof en niet gelijk zijn aan de naam van een ander blad (hoofdletters maken geen verschil). De naam "History" is voor het programma gereserveerd. Wordt de naam geweigerd, dan meldt JeffCrunch waarom en behoudt het blad zijn naam.

Elke formule die het blad noemt, volgt de nieuwe naam. Voorbeeld: **=Budget!B2** wordt **='Q1 Budget'!B2** wanneer u Budget hernoemt naar Q1 Budget. Namen die naar het blad verwijzen, de regels van voorwaardelijke opmaak en validatie, en grafieken volgen de nieuwe naam eveneens.

## Een blad verwijderen

Klik met de rechtermuisknop op het tabblad en kies **Blad verwijderen**, of gebruik **Start > Verwijderen > Blad verwijderen**. Staat er iets op het blad, dan vraagt JeffCrunch eerst om bevestiging. Formules op andere bladen die het verwijderde blad lezen, tonen **#VERW!**, wat betekent dat wat ze lezen niet meer bestaat. **Ongedaan maken** haalt het blad en die formules terug.

Een werkmap houdt altijd minstens één blad dat zichtbaar is.

## Een blad verplaatsen of kopiëren

- **Sleep** een tabblad naar een nieuwe plaats om het blad te verplaatsen.
- Of gebruik **Start > Opmaak > Blad verplaatsen of kopiëren**. Kies voor welk blad het gekozen blad moet komen (of kies **(na het laatste blad)**), en vink **Kopiëren en de originelen laten staan** aan om het blad te kopiëren in plaats van te verplaatsen. Een kopie krijgt een naam zoals **Budget (2)** en wordt geopend.

Formules blijven de bladen lezen die ze lezen, waar die bladen ook staan. Een kopie heeft dezelfde cellen, formules, opmaak en afmetingen als het blad waarvan hij is gemaakt. Een verwijzing zonder bladnaam (zoals **A1**) leest de eigen cel van de kopie; een verwijzing die een blad noemt (zoals **Blad1!A1**) noemt nog steeds dat blad, ook als dat het blad is waarvan de kopie is gemaakt.

Een kopie heeft ook de beveiliging, het filter, de koppelingen, de pagina-instelling en pagina-einden, de groepen van rijen en kolommen en de weergave van de gegevensbalken van het origineel. Koppelingen volgen de rijen die u invoegt of verwijdert: in het origineel vóór de kopie en in de kopie erna. Afbeeldingen, grafieken, notities, draaitabellen, sparklines en de namen die alleen bij het origineel horen, worden niet gekopieerd, omdat JeffCrunch dat nog niet kan. Wanneer dit gebeurt, vertelt het programma u welke onderdelen dat zijn. Een blad met tabellen wordt met kopieën daarvan gekopieerd (elk met een eigen naam, id en onderdeel; de formules van de kopie noemen de eigen tabellen). Het verwijderen van een blad verwijdert ook zijn tabellen; een formule op een ander blad die er een noemde, krijgt dan #VERW!, en **Ongedaan maken** haalt die terug.

Om een blad naar **een andere werkmap** te kopiëren die open is, kiest u die werkmap bij **Naar werkmap** in hetzelfde vak. Het vak toont wat de kopie niet zal hebben (voorlopig de afbeeldingen, grafieken, voorwaardelijke opmaak, tabellen, notities en de manier waarop het blad wordt afgedrukt) en zegt dat voordat er iets wordt gekopieerd. Formules die andere bladen van de eerste werkmap lezen, worden koppelingen naar het bestand daarvan. De eerste werkmap moet daarom minstens één keer als .xlsx-bestand zijn opgeslagen. De kopie toont de waarden die de eerste werkmap nu heeft; ze veranderen wanneer de koppelingen worden bijgewerkt (**Waarden bijwerken** in **Gegevens > Koppelingen bewerken**). Als de eerste werkmap niet-opgeslagen wijzigingen heeft, lezen de koppelingen het opgeslagen bestand zodra ze worden bijgewerkt; sla de werkmap daarom op om deze waarden te behouden. Formules die het gekopieerde blad zelf lezen, blijven op de kopie staan. Cellen die namen, tabellen, een matrix of een overloop gebruiken, of die een derde werkmap lezen, komen mee als de waarden die ze tonen; het vak zegt hoeveel het er zijn. Hoe het blad wordt weergegeven (vastgezette rijen en kolommen, zoom, rasterlijnen en leesrichting) en de standaard rijhoogte en kolombreedte komen ook mee. De kopie heeft dezelfde uitstraling: opmaak, lettertypen en kleuren worden aan de andere werkmap toegevoegd (een kleur uit het thema van de eerste werkmap blijft behouden als de kleur die ze nu toont, wanneer de twee thema's verschillen). Beide werkmappen moeten datums tellen vanaf hetzelfde jaar (1900 of 1904). **Ongedaan maken** in de andere werkmap haalt de kopie weg; de eerste werkmap wordt niet gewijzigd. Een blad naar een andere werkmap verplaatsen is nog niet mogelijk.

## Een blad verbergen en weer tonen

Klik met de rechtermuisknop op het tabblad en kies **Blad verbergen** (of gebruik **Start > Opmaak > Blad verbergen**). Het blad wordt niet getoond, maar het blijft in de werkmap en formules lezen het nog steeds. Om het weer te tonen, klikt u met de rechtermuisknop op een tabblad, kiest u **Blad zichtbaar maken** en klikt u daarna op de naam in de lijst. De lijst is leeg wanneer geen blad verborgen is.

## Een tabblad kleuren

Klik met de rechtermuisknop op het tabblad en kies **Tabbladkleur**, kies daarna een kleur of kies **Geen kleur**. De kleur verschijnt als een lijn onder de naam. Een kleur die u uit het themapalet kiest, wordt opgeslagen als de gewone kleur die het nu is.

## Meerdere bladen tegelijk

**Ctrl+klik** op tabbladen om er meer dan één te kiezen, of klik op één tabblad en **Shift+klik** op een ander om alle tabbladen daartussen te kiezen. De gekozen tabbladen worden gearceerd. **Verwijderen**, **Verbergen**, **Tabbladkleur** en **Verplaatsen of kopiëren** werken daarna op alle bladen tegelijk als één stap. Klik op een tabblad dat niet gekozen is om alleen dat blad te kiezen.

Wat u typt of opmaakt terwijl meerdere bladen zijn gekozen, verandert alleen het blad waarnaar u kijkt.

## Wat niet kan

- Een werkmap met een formule over een reeks bladen, zoals **=SOM(Blad1:Blad3!A1)**, kan voorlopig geen bladen krijgen, verwijderd, hernoemd of verplaatst worden, omdat dat wijzigt welke bladen de formule omvat.
- Na het opslaan kan **Ongedaan maken** niet verder teruggaan dan een blad dat vóór het opslaan is toegevoegd, gekopieerd, verwijderd, hernoemd of verplaatst. Het opgeslagen bestand bevat ze zoals ze zijn. Verbergen, weer tonen en de tabbladkleur kunnen nog steeds ongedaan worden gemaakt.
