# Woorden zoeken en vervangen
<!-- keywords: geavanceerd zoeken, klinkt als, soundex, woordvormen, alle vormen van een woord, leestekens negeren, spaties negeren, speciaal, alineamarkering, tabteken, zoeken op opmaak, opmaak, vet, lettertype, stijl, jokertekens, joker, patroon, reguliere expressie, groepen, aantal overeenkomsten, alle overeenkomsten markeren, vervangen, alles vervangen, zoeken, woorden wijzigen, hoofdlettergevoelig, hele woorden, volgende zoeken, ctrl+f, ctrl+h, bewerken, deelvenster, verborgen tekst, ongedaan maken -->

Gebruik **Zoeken** om naar woorden in uw document te springen. Gebruik **Vervangen** om ze te wijzigen. Beide werken in één deelvenster aan de rechterkant van het venster.

## Woorden zoeken

1. Druk op **Ctrl+F**, of klik op **Start > Bewerken > Zoeken**. Het deelvenster **Zoeken en vervangen** wordt geopend, met de cursor in **Zoeken naar**.
2. Typ de woorden die u zoekt. Druk daarna op **Enter**, of klik op **Volgende zoeken**.

JeffWrite begint waar uw cursor staat. Het selecteert de volgende plaats waar het uw woorden vindt, en scrolt de pagina's zodat u die kunt zien. Zo'n plaats heet een **overeenkomst**. Klik nogmaals op **Volgende zoeken** om naar de volgende overeenkomst te gaan. Na de laatste overeenkomst begint het weer bij het begin van het document.

Terwijl u typt, telt het deelvenster de overeenkomsten, bijvoorbeeld "9 overeenkomsten", en krijgt elke overeenkomst een gele streep op de pagina's. De streepen verschijnen ook in tabelcellen, kopteksten, voetteksten en voetnoten. Na **Volgende zoeken** meldt het deelvenster op welke overeenkomst u staat, bijvoorbeeld "Overeenkomst 2 van 4". Het aantal en de streepen veranderen wanneer u **Hoofdlettergevoelig** of **Alleen hele woorden zoeken** aanklikt, of wanneer u de tekst wijzigt. Ze verdwijnen wanneer u het deelvenster sluit.

## Zoeken met jokertekens

Schakel **Jokertekens gebruiken** in om naar een patroon te zoeken, niet alleen naar woorden. Een **joker** is een teken dat voor andere letters staat. Dit zijn de tekens:

- **?** staat voor één willekeurige letter. **c?t** vindt "kat" en "cot".
- **\*** staat voor een willekeurige reeks letters. **c\*t** vindt "kat" en "kort".
- **<** is het begin van een woord, en **>** is het einde ervan. **<de>** vindt "de", maar niet "onder".
- **[abc]** is één van die letters, **[a-c]** is elke letter van a tot c, en **[!abc]** is elke letter behalve die letters.
- **{2}** herhaalt de letter ervoor twee keer. **{1,3}** herhaalt die één tot drie keer.
- **@** betekent één of meer keer de letter ervoor.
- **\\** voor een teken zoekt het teken zelf. **\\?** vindt een vraagteken.
- **^p** is een alineamarkering, **^t** is een tabteken, **^#** is een cijfer, **^$** is een letter, en **^w** is witruimte.

Zet een deel van het patroon tussen **( )** om een **groep** te maken. In **Vervangen door** staat **\1** voor de eerste groep, **\2** voor de tweede, enzovoort tot **\9**. **^&** is alles wat is gevonden. Zoek bijvoorbeeld naar **(<[A-Z]@>), (<[A-Z]@>)** en vervang door **\2 \1** om "Jansen, Piet" in "Piet Jansen" te veranderen. **Alleen hele woorden zoeken** is grijs zolang jokertekens aan staan, omdat **<** en **>** dat werk al doen.

Als het patroon een fout bevat, meldt het deelvenster welke fout het is.

Als uw woorden niet in het document staan, meldt het deelvenster dat, en verandert er niets.

U kunt in het document blijven werken terwijl het deelvenster open is. Klik op de pagina om te typen. Klik op de **X** bovenaan het deelvenster om het te sluiten.

## Geavanceerd zoeken

Deze opties staan in het deelvenster onder **Jokertekens gebruiken**. Elke optie verandert hoe JeffWrite de woorden in **Zoeken naar** leest.

- **Klinkt als** zoekt woorden die klinken als uw woord. Het geeft elk woord een code op basis van de klanken, en zoekt naar woorden met dezelfde code. Typt u **Smit**, dan vindt het ook "Smith".
- **Alle woordvormen zoeken** vindt elke vorm van het woord. Typt u **lopen**, dan vindt het ook "loop", "liep" en "gelopen". Het werkt op hele woorden. Typt u twee woorden, zoals **grote hond**, dan vindt het ze naast elkaar, in elke vorm.
- **Leestekens negeren** slaat tekens over zoals koppeltekens en punten tussen de letters. Typt u **email**, dan vindt het ook "e-mail".
- **Witruimtetekens negeren** slaat spaties en tabs tussen de letters over. Typt u **e mail**, dan vindt het "email".

Van **Klinkt als**, **Alle woordvormen zoeken** en **Jokertekens gebruiken** kan er maar één tegelijk aan staan. **Klinkt als** en **Alle woordvormen zoeken** vinden altijd hele woorden, dus **Alleen hele woorden zoeken** en **Hoofdlettergevoelig** zijn grijs.

### Speciaal

Klik op **Speciaal** om een code in **Zoeken naar** te zetten: **Alineamarkering** (^p), **Tabteken** (^t), **Willekeurig cijfer** (^#), **Willekeurige letter** (^$), **Witruimte** (^w), **Sectie-einde** (^b), **Veld** (^d), **Voetnootmarkering** (^f), **Eindnootmarkering** (^e) of **Circumflex-teken** (^^). De codes werken ook zonder **Jokertekens gebruiken**. Een **sectie-einde** is de markering die een sectie beëindigt. Een **veld** is een stuk tekst dat het programma berekent, zoals een paginanummer of een datum. Met **^d** vindt u waar elk veld begint. De notemarkeringen zijn de kleine cijfers in de tekst die naar een voetnoot of eindnoot verwijzen. Deze vier worden gevonden, maar **Vervangen** en **Alles vervangen** laten ze met rust, omdat het wijzigen ervan betekenis zou laten verloren gaan. De andere tekens in de lijst, zoals **?** en **\***, zijn dan gewoon die tekens.

### Opmaak

Klik op **Opmaak** om alleen tekst te zoeken met een bepaald uiterlijk: **Vet**, **Cursief**, **Onderstreept**, **Doorgehaald**, **Gemarkeerd**, een **Lettertype** of een alinea-**Stijl**. Het deelvenster toont de opmaak die u hebt gekozen onder de knoppen. Kies **Geen opmaak** om die te wissen.

U kunt **Zoeken naar** leeg laten. Dan is elk stuk tekst met die opmaak een overeenkomst. Met woorden in **Zoeken naar** worden alleen de woorden met die opmaak gevonden. **Alles vervangen** wijzigt dan alleen die woorden.

## De zoekopdracht beperken

- **Hoofdlettergevoelig** zoekt alleen woorden met dezelfde hoofdletters. Met deze optie aan vindt "Plan" niet "plan".
- **Alleen hele woorden zoeken** slaat langere woorden over die uw woord bevatten. Met deze optie aan vindt "plan" niet "planeet".
- **Zoeken** bepaalt de richting waarin **Volgende zoeken** vanaf de cursor gaat. **Omlaag** gaat naar het einde van het document, en **Omhoog** naar het begin. Bij elk van beide stopt het en meldt het dat. **Alles** gaat door naar het andere einde en is de gebruikelijke keuze. **Alles vervangen** wijzigt altijd het hele document.

## Woorden vervangen

1. Druk op **Ctrl+H**, of klik op **Start > Bewerken > Vervangen**. De cursor gaat naar **Vervangen door**.
2. Typ de nieuwe woorden in **Vervangen door**. Wilt u de gevonden woorden verwijderen, laat het vak dan leeg.
3. Typ de oude woorden in **Zoeken naar**.
4. Klik op **Vervangen** om de eerste overeenkomst te zien. Klik nogmaals op **Vervangen** om die te wijzigen. JeffWrite gaat dan naar de volgende overeenkomst. Elke wijziging is één stap, dus met **Ongedaan maken** (Ctrl+Z) haalt u er één per keer terug.
5. Klik op **Alles vervangen** om alle overeenkomsten in één keer te wijzigen. Het deelvenster meldt hoeveel er zijn gewijzigd. **Alles vervangen** wijzigt ook de woorden in tekstvakken en vormen en is één stap, dus met één keer **Ongedaan maken** zet u alle woorden terug. Als **Wijzigingen bijhouden** aan staat, wordt elke vervanging vastgelegd als gewijzigd, in de tekst en in de tekstvakken. Een vak dat te groot is om te wijzigen, wordt in het bericht meegeteld. **Alles vervangen** bereikt geen tekstvakken in koptekst en voettekst, samen gegroepeerde vormen of aan elkaar gekoppelde tekstvakken. **Volgende zoeken** kijkt nog niet in tekstvakken.

De nieuwe woorden krijgen de opmaak van de woorden die ze vervangen. Als u een vet woord vervangt, wordt het nieuwe woord ook vet.

Om bijvoorbeeld "color" overal in "colour" te veranderen: typ **color** in **Zoeken naar**, typ **colour** in **Vervangen door** en klik op **Alles vervangen**.

## Waarin het zoeken kijkt

- Het zoekt in de tekst van het document, ook in de tekst van tabellen. Een bladwijzer midden in sommige woorden, die sommige bestanden hebben, verhindert niet dat die woorden worden gevonden. **Vervangen** en **Alles vervangen** behouden de bladwijzer.
- **Volgende zoeken** kijkt eerst in de tekst. Daarna gaat het naar de koppen, dan de voetteksten, dan de voetnoten en daarna weer terug naar het begin van de tekst. Als een overeenkomst in een van die delen staat, opent JeffWrite dat deel om te bewerken en selecteert het de overeenkomst. Het deelvenster meldt waar het is, zoals "Gevonden in de koptekst van pagina 3." **Vervangen** wisselt de overeenkomst waarop u staat, in de tekst of in een kop, voet of voetnoot. **Alles vervangen** wijzigt elke overeenkomst in al die delen, en met één keer **Ongedaan maken** zet u ze allemaal terug. Eindnoten horen bij de tekst en worden dus ook met de tekst doorzocht. Zie [Koptekst en voettekst](headers-footers).
- Het slaat tekst over die is ingesteld als **Verborgen**, omdat die tekst niet wordt weergegeven. Zie [Tekstopmaak wijzigen](font).

## Zie ook

- [Tekstopmaak wijzigen](font)
- [JeffWrite met het toetsenbord gebruiken](keyboard)
