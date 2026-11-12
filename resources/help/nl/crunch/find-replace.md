# Zoeken, vervangen en naar gaan

<!-- keywords: zoeken, vervangen, wijzigen, zoeken naar, vervangen door, alles vervangen, volgende zoeken, wijzigen in, alles wijzigen, dezelfde hoofdletters, jokerteken, sterretje, vraagteken, tilde, hoofdlettergevoelig, hele cel, naar gaan, verwijzing, naam, selecteren op soort, lege cellen, constanten, formules, fouten, huidig gebied, huidige matrix, rijverschillen, kolomverschillen, laatste cel, zichtbare cellen, voorwaardelijke opmaak, validatie, Ctrl+F, Ctrl+H, F5, notities -->

## Zoeken

Druk op **Ctrl+F** (of kies **Zoeken** op het tabblad Start en kies daarna opnieuw **Zoeken**). Typ de tekst in **Zoeken naar** en druk op **Enter** voor de volgende cel die die tekst bevat, of kies **Alles weergeven** voor een lijst van alle cellen die overeenkomen. Klik op een regel van de lijst om naar die cel te gaan.

| Optie | Wat het doet |
| --- | --- |
| **Waar** | Zoek in dit blad, of in elk blad dat wordt weergegeven. Als meer dan één cel is geselecteerd, kijkt een zoekopdracht in één blad alleen in die cellen. |
| **Volgorde** | Bezoek de cellen per rij (van links naar rechts, dan omlaag) of per kolom (van boven naar beneden, dan naar rechts). **Volgende zoeken** stopt bij de eerste overeenkomst in die volgorde. |
| **Zoeken in** | **Formules** zoekt in wat in elke cel is getypt, **Waarden** in wat er wordt weergegeven, en **Notities** in de notities bij cellen. |
| **Hoofdlettergevoelig** | Hoofdletters en kleine letters moeten gelijk zijn. |
| **Alleen hele cel** | De hele cel moet de tekst zijn, niet alleen een deel ervan. |

In de tekst die u typt bij **Zoeken naar** staat **?** voor elk willekeurig teken en **\*** voor een willekeurig stuk tekst (ook geen tekst). Typ **~** voor ?, \* of ~ om het teken zelf te zoeken: **a~\*b** vindt a\*b.

Een zoekopdracht loopt stap voor stap, zodat u kunt doorwerken. **Esc** (of **Annuleren**) stopt het zoeken, en dat gebeurt ook wanneer u een cel wijzigt. Een zeer lange tekst kost bij elke stap meer tijd dan een korte tekst, dus een blad met veel lange teksten houdt het venster toch bruikbaar.

Notities bij cellen volgen de cellen: na een sortering, of nadat rijen of kolommen zijn ingevoegd of verwijderd, vindt een zoekopdracht in notities elke notitie bij de cel waarop hij nu staat. Een notitie bij een verwijderde cel is niet meer in de zoekopdracht (en komt terug na Ongedaan maken). Als een werkmap zo vaak is gewijzigd dat de notities niet door al die wijzigingen kunnen worden gevolgd, wordt u gevraagd het bestand eerst op te slaan. Een werkmap die is geopend uit een oudere bestandssoort of uit een tekstbestand heeft zijn notities in een vorm die JeffCrunch niet doorzoekt, en u krijgt daarvan een melding.

## Vervangen

Druk op **Ctrl+H**, typ de nieuwe tekst bij **Vervangen door** en kies een knop. **Vervangen** zet de nieuwe tekst in de cel met de cursor als die de tekst bevat, en gaat door naar de volgende. **Alles vervangen** doet dit in elke cel die de tekst bevat, als één stap die u ongedaan kunt maken; een grote wijziging wordt stapsgewijs doorgevoerd. De nieuwe tekst wordt ingevoerd alsof u die typt: een getal wordt een getal, en een formule die wordt gewijzigd blijft een formule.

Er wordt niets gewijzigd, en u krijgt te horen waarom, als een cel die zou worden gewijzigd vergrendeld is op een beveiligd blad, deel is van een matrix of een gegevenstabel, in de eerste rij van een tabel staat, of geen formule meer zou zijn.

## Naar gaan

Druk op **F5** (of **Ctrl+G**), of typ in het naamvak. U kunt een cel opgeven, zoals B7, een bereik, zoals B3:D9, hele kolommen of rijen (C:E, 4:9), een cel van een ander blad (Blad2!A1, of 'Mijn blad'!A1), of de naam van een bereik.

## Selecteren op soort

Kies **Selecteren op soort** in het menu **Zoeken** om cellen te selecteren op wat ze zijn:

- **Lege cellen**, **Constanten** (ingetypt), **Formules** en **Cellen met een fout**. Voor constanten en formules kunt u getallen, tekst, WAAR en ONWAAR of fouten uitsluiten.
- **Het blok rond de cel met de cursor** en **De laatste cel die wordt gebruikt**.
- **De matrixformule waarin de cursor staat** selecteert alle cellen van die matrixformule.
- **Cellen die in hun rij** en **in hun kolom** afwijken van de cel met de cursor zoeken de cellen die niet gelijk zijn aan de cel met de cursor. Selecteer eerst een blok en zet de cursor op de cel waarmee de andere moeten overeenkomen. In elke rij (of kolom) van het blok wordt een cel gekozen als die een ander getal of andere tekst bevat, of een formule die niet hetzelfde is wanneer de celnamen worden verschoven naar de positie van die cel. Een cel met een formule wordt ook gekozen als de cel met de cursor geen formule heeft. Als u maar één cel selecteert, kijkt JeffCrunch naar het blok eromheen.
- **Cellen die worden weergegeven** (niet in verborgen rijen of kolommen), **Cellen met voorwaardelijke opmaak** en **Cellen met een regel voor wat er mag worden ingevoerd** (allemaal, of alleen die met dezelfde regel als de cel met de cursor).

Het eerste gevonden blok wordt geselecteerd en eventuele andere worden gekleurd; de kleur verdwijnt wanneer u op een cel klikt.
