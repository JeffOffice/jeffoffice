# Pack and Go: een werkmap verzenden met alles wat erbij hoort
<!-- keywords: pack and go, pakket, verpakken, verzenden, delen, andere computer, zip, map, lettertypen, gekoppelde bestanden, gekoppelde afbeeldingen, ontbrekende afbeeldingen, kopiëren, meenemen, usb-stick, licentie, externe koppeling, gekoppelde werkmap, cellen, grafieken, pack and go zip, bestanden bundelen -->

Wanneer u een werkmap naar iemand stuurt, kunnen de afbeeldingen en andere werkmappen waarnaar hij verwijst, onderweg verloren gaan. **Pack and Go** slaat de werkmap op samen met die bestanden en zijn lettertypen, in één zip-bestand of in één map. Zo ziet de werkmap er ook op een andere computer hetzelfde uit.

## Zo gebruikt u het

1. Klik op **Bestand** en vervolgens op **Exporteren**.
2. Klik op **Pack and Go: opslaan voor een andere computer** om één zip-bestand te maken, of op **Pack and Go: opslaan in een map** om een nieuwe map te maken.
3. Kies waar u het wilt opslaan. Voor een zip-bestand typt u een naam. Voor een map kiest u de map waarin de nieuwe map moet komen.
4. Lees het bericht dat verschijnt. Daarin staat waar het pakket staat, hoeveel gekoppelde bestanden en lettertypebestanden er in zitten, en wat niet kon worden ingepakt.

Een **zip-bestand** is één bestand dat andere bestanden bevat, dus het is makkelijk te verzenden. De ontvanger opent het om de bestanden eruit te halen. Een map is handiger wanneer u alles naar een schijf wilt kopiëren.

## Wat er in het pakket komt

- **De werkmap zelf**, met dezelfde naam als nu. Het is een kopie. Uw eigen bestand verandert niet.
- **Gekoppelde afbeeldingen.** Een afbeelding in de werkmap kan aan haar bestand zijn gekoppeld. Het pakket bevat dan een kopie van elk bestand. Afbeeldingen die rechtstreeks in de werkmap zijn ingevoegd, zitten al in de werkmap.
- **Gekoppelde werkmappen.** Een formule kan getallen uit een andere werkmap halen. Het pakket bevat ook een kopie van die werkmap, en de koppeling verwijst nu naar de kopie. De lettertypen zijn die uit cellen, grafieken en vormen.
- **Lettertypen.** Een lettertype bepaalt het uiterlijk van de letters. Het pakket bevat de lettertypebestanden die bij het programma horen en die de werkmap gebruikt, in een map met de naam **fonts**. Pack and Go verpakt nooit een lettertype waarvan de licentie zegt dat het niet mag worden meegegeven. Het bericht noemt elk lettertype dat is weggelaten. Lettertypen die u zelf op uw computer hebt geïnstalleerd, worden niet ingepakt, dus de ontvanger heeft die mogelijk nodig.

## Hoe de koppelingen veranderen

In de kopie wijst elke koppeling die is ingepakt naar het bestand binnen het pakket, met een korte naam, zoals **linked/photo.png**. De koppelingen gebruiken niet langer het pad op uw computer. Daardoor werken ze overal waar het pakket wordt geopend. Hebben twee bestanden dezelfde naam, dan krijgt het tweede bestand een nummer achter de naam.

Een koppeling die Pack and Go niet kan volgen, blijft zoals hij was, en het bericht somt die koppelingen op. Pack and Go verpakt het volgende niet:

- Een bestand dat niet kan worden gevonden.
- Een webadres. Die blijven zoals ze zijn.
- Een bestand op een andere computer in uw netwerk.
- Een koppeling die omhoog en buiten de map van de werkmap gaat, zoals **../../notities.png**.
- Een document, werkmap of ander object dat niet in de eigen map van de werkmap staat (of in een map daarbinnen), ook niet wanneer de koppeling het volledige pad noemt. Alleen afbeeldingen, films en geluiden worden van overal ingepakt.
- Een koppeling die via een snelkoppeling naar een andere map uit de map van de werkmap leidt (een symbolische koppeling).
- Een koppeling naar de werkmap zelf.
- Een bestand dat geen afbeelding, film, geluid of document is. Een programma wordt nooit ingepakt.

Pack and Go zoekt in de gekoppelde bestanden niet naar verdere koppelingen. Daardoor kan een bestand dat terugverwijst naar dit bestand nooit een lus veroorzaken.

## Grenzen

Om te voorkomen dat een pakket te groot wordt, vraagt Pack and Go eerst om bevestiging wanneer er meer dan 200 gekoppelde bestanden zijn, of meer dan 256 MB aan gekoppelde bestanden. Kiest u **Nee**, dan wordt er niets opgeslagen. Pack and Go verpakt nooit meer dan 2.000 bestanden of meer dan 1 GB, wat u ook antwoordt. De bestanden daarboven worden weggelaten en in het bericht vermeld.

Gebruikt u een wachtwoord voor de werkmap, dan heeft de kopie in het pakket hetzelfde wachtwoord.
