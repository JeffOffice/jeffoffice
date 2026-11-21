# Zoeken, vervangen en naar gaan
<!-- keywords: zoeken, vervangen, wijzigen, zoekopdracht, zoeken naar, vervangen door, alles vervangen, volgende zoeken, zoeken naar, wijzigen in, alles wijzigen, dezelfde hoofdletters, jokerteken, sterretje, vraagteken, tilde, hoofdlettergevoelig, hele cel, zoeken op opmaak, opmaak, notities vervangen, naar gaan, verwijzing, naam, selecteren op soort, lege cellen, constanten, formules, fouten, huidig gebied, huidige matrix, rijverschillen, kolomverschillen, laatste cel, zichtbare cellen, voorwaardelijke opmaak, validatie, Ctrl+F, Ctrl+H, F5, notities -->

## Zoeken

Druk op **Ctrl+F** (of kies **Zoeken** op het tabblad Start en kies daarna opnieuw **Zoeken**). Typ de tekst in **Zoeken naar** en druk op **Enter** voor de volgende cel die die tekst bevat, of kies **Alles weergeven** voor een lijst van alle cellen die overeenkomen. Klik op een regel van de lijst om naar die cel te gaan.

| Optie | Wat het doet |
| --- | --- |
| **Waar** | Zoekt in dit blad, of in alle bladen die zichtbaar zijn. Als meer dan één cel is geselecteerd, zoekt een zoekopdracht in één blad alleen in die cellen. |
| **Volgorde** | Bekijkt de cellen per rij (van links naar rechts, daarna omlaag) of per kolom (van boven naar beneden, daarna naar rechts). **Volgende zoeken** stopt bij de eerste overeenkomst in die volgorde. |
| **Zoeken in** | **Formules** zoekt in wat in elke cel is getypt, **Waarden** in wat er wordt weergegeven, en **Notities** in de notities bij cellen. |
| **Opmaak…** | Zoekt alleen de cellen die een bepaalde opmaak hebben. **Opmaak…** opent het dialoogvenster Celeigenschappen, gestart vanuit de cel met de cursor: wat u daar wijzigt, is wat wordt gezocht (een vet lettertype, een opvulkleur, een getalnotatie). **Opmaak uit cel overnemen** neemt alle onderdelen van de opmaak over van de cel met de cursor, en **Opmaak wissen** zet het zoeken terug op alleen de tekst. Het dialoogvenster laat zien wat er wordt gezocht, en meldt als de gekozen opmaak niets bevat. De weergave die een voorwaardelijke opmaak aan een cel geeft, wordt niet doorzocht, alleen de opmaak die de cel zelf heeft. Zonder tekst in **Zoeken naar** vinden **Alles weergeven** en **Volgende zoeken** de cellen met de opmaak; met tekst moet een cel beide hebben. |
| **Hoofdlettergevoelig** | Hoofdletters en kleine letters moeten gelijk zijn. |
| **Alleen hele cel** | De hele cel moet de tekst zijn, niet alleen een deel ervan. |

In de tekst die u typt bij **Zoeken naar** staat **?** voor één willekeurig teken en **\*** voor een willekeurig aantal tekens (ook geen). Typ **~** vóór ?, \* of ~ om het teken zelf te zoeken: **a~\*b** vindt a\*b.

Een zoekopdracht loopt stap voor stap, zodat u kunt doorwerken. **Esc** (of **Annuleren**) stopt de zoekopdracht, en dat geldt ook als u een cel wijzigt. Een zeer lange tekst kost bij elke stap meer tijd dan een korte tekst, zodat een blad met lange teksten het venster toch vrij laat.

Notities bij cellen volgen de cellen: na het sorteren, of nadat rijen of kolommen zijn ingevoegd of verwijderd, vindt een zoekopdracht in notities elke notitie bij de cel waarop hij nu staat. Een notitie bij een cel die is verwijderd, is niet meer te doorzoeken (en komt terug na Ongedaan maken). Als een werkmap zo vaak is gewijzigd dat de notities niet meer bij alle wijzigingen kunnen worden bijgehouden, wordt u gevraagd die eerst op te slaan. Een werkmap die is geopend uit een ouder bestandstype of uit een tekstbestand, heeft notities in een vorm die JeffCrunch niet doorzoekt; u krijgt daarvan een melding.

## Vervangen

Druk op **Ctrl+H**, typ de nieuwe tekst bij **Vervangen door** en kies een knop. **Vervangen** zet de nieuwe tekst in de cel met de cursor als die de tekst bevat, en gaat door naar de volgende. **Alles vervangen** doet dit in elke cel die de tekst bevat, als één stap die u ongedaan kunt maken; een grote wijziging wordt stap voor stap doorgevoerd. De nieuwe tekst wordt ingevoerd alsof u die typt: een getal wordt een getal, en een formule die wordt gewijzigd blijft een formule.

Om in notities te vervangen, kiest u **Notities** bij **Zoeken in** op het tabblad Vervangen. **Alles vervangen** wijzigt dan de woorden van elke notitie die de tekst bevat, op dit blad of op alle bladen, als één stap die u ongedaan kunt maken. **Vervangen** wijzigt de notitie bij de cel met de cursor en gaat door naar de volgende. Een notitie die een gesprek met opmerkingen is, blijft ongewijzigd (wijzig die op het tabblad Controleren), en u krijgt te zien hoeveel er zijn overgeslagen. Er wordt niets gewijzigd, en u krijgt te zien waarom, als een blad met een te wijzigen notitie is beveiligd, of als een notitie zonder woorden zou blijven of meer dan 32.767 tekens zou krijgen. Een notitie waarvan de woorden worden vervangen, verliest de datum waarop hij is geschreven, net zoals bij het bewerken van de woorden. De notities worden opgeslagen met de werkmap, in elke vorm die notities heeft. **Waarden** kunnen wel worden doorzocht, maar niet worden vervangen, omdat een waarde is wat een formule berekent.

Er wordt niets gewijzigd, en u krijgt te zien waarom, als een cel die zou worden gewijzigd is vergrendeld op een beveiligd blad, deel is van een matrix of een gegevenstabel, in de eerste rij van een tabel staat, of geen formule meer zou zijn.

## Naar gaan

Druk op **F5** (of **Ctrl+G**), of typ in het naamvak. U kunt een cel opgeven, zoals B7, een bereik zoals B3:D9, hele kolommen of rijen (C:E, 4:9), een cel van een ander blad (Blad2!A1, of 'Mijn blad'!A1), of de naam van een bereik.

## Selecteren op soort

Kies **Selecteren op soort** in het menu **Zoeken** om cellen te selecteren op wat ze zijn:

- **Lege cellen**, **Constanten** (ingetypt), **Formules**, en **Cellen met een fout**. Voor constanten en formules kunt u getallen, tekst, WAAR en ONWAAR, of fouten buiten beschouwing laten.
- **Het blok rond de cel met de cursor**, en **De laatste cel die wordt gebruikt**.
- **De matrixformule waarin de cursor staat** selecteert alle cellen van die matrixformule.
- **Cellen die in hun rij** en **in hun kolom** afwijken van de cel met de cursor, zoeken de cellen die niet gelijk zijn aan de cel met de cursor. Selecteer eerst een blok en plaats de cursor op de cel waarmee de andere moeten overeenkomen. In elke rij (of kolom) van het blok wordt een cel gekozen als die een ander getal of andere tekst bevat, of een formule die niet hetzelfde is wanneer de celnamen worden verschoven naar de plek van die cel. Een cel met een formule wordt ook gekozen als de cel met de cursor geen formule heeft. Als u maar één cel selecteert, kijkt JeffCrunch naar het blok eromheen.
- **Cellen die worden weergegeven** (niet in verborgen rijen of kolommen), **Cellen met voorwaardelijke opmaak**, en **Cellen met een regel voor wat mag worden ingevoerd** (allemaal, of alleen die met dezelfde regel als de cel met de cursor).

Het eerste gevonden blok wordt geselecteerd en andere worden in een tint weergegeven; de tint verdwijnt als u op een cel klikt.
