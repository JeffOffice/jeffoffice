# Een blad of de werkmap beveiligen

<!-- keywords: beveiligen, beveiliging, wachtwoord, vergrendelen, vergrendelde cellen, ontgrendelen, beveiliging opheffen, structuur, bladen, alleen-lezen, toestaan, cellen opmaken, rijen invoegen, sorteren, filteren, hash, bewerkingsbereiken toestaan, bewerkbaar bereik, wachtwoord voor bereik -->

## Een blad beveiligen

**Controleren > Blad beveiligen** voorkomt dat mensen per ongeluk iets in het blad wijzigen. In een beveiligd blad kunnen de cellen die **vergrendeld** zijn, niet worden gewijzigd. Elke cel is vergrendeld, tenzij u anders aangeeft: selecteer de cellen die mogen worden gewijzigd, open **Celeigenschappen > Beveiliging** en schakel **Vergrendeld** uit. (Dit heeft alleen effect zodra het blad is beveiligd.)

In het dialoogvenster staat wat iedereen nog wel mag doen op het blad: vergrendelde of ontgrendelde cellen selecteren, cellen opmaken, kolommen of rijen wijzigen, rijen en kolommen invoegen of verwijderen, koppelingen invoegen, sorteren, filters gebruiken, en tekenobjecten en notities wijzigen. Wat u niet aanvinkt, wordt geweigerd, met een melding waarin staat wat er is geweigerd.

Een **wachtwoord** is optioneel. Als u er een opgeeft, kan het blad alleen met dat wachtwoord worden ontgrendeld. JeffCrunch bewaart een versleutelde vorm van het wachtwoord (een hash met zout, 100.000 keer herhaald), en nooit het wachtwoord zelf. Daardoor kan het programma het wachtwoord later niet laten zien: bewaar het dus goed. Wachtwoorden die met andere programma's zijn gemaakt, in de oudere of de nieuwere vorm, worden op dezelfde manier gecontroleerd.

Beveiligen en het opheffen van de beveiliging zijn stappen die u ongedaan kunt maken. Om de beveiliging op te heffen, gebruikt u **Beveiliging blad opheffen** en typt u het wachtwoord, als er een is.

Een paar dingen zijn op een beveiligd blad altijd niet toegestaan, wat u ook toestaat: cellen samenvoegen, en rijen of kolommen verwijderen die vergrendelde cellen bevatten.

## Sommige cellen toch laten wijzigen: Bewerkingsbereiken toestaan

**Controleren > Bewerkingsbereiken toestaan** toont de **bereiken** die mensen mogen wijzigen, ook als het blad is beveiligd. Een bereik is een celblok, zoals A1:C5. Klik op **Nieuw** om er een toe te voegen. Geef het een **titel**, typ de cellen die het bereik omvat (gebruik een spatie tussen blokken) en eventueel een **wachtwoord**. Een bereik zonder wachtwoord kan door iedereen worden gewijzigd. Een bereik met een wachtwoord kan pas worden gewijzigd nadat iemand bij de eerste poging om een van de cellen te wijzigen het wachtwoord heeft ingetypt. Daarna blijft het bereik open totdat de werkmap wordt gesloten.

Met **Wijzigen** past u een bereik aan en met **Verwijderen** haalt u het weg. U kunt bereiken instellen voordat u het blad beveiligt. Ze worden opgeslagen met de werkmap, en de wachtwoorden worden bewaard in dezelfde versleutelde vorm als het wachtwoord van een blad. Bewerkingsbereiken toestaan is een stap die u ongedaan kunt maken. Wanneer u rijen, kolommen of cellen invoegt of verwijdert, verschuiven de bereiken mee met hun cellen, worden ze kleiner of groter, of verdwijnen ze als al hun cellen zijn verwijderd. Met Ongedaan maken haalt u ze terug.

## De werkmap beveiligen

**Controleren > Werkmap beveiligen** vergrendelt de **structuur**: er kan geen blad worden toegevoegd, verwijderd, hernoemd, verplaatst, verborgen of weergegeven. U kunt ook vragen dat de vensters blijven zoals ze zijn. Ook hier kan een wachtwoord worden ingesteld. Met **Beveiliging werkmap opheffen** neemt u de beveiliging weer weg.
