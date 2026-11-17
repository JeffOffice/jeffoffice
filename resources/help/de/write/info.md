# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Betreff, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, DOCPROPERTY, als final markieren, final, schreibgeschützt, Kennwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, Versionsverlauf, Versionen, Version speichern, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um die Angaben zu Ihrem Dokument zu sehen und es zu schützen, bevor Sie es weitergeben. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei liegt

Oben auf der Seite steht, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, etwa der Titel und der Verfasser. Die Seite hat für jede ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie in einem Feld Text ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es hat.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, etwa **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum), und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Code in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Drücken Sie **F9**, um ihn nach einer Änderung der Eigenschaft zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um den Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Beim Öffnen steht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis sagt, dass es als final markiert wurde. Das ist ein freundlicher Hinweis, etwa in dem Sinne: „Bitte nicht ändern.“ Es ist keine Sperre, denn jeder kann **Als final markiert** erneut wählen, um es zu bearbeiten.

## Immer schreibgeschützt öffnen

Klicken Sie auf **Immer schreibgeschützt öffnen**, damit das Dokument jedes Mal nur zum Lesen geöffnet wird und niemand es versehentlich ändert. Die Auswahl wird beim Speichern zusammen mit der Datei gespeichert. Wenn die Datei das nächste Mal geöffnet wird, zeigt die Statusleiste an, dass sie nur zum Lesen geöffnet ist. Zum Bearbeiten öffnen Sie **Datei > Info** und klicken auf **Immer schreibgeschützt öffnen**. Das geht mit einem Klick, daher ist dies eine Erinnerung und keine Sperre. Andere Programme lesen diese Auswahl nicht, und eine .odt-Datei kann sie nicht speichern. Speichern Sie das Dokument deshalb zuerst als .docx-Datei. Die Kompatibilitätsprüfung führt sie für eine .odt-Datei auf.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, um beim Öffnen der Datei jedes Mal nach einem Kennwort zu fragen. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet den starken Schutz, den auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es sicher auf.
- Die gespeicherten Versionen und die Sicherungskopie richten sich nach dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie wieder auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren für **.docx**-Dateien und für **.odt**-Dateien. Eine .odt-Datei wird so geschützt, wie es der OpenDocument-Standard beschreibt, sodass auch andere Programme, die diesem Standard folgen, sie mit dem Kennwort öffnen können. Eine reine Textdatei oder eine .rtf-Datei kann kein Kennwort speichern. JeffWrite speichert ein geschütztes Dokument deshalb nicht in diesem Format, sondern zeigt einen Hinweis an, statt ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

JeffWrite öffnet auch eine alte **.doc**-Datei, die ein Kennwort verlangt. Auch dabei wird nach dem Kennwort gefragt. JeffWrite speichert nie eine .doc-Datei. Beim Aufruf von **Speichern unter** fragt das Programm daher nach einem neuen Namen, und die .docx-Kopie behält dasselbe Kennwort.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument an jemand anderen senden. Die Prüfung listet auf, was die Datei enthält, das Sie vielleicht nicht weitergeben möchten, und wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung in den Text.
- **Dokumenteigenschaften und persönliche Informationen** sind Verfasser, die Person, die es zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, befindet sich aber in der Datei.
- **Kopf- und Fußzeilen** sind der Text oben und unten auf den Seiten.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei geschrieben haben. Auf einer Seite sind sie nicht sichtbar.
- **Externe Verknüpfungen** sind Verweise auf andere Dateien, nicht Weblinks: ein Bild oder ein Objekt, das nur verknüpft ist (es zeigt eine Datei, die nicht im Dokument enthalten ist), sowie ein Feld, das seinen Text oder sein Bild aus einer anderen Datei bezieht (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE und DDEAUTO). **Alle entfernen** wandelt jedes Feld in den Text um, den es jetzt anzeigt, entfernt jedes verknüpfte Bild und Objekt und lässt die Verknüpfung beim Speichern aus der Datei heraus. Die anderen Dateien werden nicht verändert. Verknüpfungen in Kopf- und Fußzeilen sowie in Fußnoten werden nicht gezählt, und Weblinks bleiben erhalten.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu entfernen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

Die Option **Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten führt diese Bereinigung bei jedem Speichern aus, für jede Datei, ohne das geöffnete Dokument zu verändern. Es ist dasselbe Kontrollkästchen wie in **Optionen** und in jeder JeffOffice-App.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen**, um zu sehen, was ein älteres Dateiformat nicht speichern kann. JeffWrite speichert **.docx**- und **.odt**-Dateien. Eine .docx-Datei enthält alles aus Ihrem Dokument. Eine .odt-Datei kann einige Dinge nicht speichern, und diese Liste zeigt, welche. Wenn nichts aufgeführt ist, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht speichern kann, **bevor** etwas geschrieben wird. Sie können **Trotzdem speichern**, **Als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen. Aktivieren Sie **Für diese Datei nicht erneut fragen**, dann fragt JeffWrite bei dieser Datei nicht mehr nach, bis Sie das Programm schließen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Punkte zu öffnen, die manche Menschen schwer lesen oder bedienen können, etwa ein Bild ohne Beschreibung. Es ist dieselbe Prüfung wie **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer aufzubewahren. Es wird nichts irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in dem Zustand, den es in diesem Moment hat. Legen Sie vor einer größeren Änderung eine Version an, damit Sie zu ihr zurückkehren können. Das Dokument muss vorher gespeichert sein, weil Versionen unter dem Dateinamen abgelegt werden.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst wird nicht verändert.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, wobei die Unterschiede als nachverfolgte Änderungen angezeigt werden. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument so wie die gewählte Version. JeffWrite legt zuvor den bisherigen Stand des Dokuments als neue Version ab, damit nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite behält bis zu 50 Versionen einer Datei. Sind es mehr, werden die ältesten entfernt.
