# Getalnotaties en uw taal
<!-- keywords: getalnotatie, getallen, decimaal, decimaalteken, decimalen, duizendtallen, scheidingsteken, groeperen, komma, punt, taal, landinstelling, valuta, procent, percentage, datum, tijd, wetenschappelijk, breuk, tekst, boekhoudkundig, algemeen, aangepast, code, voorbeeld, Cellen opmaken, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, decimaal toevoegen, decimaal verwijderen, duizendtalnotatie, formulescheidingsteken, puntkomma -->

Een cel bevat een getal, en de **getalnotatie** bepaalt hoe dat getal op het scherm wordt geschreven. De notatie verandert wat u ziet, nooit wat de cel bevat. Voorbeeld: de cel bevat 0,256. Met de notatie Percentage toont hij **25,6%**, met Valuta toont hij **€ 0,26**, en met Algemeen toont hij **0,256**. Klikt u op de cel, dan staat in de formulebalk nog steeds 0,256.

## Getallen worden geschreven zoals uw computer ze schrijft

JeffCrunch gebruikt de tekens van de taal die op uw computer is ingesteld. In het Engels (Verenigde Staten) is één duizend tweehonderdvierendertig en een half **1,234.5**. In het Duits is het **1.234,5**, en in het Frans **1 234,5**. De werkmap verandert niet: die bewaart één neutrale vorm, zodat hetzelfde bestand voor de ene persoon 1,234.5 toont en voor de andere 1.234,5.

Dit geldt overal waar een getal wordt getoond of getypt:

- Een cel, de kopie van een cel, en de breedte die **Kolombreedte aanpassen** aan een kolom geeft, volgen de tekens van uw taal.
- De **formulebalk** en een cel die u met **F2** bewerkt, tonen een getal zoals u het typt. Als uw taal decimalen met een komma schrijft, typt u **2,5**, niet 2.5.
- In een taal die decimalen met een komma schrijft, staan in een formule **puntkomma's** tussen de argumenten: **=SOM(A1;A2)**. De namen van de functies blijven in het Engels.
- Datums en tijden volgen de code van de notatie: een notatie die "maand, dag, jaar" zegt, toont ze in die volgorde, en een notatie die voor uw taal is bedoeld (de kant-en-klare in de lijst) gebruikt de volgorde en de namen van uw taal.

Tekst die de notatie tussen aanhalingstekens zet, zoals "stuks", wordt precies zo getoond.

## Een notatie kiezen

1. Selecteer de cellen.
2. Gebruik de groep **Getal** op het tabblad **Start**, of druk op **Ctrl+1** (op een Mac **Command+1**) om **Cellen opmaken** te openen, die op het tabblad **Getal** begint.

Het vak in de groep **Getal** bevat de veelgebruikte notaties: Algemeen, Getal, Valuta, Boekhoudnotatie, Korte datum, Lange datum, Tijd, Percentage, Breuk, Wetenschappelijk, Tekst en Aangepast. **Meer getalnotaties** aan het eind opent het venster Cellen opmaken. De knoppen onder het vak zijn:

| Knop | Wat hij doet |
| --- | --- |
| Valuta | Toont het getal als geld, met het valutateken van uw taal (**Ctrl+Shift+4**). De pijl ernaast biedt **Boekhoudnotatie** met een valuta om uit te kiezen (Amerikaanse dollar, Britse pond, euro, Japanse yen, Chinese yuan of Zwitserse frank), Algemeen en **Meer getalnotaties**. |
| Procentstijl | Vermenigvuldigt met 100 en voegt een procentteken toe, zonder decimalen (**Ctrl+Shift+5**). Klik er nogmaals op om die weg te halen. |
| Duizendtalnotatie | Groepeert de cijfers per duizendtal, toont twee decimalen en lijnt de decimaalkomma's onder elkaar uit, zonder teken. |
| Decimaal toevoegen, Decimaal verwijderen | Voegt één decimaal toe of haalt er één weg. |
| Algemeen (in het vak, en in de pijl naast Valuta) | Haalt de notatie weg: het getal wordt getoond zoals het is (**Ctrl+Shift+~**). |

## Het tabblad Getal van Cellen opmaken

Kies links een **Categorie**; de opties ervoor verschijnen rechts, en het **Voorbeeld** laat zien hoe de eerste geselecteerde cel er in uw taal uitziet.

- **Getal**: decimalen, of de cijfers per duizendtal worden gegroepeerd, en hoe negatieve getallen eruitzien (een minteken, rood, of tussen haakjes).
- **Valuta** en **Boekhoudkundig**: decimalen en een teken. Boekhoudkundig lijnt de tekens en de decimaalkomma's onder elkaar uit.
- **Datum** en **Tijd**: een lijst met kant-en-klare notaties, met de volgorde en de namen van uw taal.
- **Percentage**, **Wetenschappelijk**: decimalen.
- **Breuk**: kant-en-klare notaties zoals "maximaal één cijfer" (1/4), "maximaal twee cijfers" (21/25), "als kwarten" en "als zestienden".
- **Speciaal**: kant-en-klare notaties voor een postcode, een postcode plus 4, een telefoonnummer en een burgerservicenummer.
- **Tekst**: de cel bevat wat u typt als tekst, zodat een getal zoals 00123 zijn nullen houdt.
- **Aangepast**: typ een eigen code, of begin met een code uit de lijst (de veelgebruikte codes, en die de werkmap al gebruikt).

De **code** is de korte tekst die JeffCrunch vertelt hoe een getal moet worden geschreven. Voorbeeld: **#.##0,00** betekent "groepeer de cijfers per duizendtal en toon altijd twee decimalen", zodat 1234,5 als 1.234,50 wordt getoond. Elke **0** is een cijfer dat altijd wordt getoond, elke **#** een cijfer dat alleen wordt getoond als het nodig is, en **%** vermenigvuldigt met 100. Een code kan uit maximaal vier delen bestaan, gescheiden door puntkomma's, voor positieve getallen, negatieve getallen, nul en tekst.

## Wat wordt bewaard

De code wordt in de werkmap in de neutrale vorm bewaard, zodat de werkmap op elke computer goed wordt weergegeven. Opmaak is één stap die u ongedaan kunt maken, en een werkmap die al een notatie gebruikte, behoudt die precies: een notatie die in de lijst staat, wordt weer gevonden, en er wordt er geen twee keer gemaakt.
