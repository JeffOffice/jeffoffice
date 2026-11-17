# Werkmappen met wachtwoord openen

<!-- keywords: wachtwoord, beveiligd, versleuteld, versleuteling, openen, vergrendeld, verkeerd wachtwoord, annuleren, xlsx, xls, ods, bestandsbeveiliging, standaardwachtwoord, privé -->

Sommige werkmappen worden bij het opslaan met een wachtwoord beveiligd: zonder wachtwoord kan geen enkel programma lezen wat er in het bestand staat. Dit verschilt van **Blad beveiligen** en **Werkmap beveiligen** (zie [Een blad of de werkmap beveiligen](protect)), die alleen wijzigingen tegengaan. Een werkmap die met een wachtwoord is beveiligd, kan pas worden geopend als het wachtwoord is ingetypt. Op deze pagina leest u hoe u er een opent en hoe u er een maakt.

## Een werkmap openen

Open het bestand zoals u elk ander bestand opent: druk op **{key:Open}**, kies het bestand, of sleep het naar het venster. JeffCrunch ziet dat het bestand beveiligd is en vraagt om het wachtwoord voordat er iets wordt gelezen:

1. Typ het wachtwoord in het vak met de tekst **Dit bestand is beveiligd met een wachtwoord.** De tekens worden als stippen weergegeven.
2. Klik op **Openen**.

Als het wachtwoord niet juist is, meldt JeffCrunch dit en vraagt het opnieuw, zo vaak als nodig. Klik op **Annuleren** om te stoppen: er wordt niets geopend en de werkmappen die u al open hebt, blijven zoals ze zijn. Wachtwoorden moeten exact kloppen: hoofdletters, spaties en accenten tellen mee.

JeffCrunch slaat het wachtwoord nergens op. Het bewaart het alleen in het geheugen zolang de werkmap open is, zodat **Opslaan** het bestand weer kan beveiligen, en wist het wanneer u de werkmap sluit. De volgende keer dat u het bestand opent, wordt er opnieuw om het wachtwoord gevraagd.

## Wat kan worden geopend

- **.xlsx- en .xlsm-werkmappen** die beveiligd zijn op een van de twee manieren die het oorspronkelijke programma sinds 2007 gebruikt: de nieuwere (de sleutel is 128, 192 of 256 bits, en JeffCrunch controleert ook of het bestand niet is gewijzigd nadat het is beveiligd) en de oudere (AES met een sleutel van 128, 192 of 256 bits).
- **.xls-werkmappen** die beveiligd zijn op een van de drie manieren van dat formaat: de eenvoudige versleuteling van de oudste bestanden, de versleuteling van 40 bits, en de sterkere versleuteling die gebruikmaakt van de sleutelservice van de computer.
- **.ods-spreadsheets** die met een wachtwoord zijn beveiligd, op alle drie de manieren waarop LibreOffice en OpenOffice ze hebben opgeslagen: de nieuwste (LibreOffice 25 en later vergrendelen het hele bestand met een sleutel van 256 bits en maken de sleutel uit het wachtwoord met een methode die bewust traag is gemaakt en veel geheugen gebruikt, dus het openen van zo'n bestand duurt even langer), de variant met 256 bits van de versies daarvoor, en de oudste variant.

Oudere .xls-bestanden hebben beperkingen voor het wachtwoord zelf. De eenvoudige versleuteling van de oudste bestanden gebruikt alleen de eerste 15 tekens van een wachtwoord en kan alleen letters, cijfers en tekens uit de West-Europese (Latin-1) tekenset bevatten: een wachtwoord met andere letters (zoals Grieks of Chinees) wordt daar nooit als juist herkend. De versleuteling van 40 bits accepteert alle letters, maar alleen de eerste 16 tekens. JeffCrunch schrijft geen .xls-bestanden met wachtwoord.

Sommige bestanden zijn vergrendeld met een wachtwoord dat iedereen kent, zodat ze door iedereen kunnen worden geopend maar niet per ongeluk kunnen worden gewijzigd. JeffCrunch opent die zonder te vragen.

## Wanneer een bestand niet kan worden geopend

- **"Het wachtwoord is onjuist."** Het wachtwoord dat u hebt ingetypt, is niet het wachtwoord waarmee het bestand is beveiligd. Probeer het opnieuw en let op hoofdletters en spaties; JeffCrunch kan een verloren wachtwoord niet herstellen.
- **"Dit bestand is beveiligd op een manier die dit programma niet kan openen."** Het bestand is vergrendeld door iets anders dan een wachtwoord (bijvoorbeeld een sleutel uit een certificaat), of in een vorm die JeffCrunch niet leest.
- **"De wachtwoordbeveiliging van dit bestand vraagt om meer geheugen of tijd dan dit programma toestaat, daarom wordt het niet geopend."** Een bestand van LibreOffice 25 of later bepaalt hoeveel geheugen en tijd het controleren van het wachtwoord kost. JeffCrunch accepteert maximaal tien keer wat LibreOffice zelf instelt (640 MiB geheugen), zodat een bestand dat uw computer lam kan leggen, niet kan worden geopend. Een bestand dat LibreOffice zelf heeft geschreven, zit ruim binnen die grens.
- **"Het bestand is beschadigd of is gewijzigd nadat het was beveiligd."** Het wachtwoord was juist, maar de inhoud van het bestand is niet wat er is vergrendeld: het is afgekapt, beschadigd of door een ander programma gewijzigd. Kopieer het opnieuw vanaf het origineel.

## Opslaan met een wachtwoord

Klik op **Bestand** en vervolgens op **Beveiligen met wachtwoord** (of **Controleren > Beveiligen met wachtwoord**), typ het wachtwoord twee keer en klik op **Beveiligen**. De werkmap wordt de volgende keer dat u hem opslaat beveiligd geschreven: wie het bestand opent, moet het wachtwoord typen. Om het wachtwoord te wijzigen, kiest u dezelfde opdracht en typt u een nieuw wachtwoord; om het te verwijderen, klikt u op **Wachtwoord verwijderen** en slaat u op.

Een werkmap die u met een wachtwoord hebt geopend, behoudt dat wachtwoord zolang hij open is, dus **Opslaan** schrijft het bestand weer beveiligd met hetzelfde wachtwoord. JeffCrunch houdt het wachtwoord alleen in het geheugen tot u de werkmap sluit, en wist het dan. Het bestand dat u met het wachtwoord hebt geopend, blijft niet per ongeluk onbeveiligd: een kopie die u opslaat als .ods- of .ots-bestand, wordt met hetzelfde wachtwoord beveiligd, op de manier die de OpenDocument-standaard beschrijft, zodat andere programma's die de standaard volgen het met het wachtwoord kunnen openen. Een .xls-kopie of een tekstkopie kan niet worden beveiligd; JeffCrunch laat u daarom vooraf weten dat de kopie niet beveiligd wordt, en u kunt dan annuleren.

Gebruik een lang wachtwoord en bewaar het op een veilige plek: niemand, ook wij niet, kan het bestand zonder wachtwoord openen.
