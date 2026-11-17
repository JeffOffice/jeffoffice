# Werkmappen met wachtwoord openen

<!-- keywords: wachtwoord, beveiligd, versleuteld, versleuteling, openen, vergrendeld, fout wachtwoord, annuleren, xlsx, xls, ods, bestandsbeveiliging, standaardwachtwoord, privé -->

Sommige werkmappen worden bij het opslaan met een wachtwoord beveiligd: zonder het wachtwoord kan geen enkel programma lezen wat in het bestand staat. Dit verschilt van **Blad beveiligen** en **Werkmap beveiligen** (zie [Een blad of de werkmap beveiligen](protect)), die alleen wijzigingen tegenhouden. Een werkmap die met een wachtwoord is beveiligd, kan pas worden geopend als het wachtwoord is ingetypt. Op deze pagina leest u hoe u er een opent en hoe u er een maakt.

## Een werkmap openen

Open het bestand zoals u elk ander bestand opent: druk op **{key:Open}**, kies het bestand, of sleep het naar het venster. JeffCrunch ziet dat het bestand beveiligd is en vraagt om het wachtwoord voordat het iets leest:

1. Typ het wachtwoord in het vak met de tekst **Dit bestand is beveiligd met een wachtwoord.** De letters worden als stippen getoond.
2. Klik op **Openen**.

Als het wachtwoord niet juist is, meldt JeffCrunch dat en vraagt het opnieuw, zo vaak als u wilt. Klik op **Annuleren** om te stoppen: er wordt niets geopend en de werkmappen die u al open hebt, blijven zoals ze zijn. Wachtwoorden moeten exact kloppen: hoofdletters, spaties en accenten tellen mee.

JeffCrunch slaat het wachtwoord nergens op. Het bewaart het alleen in het geheugen zolang de werkmap open is, zodat **Opslaan** het bestand opnieuw kan beveiligen, en wist het wanneer u de werkmap sluit. De volgende keer dat u het bestand opent, wordt opnieuw om het wachtwoord gevraagd.

## Wat kan er worden geopend

- **.xlsx- en .xlsm-werkmappen** die beveiligd zijn op een van de twee manieren die het oorspronkelijke programma sinds 2007 gebruikt: de nieuwere (de sleutel is 128, 192 of 256 bits, en JeffCrunch controleert ook of het bestand na het beveiligen niet is gewijzigd) en de oudere (AES met een sleutel van 128, 192 of 256 bits).
- **.xls-werkmappen** die beveiligd zijn op een van de drie manieren die dit formaat kent: de eenvoudige versleuteling van de oudste bestanden, de versleuteling met 40 bits en de sterkere versie die de sleutelservice van de computer gebruikt.
- **.ods-spreadsheets** die met een wachtwoord zijn beveiligd, op alle drie de manieren waarop LibreOffice en OpenOffice ze hebben opgeslagen: de nieuwste (LibreOffice 25 en later vergrendelen het hele bestand met een sleutel van 256 bits en maken de sleutel uit het wachtwoord met een methode die bewust traag is en veel geheugen gebruikt, dus het openen van zo'n bestand duurt even langer), de versie met 256 bits van de versies daarvoor, en de oudste versie.

Oudere .xls-bestanden hebben beperkingen voor het wachtwoord zelf. De eenvoudige versleuteling van de oudste bestanden gebruikt alleen de eerste 15 tekens van een wachtwoord en kan alleen letters, cijfers en tekens uit de West-Europese (Latin-1) tekenset bevatten: een wachtwoord met andere letters (zoals Grieks of Chinees) wordt daar nooit als juist herkend. De versleuteling met 40 bits accepteert alle letters, maar gebruikt alleen de eerste 16 tekens. JeffCrunch schrijft een .xls-kopie met wachtwoord in de sterkere versie (die de sleutelservice van de computer gebruikt, met een sleutel van 128 bits), omdat de andere twee zwakker zijn. Die versie gebruikt de eerste 255 tekens van het wachtwoord, die uit alle letters kunnen bestaan.

Sommige bestanden zijn vergrendeld met een wachtwoord dat iedereen kent, zodat ze door iedereen kunnen worden geopend maar niet per ongeluk worden gewijzigd. JeffCrunch opent die zonder te vragen.

## Wanneer een bestand niet kan worden geopend

- **"Het wachtwoord is onjuist."** Het ingetypte wachtwoord is niet het wachtwoord waarmee het bestand is beveiligd. Probeer het opnieuw en let op hoofdletters en spaties; JeffCrunch kan een kwijtgeraakt wachtwoord niet herstellen.
- **"Dit bestand is beveiligd op een manier die dit programma niet kan openen."** Het bestand is vergrendeld door iets anders dan een wachtwoord (bijvoorbeeld een sleutel uit een certificaat), of in een vorm die JeffCrunch niet leest.
- **"De wachtwoordbeveiliging van dit bestand vraagt om meer geheugen of tijd dan dit programma toestaat, daarom wordt het niet geopend."** Een bestand van LibreOffice 25 of later bepaalt hoeveel geheugen en tijd het controleren van het wachtwoord kost. JeffCrunch accepteert maximaal tien keer wat LibreOffice zelf instelt (640 MiB geheugen), zodat een bestand dat uw computer vastzet, niet kan worden geopend. Een bestand dat LibreOffice zelf heeft geschreven, valt ruim binnen die grens.
- **"Het bestand is beschadigd of na het beveiligen gewijzigd."** Het wachtwoord was juist, maar de inhoud van het bestand klopt niet met wat is vergrendeld: het is afgekapt, beschadigd of door een ander programma gewijzigd. Kopieer het opnieuw vanaf het origineel.

## Opslaan met een wachtwoord

Klik op **Bestand** en daarna op **Beveiligen met wachtwoord** (of **Controleren > Beveiligen met wachtwoord**), typ het wachtwoord twee keer en klik op **Beveiligen**. De werkmap wordt bij de volgende keer opslaan beveiligd geschreven: wie het bestand opent, moet het wachtwoord typen. Wilt u het wachtwoord wijzigen, kies dan dezelfde opdracht en typ een nieuw wachtwoord. Wilt u het verwijderen, klik dan op **Wachtwoord verwijderen** en sla op.

Een werkmap die u met een wachtwoord hebt geopend, behoudt het wachtwoord zolang de werkmap open is, zodat **Opslaan** het bestand weer met hetzelfde wachtwoord beveiligt. JeffCrunch houdt het wachtwoord alleen in het geheugen tot u de werkmap sluit, en wist het dan. Het bestand dat u met het wachtwoord hebt geopend, blijft niet per ongeluk onbeveiligd: een kopie die u als .xls-bestand opslaat, wordt met hetzelfde wachtwoord beveiligd op de sterkere .xls-manier, en een kopie die u als .ods- of .ots-bestand opslaat, wordt op de manier beveiligd die de OpenDocument-standaard beschrijft, zodat andere programmas die die standaarden volgen de kopie met het wachtwoord kunnen openen. Een tekstkopie kan niet worden beveiligd. JeffCrunch laat u daarom eerst weten dat de kopie niet beveiligd wordt, en u kunt dan annuleren.

Gebruik een lang wachtwoord en bewaar het op een veilige plek: niemand, ook wij niet, kan het bestand zonder wachtwoord openen.
