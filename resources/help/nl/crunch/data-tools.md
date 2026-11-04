# Gegevenshulpmiddelen
<!-- keywords: gegevens, tabblad Gegevens, tekst naar kolommen, splitsen, scheidingsteken, komma, tab, vaste breedte, dubbele waarden verwijderen, herhaalde rijen, uniek, tekstbestand, csv, uit tekst, kolommen, scheidingsteken, aanhalingstekens -->

Op het tabblad **Gegevens** staan de hulpmiddelen die werken op een lijst met waarden: tekst uit een bestand ophalen, een lijst sorteren, tekst in kolommen splitsen en herhaalde rijen verwijderen.

## Uit tekst/CSV

**Uit tekst/CSV** opent een tekstbestand (.csv, .tsv of .txt). Dit is hetzelfde als **Tekstbestand openen** op het tabblad Bestand. JeffCrunch vraagt hoe het bestand in kolommen is verdeeld en wat voor soort tekst het bevat, en toont het daarna als een nieuwe werkmap.

## Sorteren en filteren

De groep **Sorteren en filteren** bevat dezelfde opdrachten als het menu **Sorteren en filteren** op het tabblad Start. Zie [Sorteren en filteren](sort-filter).

## Tekst naar kolommen

Gebruik dit wanneer één kolom meerdere stukjes tekst bevat die u in afzonderlijke kolommen wilt hebben, zoals "Jansen, Ann, 31".

1. Selecteer de cellen van **één** kolom.
2. Kies **Tekst naar kolommen** op het tabblad **Gegevens**.
3. Kies hoe u de tekst wilt splitsen:
   - **Splitsen op scheidingsteken**: vink de tekens aan die tussen de stukjes staan: **Tab**, **Puntkomma**, **Komma**, **Spatie** of **Ander scheidingsteken**, waar u zelf het teken typt. **Opeenvolgende scheidingstekens als één behandelen** voorkomt dat twee komma's achter elkaar een lege kolom maken. **Tekst tussen aanhalingstekens** geeft het teken aan dat een stukje omsluit dat niet mag worden gesplitst, zoals "Lee, Bo". Binnen de aanhalingstekens tellen twee tekens achter elkaar als één.
   - **Splitsen op vaste posities**: typ de posities waar een nieuwe kolom begint, met tekens geteld vanaf 1. Bijvoorbeeld: 5, 12 start een nieuwe kolom bij het vijfde en bij het twaalfde teken.
4. Kies wat elk stukje is: **Algemeen** herkent getallen en datums zoals u ze hebt getypt, en **Tekst** behoudt elk stukje zoals het is, zodat "007" "007" blijft.
5. Typ bij **Delen plaatsen vanaf** de eerste cel voor de stukjes. Standaard is dat de eerste cel die u hebt geselecteerd.
6. Het vak onder de keuzes toont de eerste rijen zoals u ze hebt gesplitst. Klik op **OK**.

De stukjes komen in de cellen rechts van de eerste cel en vervangen wat er in die cellen stond. Tekst naar kolommen is één stap die u ongedaan kunt maken.

## Dubbele waarden verwijderen

Gebruik dit om van elke groep herhaalde rijen alleen de eerste te bewaren.

1. Selecteer de cellen van de lijst, of één cel erin. JeffCrunch neemt het hele gegevensblok rond één cel.
2. Kies **Dubbele waarden verwijderen**.
3. Vink **Mijn gegevens hebben koppen** aan als de eerste rij namen bevat. Die rij blijft staan en wordt niet vergeleken.
4. Vink de kolommen aan die moeten overeenkomen, wil twee rijen als herhaling gelden. Twee rijen zijn alleen herhalingen als elke aangevinkte kolom gelijk is. Hoofd- en kleine letters tellen als gelijk, en een formule telt als de waarde die ze geeft.
5. Klik op **OK**. Het dialoogvenster meldt hoeveel rijen zijn verwijderd en hoeveel er over zijn.

De rijen onder een verwijderde rij schuiven omhoog binnen de geselecteerde cellen, met hun opmaak en formules. De cellen onder de selectie verschuiven niet. De lege rijen die onderaan de selectie overblijven, worden gewist. Dubbele waarden verwijderen is één stap die u ongedaan kunt maken.
