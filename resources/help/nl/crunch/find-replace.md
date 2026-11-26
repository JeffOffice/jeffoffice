# Zoeken, vervangen en naar een cel gaan
<!-- keywords: zoeken, vervangen, wijzigen, zoeken naar, zoekterm, vervangen door, alles vervangen, volgende zoeken, zoeken naar, wijzigen in, alles wijzigen, dezelfde hoofdletters, jokerteken, sterretje, vraagteken, tilde, hoofdlettergevoelig, hele cel, zoeken op opmaak, opmaak, notities vervangen, naar cel gaan, verwijzing, naam, selecteren op soort, lege cellen, constanten, formules, fouten, huidig gebied, huidige matrix, verschillen per rij, verschillen per kolom, laatste cel, zichtbare cellen, voorwaardelijke opmaak, validatie, Ctrl+F, Ctrl+H, F5, notities -->

## Zoeken

Druk op **Ctrl+F** (of kies op het tabblad Start **Zoeken en selecteren** en daarna opnieuw **Zoeken**). Typ de tekst bij **Zoeken naar** en druk op **Enter** voor de volgende cel die de tekst bevat, of kies **Alles zoeken** om een lijst te krijgen van elke cel die overeenkomt. Klik op een regel in de lijst om naar die cel te gaan.

| Optie | Wat het doet |
| --- | --- |
| **Waar** | Zoek in dit blad, of in alle zichtbare bladen. Bij meer dan één geselecteerde cel zoekt een zoekopdracht in één blad alleen in die cellen. |
| **Volgorde** | Bekijk de cellen per rij (van links naar rechts, dan omlaag) of per kolom (van boven naar beneden, dan naar rechts). **Volgende zoeken** stopt bij de eerste overeenkomst in die volgorde. |
| **Zoeken in** | **Formules** zoekt in wat in elke cel is getypt, **Waarden** in wat de cel toont, en **Notities** in de notities bij cellen. |
| **Opmaak…** | Zoekt alleen naar cellen die een bepaalde opmaak hebben. **Opmaak…** opent het dialoogvenster Cellen opmaken, gestart vanaf de cel met de cursor: wat u daar wijzigt, is waarnaar wordt gezocht (een vet lettertype, een opvulkleur, een getalnotatie). **Opmaak overnemen uit cel** neemt alle onderdelen van de opmaak over van de cel met de cursor, en **Opmaak wissen** zet het weer zo dat alleen op tekst wordt gezocht. Het dialoogvenster laat zien waarnaar wordt gezocht, en meldt het als de gekozen opmaak niets bevat. De uiterlijk die een voorwaardelijke opmaak aan een cel geeft, wordt niet doorzocht, alleen de opmaak die de cel zelf heeft. Zonder tekst bij **Zoeken naar** vinden **Alles zoeken** en **Volgende zoeken** de cellen met de opmaak; met tekst moet een cel beide hebben. |
| **Hoofdlettergevoelig** | Hoofdletters en kleine letters moeten gelijk zijn. |
| **Hele cel** | De hele cel moet de tekst zijn, niet alleen een deel ervan. |

In de tekst die u bij **Zoeken naar** typt, staat **?** voor één willekeurig teken en **\*** voor een willekeurig aantal tekens (ook geen). Typ **~** vóór ?, \*, of ~ om het teken zelf te zoeken: **a~\*b** vindt a\*b.

Een zoekopdracht loopt stap voor stap, zodat u kunt doorwerken. **Esc** (of **Annuleren**) stopt de zoekopdracht, en dat geldt ook als u een cel wijzigt. Een zeer lange tekst kost bij elke stap meer tijd dan een korte tekst, zodat een blad met lange teksten het venster toch vrijlaat.

Notities bij cellen volgen de cellen: na een sortering, of nadat rijen of kolommen zijn ingevoegd of verwijderd, vindt een zoekopdracht in notities elke notitie bij de cel waarop hij nu staat. Een notitie bij een verwijderde cel verdwijnt uit de zoekopdracht (en komt terug na Ongedaan maken). Als een werkmap zo vaak is gewijzigd dat de notities niet meer in alle wijzigingen kunnen worden gevolgd, wordt u gevraagd eerst op te slaan. Een werkmap die is geopend uit een ouder type bestand of uit een tekstbestand heeft zijn notities in een vorm die JeffCrunch niet doorzoekt, en u krijgt daarvan een melding.

## Vervangen

Druk op **Ctrl+H**, typ de nieuwe tekst bij **Vervangen door**, en kies een knop. **Vervangen** zet de nieuwe tekst in de cel met de cursor als die de tekst bevat, en gaat door naar de volgende. **Alles vervangen** doet dat in elke cel die de tekst bevat, als één stap die u ongedaan kunt maken; een grote wijziging wordt stap voor stap doorgevoerd. De nieuwe tekst wordt ingevoerd alsof u hem typte: een getal wordt een getal, en een formule die wordt gewijzigd blijft een formule.

Om in notities te vervangen, kiest u **Notities** bij **Zoeken in** op het tabblad Vervangen. **Alles vervangen** wijzigt dan de woorden van elke notitie die de tekst bevat, op dit blad of op alle bladen, als één stap die u ongedaan kunt maken, en **Vervangen** wijzigt de notitie bij de cel met de cursor en gaat door naar de volgende. Een notitie die een gesprek van opmerkingen is, blijft ongewijzigd (wijzig die op het tabblad Controleren), en u krijgt te zien hoeveel er zijn overgeslagen. Er wordt niets gewijzigd, en u krijgt een melding waarom, als een blad met een te wijzigen notitie is beveiligd, of als een notitie zonder woorden zou blijven of meer dan 32.767 tekens zou krijgen. Een notitie waarvan de woorden worden vervangen, verliest de datum waarop ze is geschreven, zoals ook gebeurt wanneer u de woorden bewerkt. De notities worden opgeslagen met de werkmap, in elke vorm die notities heeft. **Waarden** kunnen worden doorzocht maar niet vervangen, omdat een waarde is wat een formule berekent.

Er wordt niets gewijzigd, en u krijgt een melding waarom, als een cel die zou worden gewijzigd is vergrendeld op een beveiligd blad, deel is van een matrix of een gegevenstabel, in de eerste rij van een tabel staat, of geen formule meer zou zijn.

## Naar een cel gaan

Druk op **F5** (of **Ctrl+G**), of typ in het naamvak. U kunt een cel opgeven zoals B7, een bereik zoals B3:D9, hele kolommen of rijen (C:E, 4:9), een cel van een ander blad (Blad2!A1, of 'Mijn blad'!A1), of de naam van een bereik. Een naam die voor dezelfde cellen op een reeks bladen staat, zoals Jan:Dec!B2, brengt u naar het eerste blad van die reeks.

## Selecteren op soort

Kies **Selecteren op soort** in het menu **Zoeken** om cellen te selecteren op wat ze zijn:

- **Lege cellen**, **Constanten** (ingetypt), **Formules**, en **Cellen met een fout**. Bij constanten en formules kunt u getallen, tekst, WAAR en ONWAAR, of fouten weglaten.
- **Het blok rond de cel met de cursor**, en **De laatste cel die wordt gebruikt**.
- **De matrixformule waarin de cursor staat** selecteert alle cellen van die matrixformule.
- **Cellen die in hun rij afwijken van de cel met de cursor** en **in hun kolom** zoeken de cellen die niet gelijk zijn aan de cel met de cursor. Selecteer eerst een blok en zet de cursor op de cel waarmee de andere moeten overeenkomen. In elke rij (of kolom) van het blok wordt een cel gekozen als die een ander getal of andere tekst bevat, of een formule die, nadat de celnamen zijn verschoven naar de plek van die cel, niet hetzelfde is. Een cel met een formule wordt ook gekozen als de cel met de cursor geen formule heeft. Als u maar één cel selecteert, kijkt JeffCrunch naar het blok eromheen.
- **Cellen die de formules in de selectie gebruiken** en **Cellen met formules die de selectie gebruiken** selecteren de cellen op dit blad die de formules van de geselecteerde cellen gebruiken, of de cellen waarvan de formules de geselecteerde cellen gebruiken. Bij één geselecteerde cel begint de zoekopdracht bij die cel. Vink **doorgaan naar elk niveau** aan om verder te gaan vanaf elke gevonden cel (de cellen die die gebruiken, enzovoort); zonder dit vinkje wordt alleen het eerste niveau geselecteerd. Er wordt begonnen bij maximaal 1.000 van de geselecteerde cellen, en de statusbalk meldt als een zoekopdracht bij een limiet is gestopt.
- **Cellen die zichtbaar zijn** (niet in verborgen rijen of kolommen), **Cellen met voorwaardelijke opmaak**, en **Cellen met een regel voor wat mag worden ingevoerd** (allemaal, of alleen die met dezelfde regel als de cel met de cursor).

Het eerste gevonden blok wordt geselecteerd en eventuele andere worden gekleurd; die kleur verdwijnt wanneer u op een cel klikt.
