# Een vergelijking toevoegen
<!-- keywords: vergelijkingstabblad, structuren, symbolen, converteren, vergelijking, vergelijkingen, wiskunde, formule, breuk, vierkantswortel, exponent, index, bovenindex, onderindex, som, integraal, grieks, symbool, vergelijking invoegen, alt+=, accent, matrix, haakjes, ingebouwde vergelijkingen, galerie, vergelijking wijzigen, vergelijking bewerken, vector, pdf, jpub, pub-bestand -->

Een **vergelijking** is een regel wiskunde, zoals een breuk of een vierkantswortel. In JeffPub is een vergelijking een object op de pagina, net als een afbeelding: u kunt het verplaatsen, het formaat wijzigen, het draaien en er een kleur aan geven.

## Een vergelijking op de pagina zetten

1. Klik op **Invoegen > Tekst > Vergelijking**, of druk op **Alt+=**.
2. Typ de vergelijking in één regel. Onder het vak ziet u hoe deze eruitziet. Klik op **OK**.

De vergelijking komt midden op de pagina te staan, geselecteerd. Wilt u beginnen met een bekende vergelijking, klik dan op de pijl naast **Vergelijking** en kies een ingebouwde vergelijking, zoals de kwadratische formule of de oppervlakte van een cirkel.

## Zo typt u een vergelijking

- **Breuken:** typ een schuine streep. `a/b` is a gedeeld door b. `(x+1)/(x-1)` neemt elke groep tussen haakjes als één deel.
- **Machten en indices:** `x^2` is x kwadraat, en `x_i` is x met een kleine i eronder. U kunt beide gebruiken, zoals in `x_i^2`.
- **Wortels:** typ `sqrt(x+1)` voor een vierkantswortel, of `sqrt[3]{x}` voor een derdemachtswortel.
- **Sommen, integralen en producten:** typ `\sum_(i=1)^n i`, `\int_0^1 x`, of `\prod_(k=1)^n k`. Het deel na de underscore komt eronder, en het deel na het dakje erboven.
- **Symbolen:** typ een backslash en een naam. `\alpha` is α, `\pi` is π, `\infty` is ∞, `\leq` is ≤, en `\times` is ×. Ongeveer 70 namen werken.
- **Accenten:** `\hat{x}`, `\vec{v}` en `\bar{x}` zetten een teken boven de letter.
- **Haakjes die meegroeien:** typ `\left( ... \right)`, dan groeien de haakjes mee met de hoogte van wat ertussen staat.
- **Matrices:** een **matrix** is een raster van getallen. Typ `\pmatrix{a & b \\ c & d}`. Zet een `&` tussen de cellen van een rij, en twee backslashes tussen de rijen.

## Typen in UnicodeMath of LaTeX

Naast het vak staat een lijst met invoerformaten: **Eenvoudige regel** (de hierboven gebruikte), **UnicodeMath** en **LaTeX**. Dit zijn twee manieren om wiskunde als gewone tekst te schrijven, die veel mensen al kennen. Wanneer u een ander formaat kiest, wordt wat u hebt getypt opnieuw in dat formaat geschreven.

- **UnicodeMath:** `a/b` is een breuk, `x^2_i` heeft een macht en een index, `√(x)` is een wortel, en `∑_(i=1)^n i` is een som. Zet woorden tussen aanhalingstekens, zoals `"als x"`, dan blijven ze gewone tekst in plaats van wiskundige letters. `(a|b)` maakt haakjes met twee delen, en `\matrix(a&b@c&d)` maakt een matrix (`&` tussen cellen, `@` tussen rijen).
- **LaTeX:** `\frac{a}{b}` is een breuk, `\sqrt[3]{x}` is een derdemachtswortel, `\sum_{i=1}^n i` is een som, `\text{als x}` is gewone tekst, en `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` is een matrix.

Met de twee knoppen **Professioneel** en **Lineair** verandert u de manier waarop het vak uw vergelijking toont. Professioneel tekent de vergelijking opgebouwd, zoals ze eruit zal zien. Lineair toont alleen de regel die u hebt getypt. Als een vergelijking niet in het gekozen formaat kan worden geschreven, meldt het vak dat en behoudt het wat u hebt getypt.

## Een vergelijking wijzigen

Selecteer de vergelijking en druk op Enter, of dubbelklik erop, of klik op **Invoegen > Tekst > Vergelijking**. Het vak opent met de vergelijking uitgeschreven in één regel. Wijzig deze en klik op **OK**. Eén stap is voldoende om het ongedaan te maken.

Een vergelijking die uit een bestand is geopend, wordt geopend in UnicodeMath, of in LaTeX wanneer UnicodeMath deze niet kan schrijven. Als geen van beide dat kan, begint het vak met de tekst van de vergelijking. Klik op **OK** zonder wijziging, of op **Annuleren**, om de vergelijking te laten zoals ze is, of typ een nieuwe om haar te vervangen.

## Het tabblad Vergelijking

Selecteer een vergelijking, dan verschijnt aan het eind van het lint een tabblad met de naam **Vergelijking**. Het verdwijnt wanneer u iets anders selecteert.

- **Hulpmiddelen:** **Vergelijking bewerken** opent het vak. **Professioneel** en **Lineair** veranderen de manier waarop de geselecteerde vergelijking op de pagina wordt getoond: Professioneel tekent haar opgebouwd, en Lineair toont de regel waarin ze is getypt. De keuze wordt bewaard bij de vergelijking in het `.jpub`-bestand, zodat ze hetzelfde is wanneer u het bestand opnieuw opent. Een `.pub`-bestand krijgt de afbeelding van de vergelijking zoals die op de pagina wordt getoond. **Normale tekst** opent het vak met een paar aanhalingstekens erbij, voor woorden die geen wiskunde zijn.
- **Converteren:** kies het formaat waarin het vak opent: **Eenvoudige regel**, **UnicodeMath** of **LaTeX**. De volgende keer dat u een vergelijking bewerkt, wordt ze geschreven in het formaat dat u hebt gekozen. Een vergelijking die geen regel van het eenvoudige type kan schrijven, zoals een som met grenzen boven en onder, opent in UnicodeMath.
- **Symbolen:** Griekse letters, operatoren, relaties, pijlen, en verzamelingen en logica. Kies een symbool, dan opent het vak met het symbool toegevoegd aan de regel.
- **Structuren:** breuk, index, radicaal, integraal, grote operator, haakje, functie, accent, limiet, operator en matrix. Klik op een knop voor de gebruikelijke vorm, of op de pijl ernaast voor meer vormen, zoals een afgeleide, een wortel met een graad, of een eenheidsmatrix.

Een structuur wordt aan het eind van de regel ingevoegd, met voorbeeldletters erin, en de eerste wordt geselecteerd zodat u eroverheen kunt typen. Het vak heeft dezelfde structuren in een rij kleine knoppen boven de regel. Daar wordt een structuur ingevoegd op de plaats van de cursor, en als een deel van de regel is geselecteerd, gaat dat deel naar de eerste plaats van de structuur.

## Verplaatsen, formaat wijzigen en kleuren

Sleep de vergelijking om haar te verplaatsen. Sleep een hoek om haar groter of kleiner te maken; de vergelijking behoudt haar verhoudingen. Gebruik **Vormopvulling** om de kleur van de letters te wijzigen.

## Opslaan

- **.jpub** bewaart de vergelijking, zodat u haar opnieuw kunt bewerken.
- **PDF en afdrukken** tekenen de vergelijking als vormen, zodat ze bij elk formaat scherp blijft.
- **.pub** kent geen vergelijkingen. JeffPub slaat elke vergelijking op als afbeelding ervan, op dezelfde plaats en in dezelfde grootte, en meldt dit wanneer u opslaat.
- **EPUB** en **HTML** bewaren de vergelijking als de regel die u hebt getypt.
