# Woorden zoeken en vervangen
<!-- keywords: geavanceerd zoeken, klinkt als, soundex, woordvormen, alle vormen van een woord, leestekens negeren, spaties negeren, speciaal, alineamarkering, tab, zoeken op opmaak, opmaak, vet, lettertype, stijl, jokertekens, jokerteken, patroon, reguliere expressie, groepen, aantal overeenkomsten, alle overeenkomsten markeren, vervangen, alles vervangen, zoeken, zoek naar, woorden wijzigen, hoofdlettergevoelig, hele woorden, volgende zoeken, ctrl+f, ctrl+h, bewerken, deelvenster, verborgen tekst, ongedaan maken -->

Gebruik Zoeken om naar woorden in uw document te springen. Gebruik Vervangen om ze te wijzigen. Beide werken in één deelvenster aan de rechterkant van het venster.

## Woorden zoeken

1. Druk op **Ctrl+F**, of klik op **Start > Bewerken > Zoeken**. Het deelvenster **Zoeken en vervangen** wordt geopend, met de cursor in **Zoeken naar**.
2. Typ de woorden die u zoekt. Druk daarna op **Enter**, of klik op **Volgende zoeken**.

JeffWrite begint waar uw cursor staat. Het selecteert de volgende plek waar het uw woorden vindt, en scrolt de pagina's zodat u die kunt zien. Zo'n plek heet een **overeenkomst**. Klik nogmaals op **Volgende zoeken** om naar de volgende overeenkomst te gaan. Na de laatste overeenkomst begint het weer bij het begin van het document.

Terwijl u typt, telt het deelvenster de overeenkomsten, zoals "9 overeenkomsten", en krijgt elke overeenkomst een gele band op de pagina's. De banden verschijnen ook in tabelcellen, kopteksten, voetteksten en voetnoten. Na **Volgende zoeken** vermeldt het deelvenster welke overeenkomst u hebt, zoals "Overeenkomst 2 van 4". De telling en de banden veranderen wanneer u op **Hoofdlettergevoelig** of **Alleen hele woorden zoeken** klikt, of wanneer u de tekst wijzigt. Ze verdwijnen wanneer u het deelvenster sluit.

## Zoeken met jokertekens

Vink **Jokertekens gebruiken** aan om naar een patroon te zoeken, en niet alleen naar woorden. Een **jokerteken** is een teken dat voor andere letters staat. Dit zijn de tekens:

- **?** staat voor elke willekeurige letter. **c?t** vindt "kat" en "kot".
- **\*** staat voor een willekeurige reeks letters. **k\*t** vindt "kat" en "kart".
- **<** is het begin van een woord, en **>** is het einde ervan. **<de>** vindt "de", maar niet "deze".
- **[abc]** is een van die letters, **[a-c]** is elke letter van a tot c, en **[!abc]** is elke letter behalve die.
- **{2}** herhaalt de letter ervoor twee keer. **{1,3}** herhaalt die één tot drie keer.
- **@** betekent een of meer keer de letter ervoor.
- **\\** voor een teken zoekt het teken zelf. **\\?** vindt een vraagteken.
- **^p** is een alineamarkering, **^t** is een tab, **^#** is een cijfer, **^$** is een letter, en **^w** is witruimte.

Zet een deel van het patroon tussen **( )** om een **groep** te maken. In **Vervangen door** staat **\1** voor de eerste groep, **\2** voor de tweede, enzovoort tot **\9**. **^&** is alles wat is gevonden. Zoek bijvoorbeeld naar **(<[A-Z]@>), (<[A-Z]@>)** en vervang door **\2 \1** om "Jansen, Jan" te veranderen in "Jan Jansen". **Alleen hele woorden zoeken** is grijs zolang jokertekens aan staan, omdat **<** en **>** dat werk doen.

Als het patroon een fout bevat, vermeldt het deelvenster wat er mis is.

Als uw woorden niet in het document staan, zegt het deelvenster dat, en verandert er niets.

U kunt in het document blijven werken terwijl het deelvenster open is. Klik op de pagina om te typen. Klik op de **X** boven in het deelvenster om het te sluiten.

## Zoeken met reguliere expressies

Vink **Reguliere expressies gebruiken** aan voor patronen die verder gaan dan jokertekens. Een **reguliere expressie** is een kort patroon van letters en tekens. Deze optie wordt aangezet in plaats van **Jokertekens gebruiken**: slechts een van beide kan aan staan. Dit zijn de veelgebruikte tekens:

- **.** is elk willekeurig teken, **\d** is een cijfer, **\w** is een letter, cijfer of underscore, en **\s** is een spatie of tab. **[a-c]** is elke letter van a tot c, en **[^abc]** is elk teken behalve die.
- **^** is het begin van een alinea, **$** is het einde ervan, en **\b** is de rand van een woord. **\bkat\b** vindt "kat", maar niet "katten".
- **\*** betekent nul of meer keer wat ervoor staat, **+** betekent één of meer keer, en **?** betekent nul of één keer. **{2}** betekent precies twee keer, en **{2,5}** betekent twee tot vijf keer; een aantal kan oplopen tot 200. Voeg **?** toe na een herhaling, zoals in **.+?**, om de kortst mogelijke reeks te vinden die past.
- **|** betekent "of": **kat|hond** vindt een van beide woorden.
- **( )** maakt een **groep**. Gebruik **(?: )** voor een groep die u niet nodig hebt in **Vervangen door**.
- **\\** voor een teken zoekt het teken zelf: **\.** vindt een punt. **\t** is een tab.

**Hoofdlettergevoelig** werkt hier net als bij woorden. Letters met accenten en letters uit andere alfabetten tellen als letters.

Een patroon wordt per alinea doorzocht, zodat **^** en **$** het begin en einde van elke alinea betekenen. Om over het einde van een alinea heen te zoeken, zet u **\n** (een alineamarkering) in het patroon: **thuis\.\nLee** vindt "thuis." aan het einde van de ene alinea en "Lee" aan het begin van de volgende.

In **Vervangen door** staan **$1** of **\1** voor de eerste groep, **$2** of **\2** voor de tweede, enzovoort tot negen groepen. **$0** of **\0** is alles wat is gevonden. **$$** is een dollarteken, **\\** is een backslash, en **\t** is een tab. Zoek bijvoorbeeld naar **(\w+), (\w+)** en vervang door **$2 $1** om "Jansen, Jan" te veranderen in "Jan Jansen".

**Alles vervangen** wijzigt elke overeenkomst in de tekst, koppen, voetteksten, notities en tekstvakken in één stap, zodat één **Ongedaan maken** ze allemaal terugzet. Met **Wijzigingen bijhouden** aan wordt elke wijziging bijgehouden, net als bij woorden. Verborgen tekst blijft ongemoeid.

Als het patroon een fout bevat, vermeldt het deelvenster bij welk teken die zit. Een patroon kan ook helemaal geen tekens vinden, zoals **^** (het begin van een alinea) of **$** (het einde): **Vervangen door** zet dan zijn tekst op die plek. Zo voegt **^** met **> ** die tekens toe aan het begin van elke alinea. Een patroon dat te lang zou duren op uw tekst, zoals **(a+)+$** op een lange regel met a's, wordt door JeffWrite gestopt, en het programma meldt dat; er verandert niets. Een lange zoekactie toont een vak **Zoeken** met een knop **Annuleren**; annuleren laat uw document zoals het was, en u kunt in het document niet typen tot het zoeken klaar is. Patronen die vooruit of achteruit kijken zonder te gebruiken wat ze controleren, zoals **x(?=\d)**, en patronen in de uitgebreide modus, zoals **(?x)**, worden niet ondersteund; JeffWrite meldt dat. Een zeer lange alinea wordt in stukken doorzocht, en elke overeenkomst is dezelfde die een zoekactie over de hele alinea zou vinden. Een patroon dat een zeer lang stuk van een zeer lange alinea (tienduizenden tekens) beslaat, kan worden gestopt met de melding "Er is te veel werk nodig", en een enkele overeenkomst van meer dan 200.000 tekens wordt niet vervangen: JeffWrite stopt, meldt dat, en verandert niets.

## Geavanceerd zoeken

Deze keuzes staan in het deelvenster onder **Reguliere expressies gebruiken**. Elk van them verandert hoe JeffWrite de woorden in **Zoeken naar** leest.

- **Klinkt als** vindt woorden die op dezelfde manier worden uitgesproken als uw woord. Het geeft elk woord een code op basis van de klanken, en vindt de woorden met dezelfde code. Typ **Smit**, en het vindt ook "Smidt".
- **Alle woordvormen zoeken** vindt elke vorm van het woord. Typ **lopen**, en het vindt "loopt", "liep" en "gelopen". Het werkt op hele woorden. Als u twee woorden typt, zoals **grote hond**, vindt het ze naast elkaar, in elke vorm.
- **Leestekens negeren** slaat tekens over zoals koppeltekens en punten tussen de letters. Typ **email**, en het vindt ook "e-mail".
- **Witruimtetekens negeren** slaat spaties en tabs tussen de letters over. Typ **e mail**, en het vindt "email".

Slechts één van **Klinkt als**, **Alle woordvormen zoeken**, **Jokertekens gebruiken** en **Reguliere expressies gebruiken** kan tegelijk aan staan. **Klinkt als** en **Alle woordvormen zoeken** vinden altijd hele woorden, dus **Alleen hele woorden zoeken** en **Hoofdlettergevoelig** zijn grijs.

### Speciaal

Klik op **Speciaal** om een code in **Zoeken naar** te zetten: **Alineamarkering** (^p), **Tabteken** (^t), **Willekeurig cijfer** (^#), **Willekeurige letter** (^$), **Witruimte** (^w), **Sectie-einde** (^b), **Veld** (^d), **Voetnootmarkering** (^f), **Eindnootmarkering** (^e), of **Invoegteken** (^^). De codes werken zonder **Jokertekens gebruiken**. Een **sectie-einde** is de markering die een sectie beëindigt. Een **veld** is een stuk tekst dat het programma berekent, zoals een paginanummer of een datum, en **^d** vindt waar elk veld begint. De nootmarkeringen zijn de kleine cijfers in de tekst die naar een voetnoot of eindnoot verwijzen. Deze vier worden gevonden, maar **Vervangen** en **Alles vervangen** laten ze met rust, omdat een wijziging zou verliezen waar ze voor staan. De andere tekens in het vak, zoals **?** en **\***, zijn dan gewoon die tekens.

### Opmaak

Klik op **Opmaak** om alleen tekst met een bepaald uiterlijk te zoeken: **Vet**, **Cursief**, **Onderstreept**, **Doorgehaald**, **Gemarkeerd**, een **Lettertype**, of een alineastijl **Stijl**. Het deelvenster toont de opties die u onder de knoppen hebt gekozen. Kies **Geen opmaak** om ze te wissen.

U kunt **Zoeken naar** leeg laten. Dan is elk stuk tekst met dat uiterlijk een overeenkomst. Met woorden in **Zoeken naar** worden alleen de woorden met dat uiterlijk gevonden. **Alles vervangen** wijzigt alleen die woorden.

## Zoeken beperken

- **Hoofdlettergevoelig** zoekt alleen woorden met dezelfde hoofdletters. Met deze optie aan vindt "Plan" niet "plan".
- **Alleen hele woorden zoeken** slaat langere woorden over die het uwe bevatten. Met deze optie aan vindt "plan" niet "planeet".
- **Zoeken** bepaalt welke kant **Volgende zoeken** vanaf de cursor op gaat. **Omlaag** gaat naar het einde van het document, en **Omhoog** naar het begin. Elk stopt daar en zegt dat. **Alles** gaat door naar het andere einde en is de gebruikelijke keuze. **Alles vervangen** wijzigt altijd het hele document.

## Woorden vervangen

1. Druk op **Ctrl+H**, of klik op **Start > Bewerken > Vervangen**. De cursor gaat naar **Vervangen door**.
2. Typ de nieuwe woorden in **Vervangen door**. Om de gevonden woorden te verwijderen, laat u dit leeg.
3. Typ de oude woorden in **Zoeken naar**.
4. Klik op **Vervangen** om de eerste overeenkomst te zien. Klik nogmaals op **Vervangen** om die te wijzigen. JeffWrite gaat dan naar de volgende overeenkomst. Elke wijziging is één stap, zodat **Ongedaan maken** (Ctrl+Z) er één tegelijk terugzet.
5. Klik op **Alles vervangen** om alle overeenkomsten in één keer te wijzigen. Het deelvenster meldt hoeveel er zijn gewijzigd. **Alles vervangen** wijzigt ook de woorden in tekstvakken en vormen, en is één stap, zodat één **Ongedaan maken** alle woorden terugzet. Met **Wijzigingen bijhouden** aan wordt elke vervanging vastgelegd als gewijzigde tekst, zowel in de tekst als in de vakken. Een vak dat te groot is om te wijzigen, wordt in het bericht meegeteld. **Alles vervangen** bereikt geen tekstvakken in koppen en voetteksten, samengevoegde vormen of aan elkaar gekoppelde vakken. **Volgende zoeken** kijkt nog niet in tekstvakken.

De nieuwe woorden krijgen de hoofdletters van de woorden die ze vervangen. Als u een vet woord vervangt, wordt het nieuwe woord ook vet.

Om bijvoorbeeld "color" overal in "colour" te veranderen: typ **color** in **Zoeken naar**, typ **colour** in **Vervangen door**, en klik op **Alles vervangen**.

## Waar het zoeken naar kijkt

- Het kijkt naar de tekst van het document, ook de tekst in tabellen. Een bladwijzer midden in sommige woorden, zoals sommige bestanden hebben, verhindert niet dat die woorden worden gevonden. **Vervangen** en **Alles vervangen** behouden de bladwijzer.
- **Volgende zoeken** kijkt eerst in de tekst. Daarna gaat het naar de koppen, dan de voetteksten, dan de voetnoten, en weer terug naar het begin van de tekst. Wanneer een overeenkomst in een van die delen staat, opent JeffWrite het om te bewerken en selecteert de overeenkomst. Het deelvenster vermeldt waar het is, zoals "Gevonden in de koptekst van pagina 3." **Vervangen** wijzigt de overeenkomst waarop u staat, in de tekst of in een kop, voettekst of voetnoot. **Alles vervangen** wijzigt elke overeenkomst in al die delen, en één **Ongedaan maken** brengt ze allemaal terug. De eindnoten horen bij de tekst en worden dus met de tekst doorzocht. Zie [Koptekst en voettekst](headers-footers).
- Het slaat tekst over die is ingesteld op **Verborgen**, omdat die tekst niet wordt getoond. Zie [Tekstopmaak wijzigen](font).

## Zie ook

- [Tekstopmaak wijzigen](font)
- [JeffWrite met het toetsenbord gebruiken](keyboard)
