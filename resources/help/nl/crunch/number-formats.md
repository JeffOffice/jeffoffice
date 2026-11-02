# Getalnotaties en uw taal
<!-- keywords: getalnotatie, getalopmaak, getallen, decimaal, decimaalteken, decimalen, duizendtallen, scheidingsteken, groeperen, komma, punt, taal, landinstelling, valuta, procent, percentage, datum, tijd, wetenschappelijk, breuk, tekst, financieel, boekhoudkundig, standaard, aangepast, code, voorbeeld, Celeigenschappen, Ctrl+1, Ctrl+Shift+4, Ctrl+Shift+5, meer decimalen, minder decimalen, duizendtalnotatie, scheidingsteken in formules, puntkomma -->

Een cel bevat een getal, en de **getalnotatie** bepaalt hoe dat getal op het scherm wordt weergegeven. De notatie verandert wat u ziet, nooit wat de cel bevat. Voorbeeld: de cel bevat 0,256. Met de notatie Procent wordt **25,6%** weergegeven, met Valuta **€ 0,26** en met Standaard **0,256**. Klik op de cel en de formulebalk toont nog steeds 0,256.

## Getallen worden geschreven zoals uw computer ze schrijft

JeffCrunch gebruikt de tekens van de taal waarop uw computer is ingesteld. In het Engels (Verenigde Staten) is twaalfhonderdvierendertig en een half **1,234.5**. In het Duits is het **1.234,5** en in het Frans **1 234,5**. De werkmap verandert niet: die bewaart één neutrale vorm, zodat hetzelfde bestand bij de een 1,234.5 toont en bij de ander 1.234,5.

Dit geldt overal waar een getal wordt weergegeven of getypt:

- Een cel, de kopie van een cel en de breedte die **Kolombreedte aanpassen** aan een kolom geeft, volgen de tekens van uw taal.
- De **formulebalk** en een cel die u bewerkt met **F2** tonen een getal zoals u het typt. Als uw taal een decimaal met een komma schrijft, typt u **2,5** en niet 2.5.
- In een taal die decimalen met een komma schrijft, staan in een formule **puntkomma's** tussen de argumenten: **=SUM(A1;A2)**. De namen van de functies blijven Engels.
- Datums en tijden volgen de code van de notatie: een notatie die 'maand, dag, jaar' aangeeft, toont ze in die volgorde, en een notatie voor uw taal (de kant-en-klare notaties in de lijst) gebruikt de volgorde en de namen van uw taal.

Tekst die de notatie tussen aanhalingstekens zet, zoals "stuks", wordt weergegeven zoals hij is.

## Een notatie kiezen

1. Selecteer de cellen.
2. Gebruik de groep **Getal** op het tabblad **Start**, of druk op **Ctrl+1** (op een Mac **Command+1**) om **Celeigenschappen** te openen, dat opent op het tabblad **Getal**.

Het vak in de groep **Getal** bevat de gangbare notaties: Standaard, Getal, Valuta, Financieel, Korte datum, Lange datum, Tijd, Percentage, Breuk, Wetenschappelijk, Tekst en Aangepast. Met **Meer getalnotaties** onderaan opent u het venster Celeigenschappen. Onder het vak staan deze knoppen:

| Knop | Wat de knop doet |
| --- | --- |
| Valuta | Geeft het getal weer als geldbedrag, met het valutasymbool van uw taal (**Ctrl+Shift+4**). De pijl ernaast biedt Standaard en **Meer getalnotaties**. |
| Procentnotatie | Vermenigvuldigt met 100 en voegt een procentteken toe, zonder decimalen (**Ctrl+Shift+5**). Klik nogmaals om dit ongedaan te maken. |
| Duizendtalnotatie | Groepeert de cijfers per duizendtal, toont twee decimalen en lijnt de decimaaltekens in een kolom onder elkaar uit, zonder symbool. |
| Meer decimalen, Minder decimalen | Voegt één decimaal toe of haalt er één weg. |
| Standaard (in het vak en in de pijl naast Valuta) | Verwijdert de notatie: het getal wordt weergegeven zoals het is (**Ctrl+Shift+~**). |

## Het tabblad Getal van Celeigenschappen

Kies links een **Categorie**; rechts verschijnen de bijbehorende opties, en **Voorbeeld** laat zien hoe de eerste geselecteerde cel eruit komt te zien, in uw taal.

- **Getal**: het aantal decimalen, of de cijfers per duizendtal worden gegroepeerd, en hoe negatieve getallen eruitzien (met een minteken, in rood of tussen haakjes).
- **Valuta** en **Financieel**: het aantal decimalen en een symbool. Financieel lijnt de symbolen en de decimaaltekens in een kolom onder elkaar uit.
- **Datum** en **Tijd**: een lijst met kant-en-klare notaties, met de volgorde en de namen van uw taal.
- **Percentage**, **Wetenschappelijk**: het aantal decimalen.
- **Breuk**: kant-en-klare notaties zoals 'Maximaal één cijfer' (1/4), 'Maximaal twee cijfers' (21/25), 'Als kwarten' en 'Als zestienden'.
- **Speciaal**: kant-en-klare notaties voor een postcode, een postcode + 4, een telefoonnummer en een burgerservicenummer.
- **Tekst**: de cel bewaart wat u typt als tekst, zodat een getal als 00123 zijn nullen behoudt.
- **Aangepast**: typ een eigen code, of begin met een code uit de lijst (de gangbare codes en de codes die de werkmap al gebruikt).

De **code** is de korte tekst die JeffCrunch vertelt hoe een getal moet worden geschreven. Voorbeeld: **#,##0.00** betekent 'groepeer de cijfers per duizendtal en toon altijd twee decimalen', zodat 1234,5 wordt weergegeven als 1.234,50. Elke **0** is een cijfer dat altijd wordt weergegeven, elke **#** een cijfer dat alleen wordt weergegeven als het nodig is, en **%** vermenigvuldigt met 100. Een code kan uit maximaal vier secties bestaan, gescheiden door puntkomma's: voor positieve getallen, negatieve getallen, nul en tekst.

## Wat bewaard blijft

De code wordt in de werkmap opgeslagen in de neutrale vorm, zodat de werkmap op elke computer goed wordt gelezen. Opmaak toepassen is één stap die u ongedaan kunt maken, en een werkmap die al een notatie gebruikte, behoudt die precies: een notatie die in de lijst staat, wordt teruggevonden en geen enkele wordt dubbel gemaakt.
