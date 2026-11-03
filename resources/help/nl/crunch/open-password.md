# Werkmappen met een wachtwoord openen
<!-- keywords: wachtwoord, beveiligd, versleuteld, versleuteling, encryptie, openen, vergrendeld, verkeerd wachtwoord, onjuist wachtwoord, annuleren, xlsx, xls, ods, bestandsbeveiliging, standaardwachtwoord, privé -->

Sommige werkmappen worden bij het opslaan met een wachtwoord beveiligd: zonder dat wachtwoord kan geen enkel programma lezen wat er in het bestand staat. Dit is iets anders dan **Blad beveiligen** en **Werkmap beveiligen** (zie [Een blad of de werkmap beveiligen](protect)), die alleen wijzigingen tegenhouden. Een werkmap die met een wachtwoord is beveiligd, kan pas worden geopend als het wachtwoord is getypt. Op deze pagina leest u hoe u zo'n werkmap opent en hoe u er een maakt.

## Een beveiligde werkmap openen

Open het bestand zoals u elk ander bestand opent: druk op **{key:Open}**, kies het bestand of sleep het naar het venster. JeffCrunch ziet dat het bestand beveiligd is en vraagt om het wachtwoord voordat het iets leest:

1. Typ het wachtwoord in het vak bij **Dit bestand is beveiligd met een wachtwoord.** De tekens worden als stippen weergegeven.
2. Klik op **Openen**.

Als het wachtwoord niet juist is, meldt JeffCrunch dat en vraagt het opnieuw, zo vaak als u wilt. Klik op **Annuleren** om te stoppen: er wordt niets geopend en de werkmappen die u al open hebt, blijven zoals ze zijn. Wachtwoorden moeten exact kloppen: hoofdletters, spaties en accenten tellen allemaal mee.

JeffCrunch slaat het wachtwoord nergens op. Het programma bewaart het in het geheugen zolang de werkmap open is, zodat **Opslaan** het bestand opnieuw kan beveiligen, en wist het wanneer u de werkmap sluit. De volgende keer dat u het bestand opent, wordt opnieuw om het wachtwoord gevraagd.

## Wat kan worden geopend

- **.xlsx- en .xlsm-werkmappen** die zijn beveiligd op een van de twee manieren die het oorspronkelijke programma sinds 2007 gebruikt: de nieuwere manier (de sleutel is 128, 192 of 256 bits, en JeffCrunch controleert ook of niemand het bestand na het beveiligen heeft gewijzigd) en de oudere manier (AES met een sleutel van 128, 192 of 256 bits).
- **.xls-werkmappen** die zijn beveiligd op een van de drie manieren van die indeling: de eenvoudige versleuteling van de oudste bestanden, de 40-bits variant en de sterkere variant die de sleutelservice van de computer zelf gebruikt.
- **.ods-spreadsheets** die met een wachtwoord zijn beveiligd, op alle drie de manieren waarop LibreOffice en OpenOffice ze hebben geschreven: de nieuwste (LibreOffice 25 en later vergrendelen het hele bestand met een 256-bits sleutel en leiden de sleutel af van het wachtwoord met een methode die bewust traag is en veel geheugen gebruikt, waardoor het openen van zo'n bestand iets langer duurt), de 256-bits variant van de versies daarvoor en de oudste variant.

Oudere .xls-bestanden stellen beperkingen aan het wachtwoord zelf. De eenvoudige versleuteling van de oudste bestanden gebruikt alleen de eerste 15 tekens van een wachtwoord en kan alleen letters, cijfers en tekens uit de West-Europese tekenset (Latin-1) bevatten: een wachtwoord met andere letters (zoals Griekse of Chinese) wordt daar nooit als juist herkend. De 40-bits variant accepteert alle letters, maar alleen de eerste 16 tekens. JeffCrunch schrijft geen .xls-bestanden met een wachtwoord.

Sommige bestanden zijn vergrendeld met een wachtwoord dat iedereen kent, zodat iedereen ze kan openen maar niemand ze per ongeluk wijzigt. JeffCrunch opent die zonder te vragen.

## Als een bestand niet kan worden geopend

- **"Het wachtwoord is onjuist."** Het wachtwoord dat u hebt getypt, is niet het wachtwoord waarmee het bestand is beveiligd. Probeer het opnieuw en let op hoofdletters en spaties; JeffCrunch kan een vergeten wachtwoord niet herstellen.
- **"Dit bestand is beveiligd op een manier die dit programma niet kan openen."** Het bestand is vergrendeld met iets anders dan een wachtwoord (bijvoorbeeld een sleutel uit een certificaat), of in een vorm die JeffCrunch niet leest.
- **"De wachtwoordbeveiliging van dit bestand vraagt meer geheugen of tijd dan dit programma toestaat, dus wordt het niet geopend."** Een bestand uit LibreOffice 25 of later bepaalt hoeveel geheugen en tijd het controleren van het wachtwoord kost. JeffCrunch accepteert tot tien keer wat LibreOffice zelf instelt (640 MiB geheugen), zodat een bestand dat is gemaakt om uw computer lam te leggen dat niet kan. Een bestand dat LibreOffice zelf heeft geschreven, blijft daar ruim binnen.
- **"Het bestand is beschadigd of is gewijzigd nadat het is beveiligd."** Het wachtwoord was juist, maar de inhoud van het bestand is niet wat werd vergrendeld: het is afgekapt, beschadigd of door een ander programma gewijzigd. Kopieer het opnieuw vanaf het origineel.

## Opslaan met een wachtwoord

Klik op **Bestand** en dan op **Beveiligen met wachtwoord** (of **Controleren > Beveiligen met wachtwoord**), typ het wachtwoord twee keer en klik op **Beveiligen**. De werkmap wordt beveiligd weggeschreven wanneer u deze de volgende keer opslaat: iedereen die het bestand opent, moet het wachtwoord typen. Wilt u het wachtwoord wijzigen, kies dan dezelfde opdracht en typ een nieuw wachtwoord; wilt u het verwijderen, klik dan op **Wachtwoord verwijderen** en sla op.

Een werkmap die u met een wachtwoord hebt geopend, houdt dat wachtwoord zolang deze open is, zodat **Opslaan** het bestand opnieuw met hetzelfde wachtwoord beveiligt. JeffCrunch bewaart het wachtwoord alleen in het geheugen tot u de werkmap sluit en wist het daarna. Het bestand dat u met het wachtwoord hebt geopend, blijft niet per ongeluk onbeveiligd achter: als u een kopie opslaat als .xls- of .ods-bestand, die JeffCrunch niet kan beveiligen, meldt het programma eerst dat de kopie niet beveiligd wordt, en kunt u annuleren.

Gebruik een lang wachtwoord en bewaar het op een veilige plek: niemand, ook wij niet, kan het bestand zonder dat wachtwoord openen.
