# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: info, eigenschaften, titel, autor, betreff, stichwörter, kategorien, kommentare, firma, manager, benutzerdefinierte eigenschaften, docproperty, als final markieren, final, schreibgeschützt, kennwort, verschlüsseln, schützen, öffnungskennwort, dokument prüfen, persönliche informationen, ausgeblendeter text, benutzerdefiniertes xml, kompatibilität prüfen, versionsverlauf, versionen, version speichern, zurückkehren, wiederherstellen, versionen vergleichen, im ordner anzeigen, pfad kopieren, ordner, pfad -->

Klicken Sie auf **Datei > Info**, um Angaben zu Ihrem Dokument zu sehen und es vor der Weitergabe zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei gespeichert ist

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, etwa Titel und Verfasser. Die Seite enthält für jede Eigenschaft ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie in einem Feld einen Wert ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es enthält.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, etwa **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum) und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Code in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Nach einer Änderung der Eigenschaft drücken Sie **F9**, um das Feld zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um anderen mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wer es öffnet, sieht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, das Dokument nicht zu ändern. Es ist keine Sperre, denn jeder kann **Als final markiert** erneut wählen, um es zu bearbeiten.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei ein Kennwort abgefragt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet die starke Verschlüsselung, die auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **kann nicht wiederhergestellt werden**, wenn Sie es vergessen. Bewahren Sie es daher an einem sicheren Ort auf.
- Um das Kennwort zu ändern, klicken Sie erneut auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um den Schutz aufzuheben. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren bei **.docx**-Dateien. Eine **.odt**-Datei kann noch kein Kennwort speichern. Deshalb speichert JeffWrite ein geschütztes Dokument nicht als .odt. Stattdessen erhalten Sie einen Hinweis, statt ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument weitergeben. Es listet auf, was die Datei enthält und was Sie möglicherweise nicht weitergeben möchten, und zeigt an, wie viele davon gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung als Teil des Textes.
- **Dokumenteigenschaften und persönliche Informationen** sind der Autor, die Person, die das Dokument zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text oben und unten auf den Seiten.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei eingefügt haben. Auf einer Seite sind sie nicht sichtbar.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie aus dem Dokument zu löschen. Änderungen am Text machen Sie mit **Strg+Z** rückgängig. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen**, um zu sehen, was ein älteres Dateiformat nicht speichern kann. JeffWrite speichert **.docx**- und **.odt**-Dateien. Eine .docx-Datei enthält alles aus Ihrem Dokument. Eine .odt-Datei kann einige Dinge nicht speichern; diese Liste zeigt, welche. Wird nichts aufgeführt, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht speichern kann, **bevor** etwas geschrieben wird. Sie können dann **Trotzdem als .odt speichern**, **Stattdessen als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen.

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer aufzubewahren. Es wird nichts irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in genau diesem Moment. Legen Sie vor einer größeren Änderung eine Version an, damit Sie später dorthin zurückkehren können. Das Dokument muss vorher gespeichert sein, denn Versionen werden unter dem Namen der Datei abgelegt.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst ändert sich dabei nicht.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar; die Unterschiede werden als nachverfolgte Änderungen angezeigt. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument der gewählten Version gleich. JeffWrite bewahrt das bisherige Dokument zuvor als neue Version auf, sodass nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite bewahrt bis zu 50 Versionen einer Datei auf und entfernt die älteste, wenn es mehr sind.
