# Instellingen van JeffCrunch wijzigen
<!-- keywords: formules opties, berekeningsinstellingen, maximale wijziging, iteratie, herstel, niet-opgeslagen werk herstellen, automatisch herstel, controle, bewerken, automatisch aanvullen, vulgreep, slepen en neerzetten, decimalen, opties, instellingen, voorkeuren, gebruiksstatistieken, installatienummer, privacy, privacyverklaring, JP_NO_TELEMETRY, statistieken verwijderen, updates, info, bewerken in cel, rasterlijnkleur, standaardzoom, standaardbreedte, privacy, persoonlijke gegevens, toetsenbord aanpassen, lint aanpassen, snelle werkbalk, opdracht toevoegen aan snelle werkbalk, tabblad verbergen, groep verbergen, tabblad toevoegen, groep toevoegen, tabblad hernoemen, lint herstellen, sneltoetsen wijzigen, toetsen opnieuw toewijzen, toetsen herstellen -->

Uw instellingen staan in het venster **JeffCrunch-opties**. Klik op **Bestand > Opties** om het te openen. Klik op **OK** om uw wijzigingen te bewaren, of op **Annuleren** om ze weg te gooien.

## Algemeen

**Minibalk weergeven bij selectie** staat standaard aan. Hiermee bepaalt u of de kleine opmaakbalk naast geselecteerde tekst verschijnt en bovenaan het snelmenu van cellen (zie [Tekst invoeren in cellen](edit-cells)). De keuze geldt voor alle JeffOffice-apps.

## Bewerken

De pagina **Bewerken** bepaalt hoe typen in cellen werkt.

- **Na drukken op Enter de selectie verplaatsen** bepaalt waar de selectie naartoe gaat wanneer u een invoer afrondt: omlaag, naar rechts, omhoog, naar links of nergens naartoe.
- **Tekst uit de cellen erboven en eronder aanvullen** heet automatisch aanvullen. Wanneer u de eerste letters van een tekst typt en slechts één tekst in die kolom met die letters begint, toont JeffCrunch het restant, geselecteerd. Druk op **Enter** om het te behouden. Typ verder om het te negeren, of druk op **Backspace** om het weg te halen. Er wordt alleen gekeken naar het celbereik naast de cel waarin u typt, en alleen naar teksten, niet naar getallen.
- **Vulgreep gebruiken en cellen slepen om ze te verplaatsen of kopiëren** schakelt het vierkantje op de hoek van de selectie en het slepen van de rand van de selectie in. Schakel dit uit als u per ongeluk cellen blijft verplaatsen. Kopiëren en Plakken werken nog steeds.
- **Een nul tonen in cellen met de waarde nul (dit blad)** schakelt nullen in of uit voor het blad dat u bekijkt. Wanneer dit uit staat, lijkt een cel met 0 leeg, maar bevat hij nog steeds de 0 en telt hij mee als 0. De keuze wordt bij het blad opgeslagen.
- **Een decimaalteken plaatsen in getallen zonder decimalen die worden getypt** heet automatische decimalen. Stel **Decimalen** in op 2, dan voert u bij het typen van 1234 de waarde 12,34 in. Stel u het in op -2, dan voert u bij het typen van 5 de waarde 500 in. Een getal dat u met een decimaalteken typt, wordt ingevoerd zoals u het typt.

- **Bewerken direct in cellen toestaan** staat standaard aan. Wanneer u dit uitschakelt, gaat een getypte invoer of F2 naar de formulebalk, en toont de cel het resultaat pas wanneer u klaar bent.

## Opties voor formules

De pagina **Formules** bepaalt hoe de geopende werkmap zijn formules berekent: **Berekening** (automatisch, automatisch behalve gegevenstabellen, of handmatig), of de formules vóór het opslaan van de werkmap worden berekend wanneer u handmatig rekent, of een formule die naar zichzelf verwijst wordt berekend door er meerdere keren doorheen te gaan, het maximale aantal rondes en de wijziging waarbij het rondgaan stopt. Deze instellingen horen bij de werkmap, niet bij u: ze worden in het bestand opgeslagen en niet onthouden voor andere werkmappen, en de keuzes zijn uitgeschakeld wanneer er geen werkmap is geopend. De wijziging wordt met alle cijfers getoond, en u kunt haar typen als getal, zoals 0,0000001 of 1e-7. Alleen een instelling die u wijzigt, wordt opgeslagen: **OK** zonder wijzigingen voegt geen stap voor Ongedaan maken toe en laat de werkmap zoals hij was. Een wijziging is één stap van Ongedaan maken. Het nieuwere werkmapbestand en het oudere .xls-bestand bewaren alle instellingen. Een .ods-kopie bewaart de iteratie-instellingen, maar heeft geen plaats voor handmatig berekenen. De kopie rekent de formules daarom bij elke wijziging opnieuw, en het rapport vóór het opslaan van de kopie meldt dit. Dezelfde instellingen staan op het tabblad **Formules**.

## Opslaanopties

**Gegevens voor automatisch herstel elke** bepaalt hoe vaak JeffCrunch een kopie bewaart van een werkmap met wijzigingen die u nog niet hebt opgeslagen, van 1 tot 120 minuten. De kopie is een .xlsx-bestand in een map die JeffCrunch hiervoor bijhoudt. Die wordt op de achtergrond gemaakt, zodat u verder kunt typen. De kopie wordt verwijderd wanneer u de werkmap opslaat of sluit. Als JeffCrunch onverwacht sluit, biedt het de kopieën aan de volgende keer dat het start. U kunt ook klikken op **Bestand > Openen > Niet-opgeslagen werk herstellen**. Een werkmap die met een wachtwoord is beveiligd, wordt niet gekopieerd, omdat de kopie dan niet beveiligd zou zijn.

## Opties voor controle

De pagina **Controle** wordt gedeeld door de JeffOffice-apps. Hier bepaalt u of spelling wordt gecontroleerd tijdens het typen in een cel (schakel **Spelling controleren tijdens typen** uit om de rode golflijnen in elk open venster en de andere apps te laten verdwijnen; **Spelling** op het tabblad Controleren werkt nog steeds), of woorden in hoofdletters en woorden met cijfers worden genegeerd bij de spellingcontrole, of AutoCorrectie woorden corrigeert tijdens het typen, of rechte aanhalingstekens worden omgezet in krulaanhalingstekens, of streepjes, breuken en rangtelwoorden worden aangepast, en of een webadres een hyperlink wordt. Het tabblad **Wiskundesymbolen** van AutoCorrectie-opties is voor JeffWrite, JeffDeck en JeffPub: JeffCrunch zet een backslash en een naam in een cel niet om in een symbool. Zie [Spelling](notes-links) en [Tekst invoeren in cellen](edit-cells).

## Weergaveopties

- **Rasterlijnkleur** is de kleur van de lijnen tussen cellen op uw scherm. Het is alleen een keuze voor uw scherm en wordt niet in de werkmap opgeslagen.
- **Zoomen van een nieuwe werkmap** bepaalt hoe groot de bladen van een nieuwe werkmap worden getoond, van 10% tot 400%.
- **Standaardkolombreedte (dit blad)** is de breedte van elke kolom van dit blad die geen eigen breedte heeft. Het is hetzelfde als **Standaardbreedte** op het tabblad Start.
- **Bladtabs tonen** en **Schuifbalken tonen** verbergen of tonen de rij met bladtabs onder de cellen en de schuifbalken rechts en onderaan. Het zijn keuzes voor uw scherm, die voor elke werkmap die u opent gelden.
- **Notitie van een cel tonen als** bepaalt hoe een cel met een notitie eruitziet: een klein teken op de cel en de notitie wanneer de aanwijzer erop blijft staan, alleen het teken, of niets. Een notitie die u altijd wilt tonen (**Notitie weergeven**), blijft in alle gevallen op het blad staan.

## Aanpassen

Het tabblad **Aanpassen** heeft drie pagina's: **Lint**, **Snelle werkbalk** en **Toetsenbord**. Op de pagina **Toetsenbord** kiest u de sneltoetsen voor de opdrachten van JeffCrunch. Zie "Eigen sneltoetsen kiezen" in het onderwerp over het toetsenbord.

### Het lint aanpassen

De pagina **Lint** toont de tabbladen van het lint, elk met hun groepen.

- **Een tabblad of groep verbergen of tonen.** Haal het vinkje weg naast het tabblad of de groep om het te verbergen, en zet het vinkje weer aan om het opnieuw te tonen. Een verborgen groep neemt geen ruimte in beslag, en de opdrachten ervan werken nog steeds via hun sneltoetsen. **Start** heeft geen vinkje, omdat het altijd blijft staan, en dat geldt ook voor de knop **Bestand**. Een tabblad dat alleen verschijnt wanneer iets is geselecteerd, zoals een afbeelding of een tabel, staat aan het eind van de lijst en heeft geen vinkje.
- **Een eigen tabblad of groep maken.** Typ een naam in het vak **Naam** en klik op **Nieuw tabblad**. Om een groep te maken, kiest u een tabblad en klikt u op **Nieuwe groep**. Een eigen groep kan op elk tabblad komen, ook op een tabblad dat alleen verschijnt wanneer iets is geselecteerd.
- **Opdrachten in uw groep zetten.** Kies uw groep, zoek een opdracht in de lijst **Opdrachten** (typ in het vak **Opdrachten zoeken** om de lijst te beperken) en klik op **Opdracht toevoegen**. Elke opdracht met een naam kan erin. Een opdracht kan maar één keer in een groep staan.
- **Hernoemen.** Kies een eigen tabblad, groep of opdracht, typ de nieuwe naam en klik op **Hernoemen**. Een opdracht die u hernoemt, doet hetzelfde als voorheen; alleen de tekst op de knop verandert. Hernoem een opdracht met een lege naam om haar eigen naam terug te geven.
- **Opnieuw ordenen.** Kies een tabblad en klik op **Omhoog verplaatsen** of **Omlaag verplaatsen**. U kunt de eigen tabbladen van het lint verplaatsen, net als uw eigen tabbladen, maar niet de tabbladen die alleen verschijnen wanneer iets is geselecteerd. Uw groepen en opdrachten verplaatsen op dezelfde manier binnen hun tabblad of groep. De eigen groepen van het lint blijven in hun volgorde staan.
- **Verwijderen.** Kies een eigen tabblad, groep of opdracht en klik op **Verwijderen**.

**Eén herstellen** zet het gekozen tabblad terug: het wordt getoond, de groepen ervan worden getoond, en de groepen die u eraan hebt toegevoegd, verdwijnen. Een eigen tabblad wordt verwijderd. **Alles herstellen** zet het hele lint terug, inclusief de volgorde van de tabbladen.

Er verandert niets totdat u op **OK** klikt. Daarna toont elk open venster van JeffCrunch uw lint, en elk venster dat u later opent ook. JeffCrunch bewaart alleen wat afwijkt van het gebruikelijke lint, zodat een tabblad of groep die een latere versie toevoegt, vanzelf verschijnt. Uw tabbladen en groepen krijgen toetsenblokjes (de letters die verschijnen wanneer u op **Alt** drukt) die niet botsen met de andere. U kunt 20 eigen tabbladen hebben, 30 eigen groepen per tabblad en 60 opdrachten per groep.

### De snelle werkbalk aanpassen

De pagina **Snelle werkbalk** is voor de kleine werkbalk linksboven in het venster. De linkerlijst toont wat er nu op de werkbalk staat, en de lijst **Opdrachten** rechts bevat elke opdracht met een naam (typ in het vak **Opdrachten zoeken** om de lijst te beperken, net als op de pagina **Lint**).

- **Een opdracht toevoegen.** Kies de opdracht in de lijst **Opdrachten** en klik op **Opdracht toevoegen**. Ze komt aan het eind van de werkbalk. Een opdracht die een menu opent, houdt zijn menu op de werkbalk. Een opdracht kan maar één keer op de werkbalk staan.
- **Met de rechtermuisknop klikken.** U kunt ook met de rechtermuisknop op een opdracht op het lint klikken en **Toevoegen aan snelle werkbalk** kiezen, of met de rechtermuisknop op een knop op de werkbalk klikken en **Verwijderen uit snelle werkbalk** kiezen. Hetzelfde menu heeft **Snelle werkbalk aanpassen** en **Lint aanpassen**, die deze pagina openen.
- **Onder het lint plaatsen.** Vink **Snelle werkbalk onder het lint weergeven** aan om de werkbalk in een rij onder het lint te zetten in plaats van linksboven. De twee menu's met de rechtermuisknop hebben ook **Snelle werkbalk weergeven onder het lint**. JeffCrunch onthoudt de keuze.
- **Een scheidingslijn toevoegen.** Klik op **Scheidingsteken toevoegen** om een dunne lijn aan het eind te zetten, en verplaats die daarna naar de gewenste plek.
- **Opnieuw ordenen.** Kies een opdracht of scheidingslijn in de linkerlijst en klik op **Omhoog verplaatsen** of **Omlaag verplaatsen**.
- **Verwijderen.** Kies de opdracht of scheidingslijn en klik op **Verwijderen**.

**Eén herstellen** zet de gekozen opdracht terug: waar JeffCrunch haar gewoonlijk plaatst, of van de werkbalk af wanneer JeffCrunch haar daar gewoonlijk niet heeft. **Alles herstellen** zet de werkbalk terug zoals JeffCrunch hem meelevert. Alles herstellen op deze pagina wijzigt uw lint niet, en Alles herstellen op de pagina **Lint** wijzigt uw werkbalk niet.

Er verandert niets totdat u op **OK** klikt. Daarna toont elk open venster van JeffCrunch uw werkbalk, en elk venster dat u later opent ook. De werkbalk kan in totaal 100 opdrachten en scheidingslijnen bevatten. Wanneer er meer knoppen zijn dan er in een derde van de breedte van het venster passen, wachten de rest achter een knop **»** aan het eind van de werkbalk, zodat de titel van het venster nooit van het scherm wordt geduwd. JeffCrunch houdt een opdracht in uw lijst wanneer een latere versie haar niet meer heeft, en toont haar weer wanneer de opdracht terugkomt. De knoppen krijgen toetsenblokjes: de cijfers 1 tot en met 9 en daarna twee letters, in de volgorde waarin ze op de werkbalk staan. De werkbalk staat altijd linksboven, naast de knop **Bestand**.

## Privacyopties

- **Persoonlijke gegevens uit dit bestand verwijderen bij het opslaan** haalt bij het geschreven bestand (een werkmap, een kopie en een werkmap die per e-mail wordt verzonden) de auteur, de persoon die het bestand als laatste heeft opgeslagen, het bedrijf, de manager, en de namen en tijden bij notities en opmerkingen weg. Wat er in het bestand staat, blijft zoals het is. De eigen eigenschappen van het bestand blijven ongewijzigd, en het document dat u geopend hebt, wordt niet veranderd. Het is dezelfde keuze in elke JeffOffice-app: wijzig hem in één app en hij verandert in alle. Om dit eenmalig te doen, gebruikt u **Werkmap controleren** in [Info](info). Hetzelfde vak staat op de pagina **Info**.

## Gebruiksstatistieken

**Anonieme gebruiksstatistieken verzenden** laat u gegevens delen over hoe JeffCrunch wordt gebruikt. Anoniem betekent dat de gegevens niets zeggen over wie u bent. Wanneer dit aan staat, stuurt JeffCrunch deze gegevens één keer per dag, en op de dag dat u bijwerkt: dat het om JeffCrunch gaat, de versie, uw besturingssysteem en taal, hoe vaak het is gestart, hoe vaak elke opdracht wordt gebruikt, en welk soort bestand u hebt geopend of opgeslagen. Als JeffCrunch ooit onverwacht sluit, stuurt het bij de volgende start ook een foutrapport. JeffCrunch stuurt nooit uw bestanden, hun namen of iets wat erin staat, en niets wat u hebt getypt. Als dit uit staat, wordt ook geen foutrapport verzonden.

De keuze geldt voor alle JeffOffice-apps: wijzig haar hier en ze verandert in elk van hen. JeffCrunch vraagt er de eerste keer dat het start naar, in een venster met de naam **Help JeffCrunch verbeteren**.

- **Privacyverklaring** naast het selectievakje opent onze privacyverklaring op jeffoffice.com/privacy. Daarin staat wat wordt verzonden, waarheen het gaat en hoe lang we het bewaren. We bewaren statistieken 24 maanden.
- **Installatienummer** staat onder het selectievakje. Het is een willekeurig nummer dat is gemaakt voor uw exemplaar van JeffOffice, hetzelfde in elke JeffOffice-app. Het zegt niets over u. Klik op **Kopiëren** om het te kopiëren. Als statistieken uit staan, of er nog geen nummer is gemaakt, staat hier **geen**.
- Wilt u de statistieken die we al hebben laten verwijderen, stuur dan uw installatienummer naar info@jeffoffice.com **voordat** u de statistieken uitschakelt. Uitschakelen wist het nummer van uw computer, en zonder het kunnen we niet zien welke gegevens van u zijn.
- Om statistieken uit te schakelen zonder JeffCrunch te openen, stelt u de omgevingsvariabele `JP_NO_TELEMETRY` in op een willekeurige waarde voordat u het start. Er wordt dan niets geteld of verzonden, en JeffCrunch stelt de vraag bij de eerste start niet.

## Beveiliging

Het tabblad **Beveiliging** bepaalt wat JeffCrunch doet met de dingen waarnaar een bestand kan verwijzen: gekoppelde bestanden, gekoppelde films en geluiden, en bestanden van internet. Macro's worden in een bestand bewaard en nooit uitgevoerd, en een koppeling die een programma zou starten, wordt altijd geweigerd. Zie [Beveiliging](security).

## Gerelateerd

- [Welkom bij JeffCrunch](start)
