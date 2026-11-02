# Kruisverwijzingen
<!-- keywords: kruisverwijzing, verwijzing, kruisverwijzen, REF, PAGEREF, NOTEREF, zie pagina, zie afbeelding, zie figuur, boven, onder, kopnummer, koppeling, hyperlink -->

Een **kruisverwijzing** is tekst in uw document die naar een andere plaats verwijst, zoals "zie Afbeelding 2 op pagina 7". Het is een veld: als de afbeelding of de pagina verandert, werkt u het veld bij en past de tekst zich aan.

## Een kruisverwijzing invoegen

1. Klik op de plaats waar de tekst moet komen.
2. Klik op **Verwijzingen > Kruisverwijzing**.
3. Kies het **Type verwijzing**: een kop, een bladwijzer, een voetnoot, een eindnoot, een genummerd item of een bijschrift zoals Afbeelding of Tabel.
4. Kies bij **Verwijzing invoegen naar** wat u wilt invoegen: de tekst, het nummer, het paginanummer of "boven" en "onder".
5. Kies het item in de lijst, laat **Als hyperlink invoegen** ingeschakeld als u ernaartoe wilt kunnen springen en klik op **Invoegen**.

Als u bijvoorbeeld "Zie de kop Resultaten op pagina 4" wilt schrijven: typ `Zie de kop `, voeg een verwijzing naar de kop in met **Tekst van kop**, typ ` op pagina ` en voeg een tweede verwijzing naar dezelfde kop in met **Paginanummer**.

## Een verwijzing volgen

Houd **Ctrl** ingedrukt en klik op een verwijzing die een hyperlink is. De cursor gaat naar de plaats waarnaar wordt verwezen.

## De tekst actueel houden

Nadat u pagina's hebt toegevoegd, koppen hebt gewijzigd of afbeeldingen hebt verplaatst, selecteert u de tekst en drukt u op **F9**. Alle verwijzingen berekenen hun tekst dan opnieuw. Als de plaats waarnaar een verwijzing wijst is verwijderd, toont het veld `Fout: de bladwijzer waarnaar wordt verwezen, ontbreekt.` en kunt u een nieuw doel kiezen.

## Wat er achter de schermen gebeurt

JeffWrite markeert het doel met een verborgen bladwijzer (de naam begint met `_Ref`) en maakt een veld dat naar die bladwijzer verwijst: `REF` voor tekst en nummers, `PAGEREF` voor paginanummers en `NOTEREF` voor noten. Als u het bestand in een ander programma opent, werken de verwijzingen nog steeds.
