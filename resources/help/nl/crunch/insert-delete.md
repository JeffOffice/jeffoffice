# Rijen, kolommen en cellen invoegen en verwijderen
<!-- keywords: invoegen, verwijderen, weghalen, rij, rijen, kolom, kolommen, cel, cellen, verschuiven, verplaatsen, toevoegen, leeg, Ctrl+plus, Ctrl+min, rechtsklikken, koprij, ongedaan maken, verwijzing, #VERW!, #REF!, formule, naam, samengevoegd, tabel, matrix, rand -->

U kunt lege rijen, kolommen of cellen midden in uw gegevens toevoegen, en rijen, kolommen of cellen eruit halen. Alles wat erna komt, schuift op om ruimte te maken of om het gat te dichten, en formules die naar de verplaatste cellen verwijzen, blijven naar die cellen verwijzen.

## Invoegen

1. Selecteer de rijnummers, de kolomletters of de cellen waar u de nieuwe wilt hebben. Wilt u drie rijen toevoegen, selecteer dan drie rijen.
2. Druk op **Ctrl en de plustoets** (**Ctrl en de mintoets** verwijdert), of kies **Start > Invoegen**, of klik met de rechtermuisknop op de selectie en kies **Invoegen**.

Als hele rijen of kolommen zijn geselecteerd, worden de nieuwe direct ingevoegd: boven de rijen of links van de kolommen. Als cellen zijn geselecteerd, vraagt een klein venster hoe de cellen erna moeten verschuiven:

- **Cellen naar rechts verschuiven** of **Cellen naar beneden verschuiven** verschuift alleen de cellen in de kolommen (of rijen) van uw selectie en laat de rest van het blad staan.
- **Hele rij** of **Hele kolom** voegt hele rijen of kolommen toe.

## Verwijderen

Selecteer wat u wilt weghalen en druk op **Ctrl en de mintoets**, of kies **Start > Verwijderen**. Als cellen zijn geselecteerd, vraagt het venster of u **cellen naar links wilt verschuiven**, **cellen naar boven wilt verschuiven** of de **hele rij** of **hele kolom** wilt verwijderen. Verwijderen is één stap, en **Ongedaan maken** zet alles terug wat is weggehaald, ook de formules.

## Wat er dan gebeurt

Een formule die verwijst naar cellen die verschuiven, verwijst naar hun nieuwe plaats. Voorbeeld: in B1 staat **=SOM(A1:A5)**. Voegt u boven rij 3 een rij in, dan wordt de formule **=SOM(A1:A6)**, omdat het bereik is gegroeid om de nieuwe rij te bevatten. Verwijdert u rij 3, dan wordt het **=SOM(A1:A4)**. Een formule die verwees naar een cel die u hebt verwijderd, geeft **#VERW!** weer: de cel die de formule nodig heeft, bestaat niet meer. Met **Ongedaan maken** komen de cel en de formule terug.

Ook het volgende verschuift mee met de cellen: formules op alle bladen, namen (zoals afdrukbereiken en afdruktitels), samengevoegde cellen, de cellen waarvoor regels voor gegevensvalidatie en voorwaardelijke opmaak gelden, tabellen, grafieken, afbeeldingen, notities, hyperlinks en pagina-einden.

## Wat niet kan

- U kunt geen cellen invoegen of verwijderen in een deel van een **matrixformule** of een gegevenstabel. Verplaats ze in hun geheel, of laat ze met rust. Een cel die een gegevenstabel als invoer leest, kan ook niet worden verwijderd.
- Een **draaitabel** kan nog niet worden verplaatst of doorsneden: u kunt geen rijen, kolommen of cellen invoegen of verwijderen waar dat de draaitabel zou verplaatsen of wijzigen, en u kunt geen cellen eruit knippen of erin plakken. De rest van het blad kunt u gewoon wijzigen. Een draaitabel wijzigt u met de opdrachten op het tabblad **Draaitabel** (**Vernieuwen**, **Andere gegevensbron**, velden en indeling), niet door in de cellen te typen.
- Een **tabel** kan niet worden doorsneden: u kunt er geen kolom in invoegen, en u kunt de koprij of alle rijen ervan niet verwijderen. Voeg rijen in de tabel in of verwijder ze, of wijzig de cellen eromheen.
- Op een **beveiligd blad** kunt u geen rijen, kolommen of cellen invoegen of verwijderen. Kies daarvoor eerst **Beveiliging blad opheffen** op het tabblad **Controleren**.
- Cellen aan het einde van het blad die iets bevatten, kunnen niet over de rand worden geschoven. Verwijder ze eerst.
- Na het opslaan kunt u met **Ongedaan maken** nog steeds teruggaan in wat u eerder deed, ook ingevoegde rijen en kolommen (bij de volgende keer opslaan zijn die er dan weer uit). Het enige wat niet terug kan, zijn rijen, kolommen of cellen die u hebt **verwijderd**: het opgeslagen bestand bevat niet meer wat erin stond. **Ongedaan maken** stopt daarom bij de laatste verwijdering die u hebt opgeslagen, en de regel onder in het venster meldt dat wanneer u opslaat.
- Een stap die zoveel cellen verplaatst of verwijdert dat **Ongedaan maken** hem niet kan bewaren (meer dan ongeveer 128 MB), vraagt eerst om bevestiging, zoals bekende spreadsheetprogramma's ook doen: "Dit kan niet ongedaan worden gemaakt." Gaat u door, dan wordt de stap uitgevoerd en kunt u die stap en alles daarvoor niet meer ongedaan maken.
