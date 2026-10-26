# Zoeken, vervangen en ga naar
<!-- keywords: zoeken, vervangen, wijzigen, zoekopdracht, jokerteken, wildcard, sterretje, vraagteken, tilde, hoofdlettergevoelig, alleen hele cel, ga naar, verwijzing, naam, selecteren op soort, lege cellen, constanten, formules, fouten, huidig gebied, laatste cel, zichtbare cellen, voorwaardelijke opmaak, gegevensvalidatie, Ctrl+F, Ctrl+H, F5, notities -->

## Zoeken

Druk op **Ctrl+F** (of kies **Zoeken** op het tabblad Start en dan nogmaals **Zoeken**). Typ de tekst en druk op **Enter** om naar de volgende cel met die tekst te gaan, of kies **Alles weergeven** voor een lijst van alle overeenkomende cellen. Klik op een regel in de lijst om naar die cel te gaan.

| Optie | Wat deze doet |
| --- | --- |
| **Bereik** | Zoeken in dit blad of in alle zichtbare bladen. Als er meer dan één cel is geselecteerd, wordt bij zoeken in één blad alleen in die cellen gezocht. |
| **Volgorde** | De cellen per rij doorlopen (van links naar rechts, dan omlaag) of per kolom (van boven naar beneden, dan naar rechts). **Volgende** stopt bij de eerste overeenkomst in die volgorde. |
| **Zoeken in** | **Formules** zoekt in wat in elke cel is getypt, **Waarden** in wat de cel weergeeft en **Notities** in de notities bij cellen. |
| **Hoofdlettergevoelig** | Hoofdletters en kleine letters moeten overeenkomen. |
| **Alleen hele cel** | De hele cel moet uit de tekst bestaan, niet alleen een deel ervan. |

In de zoektekst staat **?** voor één willekeurig teken en **\*** voor een willekeurig aantal tekens (ook geen). Typ **~** vóór ?, \* of ~ om naar het teken zelf te zoeken: **a~\*b** vindt a\*b.

Een zoekopdracht wordt stukje bij beetje uitgevoerd, zodat u kunt doorwerken. Met **Esc** (of **Annuleren**) stopt u de zoekopdracht; ook het wijzigen van een cel stopt die. Een heel lange tekst kost meer van elke kleine stap dan een korte tekst, zodat het venster ook bij een blad met lange teksten bruikbaar blijft.

Notities bij cellen volgen de cellen: na sorteren, of na het invoegen of verwijderen van rijen of kolommen, vindt een zoekopdracht in notities elke notitie bij de cel waar die nu staat. Een notitie bij een verwijderde cel wordt niet meer gevonden (en komt terug na Ongedaan maken). Als een werkmap op zoveel manieren is gewijzigd dat de notities niet meer door al die wijzigingen kunnen worden gevolgd, wordt u gevraagd de werkmap eerst op te slaan. Een werkmap die is geopend vanuit een ouder bestandstype of vanuit een tekstbestand, bevat notities in een vorm waarin JeffCrunch niet zoekt; u krijgt dan een melding.

## Vervangen

Druk op **Ctrl+H**. **Vervangen** zet de nieuwe tekst in de cel met de cursor als die de tekst bevat, en gaat dan door naar de volgende. **Alles vervangen** doet dit in elke cel met de tekst, als één stap die u ongedaan kunt maken; een grote wijziging wordt stukje bij beetje uitgevoerd. De nieuwe tekst wordt ingevoerd alsof u die zelf typt: een getal wordt een getal, en een gewijzigde formule blijft een formule.

Er wordt niets gewijzigd, en u krijgt te horen waarom, als een cel die zou worden gewijzigd vergrendeld is op een beveiligd blad, deel uitmaakt van een matrix of een gegevenstabel, in de eerste rij van een tabel staat, of geen formule meer zou zijn.

## Ga naar

Druk op **F5** (of **Ctrl+G**), of typ in het naamvak. U kunt een cel opgeven zoals B7, een bereik zoals B3:D9, hele kolommen of rijen (C:E, 4:9), een cel op een ander blad (Blad2!A1, of 'Mijn blad'!A1), of de naam van een bereik.

## Selecteren op soort

Kies **Selecteren op type…** in het menu **Zoeken** om cellen te selecteren op wat ze zijn:

- **Lege cellen**, **Constanten (ingetypt, niet berekend)**, **Formules** en **Cellen met een fout**. Bij constanten en formules kunt u getallen, tekst, WAAR en ONWAAR, of fouten weglaten.
- **Het blok rond de cel met de cursor** en **De laatste gebruikte cel**.
- **Zichtbare cellen** (niet in verborgen rijen of kolommen), **Cellen met voorwaardelijke opmaak** en **Cellen met een regel voor wat mag worden ingevoerd** (allemaal, of alleen de cellen met dezelfde regel als de cel met de cursor).

Het eerste gevonden blok wordt geselecteerd en eventuele andere blokken krijgen een tint; de tint verdwijnt wanneer u op een cel klikt.
