# Vertalen van tekst
<!-- keywords: vertalen, vertaling, vertaler, taal, talen, spaans, frans, duits, italiaans, portugees, nederlands, russisch, oekraïens, indonesisch, offline, op deze computer, downloaden, modellen, vervangen, onder invoegen -->

**Controleren > Taal > Vertalen** opent het taakvenster **Vertalen**. Het vertaalt uw tekst op uw eigen computer. Uw woorden worden nergens naartoe gestuurd, en er is geen internetverbinding nodig zodra de vertaalbestanden voor een taal op uw computer staan.

## Tekst vertalen

1. Selecteer de tekst die u wilt vertalen. Als er niets is geselecteerd, gebruikt het taakvenster de alinea waarin de cursor staat. In een tekstvak dat is geselecteerd maar niet wordt bewerkt, gebruikt het de volledige tekst van het vak.
2. Klik op **Controleren > Taal > Vertalen**. Het taakvenster toont uw tekst onder **Origineel**.
3. Kies onder **Van** de taal waarin uw tekst is geschreven. Kies onder **Naar** de gewenste taal. Met de knop **Omwisselen** wisselt u beide talen om.
4. Klik op **Vertalen**. De vertaling verschijnt onder **Vertaling**. Vet, cursief en onderstreepte woorden blijven vet, cursief en onderstreept.
5. Klik op **Vervangen** om de vertaling op de plaats van uw tekst te zetten, of op **Onder invoegen** om haar in nieuwe alinea's onder uw tekst toe te voegen. Beide zijn één stap, dus **Ongedaan maken** haalt ze weer weg.

Als u de cursor verplaatst of de tekst wijzigt nadat u op **Vertalen** hebt geklikt, klik dan op **Selectie gebruiken** om de tekst opnieuw in te lezen, en klik daarna opnieuw op **Vertalen**. Het taakvenster zet geen vertaling over tekst die niet meer de tekst is die is vertaald.

Met **Annuleren** stopt u een vertaling die te lang duurt, en dat doet het sluiten van het taakvenster ook: het venster wacht er nooit op. Lange teksten duren langer: een pagina tekst duurt op de meeste computers enkele seconden. Het taakvenster vertaalt in één keer maximaal 1.000.000 letters (ongeveer 200 pagina's). Als u meer selecteert, meldt het taakvenster dat en vraagt het u minder te selecteren. Lange alinea's worden in stukken van ongeveer 1.500 letters geknipt, waarbij mogelijk aan het einde van een zin wordt gesplitst, en daarna weer samengevoegd.

## De eerste keer dat u een taal gebruikt

Voor een vertaling is per richting een set bestanden nodig, een model genoemd: één voor Engels naar Duits en een ander voor Duits naar Engels. Elk model is ongeveer 37 MB. De bestanden maken geen deel uit van JeffPub. De eerste keer dat u met een taal vertaalt, vertelt het taakvenster wat er nodig is en hoe groot het is, bijvoorbeeld "Duits downloaden (74 MB)?". Er wordt niets gedownload totdat u op die knop klikt. Het taakvenster laat zien hoe ver de download is, en met **Annuleren** stopt u de download, zonder dat er iets achterblijft.

JeffPub controleert elk bestand met een vingerafdruk (een SHA-256-controlegetal) voordat het wordt gebruikt. Een bestand dat niet overeenkomt, wordt weggegooid en de taal wordt dan niet geïnstalleerd.

De bestanden worden bewaard in één map die alle JeffOffice-apps delen (een map met de naam JeffOffice in uw gebruikersmap, in Translation/models). Een taal die u in de ene app downloadt, is daardoor ook in de andere apps beschikbaar, en wordt maar één keer gedownload. Twee talen die geen eigen model hebben, zoals Duits en Frans, lopen via het Engels, dus heeft het taakvenster beide modellen nodig.

## Wanneer een taal geen model heeft

Als voor een taalcombinatie geen model bestaat, meldt het taakvenster dat en biedt het de knop **In plaats daarvan openen in uw browser** aan. Daarmee opent u een gratis vertaalwebsite, LibreTranslate (of de service waarvan u het adres hebt gekozen in de instelling daarvoor), met uw tekst. Pas dan verlaat uw tekst uw computer, omdat ze naar die website gaat, en meldt het taakvenster niet langer dat de tekst op uw computer blijft. Er wordt niets verzonden totdat u op die knop klikt.

## Over de kwaliteit

De modellen zijn klein, dus ze werken snel op een gewone computer. Ze vertalen zinnen goed en korte labels minder goed: een enkel woord of de naam van een knop kan verkeerd uitkomen. Lees de vertaling voordat u haar gebruikt. Namen, getallen en tabs blijven zoals ze waren.

Afbeeldingen, tabs en regeleinden binnen een alinea blijven ongewijzigd. De vertaling vult de woorden ertussen in.

## Zie ook

- [Spelling, synoniemen, onderzoek en woordafbreking](proofing)
