# Wijzigen van de instellingen van JeffCrunch
<!-- keywords: formules-opties, berekeningsinstellingen, maximale wijziging, iteratie, herstel, niet-opgeslagen werk herstellen, automatisch herstel, controle, bewerken, automatisch aanvullen, vulgreep, slepen en neerzetten, decimalen, opties, instellingen, voorkeuren, gebruiksstatistieken, installatienummer, privacy, privacyverklaring, JP_NO_TELEMETRY, statistieken verwijderen, updates, info, bewerken in cel, rasterlijnkleur, standaardzoom, standaardbreedte, privacy, persoonlijke gegevens, toetsenbord aanpassen, lint aanpassen, snelle werkbalk, opdracht toevoegen aan snelle werkbalk, tabblad verbergen, groep verbergen, tabblad toevoegen, groep toevoegen, tabblad hernoemen, lint herstellen, sneltoetsen wijzigen, sneltoetsen opnieuw toewijzen, sneltoetsen herstellen -->

Uw instellingen staan in het venster **JeffCrunch-opties**. Klik op **Bestand > Opties** om het te openen. Klik op **OK** om uw wijzigingen te bewaren, of op **Annuleren** om ze weg te gooien.

## Algemeen

**Minibalk weergeven bij selectie** staat standaard aan. Hiermee bepaalt u of de kleine opmaakbalk naast geselecteerde tekst en bovenaan het snelmenu van cellen verschijnt (zie [Typen in cellen](edit-cells)). Deze keuze geldt voor alle JeffOffice-apps.

## Bewerken

De pagina **Bewerken** bepaalt hoe typen in cellen werkt.

- **Na Enter drukken de selectie verplaatsen** bepaalt waar de selectie naartoe gaat wanneer u een invoer afrondt: omlaag, naar rechts, omhoog, naar links of niet verplaatsen.
- **Tekst uit de cellen erboven en eronder voltooien** heet automatisch aanvullen. Wanneer u de eerste letters van een tekst typt en slechts één tekst in die kolom met die letters begint, toont JeffCrunch het restant, geselecteerd. Druk op **Enter** om het te behouden. Typ verder om het te negeren, of druk op **Backspace** om het weg te halen. Er wordt alleen gekeken naar het celbereik naast de cel waarin u typt, en alleen naar teksten, niet naar getallen.
- **Vulgreep gebruiken en cellen slepen om ze te verplaatsen of te kopiëren** schakelt het kleine vierkantje op de hoek van de selectie en het slepen van de rand van de selectie in. Schakel dit uit als u per ongeluk cellen blijft verplaatsen. Kopiëren en Plakken werken nog steeds.
- **Een nul tonen in cellen met de waarde nul (dit blad)** schakelt nullen in of uit voor het blad dat u bekijkt. Wanneer dit uit staat, lijkt een cel met 0 leeg, maar bevat hij nog steeds de 0 en telt hij mee als 0. De keuze wordt bij het blad opgeslagen.
- **Een decimaalteken plaatsen in getallen die zonder decimaalteken worden getypt** heet automatische decimalen. Stelt u **Decimalen** in op 2, dan voert u door 1234 te typen 12,34 in. Stelt u het in op -2, dan voert u door 5 te typen 500 in. Een getal dat u met een decimaalteken typt, wordt ingevoerd zoals u het typt.
- **Direct in cellen bewerken toestaan** staat standaard aan. Wanneer u dit uitschakelt, gaat een getypte invoer of F2 naar de formulebalk, en toont de cel het resultaat pas wanneer u klaar bent.

## Opties voor formules

De pagina **Formules** bepaalt hoe de geopende werkmap zijn formules berekent: **Berekening** (automatisch, automatisch behalve gegevenstabellen, of handmatig), of de formules vóór het opslaan van de werkmap worden berekend wanneer u handmatig rekent, of een formule die naar zichzelf verwijst wordt berekend door er iteratief doorheen te rekenen, het maximale aantal rondes, en de wijziging waarbij het rekenen stopt. Deze instellingen horen bij de werkmap, niet bij u: ze worden in het bestand opgeslagen en niet onthouden voor andere werkmappen, en de keuzes zijn uitgeschakeld wanneer er geen werkmap is geopend. De wijziging wordt met alle cijfers getoond, en u kunt haar typen als getal, zoals 0,0000001 of 1e-7. Alleen een instelling die u wijzigt, wordt opgeslagen: **OK** zonder wijzigingen voegt geen stap toe aan Ongedaan maken en laat de werkmap ongewijzigd. Een wijziging is één stap van Ongedaan maken. Het nieuwere werkmapbestand en het oudere .xls-bestand bewaren alle instellingen. Een .ods-kopie bewaart de iteratie-instellingen, maar heeft geen plek voor handmatig rekenen. De kopie berekent de formules daarom terwijl ze veranderen, en het rapport vóór het opslaan van de kopie meldt dat. De instellingen staan ook op het tabblad **Formules**.

## Opslaanopties

**Gegevens voor automatisch herstel elke** bepaalt hoe vaak JeffCrunch een kopie bewaart van een werkmap met wijzigingen die u nog niet hebt opgeslagen, van 1 tot 120 minuten. De kopie is een .xlsx-bestand in een map die JeffCrunch hiervoor bijhoudt. Ze wordt op de achtergrond gemaakt, zodat u door kunt typen. De kopie wordt verwijderd wanneer u de werkmap opslaat of sluit. Als JeffCrunch onverwacht sluit, biedt het de kopieën aan de volgende keer dat het start. U kunt ook klikken op **Bestand > Openen > Niet-opgeslagen werk herstellen**. Een werkmap die met een wachtwoord is beveiligd, wordt niet gekopieerd, omdat de kopie dan niet beveiligd zou zijn.

## Controleopties

De pagina **Controle** wordt door de JeffOffice-apps gedeeld. Hiermee bepaalt u of spelling wordt gecontroleerd terwijl u in een cel typt (schakel **Spelling controleren tijdens typen** uit om de rode golflijnen in elk geopend venster en in de andere apps te laten verdwijnen; **Spelling** op het tabblad Controleren werkt nog steeds), of woorden in hoofdletters en woorden met cijfers worden overgeslagen bij de spellingcontrole, of AutoCorrectie woorden corrigeert terwijl u typt, of rechte aanhalingstekens gekrulde worden, of streepjes, breuken en rangtelwoorden worden vervangen, en of een webadres een hyperlink wordt. Zie [Spelling](notes-links) en [Typen in cellen](edit-cells).

## Weergaveopties

- **Rasterlijnkleur** is de kleur van de lijnen tussen cellen op uw scherm. Dit is alleen een keuze voor uw scherm en wordt niet in de werkmap opgeslagen.
- **Zoomniveau van een nieuwe werkmap** bepaalt hoe groot de bladen van een nieuwe werkmap eruitzien, van 10% tot 400%.
- **Standaardkolombreedte (dit blad)** is de breedte van elke kolom van dit blad die geen eigen breedte heeft. Het is hetzelfde als **Standaardbreedte** op het tabblad Start.

## Aanpassen

Het tabblad **Aanpassen** heeft drie pagina's: **Lint**, **Snelle werkbalk** en **Toetsenbord**. Op de pagina **Toetsenbord** kiest u de sneltoetsen voor de opdrachten van JeffCrunch. Zie 'Eigen sneltoetsen kiezen' in het onderwerp over het toetsenbord.

### Het lint aanpassen

De pagina **Lint** toont de tabbladen van het lint, elk met zijn groepen.

- **Een tabblad of groep verbergen of tonen.** Haal het vinkje weg naast een tabblad of groep om het te verbergen, en zet het vinkje weer aan om het opnieuw te tonen. Een verborgen groep neemt geen ruimte in beslag, en de opdrachten ervan werken nog steeds via hun sneltoetsen. **Start** heeft geen vakje, omdat het altijd blijft staan, en dat geldt ook voor de knop **Bestand**. Een tabblad dat alleen verschijnt wanneer iets is geselecteerd, zoals een afbeelding of een tabel, staat aan het eind van de lijst en heeft geen vakje.
- **Een eigen tabblad of groep maken.** Typ een naam in het vak **Naam** en klik op **Nieuw tabblad**. Om een groep te maken, kiest u een tabblad en klikt u op **Nieuwe groep**. Een eigen groep kan op elk tabblad staan, ook op een tabblad dat alleen verschijnt wanneer iets is geselecteerd.
- **Opdrachten in uw groep zetten.** Kies uw groep, zoek een opdracht in de lijst **Opdrachten** (typ in het vak **Opdrachten zoeken** om de lijst te beperken) en klik op **Opdracht toevoegen**. Elke opdracht met een naam kan in een groep worden gezet. Een opdracht kan maar één keer in een groep staan.
- **Hernoemen.** Kies een eigen tabblad, groep of opdracht, typ de nieuwe naam en klik op **Naam wijzigen**. Een opdracht die u hernoemt, doet hetzelfde als voorheen, en de nieuwe naam is alleen de tekst op de knop. Geef een opdracht een lege naam om haar eigen naam terug te krijgen.
- **Volgorde wijzigen.** Kies een tabblad en klik op **Omhoog** of **Omlaag**. U kunt de eigen tabbladen van het lint verplaatsen, net als uw eigen tabbladen, maar niet de tabbladen die alleen verschijnen wanneer iets is geselecteerd. Uw groepen en opdrachten verplaatsen op dezelfde manier binnen hun tabblad of groep. De eigen groepen van het lint blijven in hun volgorde.
- **Verwijderen.** Kies een eigen tabblad, groep of opdracht en klik op **Verwijderen**.

**Eén herstellen** zet het gekozen tabblad terug: het wordt getoond, de groepen worden getoond, en de groepen die u eraan hebt toegevoegd, verdwijnen. Een eigen tabblad wordt verwijderd. **Alles herstellen** zet het hele lint terug, inclusief de volgorde van de tabbladen.

Er verandert niets totdat u op **OK** klikt. Daarna toont elk geopend venster van JeffCrunch uw lint, en elk venster dat u later opent ook. JeffCrunch bewaart alleen wat afwijkt van het gewone lint, zodat een tabblad of groep die een latere versie toevoegt, vanzelf verschijnt. Uw tabbladen en groepen krijgen toetsenblokjes (de letters die verschijnen wanneer u op **Alt** drukt) die niet met de andere botsen. U kunt 20 eigen tabbladen hebben, 30 eigen groepen per tabblad en 60 opdrachten per groep.

### De snelle werkbalk aanpassen

De pagina **Snelle werkbalk** is voor de kleine werkbalk linksboven in het venster. De linkerlijst toont wat er nu op de werkbalk staat, en de lijst **Opdrachten** rechts bevat elke opdracht met een naam (typ in het vak **Opdrachten zoeken** om de lijst te beperken, net als op de pagina **Lint**).

- **Een opdracht toevoegen.** Kies de opdracht in de lijst **Opdrachten** en klik op **Opdracht toevoegen**. Ze komt aan het eind van de werkbalk. Een opdracht die een menu opent, behoudt haar menu op de werkbalk. Een opdracht kan maar één keer op de werkbalk staan.
- **Met de rechtermuisknop klikken.** U kunt ook met de rechtermuisknop op een opdracht op het lint klikken en **Toevoegen aan snelle werkbalk** kiezen, of met de rechtermuisknop op een knop van de werkbalk klikken en **Verwijderen uit snelle werkbalk** kiezen. In hetzelfde menu staan **Snelle werkbalk aanpassen** en **Lint aanpassen**, die deze pagina openen.
- **Onder het lint plaatsen.** Vink **Snelle werkbalk onder het lint weergeven** aan om de werkbalk in een rij onder het lint te zetten in plaats van linksboven. De twee rechtermuismenu's hebben ook **Snelle werkbalk onder het lint weergeven**. JeffCrunch onthoudt deze keuze.
- **Een scheidingslijn toevoegen.** Klik op **Scheidingsteken toevoegen** om een dunne lijn aan het eind te zetten, en verplaats die daarna naar de gewenste plek.
- **Volgorde wijzigen.** Kies een opdracht of een scheidingslijn in de linkerlijst en klik op **Omhoog** of **Omlaag**.
- **Verwijderen.** Kies de opdracht en klik op **Verwijderen**.

**Eén herstellen** zet de gekozen opdracht terug: waar JeffCrunch haar normaal heeft, of van de werkbalk af wanneer JeffCrunch haar daar normaal niet heeft. **Alles herstellen** zet de werkbalk terug zoals JeffCrunch hem meelevert. Alles herstellen op deze pagina verandert uw lint niet, en Alles herstellen op de pagina **Lint** verandert uw werkbalk niet.

Er verandert niets totdat u op **OK** klikt. Daarna toont elk geopend venster van JeffCrunch uw werkbalk, en elk venster dat u later opent ook. De werkbalk kan in totaal 100 opdrachten en scheidingslijnen bevatten. Wanneer er meer knoppen zijn dan er in een derde van de breedte van het venster passen, wachten de overige achter een knop **»** aan het eind van de werkbalk, zodat de titel van het venster nooit van het scherm wordt geduwd. JeffCrunch bewaart een opdracht in uw lijst wanneer een latere versie haar niet meer heeft, en toont haar weer wanneer de opdracht terugkomt. De knoppen krijgen toetsenblokjes: de cijfers 1 tot 9 en daarna twee letters, in de volgorde waarin ze op de werkbalk staan. De werkbalk staat altijd linksboven, naast de knop **Bestand**.

## Privacyopties

- **Persoonlijke gegevens uit dit bestand verwijderen bij het opslaan** haalt de auteur, de persoon die het bestand als laatste heeft opgeslagen, het bedrijf, de manager, en de namen en tijden bij notities en opmerkingen weg uit het bestand dat wordt geschreven (een werkmap, een kopie, en een werkmap die per e-mail wordt verstuurd). Wat het bestand zegt, blijft zoals het is. De aangepaste eigenschappen van het bestand blijven ook staan, en het document dat u geopend hebt, wordt niet gewijzigd. Dit is in elke JeffOffice-app dezelfde keuze: wijzigt u die in één app, dan verandert hij in alle. Om dit eenmalig te doen, gebruikt u **Werkmap controleren** in [Info](info). Hetzelfde vakje staat op de pagina **Info**.

## Gebruiksstatistieken

**Anonieme gebruiksstatistieken verzenden** laat u gegevens delen over hoe JeffCrunch wordt gebruikt. Anoniem betekent dat de gegevens niets zeggen over wie u bent. Wanneer dit aan staat, stuurt JeffCrunch deze gegevens eens per dag, en op de dag dat u bijwerkt: dat het JeffCrunch is, de versie, uw besturingssysteem en taal, hoe vaak het is gestart, hoe vaak elke opdracht wordt gebruikt, en welk soort bestand u hebt geopend of opgeslagen. Als JeffCrunch ooit onverwacht sluit, stuurt het ook een crashrapport de volgende keer dat het start. JeffCrunch stuurt nooit uw bestanden, hun namen of iets in die bestanden, en ook niets wat u hebt getypt. Als dit uit staat, wordt ook geen crashrapport verstuurd.

De keuze geldt voor alle JeffOffice-apps: wijzigt u die hier, dan verandert hij in elk van hen. JeffCrunch vraagt het de eerste keer dat het start, in een venster met de titel **Help JeffCrunch verbeteren**.

- **Privacyverklaring** naast het vakje opent onze privacyverklaring op jeffoffice.com/privacy. Daarin staat wat er wordt verstuurd, waar het naartoe gaat, en hoe lang we het bewaren. We bewaren statistieken 24 maanden.
- **Installatienummer** wordt onder het vakje getoond. Het is een willekeurig nummer dat voor uw exemplaar van JeffOffice is gemaakt, en in elke JeffOffice-app hetzelfde. Het zegt niets over u. Klik op **Kopiëren** om het te kopiëren. Als statistieken uit staan, of er nog geen nummer is gemaakt, staat er **geen**.
- Wilt u dat de statistieken die we al hebben, worden verwijderd, stuur dan uw installatienummer naar info@jeffoffice.com **voordat** u de statistieken uitschakelt. Wanneer u ze uitschakelt, wordt het nummer van uw computer gewist, en zonder dat nummer kunnen we niet zien welke gegevens van u zijn.
- Om de statistieken uit te schakelen zonder JeffCrunch te openen, stelt u de omgevingsvariabele `JP_NO_TELEMETRY` in op een willekeurige waarde voordat u het start. Dan wordt niets geteld of verstuurd, en vraagt JeffCrunch niet de eerste-keer-vraag.

## Beveiliging

Op het tabblad **Beveiliging** bepaalt u wat JeffCrunch doet met de dingen waarnaar een bestand kan verwijzen: gekoppelde bestanden, gekoppelde films en geluiden, en bestanden van internet. Macro's worden in een bestand bewaard en nooit uitgevoerd, en een koppeling die een programma zou starten, wordt altijd geweigerd. Zie [Beveiliging](security).

## Gerelateerd

- [Welkom bij JeffCrunch](start)
