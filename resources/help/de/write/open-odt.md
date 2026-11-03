# OpenDocument-Dateien öffnen und speichern
<!-- keywords: odt, fodt, ott, opendocument, open document, öffnen, speichern, speichern unter, Format, Dateiformat, Dateityp, andere Programme, Dokumente austauschen, weitergeben, Kompatibilität, Änderungen nachverfolgen, nachverfolgte Änderungen, ausgelassen, Vorlage -->

JeffWrite öffnet und speichert OpenDocument-Text, also Dateien, deren Name auf **.odt** endet. Viele kostenlose Programme können dieses Format lesen und schreiben. Es eignet sich daher gut, um ein Dokument mit jemandem auszutauschen, der JeffWrite nicht verwendet.

## OpenDocument-Datei öffnen

Drücken Sie **Strg+O**, oder klicken Sie auf **Datei > Öffnen**, und wählen Sie eine Datei aus:

- **.odt** ist ein Dokument.
- **.ott** ist eine Vorlage. Sie wird als neues Dokument geöffnet, das auf der Vorlage basiert. Die Vorlage selbst bleibt unverändert.
- **.fodt** ist dieselbe Art von Dokument, gespeichert in einer einzigen XML-Datei. JeffWrite schreibt keine FODT-Dateien: **Speichern** fragt bei einer solchen Datei nach einem neuen Namen und speichert sie als ODT-Datei.

JeffWrite erkennt den Dateityp am Inhalt der Datei, nicht an ihrem Namen. Daher wird auch eine Datei mit falscher Endung geöffnet. Sie können den Dateinamen auch beim Starten von JeffWrite angeben oder die Datei aus Ihrem Dateimanager auf das Programm ziehen, wie bei einer DOCX-Datei.

JeffWrite liest den Text und seine Formatierung, Überschriften, Formatvorlagen und die Formatvorlagen, auf denen sie basieren, Listen mit Aufzählungszeichen und nummerierte Listen, Tabellen (mit verbundenen Zellen, Spaltenbreiten, Rahmen, Schattierung und verschachtelten Tabellen), Abschnitte und Spalten, Seitengröße, Seitenränder und Textrichtung, Kopf- und Fußzeilen (erste Seite, linke und rechte Seiten), Fuß- und Endnoten, Felder (Seitenzahlen, Seitenanzahl, Titel, Autor, Dateiname, Querverweise und fortlaufende Nummern), Textmarken, Links, ein Inhaltsverzeichnis, Bilder, Textfelder, Tabstopps, Kommentare und nachverfolgte Änderungen. Diese werden als Markierungen angezeigt, die Sie annehmen oder ablehnen können.

Einige Elemente kann JeffWrite noch nicht anzeigen: gezeichnete Formen ohne Text, Formeln, Diagramme und andere in den Text eingebettete Objekte. Enthält eine Datei solche Elemente, teilt JeffWrite Ihnen beim Öffnen mit, welche es sind. JeffWrite überschreibt diese Datei dann nicht: **Speichern** fragt nach einem neuen Namen, damit das Original behält, was in der Kopie fehlt. Kennwortgeschützte Dokumente werden nicht geöffnet. Öffnen Sie ein solches Dokument in dem Programm, mit dem es erstellt wurde, und speichern Sie eine Kopie ohne Kennwort.

## OpenDocument-Datei speichern

Klicken Sie auf **Datei > Speichern unter**, und wählen Sie in der Liste der Dateitypen **OpenDocument-Text (*.odt)** aus. Danach speichert **Strg+S** wieder im selben Format.

Die Datei enthält alles oben Genannte, einschließlich der Kommentare und nachverfolgten Änderungen: eingefügter und gelöschter Text, geänderte Formatierung sowie eingefügte oder gelöschte Absatzumbrüche. Für einige Elemente bietet das Format keinen Platz. JeffWrite nennt sie nach dem Speichern: Sie werden in ihrem aktuellen Zustand gespeichert oder ausgelassen. Dazu gehören nachverfolgte Änderungen an Tabellen, Spalten und der Seiteneinrichtung sowie nachverfolgte Änderungen in Kopf- und Fußzeilen und Notizen (im aktuellen Zustand gespeichert), Formularfelder (als angezeigter Text gespeichert), Formeln (als Nur-Text gespeichert) sowie Zeichnungen, Formen, Diagramme und eingebettete Objekte, die nicht beibehalten werden können. Wenn Sie nach einer Änderung erneut speichern und dabei etwas Neues ausgelassen wird, weist JeffWrite Sie auch darauf hin. Über bereits Gemeldetes informiert JeffWrite nicht erneut.

Andere Programme, die OpenDocument lesen, öffnen die Datei, ohne eine Reparatur anzubieten.

## Verwandte Themen

- [Öffnen, Speichern, Drucken und Exportieren](open-save)
- [Änderungen nachverfolgen und annehmen oder ablehnen](track-changes)
- [Willkommen bei JeffWrite](start)
