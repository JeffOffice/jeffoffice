# Schermlezers, hoog contrast en alternatieve tekst
<!-- keywords: toegankelijkheid, schermlezer, verteller, blind, slechtziend, hoog contrast, donkere modus, donker thema, focusomlijning, alternatieve tekst, afbeeldingsbeschrijving, zoomen, vergroten -->

JeffPub is zo gemaakt dat mensen die schermlezers, het toetsenbord of hoogcontrastkleuren gebruiken, het ook kunnen gebruiken.

## Schermlezers

Een **schermlezer** is een programma dat het scherm hardop voorleest. JeffPub werkt met schermlezers zoals Verteller op Windows. Elk element heeft een naam en een beschrijving. Dit geldt voor de tabbladen, knoppen, galerieën en kleurknoppen van het lint, de lijst Pagina's, de pagina, de taakvensters en de statusbalk.

De objecten op de pagina worden ook voorgelezen. Druk op de pagina op **Tab** om het volgende object te selecteren: de schermlezer zegt welk soort object het is en wat het bevat, zoals "Tekstvak: Voorjaarsuitverkoop" of "Afbeelding: een rode schuur", en daarna waar het staat en hoe groot het is. Een afbeelding zonder alternatieve tekst wordt voorgelezen als "Afbeelding, geen alternatieve tekst", zodat u weet welke nog een beschrijving nodig hebben. Druk op **Enter** om in een tekstvak te typen; de schermlezer kan dan de tekst, de regel of het woord waarop de cursor staat, en wat u selecteert, voorlezen.

- De lijst Pagina's leest elke pagina voor als "Pagina 1", "Pagina 2", enzovoort. Als u een pagina een naam hebt gegeven, volgt die naam, zoals in "Pagina 3, Omslag".
- Het paginagebied heet "Pagina". De beschrijving ervan vertelt welke toetsen u kunt gebruiken: Tab selecteert het volgende object, de pijltjestoetsen verplaatsen het, Enter bewerkt de tekst en Delete verwijdert het.
- Tabbladen worden voorgelezen als lintabbladen. Tabbladen die verschijnen wanneer u iets selecteert, noemen ook hun groep, zoals Afbeeldingshulpmiddelen.

## Het toetsenbord gebruiken

U kunt het lint, de lijst Pagina's en de objecten op de pagina alleen met het toetsenbord bereiken. Druk op F6 om te wisselen tussen het lint, de lijst Pagina's en de pagina. Druk op Alt om toetsenblokjes te zien. Een gekleurde omlijning laat zien welk element de focus heeft. Zie [Use JeffPub with the keyboard](keyboard).

## De kleuren wijzigen

- **Donker of licht**: klik op **Bestand > Opties** en stel op het tabblad **Algemeen** de optie **Thema** in op **Licht**, **Donker** of **Systeeminstelling gebruiken**. De wijziging wordt meteen doorgevoerd.
- **Hoog contrast**: op Windows volgt JeffPub het hoogcontrastthema wanneer u dat inschakelt. Het gebruikt de kleuren die Windows heeft gekozen, en het past zich aan wanneer u het thema in- of uitschakelt.

## Afbeeldingen beter begrijpelijk maken

**Alternatieve tekst** (alternative text) is een korte beschrijving van een afbeelding of ander object. Schermlezers lezen deze hardop voor. JeffPub neemt de alternatieve tekst ook op in webpagina's en e-books die u opslaat.

1. Klik met de rechtermuisknop op de afbeelding of het object.
2. Klik op **Object opmaken…**.
3. Klik op het tabblad **Alternatieve tekst**.
4. Typ een beschrijving in het vak **Alternatieve tekst**.
5. Klik op **OK**.

Beschrijf in een paar woorden wat de afbeelding toont, bijvoorbeeld "Twee kinderen die een boom planten". Pictogrammen en streepjescodes die u invoegt, krijgen zelf alternatieve tekst.

## De pagina controleren

Klik op **Controleren > Toegankelijkheid controleren** om een lijst te zien van wat sommige mensen moeilijk kunnen gebruiken. Behalve afbeeldingen en vormen zonder alternatieve tekst, vindt de controle nog drie dingen.

- **Tekst die moeilijk leesbaar is.** Tekst moet voldoende *contrast* hebben met wat erachter staat: de opvulling van het vak, of de paginakleur als het vak geen opvulling heeft. De controle berekent het contrast als verhouding. Een verhouding van 4,5 tot 1 is voldoende voor gewone tekst, en 3 tot 1 voor grote tekst (18 punten of meer, of 14 punten of meer vet). Gray #777777 op wit is bijvoorbeeld 4,48 tot 1, net iets te weinig, en #767676 is 4,54 tot 1. Kies dan een donkerdere tekstkleur of een lichtere achtergrond. Staat tekst over een afbeelding, dan kan de controle dat niet beoordelen en voegt daarom een tip toe om het contrast met het oog te controleren.
- **Objecten die in een andere volgorde worden gelezen.** Een schermlezer en een getagde pdf lezen de objecten op een pagina van achteren naar voren in de stapel. Als die volgorde niet van boven naar beneden en daarna van links naar rechts loopt, somt de controle de objecten die niet op hun plaats staan op als tips. Klik op **Leesvolgorde** om de objecten te zien in de volgorde waarin ze worden gelezen. Selecteer een object en klik op **Eerder lezen** of **Later lezen**, of sleep het. Elke verplaatsing is één stap van Ongedaan maken, en ze wijzigt ook welk object voorop staat.
- **Een tabel zonder kopregel.** Een schermlezer gebruikt de eerste rij van een tabel om de kolommen een naam te geven. Een tabel waarvan de eerste rij niet als kopregel is gemarkeerd, wordt als waarschuwing vermeld. Klik op het probleem en klik vervolgens op **De eerste rij als koptekst gebruiken** om de kopregel in te schakelen. Dit is één stap van Ongedaan maken.

## De pagina vergroten

Zoom in om kleine tekst beter te kunnen lezen. Gebruik de besturing **Zoomen** op de statusbalk, houd Ctrl ingedrukt terwijl u het muiswiel draait, of kies een grootte in het vak **Zoomen** op het tabblad **Beeld**. De zoomschuif loopt van ongeveer 10% tot 1600%.

## Zie ook

- [Use JeffPub with the keyboard](keyboard)
- [Add and adjust pictures](pictures)
- [Change JeffPub's settings](options)
- [Save as a picture, web page, or e-book](export)
