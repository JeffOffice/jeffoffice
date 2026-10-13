# Overgangen tussen dia's
<!-- keywords: overgang, overgangen, diaovergang, morphen, morph, vervagen, duwen, vegen, splitsen, onthullen, bedekken, blootleggen, zoomen, knippen, effectopties, objecten, woorden, tekens, overeenkomen, paren, koppelen, naam, richting, duur, dia doorlopen, volgende dia, bij muisklik, na, automatisch, tijdsinstellingen, op alles toepassen, voorbeeld, ster -->

Een **overgang** is het effect dat wordt afgespeeld wanneer de diavoorstelling van de ene dia naar de volgende gaat. Een overgang hoort bij de dia waar de voorstelling naartoe gaat. De diavoorstelling speelt de overgang af wanneer deze vooruitgaat naar die dia.

## Een overgang toevoegen

1. Klik op de dia in het deelvenster met dia's.
2. Klik op het tabblad **Overgangen**. Klik in de galerie **Overgang naar deze dia** op een effect: **Geen**, **Morphen**, **Vervagen**, **Duwen**, **Vegen**, **Splitsen**, **Onthullen**, **Bedekken**, **Blootleggen**, **Zoomen** of **Knippen**. De miniatuur van de dia in het deelvenster met dia's krijgt een klein sterretje om aan te geven dat de dia een overgang heeft.
3. Klik op **Overgangen > Voorbeeld > Voorbeeld** om de overgang naar de dia te zien, afgespeeld over het dia-gebied. Druk op Esc om te stoppen.

Kies **Geen** om een overgang te verwijderen.

## Kiezen hoe de overgang verloopt

Klik op **Effectopties** voor de keuzes van het effect:

- **Duwen**, **Vegen**, **Onthullen**, **Bedekken** en **Blootleggen** hebben een richting: **Van rechts**, **Van links**, **Van onderen** of **Van boven**.
- **Splitsen** en **Zoomen** gaan naar **Uit** of **In**.
- **Morphen** verplaatst **Objecten**, **Woorden** of **Tekens**. Zie hieronder.
- **Vervagen** en **Knippen** hebben geen keuzes. Bij knippen gaat de overgang via zwart.

## Morphen

**Morphen** verplaatst wat twee dia's gemeenschappelijk hebben. Een **object** is één element op een dia: een vorm, een afbeelding, een tekstvak, een lijn of een tabel. De voorstelling gaat van de ene dia naar de volgende. Elk object dat op beide dia's staat, glijdt van de plek waar het op de eerste dia staat naar de plek waar het op de tweede dia staat. Onderweg wordt het groter of kleiner, draait het en verandert het van kleur. Een object dat alleen op de eerste dia staat, vervaagt. Een object dat alleen op de tweede dia staat, verschijnt geleidelijk.

Zo maakt u een morph:

1. Klik met de rechtermuisknop op een dia in het deelvenster met dia's en kies **Dia dupliceren**. De kopie heeft dezelfde objecten.
2. Verplaats op de kopie een object, maak het groter, draai het of wijzig de kleur. U kunt ook de tekst wijzigen, of objecten toevoegen en verwijderen.
3. Klik op de kopie en klik vervolgens op **Overgangen > Overgang naar deze dia > Morphen**. Een morph duurt 2 seconden. U kunt dat wijzigen met **Duur**.
4. Klik op **Voorbeeld** om de morph te bekijken of start de diavoorstelling.

### Welke objecten bij elkaar horen

JeffDeck vormt paren van een object op de eerste dia en een object op de tweede dia. Daarbij probeert JeffDeck deze manieren, in deze volgorde. Elk object hoort bij hooguit één paar.

1. **Een naam die begint met !!.** Twee objecten met dezelfde naam, waarbij de naam begint met twee uitroeptekens, vormen altijd een paar. Dat kunnen twee willekeurige objecten zijn, zelfs een cirkel en een ster.
2. **Dezelfde naam.** Twee objecten van hetzelfde soort met dezelfde naam vormen een paar. Een tekstvak en een vorm zijn verschillende soorten.
3. **Dezelfde tekst.** Twee tekstvakken met dezelfde woorden vormen een paar, ongeacht hun namen. Hetzelfde geldt voor twee vormen met dezelfde woorden.
4. **Dezelfde afbeelding.** Twee afbeeldingen van dezelfde foto of illustratie vormen een paar, ook als ze een andere naam of een andere grootte hebben.
5. **Hetzelfde soort vorm.** Twee cirkels vormen een paar. Hetzelfde geldt voor twee lijnen of twee tabellen.

Als meerdere objecten een paar kunnen vormen, worden eerst de objecten gekoppeld die het dichtst bij elkaar staan.

De namen van objecten komen uit het presentatiebestand. Een presentatie die in een ander programma is gemaakt, behoudt de namen die daar aan de objecten zijn gegeven, en JeffDeck gebruikt die.

### Hoe een paar beweegt

- Twee objecten met dezelfde vorm worden één object dat beweegt. De positie, grootte, draaiing, opvulkleur, lijnkleur en transparantie veranderen geleidelijk. Ook het bijsnijden van een afbeelding verandert mee.
- Twee objecten met een verschillende vorm, zoals een cirkel en een vierkant, gaan naar dezelfde plekken terwijl het ene object overvloeit in het andere.
- Als de tekst van een paar niet hetzelfde is, vloeit de oude tekst over in de nieuwe tekst.
- Objecten die door een animatie worden verborgen, worden overgeslagen. Een object dat door een animatie is verplaatst, begint op de plek waar het is geëindigd.

### Wat beweegt: Objecten, Woorden of Tekens

Klik op **Effectopties** om te kiezen:

- **Objecten.** De tekst van een paar blijft in het object. De oude tekst vloeit over in de nieuwe.
- **Woorden.** Een woord dat in beide teksten voorkomt, beweegt van de oude naar de nieuwe plek. De andere woorden vervagen en verschijnen geleidelijk. Een leesteken zoals een komma telt als een apart woord.
- **Tekens.** Hetzelfde, maar per letter of cijfer.

**Woorden** en **Tekens** werken bij tekstvakken en vormen. Tekst die langer is dan 16.000 tekens, verticale tekst en tekst in een gespiegelde vorm worden afgespeeld als **Objecten**. Dat geldt ook voor een paar dat geen woord of teken gemeenschappelijk heeft.

Morphen is ook een overgang in andere presentatieprogramma's (de versies van 2019 en later). JeffDeck leest en bewaart de overgang op dezelfde manier. Oudere programma's spelen een morph af als vervaging.

## De tijdsinstellingen opgeven

Met de groep **Tijdsinstellingen** bepaalt u hoe lang de overgang duurt en hoe de voorstelling de dia verlaat.

- **Duur** is het aantal seconden dat het effect duurt. Een nieuwe overgang duurt 0,7 seconden. U kunt een waarde van 0,01 tot 59 typen.
- **Bij muisklik** is standaard ingeschakeld. De voorstelling gaat verder naar de volgende dia wanneer de presentator klikt. Schakel deze optie uit als u verder wilt gaan met een toets, zoals de spatiebalk of pijl-rechts. De animaties van de dia worden nog steeds bij een klik afgespeeld.
- Met **Na** gaat de voorstelling vanzelf verder na het aantal seconden dat u typt. U kunt **Na** samen met **Bij muisklik** gebruiken: de voorstelling gaat verder bij wat het eerst gebeurt.

**Op alles toepassen** geeft elke dia de overgang en de tijdsinstellingen van de dia waarop u zich bevindt.

Elk van deze handelingen is één stap die u met Ctrl+Z ongedaan maakt.

## Overgangen uit andere bestanden

Een presentatie die in een ander programma is gemaakt, kan een overgang bevatten waarvoor JeffDeck geen effect heeft. Die overgang wordt afgespeeld als vervaging en wordt ongewijzigd opgeslagen, zodat deze nog steeds werkt in het programma waaruit deze afkomstig is.

## Verwante onderwerpen

- [Het tabblad Overgangen](tab-transitions)
- [Animaties](animations)
- [De diavoorstelling afspelen en de presentatorweergave gebruiken](show)
