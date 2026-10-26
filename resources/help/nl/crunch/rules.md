# Regels voor wat een cel mag bevatten
<!-- keywords: validatie, regel, gegevensvalidatie, lijst, vervolgkeuzelijst, keuzelijst, geheel getal, decimaal, datum, tijd, lengte, aangepast, stoppen, waarschuwing, informatie, foutmelding, niet toegestaan, keuzes, pijl, Alt+Pijl-omlaag -->

Een werkmap kan **regels** bevatten die bepalen wat er in bepaalde cellen mag staan: een geheel getal van 1 tot en met 10, een datum na een bepaalde dag, een tekst van maximaal vijf letters of één keuze uit een lijst. De regels komen uit de werkmap. JeffCrunch past ze toe wanneer u een waarde in een cel **typt** (of er een uit de lijst van de cel kiest). Waarden die u plakt, doorvoert of verplaatst, worden niet gecontroleerd, net als in bekende spreadsheetprogramma's. Een regel bepaalt dus wat er getypt mag worden, niet wat de cel bevat.

## Als een waarde niet voldoet

Elke regel heeft een **stijl**. Als de werkmap een eigen bericht voor de regel bevat, wordt dat bericht weergegeven:

- **Stoppen**: het bericht verschijnt en de invoer blijft in de cel staan, zodat u deze kunt wijzigen. Druk op **Esc** om de invoer te annuleren.
- **Waarschuwing**: u wordt gevraagd of u de waarde toch wilt behouden.
- **Informatie**: u krijgt te zien dat de waarde niet voldoet en u kunt doorgaan of annuleren.

Een waarde die u plakt, wordt niet aan de regel getoetst: deze wordt ongewijzigd ingevoegd en er wordt niets gevraagd. Een cel wissen is altijd toegestaan.

## Kiezen uit een lijst

Een cel met een lijst geeft aan de rechterrand een kleine knop met een pijl weer wanneer de cursor in de cel staat. Klik op de knop of druk op **Alt+Pijl-omlaag** om de keuzes weer te geven en klik op een keuze om deze in te voeren.

## Wat nog niet mogelijk is

JeffCrunch behoudt de regels van de werkmap, maar heeft nog geen opdracht om nieuwe regels te maken. Regels die hun keuzes op de nieuwere schrijfwijze uit een ander blad lezen, blijven in het bestand bewaard maar worden niet toegepast. De berichten die verschijnen wanneer u een cel selecteert, worden niet weergegeven.
