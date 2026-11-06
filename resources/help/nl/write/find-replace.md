# Alles zoeken en vervangen

<!-- keywords: geavanceerd zoeken, klinkt als, soundex, woordvormen, alle vormen van een woord, leestekens negeren, spaties negeren, speciaal, alineamarkering, tab, zoeken op opmaak, opmaak, vet, lettertype, stijl, jokertekens, jokerteken, patroon, reguliere expressie, groepen, aantal overeenkomsten, alle overeenkomsten markeren, vervangen, alles vervangen, zoeken, zoek naar, woorden wijzigen, hoofdlettergevoelig, hele woorden, volgende zoeken, ctrl+f, ctrl+h, bewerken, deelvenster, verborgen tekst, ongedaan maken -->

Gebruik Zoeken om naar woorden in uw document te springen. Gebruik Vervangen om ze te wijzigen. Beide werken in één deelvenster aan de rechterkant van het venster.

## Woorden zoeken

1. Druk op **Ctrl+F**, of klik op **Start > Bewerken > Zoeken**. Het deelvenster **Zoeken en vervangen** wordt geopend, met de cursor in **Zoeken naar**.
2. Typ de woorden die u zoekt. Druk daarna op **Enter**, of klik op **Volgende zoeken**.

JeffWrite begint waar uw cursor staat. Het selecteert de volgende plaats waar de woorden worden gevonden, en scrolt de pagina's zodat u die kunt zien. Zo'n plaats heet een **overeenkomst**. Klik nogmaals op **Volgende zoeken** om naar de volgende overeenkomst te gaan. Na de laatste overeenkomst gaat het terug naar het begin van het document.

Terwijl u typt, telt het deelvenster de overeenkomsten, bijvoorbeeld "9 overeenkomsten", en krijgt elke overeenkomst een gele band op de pagina's. De banden verschijnen ook in tabelcellen, kopteksten, voetteksten en voetnoten. Na **Volgende zoeken** vermeldt het deelvenster waar u bent, bijvoorbeeld "Overeenkomst 2 van 4". De telling en de banden veranderen wanneer u op **Hoofdlettergevoelig** of **Alleen hele woorden zoeken** klikt, of wanneer u de tekst wijzigt. Ze verdwijnen wanneer u het deelvenster sluit.

## Zoeken met jokertekens

Schakel **Jokertekens gebruiken** in om naar een patroon te zoeken, en niet alleen naar woorden. Een **jokerteken** is een teken dat voor andere letters staat. Dit zijn de tekens:

- **?** staat voor één willekeurige letter. **c?t** vindt "cat" en "cot".
- **\*** staat voor een willekeurige reeks letters. **c\*t** vindt "cat" en "cart".
- **<** is het begin van een woord, en **>** is het einde ervan. **<the>** vindt "the", maar niet "other".
- **[abc]** is één van die letters, **[a-c]** is een letter van a tot c, en **[!abc]** is elke letter behalve die.
- **{2}** herhaalt de letter ervoor twee keer. **{1,3}** herhaalt haar één tot drie keer.
- **@** betekent één of meer keer de letter ervoor.
- **\\** voor een teken zoekt het teken zelf. **\\?** vindt een vraagteken.
- **^p** is een alineamarkering, **^t** is een tab, **^#** is een cijfer, **^$** is een letter, en **^w** is witruimte.

Zet een deel van het patroon tussen **( )** om een **groep** te maken. In **Vervangen door** staat **\1** voor de eerste groep, **\2** voor de tweede, enzovoort tot **\9**. **^&** is alles wat is gevonden. Zoek bijvoorbeeld naar **(<[A-Z]@>), (<[A-Z]@>)** en vervang door **\2 \1** om "Smith, John" in "John Smith" te veranderen. **Alleen hele woorden zoeken** is grijs zolang jokertekens aan staan, omdat **<** en **>** dat werk al doen.

Als het patroon een fout bevat, meldt het deelvenster welke fout het is.

Als uw woorden niet in het document staan, meldt het deelvenster dat, en verandert er niets.

U kunt in het document blijven werken terwijl het deelvenster open is. Klik op de pagina om te typen. Klik op de **X** boven aan het deelvenster om het te sluiten.

## Geavanceerd zoeken

Deze keuzes staan in het deelvenster onder **Jokertekens gebruiken**. Elke keuze verandert hoe JeffWrite de woorden in **Zoeken naar** leest.

- **Klinkt als** vindt woorden die hetzelfde klinken als uw woord. Het geeft elk woord een code op basis van de klank, en vindt de woorden met dezelfde code. Typ **Smit**, dan vindt het ook "Smith".
- **Alle woordvormen zoeken** vindt elke vorm van het woord. Typ **lopen**, dan vindt het "loopt", "liep" en "gelopen". Het werkt op hele woorden. Als u twee woorden typt, zoals **grote hond**, vindt het ze naast elkaar, in elke vorm.
- **Leestekens negeren** slaat tekens over zoals koppeltekens en punten tussen de letters. Typ **email**, dan vindt het ook "e-mail".
- **Witruimtetekens negeren** slaat spaties en tabs tussen de letters over. Typ **e mail**, dan vindt het "email".

Er kan maar één van **Klinkt als**, **Alle woordvormen zoeken** en **Jokertekens gebruiken** tegelijk aan staan. **Klinkt als** en **Alle woordvormen zoeken** vinden altijd hele woorden, dus **Alleen hele woorden zoeken** en **Hoofdlettergevoelig** zijn grijs.

### Speciaal

Klik op **Speciaal** om een code in **Zoeken naar** te zetten: **Alineamarkering** (^p), **Tabteken** (^t), **Willekeurig cijfer** (^#), **Willekeurige letter** (^$), **Witruimte** (^w), **Sectie-einde** (^b), **Veld** (^d), **Voetnootmarkering** (^f), **Eindnootmarkering** (^e) of **Circumflex-teken** (^^). De codes werken zonder **Jokertekens gebruiken**. Een **sectie-einde** is de markering die een sectie beëindigt. Een **veld** is een stukje tekst dat het programma berekent, zoals een paginanummer of een datum, en **^d** vindt waar elk veld begint. De notitiemarkeringen zijn de kleine cijfers in de tekst die naar een voetnoot of eindnoot verwijzen. Deze vier worden gevonden, maar **Vervangen** en **Alles vervangen** laten ze met rust, omdat het wijzigen ervan betekent dat wat ze aanduiden verloren gaat. De andere tekens in het vak, zoals **?** en **\***, zijn dan gewoon die tekens.

### Opmaak

Klik op **Opmaak** om alleen tekst te vinden met een bepaalde uitstraling: **Vet**, **Cursief**, **Onderstrepen**, **Doorhalen**, **Gemarkeerd**, een **Lettertype** of een **Alineastijl**. Het deelvenster toont de opmaak die u onder de knoppen hebt gekozen. Kies **Geen opmaak** om die te wissen.

U kunt **Zoeken naar** leeg laten. Dan is elk stuk tekst met die opmaak een overeenkomst. Met woorden in **Zoeken naar** worden alleen de woorden met die opmaak gevonden. **Alles vervangen** wijzigt alleen die woorden.

## Zoeken beperken

- **Hoofdlettergevoelig** vindt alleen woorden met dezelfde hoofdletters. Met deze optie aan vindt "Plan" niet "plan".
- **Alleen hele woorden zoeken** slaat langere woorden over die uw woord bevatten. Met deze optie aan vindt "plan" niet "planeet".

## Woorden vervangen

1. Druk op **Ctrl+H**, of klik op **Start > Bewerken > Vervangen**. De cursor gaat naar **Vervangen door**.
2. Typ de nieuwe woorden in **Vervangen door**. Om de gevonden woorden te verwijderen, laat u het vak leeg.
3. Typ de oude woorden in **Zoeken naar**.
4. Klik op **Vervangen** om de eerste overeenkomst te zien. Klik nogmaals op **Vervangen** om die te wijzigen. JeffWrite gaat daarna naar de volgende overeenkomst. Elke wijziging is één stap, dus **Ongedaan maken** (Ctrl+Z) neemt er één tegelijk terug.
5. Klik op **Alles vervangen** om alle overeenkomsten in één keer te wijzigen. Het deelvenster meldt hoeveel er zijn gewijzigd. **Alles vervangen** is één stap, dus één keer **Ongedaan maken** zet alle woorden terug.

De nieuwe woorden krijgen de opmaak van de woorden die ze vervangen, en de eerste letter volgt de hoofdletter van het oude woord. Als u een vet woord vervangt, is het nieuwe woord vet.

Om bijvoorbeeld "color" overal in "colour" te veranderen: typ **color** in **Zoeken naar**, typ **colour** in **Vervangen door** en klik op **Alles vervangen**.

## Waarin het zoeken kijkt

- Het zoekt in de tekst van het document, inclusief de tekst in tabellen. Een bladwijzer midden in sommige woorden, die sommige bestanden hebben, belet niet dat ze worden gevonden. **Vervangen** en **Alles vervangen** behouden de bladwijzer.
- **Volgende zoeken** kijkt eerst in de tekst. Daarna gaat het naar de koptekst, dan de voettekst, dan de voetnoten, en daarna terug naar het begin van de tekst. Wanneer een overeenkomst in een van die delen staat, opent JeffWrite het voor bewerking en selecteert de overeenkomst. Het deelvenster vertelt waar het is, bijvoorbeeld "Gevonden in de koptekst van pagina 3". **Vervangen** wisselt de overeenkomst waarop u staat, in de tekst of in een koptekst, voettekst of voetnoot. **Alles vervangen** wijzigt alle overeenkomsten in al die delen, en één keer **Ongedaan maken** brengt ze allemaal terug. De eindnoten maken deel uit van de tekst en worden dus ook meegezocht. Zie [Koptekst en voettekst](headers-footers).
- Het slaat tekst over die is ingesteld op **Verborgen**, omdat die tekst niet wordt getoond. Zie [Tekst opmaken](font).

## Zie ook

- [Tekst opmaken](font)
- [JeffWrite met het toetsenbord gebruiken](keyboard)
