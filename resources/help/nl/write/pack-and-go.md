# Pack and Go: een document verzenden met wat het nodig heeft
<!-- keywords: pack and go, inpakken, verzenden, versturen, delen, andere computer, zip, map, lettertypen, gekoppelde bestanden, gekoppelde afbeeldingen, ontbrekende afbeeldingen, kopiëren, meenemen, usb-stick, licentie, INCLUDEPICTURE, INCLUDETEXT, veld, pakket, inpakken en meenemen -->

Wanneer u een document naar iemand verstuurt, kunnen de afbeeldingen en lettertypen die het gebruikt onderweg verloren gaan. **Pack and Go** slaat het document op samen met wat het nodig heeft, in één zip-bestand of in één map, zodat het er op een andere computer hetzelfde uitziet.

## Zo gebruikt u het

1. Klik op **Bestand** en daarna op **Exporteren**.
2. Klik op **Pack and Go: opslaan voor een andere computer** om één zip-bestand te maken, of op **Pack and Go: opslaan in een map** om een nieuwe map te maken.
3. Kies waar u het wilt opslaan. Voor een zip-bestand typt u een naam. Voor een map kiest u de map waarin de nieuwe map moet komen.
4. Lees het bericht dat verschijnt. Daarin staat waar het pakket is, hoeveel gekoppelde bestanden en lettertypebestanden erin zijn gezet, en wat niet kon worden ingepakt.

Een **zip-bestand** is één bestand dat andere bestanden bevat, en is daardoor makkelijk te versturen. De ontvanger opent het om de bestanden eruit te halen. Een map is handiger wanneer u alles naar een schijf wilt kopiëren.

## Wat er in het pakket komt

- **Het document zelf**, met dezelfde naam als nu. Het is een kopie. Uw eigen bestand verandert niet.
- **Gekoppelde afbeeldingen.** Een gekoppelde afbeelding toont een afbeelding uit een bestand, in plaats van de afbeelding zelf op te slaan. Het pakket bevat een kopie van elke afbeelding.
- **Bestanden die door velden worden genoemd.** Een veld is een code die zijn eigen resultaat invult. Een **INCLUDEPICTURE**-veld toont een afbeelding uit een bestand, en een **INCLUDETEXT**-veld toont tekst uit een ander document. Het pakket bevat een kopie van elk bestand, en het veld verwijst nu naar die kopie.
- **Lettertypen.** Een lettertype is de vormgeving van de letters. Het pakket bevat de lettertypebestanden die bij het programma horen en die het document gebruikt, in een map met de naam **fonts**. Pack and Go past nooit een lettertype in waarvan de licentie zegt dat het niet mag worden meegegeven. In het bericht staat welk lettertype is weggelaten. Lettertypen die u zelf op uw computer hebt geïnstalleerd, worden niet ingepakt, dus de ontvanger heeft die misschien nodig.

## Hoe de koppelingen veranderen

In de kopie wijst elke koppeling die is ingepakt naar het bestand binnen het pakket, met een korte naam, zoals **linked/foto.png**. De koppelingen gebruiken niet meer het pad op uw computer, zodat ze werken waar het pakket ook wordt geopend. Als twee bestanden dezelfde naam hebben, krijgt het tweede een nummer achter de naam.

Een koppeling die Pack and Go niet kan volgen, blijft zoals hij was, en het bericht somt die op. Pack and Go past het volgende niet in:

- Een bestand dat niet kan worden gevonden.
- Een webadres. Die blijven zoals ze zijn.
- Een bestand op een andere computer in uw netwerk.
- Een koppeling die omhoog en naar buiten de map van het document wijst, zoals **../../notities.png**.
- Een document, werkmap of ander object dat niet in de eigen map van het document staat (of in een map daarbinnen), ook niet wanneer de koppeling het volledige pad noemt. Alleen afbeeldingen, films en geluiden worden van overal ingepakt.
- Een koppeling die via een snelkoppeling naar een andere map (een symbolische koppeling) de map van het document verlaat.
- Een koppeling naar het document zelf.
- Een bestand dat geen afbeelding, film, geluid of document is. Een programma wordt nooit ingepakt.

Pack and Go zoekt in de gekoppelde bestanden niet naar verdere koppelingen. Daardoor kan een bestand dat terugverwijst naar dit bestand geen lus veroorzaken.

## Limieten

Om te voorkomen dat een pakket te groot wordt, vraagt Pack and Go eerst om bevestiging wanneer er meer dan 200 gekoppelde bestanden zijn, of meer dan 256 MB aan gekoppelde bestanden. Kiest u **Nee**, dan wordt er niets opgeslagen. Pack and Go past nooit meer dan 2.000 bestanden in, en nooit meer dan 1 GB, ongeacht uw antwoord. De bestanden daarboven worden weggelaten en in het bericht vermeld.

Gebruikt u een wachtwoord voor het document, dan heeft de kopie in het pakket hetzelfde wachtwoord.
