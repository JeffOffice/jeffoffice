# Werken met werkbladen
<!-- keywords: blad, bladen, werkblad, werkbladen, tabblad, tabbladen, toevoegen, invoegen, nieuw blad, verwijderen, wissen, naam wijzigen, hernoemen, naam, verplaatsen, kopiëren, dupliceren, volgorde wijzigen, slepen, verbergen, zichtbaar maken, weergeven, tabkleur, kleur, meerdere bladen selecteren, Shift+F11, #VERW!, 3D-verwijzing, Blad verplaatsen of kopiëren -->

Een werkmap bestaat uit **werkbladen**. De **tabbladen** onder in het venster tonen hun namen. Klik op een tabblad om dat werkblad weer te geven. Elke wijziging hieronder is één stap die u ongedaan kunt maken, en wordt in de werkmap opgeslagen.

## Een werkblad toevoegen

- Klik op de knop **+** na het laatste tabblad om aan het einde een werkblad toe te voegen.
- Druk op **Shift+F11** of kies **Start > Invoegen > Blad invoegen** om een werkblad toe te voegen vóór het werkblad dat u bekijkt.

Het nieuwe werkblad is leeg, krijgt de eerstvolgende vrije naam (Blad4, Blad5 enzovoort) en wordt meteen geopend. Een werkmap kan maximaal 1.000 werkbladen bevatten. Daarboven meldt JeffCrunch dit en voegt het geen werkblad toe, omdat het een bestand met meer werkbladen niet kan openen.

## De naam van een werkblad wijzigen

**Dubbelklik** op het tabblad, typ de nieuwe naam en druk op **Enter** (met **Esc** annuleert u). U kunt ook **Start > Opmaak > Naam van blad wijzigen** gebruiken.

Een naam mag maximaal 31 tekens bevatten. De naam mag niet leeg zijn, mag geen van de tekens **: \ / ? * [ ]** bevatten, mag niet met een apostrof beginnen of eindigen en mag niet gelijk zijn aan de naam van een ander werkblad (hoofdletters tellen niet als verschil). De naam "History" is gereserveerd voor het programma. Als de naam wordt geweigerd, meldt JeffCrunch waarom en behoudt het werkblad zijn naam.

Elke formule die het werkblad noemt, volgt de nieuwe naam. Voorbeeld: **=Budget!B2** wordt **='Budget K1'!B2** wanneer u de naam Budget wijzigt in Budget K1. Namen die naar het werkblad verwijzen, de regels van voorwaardelijke opmaak en validatie, en grafieken volgen de nieuwe naam ook.

## Een werkblad verwijderen

Klik met de rechtermuisknop op het tabblad en kies **Blad verwijderen**, of kies **Start > Verwijderen > Blad verwijderen**. Als er iets op het werkblad staat, vraagt JeffCrunch eerst om bevestiging. Formules op andere werkbladen die het verwijderde werkblad lezen, geven **#VERW!** weer: wat ze lazen, bestaat niet meer. Met **Ongedaan maken** krijgt u het werkblad en die formules terug.

Een werkmap houdt altijd minstens één zichtbaar werkblad.

## Een werkblad verplaatsen of kopiëren

- **Sleep** een tabblad naar een nieuwe plek om het werkblad te verplaatsen.
- Of kies **Start > Opmaak > Blad verplaatsen of kopiëren**. Kies het werkblad waarvóór het gekozen werkblad moet komen (of **(na het laatste blad)**), en schakel **Kopiëren en de originelen laten staan** in om het te kopiëren in plaats van te verplaatsen. Een kopie krijgt een naam als **Budget (2)** en wordt geopend.

Formules blijven de werkbladen lezen die ze lazen, waar die werkbladen ook staan. Een kopie heeft dezelfde cellen, formules, opmaak en afmetingen als het werkblad waarvan ze is gemaakt. Een verwijzing zonder bladnaam (zoals **A1**) leest de eigen cel van de kopie; een verwijzing met een bladnaam (zoals **Blad1!A1**) blijft naar dat werkblad verwijzen, ook als dat het werkblad is waarvan de kopie is gemaakt.

Een kopie neemt ook de beveiliging, het filter, de koppelingen, de pagina-instelling en pagina-einden, de groepen rijen en kolommen en het uiterlijk van de gegevensbalken van het origineel over. Koppelingen volgen de rijen die u hebt ingevoegd of verwijderd: in het origineel vóór het kopiëren en in de kopie daarna. Afbeeldingen, grafieken, notities, draaitabellen, sparklines en de namen die alleen bij het origineel horen, worden niet overgenomen, omdat JeffCrunch die nog niet kopieert; als dit gebeurt, meldt JeffCrunch welke. Een werkblad met tabellen wordt gekopieerd met kopieën van die tabellen (elk met een eigen naam, id en onderdeel, en de formules van de kopie verwijzen naar de eigen tabellen). Als u een werkblad verwijdert, worden ook de tabellen erop verwijderd; een formule op een ander werkblad die naar zo'n tabel verwees, geeft dan #VERW! weer, en Ongedaan maken herstelt dit.

## Een werkblad verbergen en weergeven

Klik met de rechtermuisknop op het tabblad en kies **Blad verbergen** (of kies **Start > Opmaak > Blad verbergen**). Het werkblad wordt niet meer weergegeven, maar staat nog in de werkmap en formules lezen het nog steeds. Als u het weer wilt weergeven, klikt u met de rechtermuisknop op een tabblad, kiest u **Blad zichtbaar maken** en klikt u op de naam in de lijst. De lijst is leeg als er geen werkblad verborgen is.

## Een tabblad een kleur geven

Klik met de rechtermuisknop op het tabblad, kies **Tabkleur** en kies een kleur, of kies **Geen kleur**. De kleur verschijnt als een lijn onder de naam. Een kleur die u uit het palet van het thema kiest, wordt opgeslagen als de vaste kleur die ze op dat moment is.

## Meerdere werkbladen tegelijk

Houd **Ctrl** ingedrukt en klik op tabbladen om er meer dan één te selecteren, of klik op een tabblad en houd **Shift** ingedrukt terwijl u op een ander tabblad klikt om alle tabbladen daartussen te selecteren. De geselecteerde tabbladen worden gearceerd. **Verwijderen**, **Verbergen**, **Tabkleur** en **Verplaatsen of kopiëren** werken dan in één stap op al deze werkbladen. Klik op een tabblad dat niet is geselecteerd om alleen dat tabblad te selecteren.

Wat u typt of opmaakt terwijl meerdere werkbladen zijn geselecteerd, wijzigt alleen het werkblad dat u bekijkt.

## Wat niet kan

- In een werkmap met een formule over een reeks werkbladen, zoals **=SOM(Blad1:Blad3!A1)**, kunt u nog geen werkbladen toevoegen, verwijderen, een andere naam geven of verplaatsen, omdat dat zou veranderen welke werkbladen de formule omvat.
- Na het opslaan kan **Ongedaan maken** niet verder teruggaan dan een werkblad dat u vóór het opslaan hebt toegevoegd, gekopieerd, verwijderd, een andere naam hebt gegeven of hebt verplaatst. Het opgeslagen bestand bevat ze zoals ze zijn. Verbergen, weergeven en de tabkleur kunt u nog wel ongedaan maken.
