# Afspelen-tabblad

<!-- keywords: afspelen-tabblad, lint, film, video, geluid, audio, afspelen, onderbreken, voorbeeld, bijknippen, ondertiteling, bijschriften, invoegen bijschriften, bijschriften verwijderen, ondertitels, webvtt, vtt, volume, dempen, starten, in klikvolgorde, automatisch, bij klikken, afspelen op volledig scherm, verborgen bij niet afspelen, herhalen tot gestopt, terugspoelen na afspelen, hulpmiddelen voor media -->

Op het tabblad **Afspelen** stelt u in hoe een film of geluid wordt afgespeeld, en voegt u ondertiteling toe aan een film. Het tabblad wordt alleen getoond terwijl één film of één geluid is geselecteerd, en verdwijnt zodra u iets anders selecteert.

- **Voorbeeld** bevat **Afspelen**. Hiermee speelt u de film of het geluid op de dia af, zodat u het kunt zien en horen. Terwijl het wordt afgespeeld, staat op de knop **Onderbreken**.
- **Bewerken** bevat **Bijknippen**. Hiermee opent u een vak met **Begintijd** en **Eindtijd**, in seconden. De film of het geluid wordt alleen afgespeeld van de begintijd tot de eindtijd. De rest van het bestand blijft in de presentatie aanwezig. Wilt u weer tot het einde van het bestand afspelen, stel dan de eindtijd in op de volledige lengte.
  - **Bijschriften invoegen** voegt bijschriften toe aan een film, op basis van een WebVTT-bestand (een tekstbestand dat eindigt op `.vtt`). Daarna wordt gevraagd naar de taal van de bijschriften, als korte code, zoals `en`, `es` of `pt-BR`. Bijschriften horen bij een film die in de presentatie is opgenomen; bij een film die alleen van buitenaf is gekoppeld, is de knop grijs. Een film heeft per taal één set bijschriften: als u voor dezelfde taal een tweede bestand invoegt, wordt het eerste vervangen. Een bestand dat geen WebVTT is, geen bijschriften bevat of te groot is (het maximum is 4 MB, 20.000 bijschriften en 2.000 tekens in één bijschrift), wordt met een melding geweigerd, en er verandert niets.
  - **Bijschriften verwijderen** haalt de bijschriften uit de film. Heeft de film bijschriften in meer dan één taal, dan wordt gevraagd welke u wilt verwijderen, of alle.
- **Mediaopties** bepaalt hoe de film of het geluid in de diavoorstelling wordt afgespeeld:
  - **Volume** is **Zacht**, **Gemiddeld**, **Hard** of **Dempen**.
  - **Starten** bepaalt wat het afspelen start. **In klikvolgorde** speelt de film af bij een klik, samen met de animaties van de dia. **Automatisch** speelt de film vanzelf af wanneer de beurt eraan is. **Bij klikken** speelt de film af wanneer u op de film of het geluid klikt.
  - **Afspelen op volledig scherm** laat een film het hele scherm vullen terwijl hij wordt afgespeeld. Dit werkt alleen voor films.
  - **Verborgen bij niet afspelen** verbergt het beeld totdat de film of het geluid wordt afgespeeld, en weer nadat het is gestopt. Het luidsprekerpictogram van een geluid is de hele tijd verborgen.
  - **Herhalen tot gestopt** speelt de film opnieuw af telkens wanneer hij eindigt, totdat hij wordt gestopt of de dia wordt gewisseld.
  - **Terugspoelen na afspelen** zet een film na afloop terug op het eerste beeld. Zonder deze optie blijft de film op het laatste beeld staan.

Bijschriften worden over de onderkant van de film getoond, in witte tekst op een donkere balk, terwijl de film wordt afgespeeld in de diavoorstelling en in de bewerkingsweergave. Ze zijn op de film afgestemd, en hun grootte volgt de grootte van de film. Heeft een film meer dan één taal, dan wordt de eerste getoond. Bijschriften worden samen met de presentatie opgeslagen als onderdeel van het `.pptx`-bestand, zodat ze er weer zijn wanneer u het opnieuw opent. Een bijschriftbestand in een ander formaat (geen WebVTT) blijft ongewijzigd bewaard en wordt ongewijzigd opnieuw opgeslagen, maar het wordt niet getoond. Zodra een film bijschriften heeft, staat hij niet meer in **Toegankelijkheidscontrole**.

Elke wijziging is één stap. Met **Ctrl+Z** maakt u haar ongedaan.

Een geluid of een film die over meerdere dia's wordt afgespeeld, wat sommige bestanden instellen, blijft afspelen zolang het bestand aangeeft. JeffDeck speelt het zo af in de diavoorstelling en behoudt de instelling bij het opslaan.

Zie [Vormen, afbeeldingen, tabellen en Tekstopmaak](objects) en [De diavoorstelling afspelen en Presentatorweergave gebruiken](show).

## Gerelateerd

- [Vormen, afbeeldingen, tabellen en Tekstopmaak](objects)
- [Tabblad Invoegen](tab-insert)
- [De diavoorstelling afspelen en Presentatorweergave gebruiken](show)
