# Vergelijkingen

<!-- keywords: tabblad vergelijking, structuren, symbolen, converteren, vergelijking, vergelijkingen, wiskunde, formule, breuk, vierkantswortel, superscript, subscript, exponent, som, integraal, grieks, symbool, vergelijking invoegen, accent, matrix, haakjes, ingebouwde vergelijkingen, galerie, vergelijking wijzigen, vergelijking bewerken, ods, afbeelding, pdf -->

Een **vergelijking** is een regel wiskunde, zoals een breuk of een vierkantswortel. In JeffCrunch is een vergelijking een object boven de cellen, net als een afbeelding of een tekstvak. Het is geen cel: ze wijzigt geen enkel getal op het blad.

## Een vergelijking op het blad zetten

1. Klik op **Invoegen > Symbolen > Vergelijking**.
2. Typ de vergelijking in één regel. Onder het vak ziet u hoe ze eruitziet. Klik op **OK**.

De vergelijking wordt geplaatst op de cel waarin u staat, en is geselecteerd. Wilt u beginnen met een bekende vergelijking, klik dan op **Ingebouwde vergelijkingen** en kies er een, zoals de abc-formule of de oppervlakte van een cirkel.

## Hoe u typt

- **Breuken:** typ een schuine streep. `a/b` is a boven b. `(x+1)/(x-1)` neemt elke groep tussen haakjes als één deel.
- **Machten en subscripts:** `x^2` is x in het kwadraat, en `x_i` is x met een kleine i eronder.
- **Wortels:** typ `sqrt(x+1)` voor een vierkantswortel.
- **Sommen, integralen en producten:** typ `\sum_(i=1)^n i`, `\int_0^1 x` of `\prod_(k=1)^n k`.
- **Symbolen:** typ een backslash en een naam. `\alpha` is α, `\pi` is π, `\infty` is ∞ en `\leq` is ≤.
- **Accenten, haakjes en matrices:** `\hat{x}`, `\left( ... \right)` en `\pmatrix{a & b \\ c & d}`.

## Typen in UnicodeMath of LaTeX

Naast het vak staat een lijst met typeopmaken: **Eenvoudige regel** (de hierboven beschreven manier), **UnicodeMath** en **LaTeX**. Dit zijn twee manieren om wiskunde als gewone tekst te schrijven die veel mensen al kennen. Wanneer u een andere opmaak kiest, wordt wat u hebt getypt in die opmaak opnieuw geschreven.

- **UnicodeMath:** `a/b` is een breuk, `x^2_i` heeft een macht en een subscript, `√(x)` is een wortel, en `∑_(i=1)^n i` is een som. Zet woorden tussen aanhalingstekens, zoals `"als x"`, dan blijven ze gewone tekst in plaats van wiskundige letters. `(a|b)` maakt haakjes met twee delen, en `\matrix(a&b@c&d)` maakt een matrix (`&` tussen cellen, `@` tussen rijen).
- **LaTeX:** `\frac{a}{b}` is een breuk, `\sqrt[3]{x}` is een derdemachtswortel, `\sum_{i=1}^n i` is een som, `\text{als x}` is gewone tekst, en `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` is een matrix.

Met de knoppen **Professioneel** en **Lineair** bepaalt u hoe het vak uw vergelijking toont. Professioneel tekent de vergelijking opgebouwd, zoals ze eruit zal zien. Lineair toont alleen de regel die u hebt getypt. Als een vergelijking niet in de gekozen opmaak kan worden geschreven, meldt het vak dit en behoudt het wat u hebt getypt.

## Het tabblad Vergelijking

Kies een vergelijking, dan verschijnt aan het eind van het lint een tabblad met de naam **Vergelijking**. Het verdwijnt wanneer u iets anders selecteert.

- **Hulpmiddelen:** **Vergelijking bewerken** opent het vak. **Professioneel** en **Lineair** kiezen hoe het vak begint: opgebouwd of alleen als regel. **Normale tekst** opent het vak met een paar aanhalingstekens erbij, voor woorden die geen wiskunde zijn.
- **Converteren:** kies de opmaak waarin het vak opent: **Eenvoudige regel**, **UnicodeMath** of **LaTeX**. De volgende keer dat u een vergelijking bewerkt, wordt ze geschreven in de opmaak die u hebt gekozen. Een vergelijking die geen regel van het eenvoudige soort kan schrijven, zoals een som met grenzen boven en onder, opent in plaats daarvan in UnicodeMath.
- **Symbolen:** Griekse letters, operatoren, relaties, pijlen, en verzamelingen en logica. Kies een symbool, dan opent het vak met het symbool aan de regel toegevoegd.
- **Structuren:** breuk, index, wortel, integraal, grote operator, haakje, functie, accent, limiet, operator en matrix. Klik op een knop voor de gebruikelijke vorm, of op de pijl ernaast voor meer vormen, zoals een afgeleide, een wortel met een graad of een eenheidsmatrix.

Een structuur wordt aan het eind van de regel geplaatst, met voorbeeldletters erin, en de eerste wordt geselecteerd zodat u eroverheen kunt typen. Het vak heeft dezelfde structuren in een rij kleine knoppen boven de regel. Daar wordt een structuur op de plaats van de cursor ingevoegd, en als een deel van de regel is geselecteerd, gaat dat deel naar de eerste plaats van de structuur.

## Wijzigen, verplaatsen en formaat

Dubbelklik op de vergelijking, of selecteer haar en klik op **Invoegen > Symbolen > Vergelijking**, om het vak met de regel te openen. Wijzig de vergelijking en klik op **OK**. Ongedaan maken kost één stap. Sleep de vergelijking om haar te verplaatsen, en sleep een greep om haar formaat te wijzigen; de vergelijking behoudt binnen het vak haar verhoudingen.

## Opslaan

- **.xlsx** bewaart de vergelijking als vergelijking, in de standaardvorm voor wiskunde, met een afbeelding ernaast voor programma's die geen wiskunde kunnen lezen. Als u het bestand opnieuw opent, wordt de vergelijking getoond. Een vergelijking die in een ander programma is gemaakt, wordt op dezelfde manier getoond; als de wiskunde niet kan worden gelezen, ziet u de afbeelding ervan.
- **PDF en afdrukken** tekenen de vergelijking als vormen, zodat ze op elk formaat scherp blijft.
- **.ods**-kopieën hebben geen vergelijkingen. JeffCrunch slaat elke vergelijking op als afbeelding ervan, op dezelfde plaats, en noemt de vergelijkingen wanneer het u vertelt wat de kopie weglaat.
