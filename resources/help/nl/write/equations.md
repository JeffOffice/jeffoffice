# Vergelijkingen
<!-- keywords: tabblad vergelijking, structuren, symbolen, converteren, vergelijking, vergelijkingen, wiskunde, formule, breuk, vierkantswortel, superscript, subscript, exponent, som, integraal, grieks, symbool, vergelijking invoegen, alt+=, accent, dakje, vector, streep, matrix, haakjes, ingebouwde vergelijkingen, galerie, vergelijking wijzigen, vergelijking bewerken -->

Een **vergelijking** is een regel wiskunde, zoals een breuk of een vierkantswortel. In dit onderwerp leest u hoe u er een typt en in een document zet.

## Een vergelijking invoegen

1. Plaats de invoegpositie waar de vergelijking moet komen.
2. Klik op **Invoegen > Vergelijking**, of druk op **Alt+=**.
3. Typ de vergelijking in één regel. Onder het vak ziet u de vergelijking zoals ze er straks uitziet, en de regel tekst die de pagina toont. Wilt u beginnen met een bekende vergelijking, kies er dan een uit de lijst **Ingebouwde vergelijkingen** in het vak. Klik op **OK**.

De vergelijking wordt in het document opgeslagen in de standaardvorm voor wiskunde, zodat andere programma's haar kunnen openen.

## Zo typt u een vergelijking

- **Breuken:** typ een schuine streep. `a/b` is a over b. `(x+1)/(x-1)` neemt elke groep tussen haakjes als één deel. `\frac{1}{2}` werkt ook.
- **Machten:** typ een dakje. `x^2` is x kwadraat. `e^(i*pi)` neemt de groep tussen haakjes.
- **Subscripts:** typ een underscore. `x_i` is x met een kleine i eronder. U kunt beide gebruiken, zoals in `x_i^2`.
- **Wortels:** typ `sqrt(x+1)` voor een vierkantswortel. Typ `sqrt[3]{x}` voor een derdemachtswortel.
- **Sommen, integralen en producten:** typ `\sum_(i=1)^n i`, `\int_0^1 x` of `\prod_(k=1)^n k`. Het deel na de underscore staat eronder (of onderaan het integraalteken), en het deel na het dakje erboven.
- **Symbolen:** typ een backslash en een naam. `\alpha` is α, `\pi` is π, `\infty` is ∞, `\leq` is ≤, `\neq` is ≠, `\times` is ×, `\pm` is ± en `\to` is →. Ongeveer 70 namen werken.
- **Accenten:** `\hat{x}`, `\check{x}`, `\tilde{x}`, `\acute{x}`, `\grave{x}`, `\breve{x}`, `\dot{x}`, `\ddot{x}`, `\bar{x}` en `\vec{v}` zetten een teken boven de letter. `\overline{ab}` trekt een lijn boven de groep, en `\underline{ab}` een lijn eronder.
- **Haakjes die meegroeien:** typ `\left( ... \right)`, en de haakjes groeien mee met de hoogte van wat ertussen staat. Gebruik na `\left` en `\right` elk haakje, of een punt om één kant weg te laten, zoals in `\left[ x \right.`.
- **Matrices:** een **matrix** is een raster van getallen. Typ `\matrix{a & b \\ c & d}`. Zet een `&` tussen de cellen van een rij, en twee backslashes tussen de rijen. Gebruik `\pmatrix` voor ronde haakjes om het raster, `\bmatrix` voor vierkante haakjes en `\vmatrix` voor rechte streepjes.
- **Functies:** `sin`, `cos`, `tan`, `log`, `ln`, `exp`, `lim`, `min` en `max` behouden hun naam.

Op de pagina toont JeffWrite een vergelijking als één regel tekst. De vergelijking in het bestand behoudt haar breuken en wortels, zodat andere programma's haar goed opmaken.

## Typen in UnicodeMath of LaTeX

Naast het vak staat een lijst met typeformaten: **Eenvoudige regel** (de hierboven beschreven manier), **UnicodeMath** en **LaTeX**. Dit zijn twee manieren om wiskunde als gewone tekst te schrijven, die veel mensen al kennen. Wanneer u een ander formaat kiest, wordt wat u hebt getypt opnieuw in dat formaat geschreven.

- **UnicodeMath:** `a/b` is een breuk, `x^2_i` heeft een macht en een subscript, `√(x)` is een wortel en `∑_(i=1)^n i` is een som. Zet woorden tussen aanhalingstekens, zoals `"if x"`, dan blijven ze gewone tekst in plaats van wiskundige letters. `(a|b)` maakt haakjes met twee delen, en `\matrix(a&b@c&d)` maakt een matrix (`&` tussen cellen, `@` tussen rijen).
- **LaTeX:** `\frac{a}{b}` is een breuk, `\sqrt[3]{x}` is een derdemachtswortel, `\sum_{i=1}^n i` is een som, `\text{if x}` is gewone tekst en `\begin{pmatrix} a & b \\ c & d \end{pmatrix}` is een matrix.

Met de twee knoppen **Professioneel** en **Lineair** bepaalt u hoe het vak uw vergelijking toont. Professioneel tekent haar opgebouwd, zoals ze eruitziet. Lineair toont alleen de regel die u hebt getypt. Als een vergelijking niet in het gekozen formaat kan worden geschreven, meldt het vak dat en behoudt het wat u hebt getypt.

## Het tabblad Vergelijking

Selecteer een vergelijking (klik erop, of verplaats de invoegpositie erlangs met Shift), dan verschijnt aan het eind van het lint een tabblad met de naam **Vergelijking**. Het verdwijnt wanneer u iets anders selecteert.

- **Hulpmiddelen:** **Vergelijking bewerken** opent het vak. **Professioneel** en **Lineair** bepalen hoe het vak begint: opgebouwd of alleen als regel. Het document zelf toont een vergelijking altijd als regel tekst. **Normale tekst** opent het vak met een paar aanhalingstekens erbij, voor woorden die geen wiskunde zijn.
- **Converteren:** kies het formaat waarin het vak opent: **Eenvoudige regel**, **UnicodeMath** of **LaTeX**. De volgende keer dat u een vergelijking bewerkt, wordt ze in het formaat geschreven dat u hebt gekozen. Een vergelijking die een eenvoudige regel niet kan weergeven, zoals een som met de grenzen erboven en eronder, opent in UnicodeMath.
- **Symbolen:** Griekse letters, operatoren, relaties, pijlen, en verzamelingen en logica. Kies een symbool, dan opent het vak met het symbool aan de regel toegevoegd.
- **Structuren:** breuk, index, wortel, integraal, grote operator, haakje, functie, accent, limiet, operator en matrix. Klik op een knop voor de gebruikelijke vorm, of op de pijl ernaast voor meer vormen, zoals een afgeleide, een wortel met een graad of een eenheidsmatrix.

Een structuur wordt aan het eind van de regel ingevoegd, met voorbeeldletters erin, en de eerste wordt geselecteerd zodat u eroverheen kunt typen. Het vak heeft dezelfde structuren in een rij kleine knoppen boven de regel. Daar wordt een structuur op de plaats van de cursor ingevoegd, en als een deel van de regel is geselecteerd, gaat dat deel naar de eerste plaats van de structuur.

## Ingebouwde vergelijkingen

Klik op **Invoegen > Ingebouwde vergelijkingen** om een lijst met bekende vergelijkingen te zien, zoals de kwadratische formule en de oppervlakte van een cirkel. Klik op een vergelijking om haar op de plaats van de invoegpositie in te voegen.

## Een vergelijking wijzigen

Plaats de invoegpositie naast een vergelijking, of selecteer haar, en klik op **Invoegen > Vergelijking**. Het vak opent met de vergelijking uitgeschreven in één regel. Pas haar aan en klik op **OK**. Ongedaan maken is één stap.

## Vergelijkingen in andere documenten

JeffWrite toont de vergelijkingen die een document bevat, en bewaart ze zoals ze waren. Een vergelijking die door een ander programma is gemaakt, opent in UnicodeMath, of in LaTeX als UnicodeMath haar niet kan schrijven, zodat u haar kunt aanpassen. Als geen van beide alle onderdelen kan schrijven, zoals groeperingstekens, begint het vak met de tekst van de vergelijking. Klik op **Annuleren** om de vergelijking te behouden zoals ze is, of typ een nieuwe om haar te vervangen.
