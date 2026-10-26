# Rijen, kolommen en cellen invoegen en verwijderen
<!-- keywords: invoegen, verwijderen, wissen, rij, rijen, kolom, kolommen, cel, cellen, verschuiven, verplaatsen, toevoegen, leeg, lege rij, Ctrl+plus, Ctrl+min, rechtsklikken, koprij, ongedaan maken, verwijzing, #VERW!, #REF!, formule, naam, samengevoegd, tabel, matrix, rand -->

U kunt lege rijen, kolommen of cellen midden in uw gegevens toevoegen, en rijen, kolommen of cellen eruit halen. Alles wat erna komt, schuift op om ruimte te maken of om het gat te dichten, en formules die naar de verplaatste cellen verwijzen, blijven ernaar verwijzen.

## Invoegen

1. Selecteer de rijnummers, de kolomletters of de cellen waar u de nieuwe wilt hebben. Wilt u drie rijen toevoegen, selecteer dan drie rijen.
2. Druk op **Ctrl en de plustoets** (**Ctrl en de mintoets** verwijdert), of gebruik **Start > Invoegen**, of klik met de rechtermuisknop op de selectie en kies **Invoegen**.

Als u hele rijen of kolommen hebt geselecteerd, worden de nieuwe direct ingevoegd, boven de rijen of links van de kolommen. Als u cellen hebt geselecteerd, vraagt een klein venster hoe de cellen erna moeten verschuiven:

- **Cellen naar rechts verschuiven** of **Cellen naar beneden verschuiven** verschuift alleen de cellen in de kolommen (of rijen) van uw selectie en laat de rest van het blad op zijn plaats.
- **Hele rij** of **Hele kolom** voegt hele rijen of kolommen toe.

## Verwijderen

Selecteer wat u wilt verwijderen en druk op **Ctrl en de mintoets**, of gebruik **Start > Verwijderen**. Als u cellen hebt geselecteerd, vraagt het venster of u **cellen naar links wilt verschuiven**, **cellen naar boven wilt verschuiven** of de **hele rij** of **hele kolom** wilt verwijderen. Verwijderen is één stap, en **Ongedaan maken** zet alles terug wat is verwijderd, ook de formules.

## Wat er gebeurt

Een formule die verwijst naar cellen die verschuiven, verwijst daarna naar hun nieuwe plaats. Voorbeeld: in B1 staat **=SOM(A1:A5)**. Voegt u boven rij 3 een rij in, dan wordt de formule **=SOM(A1:A6)**, omdat het bereik is gegroeid met de nieuwe rij. Verwijdert u rij 3, dan wordt het **=SOM(A1:A4)**. Een formule die verwees naar een cel die u hebt verwijderd, geeft **#VERW!** weer. Dat betekent dat de cel die de formule nodig heeft, weg is. Met **Ongedaan maken** komen de cel en de formule terug.

Ook dit verschuift mee met de cellen: formules op alle bladen, namen (zoals afdrukbereiken en afdruktitels), samengevoegde cellen, de cellen waarvoor regels voor gegevensvalidatie en voorwaardelijke opmaak gelden, tabellen, grafieken, afbeeldingen, notities, hyperlinks en pagina-einden.

## Wat niet kan

- Cellen kunnen niet worden ingevoegd of verwijderd in een deel van een **matrixformule** of een gegevenstabel. Verplaats ze in hun geheel of laat ze ongemoeid. Een cel die een gegevenstabel als invoer gebruikt, kan ook niet worden verwijderd.
- Een **draaitabel** kan nog niet worden verplaatst of doorsneden: rijen, kolommen of cellen kunnen niet worden ingevoegd of verwijderd als de draaitabel daardoor zou verschuiven of veranderen, en cellen kunnen er niet uit worden geknipt of erop worden geplakt. De rest van het blad kunt u gewoon wijzigen.
- Een **tabel** kan niet worden doorsneden: u kunt er geen kolom in invoegen en u kunt de koprij of alle rijen niet verwijderen. Voeg rijen in de tabel in of verwijder ze, of wijzig de cellen eromheen.
- Op een **beveiligd blad** kunnen geen rijen, kolommen of cellen worden ingevoegd of verwijderd. Kies daarvoor op het tabblad Controleren **Beveiliging blad opheffen**.
- Cellen aan het einde van het blad waar iets in staat, kunnen niet over de rand worden geduwd. Verwijder ze eerst.
- Na het opslaan kunt u met **Ongedaan maken** nog steeds teruggaan naar wat u daarvoor hebt gedaan, ook ingevoegde rijen en kolommen (bij de volgende keer opslaan zijn die er dan weer uit). Het enige wat niet terug kan, zijn rijen, kolommen of cellen die u hebt **verwijderd**: in het opgeslagen bestand is hun inhoud verloren, dus **Ongedaan maken** stopt bij de laatste verwijdering die u hebt opgeslagen. De regel onder in het venster meldt dit wanneer u opslaat.
- Een stap die zoveel cellen verplaatst of verwijdert dat **Ongedaan maken** hem niet kan bewaren (meer dan ongeveer 128 MB), vraagt eerst om bevestiging, zoals bekende spreadsheetprogramma's ook doen: "Dit kan niet ongedaan worden gemaakt." Gaat u door, dan wordt de stap uitgevoerd en kan **Ongedaan maken** deze stap en alles daarvoor niet meer terugdraaien.
