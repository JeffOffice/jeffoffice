# Rijen, kolommen en cellen invoegen en verwijderen
<!-- keywords: geknipte cellen invoegen, gekopieerde cellen invoegen, cellen verschuiven naar rechts, cellen verschuiven naar beneden, invoegen, verwijderen, wissen, rij, kolom, cel, verschuiven, verplaatsen, toevoegen, leeg, Ctrl+plus, Ctrl+min, klikken met rechtermuisknop, koptekst, ongedaan maken, verwijzing, #REF!, formule, naam, samengevoegd, tabel, matrix, rand, knippen, plakken, kopiëren, cellen invoegen, rijen invoegen, kolommen invoegen, cellen verwijderen, rijen verwijderen, kolommen verwijderen -->

U kunt lege rijen, kolommen of cellen invoegen midden in uw gegevens, en rijen, kolommen of cellen verwijderen. Alles hieronder verschuift om ruimte te maken of de opening te sluiten, en formules die verwijzen naar cellen die zijn verplaatst, blijven naar diezelfde cellen verwijzen.

## Invoegen

1. Selecteer de rijnummers, de kolomletters of de cellen waar u de nieuwe wilt hebben. Wilt u drie rijen toevoegen, selecteer dan drie rijen.
2. Druk op **Ctrl en de plustoets** (**Ctrl en de mintoets** verwijdert), of gebruik **Start > Invoegen**, of klik met de rechtermuisknop op de selectie en kies **Invoegen**.

Zijn hele rijen of kolommen geselecteerd, dan worden de nieuwe in één keer ingevoegd, boven de rijen of links van de kolommen. Zijn cellen geselecteerd, dan verschijnt een klein venster waarin u kiest hoe de cellen erna moeten verschuiven:

- **Cellen naar rechts verschuiven** of **Cellen naar beneden verschuiven** verplaatst alleen de cellen in de kolommen (of rijen) van uw selectie, en laat de rest van het blad staan waar het staat.
- **Hele rij** of **Hele kolom** voegt hele rijen of kolommen toe.

## Geknipte of gekopieerde cellen invoegen

Wilt u cellen die u hebt geknipt of gekopieerd tussen andere cellen zetten, in plaats van eroverheen, knip of kopieer ze dan eerst (**Ctrl+X** of **Ctrl+C**). Selecteer vervolgens de cel waar ze naartoe moeten, en gebruik **Start > Invoegen > Geknipte of gekopieerde cellen invoegen** (de opdracht heet **Geknipte cellen invoegen** of **Gekopieerde cellen invoegen** zodra er iets is om in te voegen), of klik met de rechtermuisknop op de cel en kies die opdracht. Er verschijnt een klein venster waarin u kiest of de cellen die er al staan **naar rechts** of **naar beneden** moeten verschuiven om ruimte te maken.

De knop **Invoegen** en **Ctrl en de plustoets** (ook **Ctrl+Shift en de plustoets**) doen dit ook, maar alleen wanneer duidelijk is wat u bedoelt:

- wanneer cellen **geknipt** zijn en het knippen nog open staat (de stippellijn is zichtbaar) op het blad waar ze vandaan komen, en
- wanneer cellen **gekopieerd** zijn en u sindsdien niets hebt gewijzigd (geen typen, geen andere bewerking, geen Ongedaan maken of Opnieuw).

In alle andere gevallen voegen de knop en de toetsen lege cellen in, zoals altijd. Nadat u iets hebt gewijzigd, blijft een kopie beschikbaar voor **Plakken** en voor de opdracht in het menu. **Esc** beëindigt een knippen of kopiëren: de rand verdwijnt, en de knop en de toetsen voegen weer lege cellen in. Zijn cellen op het ene blad geknipt en drukt u op het andere blad op de toetsen, dan worden lege cellen ingevoegd; de opdracht in het menu laat zien dat geknipte cellen alleen op hun eigen blad tussen andere cellen kunnen worden gezet. Zijn hele rijen of kolommen geselecteerd, dan is de opdracht uitgeschakeld en voegt Invoegen rijen of kolommen toe zoals voorheen.

- Geknipte cellen **worden verplaatst**: ze verlaten hun oude plek, en wat ernaar verwijst, volgt hen. Dat is hetzelfde als de **Shift**-toets ingedrukt houden terwijl u de rand van de selectie sleept, en het gebeurt in zijn geheel of helemaal niet: wordt een deel geweigerd, dan verandert er niets.
- Gekopieerde cellen **worden ingevoegd** als kopie, zoals **Plakken** ze zou plakken (volgens de instelling van **Standaard plakken instellen**, en met formules die aan hun nieuwe plek zijn aangepast). De kopie blijft klaarstaan, zodat u haar elders opnieuw kunt invoegen.
- De cellen worden één keer ingevoegd, bij de linkerbovencel van wat u hebt geselecteerd, ook als u meer cellen hebt geselecteerd dan u hebt gekopieerd.
- Is de lijst gefilterd, dan komen de cellen in de rijen die zichtbaar zijn, zoals bij plakken, en de ruimte die ervoor wordt gemaakt, omvat de rijen die het filter daartussen verbergt. Een rij die u met de hand hebt verborgen, telt als elke andere rij.
- Elk van deze handelingen is één stap die u ongedaan kunt maken. Geknipte cellen kunnen alleen op hun eigen blad tussen andere cellen worden gezet; wilt u ze naar een ander blad verplaatsen, gebruik dan **Plakken**.
- Een cel kan niet op zijn eigen pad worden ingevoegd (cellen die zijn geknipt uit A3:A4 kunnen niet tussen A3 en A4 worden ingevoegd), en een beveiligd blad weigert het. JeffCrunch vertelt u waarom en verandert dan niets.

## Verwijderen

Selecteer wat u wilt verwijderen en druk op **Ctrl en de mintoets**, of gebruik **Start > Verwijderen**. Zijn cellen geselecteerd, dan vraagt het venster of u **cellen naar links verschuiven**, **cellen naar boven verschuiven** wilt, of de **hele rij** of **hele kolom** wilt verwijderen. Verwijderen is één stap, en met **Ongedaan maken** komt alles terug wat is verwijderd, inclusief de formules.

## Wat er meeverhuist

Een formule die verwijst naar cellen die verschuiven, verwijst daarna naar die cellen op hun nieuwe plek. Voorbeeld: in B1 staat **=SOM(A1:A5)**. Voegt u een rij in boven rij 3, dan wordt de formule **=SOM(A1:A6)**, omdat het bereik is uitgebreid om de nieuwe rij op te nemen. Verwijdert u rij 3, dan wordt het **=SOM(A1:A4)**. Een formule die verwijst naar een cel die u hebt verwijderd, toont **#REF!**. Dat betekent dat de cel die nodig is, niet meer bestaat. Met **Ongedaan maken** komen de cel en de formule terug.

Ook deze onderdelen volgen de cellen: formules op elk blad, namen (zoals afdrukbereiken en afdruktitels), samengevoegde cellen, de cellen waarvoor regels voor validatie en voorwaardelijke opmaak gelden, tabellen, grafieken, afbeeldingen, notities, hyperlinks en pagina-einden.

## Wat niet kan

- Cellen kunnen niet worden ingevoegd of verwijderd in een deel van een **matrixformule** of een gegevenstabel. Verplaats ze in hun geheel, of laat ze staan. Een cel die een gegevenstabel als invoer gebruikt, kan ook niet worden verwijderd.
- Een **draaitabel** kan nog niet worden verplaatst of doorgeknipt: rijen, kolommen of cellen kunnen niet worden ingevoegd of verwijderd waar dat de draaitabel zou verplaatsen of wijzigen, en cellen kunnen er niet uit of in worden geknipt. De rest van het blad kunt u gewoon wijzigen. Een draaitabel wijzigt u met de opdrachten op het tabblad **Draaitabel** (vernieuwen, gegevensbron wijzigen, velden en indeling), niet door in de cellen te typen.
- Een **tabel** kan niet doormidden worden geknipt: u kunt er geen kolom in invoegen, en u kunt de koprij of alle rijen niet verwijderen. Voeg rijen in de tabel in of verwijder ze daaruit, of wijzig de cellen eromheen.
- Op een **beveiligd blad** kunnen geen rijen, kolommen of cellen worden ingevoegd of verwijderd. Kies hiervoor **Bladbeveiliging opheffen** op het tabblad **Controleren**.
- Cellen aan de rand van het blad die iets bevatten, kunnen niet van de rand worden weggeduwd. Verwijder ze eerst.
- Na het opslaan gaat **Ongedaan maken** nog steeds terug over wat u eerder deed, ook over rijen en kolommen die u hebt ingevoegd (bij het volgende opslaan worden die weer verwijderd). Het enige wat niet terug kan, zijn rijen, kolommen of cellen die u **hebt verwijderd**: het opgeslagen bestand heeft wat erin stond al verloren. **Ongedaan maken** stopt daarom bij het laatste verwijderen dat u hebt opgeslagen, en de regel onderaan het venster laat dat zien wanneer u opslaat.
- Een stap die zoveel cellen verplaatst of verwijdert dat **Ongedaan maken** die niet kan bewaren (meer dan ongeveer 128 MB), vraagt eerst: "Dit kan niet ongedaan worden gemaakt." Gaat u door, dan wordt de stap uitgevoerd, en kan **Ongedaan maken** hem niet meer terugdraaien, net als wat ervoor kwam.
