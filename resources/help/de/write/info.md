# Info: Eigenschaften, Kennwörter und Dokument prüfen
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Thema, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, DocProperty, als final markieren, final, schreibgeschützt, Kennwort, Passwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, Versionsverlauf, Versionen, Version speichern, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um die Angaben zu Ihrem Dokument zu sehen und es vor der Weitergabe zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Speicherort der Datei

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, zum Beispiel der Titel und der Autor. Die Seite enthält für jede Eigenschaft ein Feld: **Titel**, **Autor**, **Thema**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie etwas in ein Feld ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es enthält.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, zum Beispiel **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum), und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Code in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Drücken Sie **F9**, um ihn nach einer Änderung der Eigenschaft zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um anderen mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wer es öffnet, sieht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, das Dokument nicht zu ändern. Es ist kein Schloss, denn jeder kann **Als final markiert** erneut wählen, um es zu bearbeiten.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei ein Kennwort abgefragt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet die starke Verschlüsselung, die auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es an einem sicheren Ort auf.
- Die gespeicherten Versionen und die Sicherungskopie folgen dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie erneut auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren bei **.docx**-Dateien. Eine **.odt**-Datei kann noch kein Kennwort enthalten. Deshalb speichert JeffWrite ein geschütztes Dokument nicht als .odt. Stattdessen erhalten Sie einen Hinweis, statt ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument weitergeben. Es listet auf, was die Datei enthält und was Sie unter Umständen nicht weitergeben möchten, und zeigt an, wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. Mit **Alle annehmen** werden alle Änderungen Teil des Textes.
- **Dokumenteigenschaften und persönliche Informationen** sind der Autor, die Person, die das Dokument zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text am oberen und unteren Seitenrand.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in der Datei hinterlegt haben. Auf der Seite sind sie nicht sichtbar.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu löschen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen**, um zu sehen, was ein älteres Dateiformat nicht speichern kann. JeffWrite speichert Dateien im Format **.docx** und **.odt**. Eine .docx-Datei enthält alles aus Ihrem Dokument. Eine .odt-Datei kann einige Dinge nicht speichern, und diese Liste nennt sie. Wird nichts aufgelistet, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht speichern kann, **bevor** etwas geschrieben wird. Sie können dann **Trotzdem als .odt speichern**, **Stattdessen als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Punkte zu öffnen, die für manche Menschen schwer zu lesen oder zu bedienen sein können, zum Beispiel ein Bild ohne Beschreibung. Es ist dieselbe Prüfung wie unter **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer aufzubewahren. Nichts wird irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in dem Zustand, in dem es in diesem Moment ist. Legen Sie vor einer größeren Änderung eine Version an, damit Sie darauf zurückgreifen können. Das Dokument muss vorher gespeichert sein, weil die Versionen unter dem Dateinamen abgelegt werden.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Als Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst ändert sich nicht.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, wobei die Unterschiede als nachverfolgte Änderungen angezeigt werden. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument identisch mit der gewählten Version. JeffWrite speichert das bisherige Dokument zuvor als neue Version, sodass nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite behält bis zu 50 Versionen einer Datei. Gibt es mehr, wird die älteste entfernt.
