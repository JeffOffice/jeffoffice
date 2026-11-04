# Info: Eigenschaften, Kennwörter und Dokument prüfen
<!-- keywords: Info, Eigenschaften, Titel, Autor, Betreff, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, Dokumenteigenschaft, als final markieren, final, schreibgeschützt, Kennwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um Angaben zu Ihrem Dokument zu sehen und es vor der Weitergabe zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei liegt

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, zum Beispiel der Titel und der Autor. Die Seite enthält für jede Eigenschaft ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie in einem Feld einen Text ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es hat.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, zum Beispiel **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum), und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Code in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Drücken Sie **F9**, um ihn nach einer Änderung der Eigenschaft zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wenn jemand es öffnet, zeigt die Titelleiste an, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, das Dokument nicht zu ändern. Es ist keine Sperre, denn jeder kann **Als final markiert** erneut wählen, um es zu bearbeiten.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei ein Kennwort abgefragt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet die starke Verschlüsselung, die auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es an einem sicheren Ort auf.
- Um das Kennwort zu ändern, klicken Sie erneut auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren bei **.docx**-Dateien. Eine **.odt**-Datei kann noch kein Kennwort speichern. Daher speichert JeffWrite ein geschütztes Dokument nicht als .odt. Stattdessen erhalten Sie eine Meldung, anstatt ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument weitergeben. Es listet auf, was die Datei enthält, das Sie vielleicht nicht weitergeben möchten, und wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung in den Text.
- **Dokumenteigenschaften und persönliche Informationen** sind der Autor, die Person, die das Dokument zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text am oberen und unteren Seitenrand.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei eingefügt haben. Auf einer Seite sind sie nicht zu sehen.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu löschen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Speichern Sie das Dokument anschließend, damit die bereinigte Fassung erhalten bleibt.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen**, um zu sehen, was ein älteres Dateiformat nicht speichern kann. JeffWrite speichert **.docx**- und **.odt**-Dateien. Eine .docx-Datei enthält alles aus Ihrem Dokument. Eine .odt-Datei kann einige Dinge nicht speichern; diese Liste zeigt, welche. Wird nichts aufgelistet, geht nichts verloren.
