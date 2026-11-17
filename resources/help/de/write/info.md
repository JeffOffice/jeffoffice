# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Betreff, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, DOCPROPERTY, als final markieren, final, schreibgeschützt, Kennwort, Passwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefinierte XML-Daten, Kompatibilität prüfen, Versionsverlauf, Versionen, Version speichern, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um die Angaben zu Ihrem Dokument zu sehen und es zu schützen, bevor Sie es weitergeben. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei liegt

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, etwa Titel und Verfasser. Die Seite enthält für jede ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie in einem Feld einen Text ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es enthält.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, etwa **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum) und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Code in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Nach einer Änderung der Eigenschaft drücken Sie **F9**, um das Feld zu aktualisieren. Siehe [Felder](fields).

## Als Final markieren

Klicken Sie auf **Als final markieren**, um Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wer es öffnet, sieht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, nichts zu ändern. Es ist kein Schutz, denn jeder kann **Als final markiert** erneut wählen, um das Dokument zu bearbeiten.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei ein Kennwort abgefragt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet die starke Verschlüsselung, die auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es an einem sicheren Ort auf.
- Die gespeicherten Versionen und die Sicherungskopie folgen dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie wieder auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren für **.docx**-Dateien und für **.odt**-Dateien. Eine .odt-Datei wird so geschützt, wie es der OpenDocument-Standard beschreibt, sodass andere Programme, die diesem Standard folgen, sie ebenfalls mit dem Kennwort öffnen können. Eine reine Textdatei oder eine .rtf-Datei kann kein Kennwort speichern. JeffWrite speichert ein geschütztes Dokument deshalb nicht in diesen Formaten, sondern meldet dies, statt es ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

JeffWrite öffnet auch eine alte **.doc**-Datei, die ein Kennwort verlangt. Die Abfrage erfolgt auf dieselbe Weise. JeffWrite speichert nie eine .doc-Datei. Daher fragt **Speichern unter** nach einem neuen Namen, und die .docx-Kopie behält dasselbe Kennwort.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument weitergeben. Die Prüfung zeigt, was die Datei enthält, das Sie vielleicht nicht weitergeben möchten, und wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung als Teil des Textes.
- **Dokumenteigenschaften und persönliche Informationen** sind Verfasser, die Person, die das Dokument zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text am oberen und unteren Seitenrand.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei geschrieben haben. Auf einer Seite sind sie nicht sichtbar.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu löschen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

**Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten sorgt dafür, dass dies bei jedem Speichern geschieht, für jede Datei, ohne das geöffnete Dokument zu verändern. Es ist dasselbe Kontrollkästchen wie unter **Optionen** und in jeder JeffOffice-App.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen**, um zu sehen, was ein älteres Dateiformat nicht speichern kann. JeffWrite speichert **.docx**- und **.odt**-Dateien. Eine .docx-Datei enthält alles aus Ihrem Dokument. Eine .odt-Datei kann einige Dinge nicht speichern; diese Liste zeigt, welche. Wird nichts aufgeführt, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht speichern kann, **bevor** etwas geschrieben wird. Sie können **Trotzdem speichern**, **Als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen. Aktivieren Sie **Für diese Datei nicht erneut fragen**, dann fragt JeffWrite bei dieser Datei nicht mehr nach, bis Sie das Programm schließen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Dinge zu öffnen, die manche Menschen schwer lesen oder nutzen können, etwa ein Bild ohne Beschreibung. Die Prüfung entspricht **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer zu speichern. Nichts wird irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in genau diesem Zustand. Legen Sie vor einer größeren Änderung eine Version an, damit Sie darauf zurückgreifen können. Das Dokument muss zuvor gespeichert sein, da Versionen unter dem Namen der Datei abgelegt werden.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst wird nicht verändert.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, wobei die Unterschiede als nachverfolgte Änderungen angezeigt werden. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument identisch mit der gewählten Version. JeffWrite speichert das Dokument zuvor als neue Version, damit nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite behält bis zu 50 Versionen einer Datei. Gibt es mehr, wird die älteste entfernt.
