# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Thema, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, DocProperty, als final markieren, final, schreibgeschützt, Kennwort, Passwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, Versionsverlauf, Versionen, Version behalten, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um die Angaben zu Ihrem Dokument zu sehen und es vor der Weitergabe zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei liegt

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, zum Beispiel der Titel und der Autor. Die Seite hat für jede Eigenschaft ein Feld: **Titel**, **Autor**, **Thema**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie in einem Feld etwas ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es hat.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, zum Beispiel **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum), und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Feldcode in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Nachdem Sie die Eigenschaft geändert haben, drücken Sie **F9**, um das Feld zu aktualisieren. Siehe [Felder](fields).

## Als Final markieren

Klicken Sie auf **Als final markieren**, um Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und hält es dann nur noch zum Lesen bereit. Wenn jemand es öffnet, zeigt die Titelleiste an, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, das Dokument nicht zu ändern. Es ist keine Sperre, denn jeder kann **Als final markiert** erneut wählen, um es zu bearbeiten.

## Immer schreibgeschützt öffnen

Klicken Sie auf **Immer schreibgeschützt öffnen**, damit das Dokument jedes Mal nur zum Lesen geöffnet wird und niemand es versehentlich ändert. Die Auswahl wird beim Speichern mit der Datei gespeichert. Wenn die Datei das nächste Mal geöffnet wird, zeigt die Statusleiste an, dass sie nur zum Lesen geöffnet ist. Zum Bearbeiten öffnen Sie **Datei > Info** und klicken auf **Wird immer schreibgeschützt geöffnet**. Dafür genügt ein Klick, deshalb ist dies eine Erinnerung und keine Sperre. Andere Programme lesen diese Auswahl nicht, und eine .odt-Datei kann sie nicht speichern. Speichern Sie das Dokument daher zuerst als .docx-Datei. Die Kompatibilitätsprüfung zeigt dies für eine .odt-Datei an.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei immer ein Kennwort abgefragt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet den starken Schutz, den auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es daher an einem sicheren Ort auf.
- Die gespeicherten Versionen und die Sicherungskopie folgen dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie erneut auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu entfernen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren für **.docx**-Dateien und für **.odt**-Dateien. Eine .odt-Datei wird so geschützt, wie es der OpenDocument-Standard beschreibt. Andere Programme, die diesem Standard folgen, können sie daher ebenfalls mit dem Kennwort öffnen. Eine einfache Textdatei oder eine .rtf-Datei kann kein Kennwort speichern. JeffWrite speichert ein geschütztes Dokument daher nicht in diesen Formaten. Stattdessen erhalten Sie eine Meldung, und das Dokument wird nicht ohne Kennwort gespeichert.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

JeffWrite öffnet auch eine ältere **.doc**-Datei, die ein Kennwort verlangt. Die Abfrage erfolgt auf die gleiche Weise. JeffWrite speichert nie eine .doc-Datei. Deshalb fragt **Speichern unter** nach einem neuen Namen, und die .docx-Kopie behält dasselbe Kennwort.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument an jemand anderen senden. Die Prüfung listet auf, was die Datei enthält, das Sie vielleicht nicht weitergeben möchten, und wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung als Teil des Textes.
- **Dokumenteigenschaften und persönliche Informationen** sind der Autor, die Person, die es zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text am oberen und unteren Seitenrand.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei geschrieben haben. Auf einer Seite sind sie nicht zu sehen.
- **Externe Verknüpfungen** sind Verweise auf andere Dateien, keine Webverknüpfungen: ein Bild oder ein Objekt, das nur verknüpft ist (es zeigt eine Datei, die nicht im Dokument enthalten ist), und ein Feld, das seinen Text oder sein Bild aus einer anderen Datei holt (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE und DDEAUTO). **Alle entfernen** wandelt jedes Feld in den Text um, den es jetzt anzeigt, entfernt jedes verknüpfte Bild und jedes verknüpfte Objekt und lässt die Verknüpfung beim Speichern aus der Datei heraus. Die anderen Dateien bleiben unverändert. Verknüpfungen in Kopf- und Fußzeilen sowie in Fußnoten werden nicht gezählt, und Webverknüpfungen bleiben erhalten.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu entfernen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Speichern Sie das Dokument anschließend, um die bereinigte Fassung zu behalten.

**Persönliche Informationen aus dieser Datei beim Speichern entfernen** unter den Karten sorgt dafür, dass dies bei jedem Speichern geschieht, für jede Datei, ohne das geöffnete Dokument zu ändern. Es ist dasselbe Kontrollkästchen wie unter **Optionen** und in jeder JeffOffice-App.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen** und wählen Sie dann einen Dateityp, um zu sehen, was er nicht speichern kann. JeffWrite speichert Dateien im Format **.docx**, **.odt**, **.rtf** und **.txt**. Eine .docx-Datei enthält alles aus Ihrem Dokument. Die anderen Formate können einige Dinge nicht speichern, und diese Liste nennt sie. Sie nennt außerdem die Einstellungen, die in der Datei selbst gespeichert werden, also Als final markieren und Immer schreibgeschützt öffnen, wenn sie aktiviert sind, sowie das Kennwort für ein Format, das keines speichern kann. Wird nichts aufgeführt, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht speichern kann, **bevor** etwas geschrieben wird. Sie können dann **Trotzdem speichern**, **Als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen. Aktivieren Sie **Für diese Datei nicht erneut fragen**, dann fragt JeffWrite bei dieser Datei nicht mehr nach, bis Sie das Programm schließen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Punkte zu öffnen, die für manche Menschen schwer zu lesen oder zu bedienen sein können, etwa ein Bild ohne Beschreibung. Es ist dieselbe Prüfung wie unter **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer zu behalten. Nichts wird irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in seinem damaligen Zustand. Legen Sie eine Version an, bevor Sie größere Änderungen vornehmen, damit Sie zu ihr zurückkehren können. Das Dokument muss vorher gespeichert sein, weil Versionen unter dem Dateinamen abgelegt werden.

- **Version behalten** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst wird nicht verändert.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, wobei die Unterschiede als nachverfolgte Änderungen angezeigt werden. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument gleich der gewählten Version. JeffWrite legt zuerst den bisherigen Stand des Dokuments als neue Version ab, damit nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite behält bis zu 50 Versionen einer Datei. Gibt es mehr, wird die älteste entfernt.
