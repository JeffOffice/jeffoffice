# Zoeken en vervangen

<!-- keywords: geavanceerd zoeken, klinkt als, soundex, woordvormen, alle vormen van een woord, leestekens negeren, spaties negeren, speciaal, alineamarkering, tabteken, zoeken op opmaak, opmaak, vet, lettertype, stijl, jokertekens, jokerteken, patroon, reguliere expressie, groepen, aantal overeenkomsten, alle overeenkomsten markeren, vervangen, alles vervangen, zoeken, zoek naar, woorden wijzigen, hoofdlettergevoelig, hele woorden, volgende zoeken, ctrl+f, ctrl+h, bewerken, deelvenster, verborgen tekst, ongedaan maken -->

Gebruik Zoeken om naar woorden in uw document te springen. Gebruik Vervangen om ze te wijzigen. Beide werken in één deelvenster aan de rechterkant van het venster.

## Woorden zoeken

1. Druk op **Ctrl+F**, of klik op **Start > Bewerken > Zoeken**. Het deelvenster **Zoeken en vervangen** wordt geopend, met de cursor in **Zoeken naar**.
2. Typ de woorden die u zoekt. Druk daarna op **Enter**, of klik op **Volgende zoeken**.

JeffWrite begint waar uw cursor staat. Het selecteert de volgende plek waar de woorden worden gevonden, en scrolt de pagina's zodat u die plek ziet. Zo'n plek heet een **overeenkomst**. Klik nogmaals op **Volgende zoeken** om naar de volgende overeenkomst te gaan. Na de laatste overeenkomst gaat het terug naar het begin van het document.

Terwijl u typt, telt het deelvenster de overeenkomsten, bijvoorbeeld "9 overeenkomsten", en krijgt elke overeenkomst een gele band op de pagina's. De banden verschijnen ook in tabelcellen, kopteksten, voetteksten en voetnoten. Na **Volgende zoeken** vermeldt het deelvenster welke overeenkomst u hebt geselecteerd, bijvoorbeeld "Overeenkomst 2 van 4". De telling en de banden veranderen wanneer u op **Hoofdlettergevoelig** of **Alleen hele woorden zoeken** klikt, of wanneer u de tekst wijzigt. De banden blijven op de pagina's staan wanneer u het deelvenster sluit en wanneer u op **Leesweergave** klikt, zodat u kunt lezen met alle overeenkomsten zichtbaar. Er verandert niets in het document, en de banden worden nooit afgedrukt of opgeslagen. Ze verdwijnen wanneer u het document wijzigt, wanneer u het deelvenster weer opent, of wanneer u op **Start > Bewerken > Markeringen zoeken wissen** klikt.

## Zoeken met jokertekens

Schakel **Jokertekens gebruiken** in om naar een patroon te zoeken, en niet alleen naar woorden. Een **jokerteken** is een teken dat voor andere letters staat. Dit zijn de tekens:

- **?** staat voor elke willekeurige letter. **c?t** vindt "cat" en "cot".
- **\*** staat voor een willekeurige reeks letters. **c\*t** vindt "cat" en "cart".
- **<** is het begin van een woord, en **>** is het einde ervan. **<the>** vindt "the", maar niet "other".
- **[abc]** is een van die letters, **[a-c]** is elke letter van a tot c, en **[!abc]** is elke letter behalve die.
- **{2}** herhaalt de letter ervoor twee keer. **{1,3}** herhaalt die één tot drie keer.
- **@** betekent één of meer keer de letter ervoor.
- **\\** voor een teken zoekt het teken zelf. **\\?** vindt een vraagteken.
- **^p** is een alineamarkering, **^t** is een tabteken, **^#** is een cijfer, **^$** is een letter, en **^w** is witruimte.

Zet een deel van het patroon tussen **( )** om een **groep** te maken. In **Vervangen door** staat **\1** voor de eerste groep, **\2** voor de tweede, enzovoort tot **\9**. **^&** is alles wat is gevonden. Zoek bijvoorbeeld naar **(<[A-Z]@>), (<[A-Z]@>)** en vervang door **\2 \1** om "Smith, John" te veranderen in "John Smith". **Alleen hele woorden zoeken** is grijs terwijl jokertekens aan staan, omdat **<** en **>** dat werk al doen.

Als het patroon een fout bevat, meldt het deelvenster wat die is.

Als uw woorden niet in het document staan, meldt het deelvenster dat, en verplaatst er niets.

U kunt in het document blijven werken terwijl het deelvenster open is. Klik op de pagina om te typen. Klik op de **X** bovenaan het deelvenster om het te sluiten.

## Zoeken met reguliere expressies

Schakel **Reguliere expressies gebruiken** in voor patronen die verder gaan dan jokertekens. Een **reguliere expressie** is een kort patroon van letters en tekens. Deze optie wordt ingeschakeld in plaats van **Jokertekens gebruiken**: slechts één van de twee kan aan staan. Dit zijn de gebruikelijke tekens:

- **.** is elk willekeurig teken, **\d** is een cijfer, **\w** is een letter, cijfer of underscore, en **\s** is een spatie of tab. **[a-c]** is elke letter van a tot c, en **[^abc]** is elk teken behalve die.
- **^** is het begin van een alinea, **$** is het einde ervan, en **\b** is de rand van een woord. **\bcat\b** vindt "cat", maar niet "category".
- **\*** betekent nul of meer van wat ervoor staat, **+** betekent één of meer, en **?** betekent nul of één. **{2}** betekent precies twee keer, en **{2,5}** betekent twee tot vijf keer; een aantal kan oplopen tot 200. Voeg **?** toe na een herhaling, zoals in **.+?**, om de kortst mogelijke reeks te vinden die past.
- **|** betekent "of": **kat|hond** vindt beide woorden.
- **( )** maakt een **groep**. Gebruik **(?: )** voor een groep die u niet nodig hebt in **Vervangen door**.
- **\\** voor een teken zoekt het teken zelf: **\.** vindt een punt. **\t** is een tabteken.

**Hoofdlettergevoelig** werkt zoals bij woorden. Letters met accenten en letters uit andere alfabetten tellen als letters.

Een patroon wordt per alinea doorzocht, zodat **^** en **$** het begin en einde van elke alinea betekenen. Om over het einde van een alinea heen te zoeken, zet u **\n** (een alineamarkering) in het patroon: **thuis\.\nLee** vindt "thuis." aan het einde van de ene alinea en "Lee" aan het begin van de volgende.

In **Vervangen door** staat **$1** of **\1** voor de eerste groep, **$2** of **\2** voor de tweede, enzovoort tot negen groepen. **$0** of **\0** is alles wat is gevonden. **$$** is een dollarteken, **\\** is een backslash, en **\t** is een tabteken. Zoek bijvoorbeeld naar **(\w+), (\w+)** en vervang door **$2 $1** om "Smith, John" te veranderen in "John Smith".

**Alles vervangen** wijzigt elke overeenkomst in de tekst, koptekst, voettekst, notities en tekstvakken in één stap, zodat één **Ongedaan maken** alles terugzet. Met **Wijzigingen bijhouden** ingeschakeld wordt elke wijziging bijgehouden, net als bij woorden. Verborgen tekst blijft ongemoeid.

Als het patroon een fout bevat, meldt het deelvenster bij welk teken. Een patroon kan geen tekens vinden, zoals **^** (het begin van een alinea) of **$** (het einde): **Vervangen door** zet dan de tekst op die plek, dus **^** met **> ** voegt die tekens toe aan het begin van elke alinea. Een patroon dat te lang zou duren op uw tekst, zoals **(a+)+$** op een lange regel a's, wordt door JeffWrite gestopt, en dat wordt gemeld; er wordt niets gewijzigd. Een lange zoekopdracht toont een vak **Zoeken** met een knop **Annuleren**; annuleren laat uw document zoals het was, en u kunt pas in het document typen als het zoeken klaar is. Patronen die vooruit of achteruit kijken zonder op te gebruiken wat ze controleren, zoals **x(?=\d)**, en patronen in uitgebreide modus, zoals **(?x)**, worden niet ondersteund; JeffWrite meldt dit. Een zeer lange alinea wordt in delen doorzocht, en elke overeenkomst is dezelfde als die een zoekopdracht over de hele alinea zou vinden. Een patroon dat een zeer lange reeks in een zeer lange alinea (tienduizenden tekens) vindt, kan worden gestopt met de melding dat er te veel werk voor nodig is, en een enkele overeenkomst van meer dan 200.000 tekens wordt niet vervangen: JeffWrite stopt, meldt dit en wijzigt niets.

## Geavanceerd zoeken

Deze keuzes staan in het deelvenster onder **Reguliere expressies gebruiken**. Elke keuze verandert hoe JeffWrite de woorden in **Zoeken naar** leest.

- **Klinkt als** vindt woorden die op dezelfde manier worden uitgesproken als uw woord. Het geeft elk woord een code op basis van de klank, en vindt de woorden met dezelfde code. Typ **Smit**, dan vindt het ook "Smith".
- **Alle woordvormen zoeken** vindt elke vorm van het woord. Typ **lopen**, dan vindt het "loop," "liep" en "gelopen." Het werkt op hele woorden. Als u twee woorden typt, zoals **grote hond**, dan vindt het die naast elkaar, in elke vorm.
- **Leestekens negeren** slaat tekens zoals koppeltekens en punten tussen de letters over. Typ **e-mail**, dan vindt het ook "email".
- **Witruimtetekens negeren** slaat spaties en tabs tussen de letters over. Typ **e mail**, dan vindt het "email".

Slechts één van **Klinkt als**, **Alle woordvormen zoeken**, **Jokertekens gebruiken** en **Reguliere expressies gebruiken** kan tegelijk aan staan. **Klinkt als** en **Alle woordvormen zoeken** vinden altijd hele woorden, dus **Alleen hele woorden zoeken** en **Hoofdlettergevoelig** zijn grijs.

### Speciaal

Klik op **Speciaal** om een code in **Zoeken naar** te zetten: **Alineamarkering** (^p), **Tabteken** (^t), **Willekeurig cijfer** (^#), **Willekeurige letter** (^$), **Witruimte** (^w), **Sectie-einde** (^b), **Veld** (^d), **Voetnootmarkering** (^f), **Eindnootmarkering** (^e), of **Invoegteken** (^^). De codes werken zonder **Jokertekens gebruiken**. Een **sectie-einde** is de markering die een sectie beëindigt. Een **veld** is een stuk tekst dat het programma berekent, zoals een paginanummer of een datum, en **^d** vindt waar elk veld begint. De notitiemarkeringen zijn de kleine cijfers in de tekst die naar een voetnoot of eindnoot verwijzen. Deze vier worden gevonden, maar **Vervangen** en **Alles vervangen** laten ze met rust, omdat wijzigen ervan betekenis zou verliezen. De andere tekens in het vak, zoals **?** en **\***, zijn dan gewoon die tekens.

### Opmaak

Klik op **Opmaak** om alleen tekst met een bepaald uiterlijk te vinden: **Vet**, **Cursief**, **Onderstreept**, **Doorgehaald**, **Gemarkeerd**, een **Lettertype**, of een alineastijl. Het deelvenster toont de uiterlijken die u onder de knoppen hebt gekozen. Kies **Geen opmaak** om ze te wissen.

U kunt **Zoeken naar** leeg laten. Dan is elk stuk tekst met dat uiterlijk een overeenkomst. Met woorden in **Zoeken naar** worden alleen de woorden met dat uiterlijk gevonden. **Alles vervangen** wijzigt alleen die woorden.

## De zoekopdracht beperken

- **Hoofdlettergevoelig** vindt alleen woorden met dezelfde hoofdletters. Met deze optie aan vindt "Plan" niet "plan".
- **Alleen hele woorden zoeken** slaat langere woorden over die uw woord bevatten. Met deze optie aan vindt "plan" niet "planeet".
- **Zoeken in** bepaalt in welke richting **Volgende zoeken** vanaf de cursor gaat. **Omlaag** gaat naar het einde van het document, en **Omhoog** gaat naar het begin. Elke keuze stopt daar en meldt dat. **Alles** gaat door tot het andere eind en is de gebruikelijke keuze. **Alles vervangen** wijzigt altijd het hele document.

## Woorden vervangen

1. Druk op **Ctrl+H**, of klik op **Start > Bewerken > Vervangen**. De cursor gaat naar **Vervangen door**.
2. Typ de nieuwe woorden in **Vervangen door**. Om de gevonden woorden te verwijderen, laat u het veld leeg.
3. Typ de oude woorden in **Zoeken naar**.
4. Klik op **Vervangen** om de eerste overeenkomst te zien. Klik nogmaals op **Vervangen** om die te wijzigen. JeffWrite gaat dan naar de volgende overeenkomst. Elke wijziging is één stap, dus **Ongedaan maken** (Ctrl+Z) neemt er één tegelijk terug.
5. Klik op **Alles vervangen** om alle overeenkomsten in één keer te wijzigen. Het deelvenster vertelt hoeveel er zijn gewijzigd. **Alles vervangen** wijzigt ook de woorden in tekstvakken en vormen, en is één stap, zodat één **Ongedaan maken** alle woorden terugzet. Wanneer **Wijzigingen bijhouden** aan staat, wordt elke vervanging als gewijzigd bijgehouden, zowel in de tekst als in de vakken. Een vak dat te groot is om te wijzigen, wordt in het bericht meegeteld. **Alles vervangen** bereikt geen tekstvakken in koptekst en voettekst, gegroepeerde vormen of aan elkaar gekoppelde vakken. **Volgende zoeken** kijkt nog niet in tekstvakken.

De nieuwe woorden krijgen de opmaak van de eerste letter van de woorden die ze vervangen. Als u een vet woord vervangt, wordt het nieuwe woord vet.

Om bijvoorbeeld "color" overal te veranderen in "colour": typ **color** in **Zoeken naar**, typ **colour** in **Vervangen door**, en klik op **Alles vervangen**.

## Waarin de zoekopdracht kijkt

- Ze kijkt naar de tekst van het document, ook de tekst in tabellen. Een bladwijzer midden in sommige woorden, die sommige bestanden bevatten, belet niet dat die woorden worden gevonden. **Vervangen** en **Alles vervangen** behouden de bladwijzer.
- **Volgende zoeken** kijkt eerst in de tekst. Daarna gaat het naar de koptekst, dan de voettekst, dan de voetnoten, en daarna terug naar het begin van de tekst. Wanneer een overeenkomst in een van die delen staat, opent JeffWrite dat deel voor bewerking en selecteert de overeenkomst. Het deelvenster vertelt waar het is, bijvoorbeeld "Gevonden in de koptekst van pagina 3." **Vervangen** wisselt de overeenkomst waarop u staat, in de tekst of in een koptekst, voettekst of voetnoot. **Alles vervangen** wijzigt elke overeenkomst in al die delen, en één **Ongedaan maken** zet ze allemaal terug. De eindnoten maken deel uit van de tekst en worden dus samen daarmee doorzocht. Zie [Koptekst en voettekst](headers-footers).
- Ze slaat tekst over die is ingesteld op **Verborgen**, omdat die tekst niet wordt getoond. Zie [Tekst opmaken](font).

## Zie ook

- [Tekst opmaken](font)
- [JeffWrite met het toetsenbord gebruiken](keyboard)
