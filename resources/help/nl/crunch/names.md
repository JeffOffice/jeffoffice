# Namen voor cellen en waarden
<!-- keywords: naam, namen, gedefinieerde naam, nieuwe naam, benoemd bereik, geldigheidsbereik, opmerking, naam uit cellen, naam invoegen, namen toepassen, Ctrl+F3, verwijst naar, namen sorteren, foutnaam, waarschuwing-->

Een **naam** is een woord dat staat voor een cel, een celbereik, een getal of een formule, zodat =Prijs*Aantal kan worden geschreven in plaats van =B2*C2. Een naam hoort bij de hele werkmap of bij één werkblad. Een naam op een werkblad verbergt een naam van de werkmap die op dat werkblad hetzelfde is gespeld.

## De regels voor een naam

- Ze begint met een letter, een underscore of een backslash, en bevat daarna alleen letters, cijfers, underscores en punten. Er staan geen spaties in.
- Ze is maximaal 255 tekens lang.
- Het is geen celadres, zoals **A1** of **R2C3**.
- Geen andere naam van hetzelfde werkblad (of van de werkmap) is hetzelfde gespeld, met of zonder hoofdletters.

Terwijl u typt, geeft het dialoogvenster aan wat er niet klopt, en de knop **OK** blijft uitgeschakeld.

## Een naam maken

1. Selecteer de cellen en klik op **Nieuwe naam** op het tabblad **Formules**.
2. Typ de naam, kies waar die geldt en geef aan waarvoor die staat. De geselecteerde cellen worden voor u ingevuld.

Er is een snellere manier voor een naam van de hele werkmap. Selecteer de cellen, klik in het **Naamvak** (links van de formulebalk), typ de naam en druk op **Enter**. Als er nog geen naam met die spelling bestaat, wordt die gemaakt voor de geselecteerde cellen. Als die er wel is, gaat **Enter** naar de cellen ervan.

Met **Naam uit cellen** maakt u in één keer veel namen op basis van de labels rond een celbereik: met de koppen in de bovenste rij en de getallen eronder, noemt elke kop de cellen eronder. Kies waar de labels staan (bovenste rij, linker kolom, onderste rij of rechter kolom). Een label met spaties of tekens die niet in een naam mogen, krijgt underscores in plaats daarvan. Een label dat leeg is, dubbel voorkomt of al een naam van de werkmap is, wordt overgeslagen, en de statusbalk meldt hoeveel dat er zijn.

## Alle namen bekijken

Klik op **Namen** op het tabblad **Formules**, of druk op **Ctrl+F3**. De lijst toont elke naam, de waarde die ze nu heeft, waarvoor ze staat, waar ze geldt en de opmerking. Typ in het filter om alleen de passende namen te tonen. Met **Nieuw**, **Wijzigen** en **Verwijderen** maakt u een naam, wijzigt u er een of verwijdert u er een. Klik op een kop om de lijst op die kop te sorteren, en klik nogmaals om de volgorde om te draaien. Een naam die naar een fout verwijst, zoals een verwijderde plaats, heeft ernaast een klein waarschuwingsteken. Een werkmap met meer dan 20.000 namen toont de eerste 20.000.

Als u de naam van een naam wijzigt, wordt die in één stap aangepast in elke formule die haar gebruikt, op elk werkblad. Als u een naam verwijdert, tonen de formules die haar gebruikten **#NAAM?**. Elke wijziging kunt u ongedaan maken.

## Een naam in een formule zetten

Terwijl u een formule typt, verschijnen de passende namen in de lijst onder de cel. Op het tabblad **Formules** toont **Naam invoegen** de namen waaruit u kunt kiezen.

## Namen zetten in formules die al zijn geschreven

Stel dat u de naam Koers hebt gemaakt voor $A$1, terwijl al veel formules naar $A$1 verwijzen. Klik op **Namen toepassen** op het tabblad **Formules** om Koers op hun plaats te zetten. Selecteer eerst de cellen die moeten worden bekeken, of selecteer één cel om het hele werkblad te bekijken. Kies de namen die moeten worden gebruikt. Een verwijzing wordt alleen gewijzigd als die precies de cellen van de naam is, met of zonder dollartekens: A1:B2 wordt niet gewijzigd in een naam voor A1. Alleen namen voor één celbereik op een werkblad worden aangeboden, en alleen namen van de hele werkmap. De wijziging is één stap, dus met **Ongedaan maken** wordt die teruggedraaid.
