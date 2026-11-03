# Werken met bladen
<!-- keywords: blad, bladen, werkblad, werkbladen, tabblad, tabbladen, toevoegen, invoegen, nieuw blad, verwijderen, naam wijzigen, hernoemen, naam, verplaatsen, kopiëren, dupliceren, volgorde wijzigen, slepen, verbergen, zichtbaar maken, weergeven, tabkleur, kleur, meerdere bladen selecteren, Shift+F11, #VERW!, 3D-verwijzing, Verplaatsen of kopiëren -->

Een werkmap bestaat uit **bladen**. De **tabbladen** onder in het venster tonen hun namen. Klik op een tabblad om dat blad weer te geven. Elke wijziging hieronder is één stap die u ongedaan kunt maken, en wordt in de werkmap opgeslagen.

## Een blad toevoegen

- Klik op de knop **+** na het laatste tabblad om een blad aan het einde toe te voegen.
- Druk op **Shift+F11** of kies **Start > Invoegen > Blad invoegen** om een blad toe te voegen vóór het blad dat u bekijkt.

Het nieuwe blad is leeg, krijgt de eerstvolgende vrije naam (Blad4, Blad5 enzovoort) en wordt meteen geopend. Een werkmap kan maximaal 1.000 bladen hebben. Daarboven meldt JeffCrunch dit en voegt het geen blad toe, omdat het een bestand met meer bladen niet kan openen.

## De naam van een blad wijzigen

**Dubbelklik** op het tabblad, typ de nieuwe naam en druk op **Enter** (met **Esc** annuleert u). U kunt ook **Start > Opmaak > Naam van blad wijzigen** kiezen.

Een naam mag maximaal 31 tekens bevatten. Hij mag niet leeg zijn, mag geen van de tekens **: \ / ? * [ ]** bevatten, mag niet met een apostrof beginnen of eindigen en mag niet gelijk zijn aan de naam van een ander blad (hoofdletters tellen niet als verschil). De naam "History" is gereserveerd voor het programma. Als de naam wordt geweigerd, meldt JeffCrunch waarom en behoudt het blad zijn naam.

Elke formule die het blad noemt, volgt de nieuwe naam. Voorbeeld: **=Budget!B2** wordt **='Budget K1'!B2** als u de naam van Budget wijzigt in Budget K1. Namen die naar het blad verwijzen, de regels van voorwaardelijke opmaak en gegevensvalidatie, en grafieken volgen de nieuwe naam ook.

## Een blad verwijderen

Klik met de rechtermuisknop op het tabblad en kies **Blad verwijderen**, of kies **Start > Verwijderen > Blad verwijderen**. Als er iets op het blad staat, vraagt JeffCrunch eerst om bevestiging. Formules op andere bladen die het verwijderde blad lezen, geven **#VERW!** weer: wat ze lezen, bestaat niet meer. Met **Ongedaan maken** krijgt u het blad en die formules terug.

Een werkmap houdt altijd ten minste één zichtbaar blad.

## Een blad verplaatsen of kopiëren

- **Sleep** een tabblad naar een nieuwe plaats om het blad te verplaatsen.
- Of kies **Start > Opmaak > Blad verplaatsen of kopiëren**. Kies het blad waarvóór het gekozen blad moet komen (of **(na het laatste blad)**) en schakel **Kopiëren en de originelen laten staan** in om het blad te kopiëren in plaats van te verplaatsen. Een kopie krijgt een naam zoals **Budget (2)** en wordt geopend.

Formules blijven de bladen lezen die ze lazen, waar die bladen ook staan. Een kopie heeft dezelfde cellen, formules, opmaak en afmetingen als het blad waarvan ze komt. Een verwijzing zonder bladnaam (zoals **A1**) leest de eigen cel van de kopie; een verwijzing met een bladnaam (zoals **Blad1!A1**) blijft naar dat blad verwijzen, ook als dat het oorspronkelijke blad is.

Een kopie neemt ook de beveiliging, het filter, de koppelingen, de pagina-instelling en pagina-einden, de groepen rijen en kolommen en het uiterlijk van de gegevensbalken van het origineel over. Koppelingen volgen de rijen die u hebt ingevoegd of verwijderd: in het origineel vóór het kopiëren en in de kopie daarna. De afbeeldingen, grafieken, notities, draaitabellen, sparklines en de namen die alleen bij het origineel horen, worden niet overgenomen, omdat JeffCrunch die nog niet kopieert; JeffCrunch meldt dan welke het zijn. Een blad met tabellen wordt gekopieerd met kopieën van die tabellen (elk met een eigen naam, id en onderdeel, en de formules van de kopie verwijzen naar de eigen tabellen). Als u een blad verwijdert, worden ook de tabellen ervan verwijderd; een formule op een ander blad die naar zo'n tabel verwees, geeft dan #VERW!, en met Ongedaan maken krijgt u alles terug.

## Een blad verbergen en weergeven

Klik met de rechtermuisknop op het tabblad en kies **Blad verbergen** (of kies **Start > Opmaak > Blad verbergen**). Het blad wordt niet weergegeven, maar staat nog in de werkmap en formules lezen het nog steeds. Om het weer weer te geven, klikt u met de rechtermuisknop op een willekeurig tabblad, kiest u **Blad zichtbaar maken** en klikt u op de naam in de lijst. De lijst is leeg als er geen verborgen bladen zijn.

## Een tabblad kleuren

Klik met de rechtermuisknop op het tabblad, kies **Tabkleur** en kies een kleur, of kies **Geen kleur**. De kleur verschijnt als een lijn onder de naam. Een kleur die u uit het themapalet kiest, wordt opgeslagen als de gewone kleur die het op dat moment is.

## Meerdere bladen tegelijk

**Ctrl+klik** op tabbladen om er meer dan één te selecteren, of klik op een tabblad en **Shift+klik** op een ander om alle tabbladen daartussen te selecteren. De geselecteerde tabbladen zijn gearceerd. **Verwijderen**, **Verbergen**, **Tabkleur** en **Verplaatsen of kopiëren** werken dan op al deze bladen in één stap. Klik op een tabblad dat niet is geselecteerd om alleen dat tabblad te selecteren.

Wat u typt of opmaakt terwijl meerdere bladen zijn geselecteerd, wijzigt alleen het blad dat u bekijkt.

## Wat niet kan

- In een werkmap met een formule over een reeks bladen, zoals **=SOM(Blad1:Blad3!A1)**, kunt u nog geen bladen toevoegen, verwijderen, een andere naam geven of verplaatsen, omdat dat zou veranderen welke bladen de formule omvat.
- Na het opslaan kan **Ongedaan maken** niet verder terug dan een blad dat u vóór het opslaan hebt toegevoegd, gekopieerd, verwijderd, een andere naam gegeven of verplaatst. Het opgeslagen bestand bevat de bladen zoals ze zijn. Verbergen, weergeven en de tabkleur kunt u nog wel ongedaan maken.
