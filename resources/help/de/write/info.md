# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Betreff, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, Dokumenteigenschaft, als final markieren, final, schreibgeschützt, Kennwort, Passwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, Versionsverlauf, Versionen, Version speichern, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um Angaben zu Ihrem Dokument zu sehen und es vor der Weitergabe zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei gespeichert ist

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, etwa Titel und Verfasser. Die Seite enthält für jede Eigenschaft ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie in einem Feld etwas ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es hat.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, etwa **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum) und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld. Ändern Sie den Code in `DOCPROPERTY "Client"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Drücken Sie **F9**, um das Feld nach einer Änderung der Eigenschaft zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wer es öffnet, sieht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, das Dokument nicht zu ändern. Es ist keine Sperre, denn jeder kann **Als final markiert** erneut wählen, um es zu bearbeiten.

## Immer schreibgeschützt öffnen

Klicken Sie auf **Immer schreibgeschützt öffnen**, damit das Dokument jedes Mal nur zum Lesen geöffnet wird und niemand es versehentlich ändert. Die Auswahl wird beim Speichern mit der Datei gespeichert. Wenn die Datei das nächste Mal geöffnet wird, zeigt die Statusleiste an, dass sie nur zum Lesen geöffnet ist. Zum Bearbeiten öffnen Sie **Datei > Info** und klicken auf **Immer schreibgeschützt öffnen**. Das geht mit einem Klick, daher ist dies eine Erinnerung und keine Sperre. Andere Programme lesen diese Einstellung nicht, und eine .odt-Datei kann sie nicht speichern. Speichern Sie das Dokument deshalb zuerst als .docx-Datei. Die Kompatibilitätsprüfung listet sie für eine .odt-Datei auf.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei jedes Mal ein Kennwort verlangt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet die starke Schutzmethode, die auch andere Textverarbeitungsprogramme nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es an einem sicheren Ort auf.
- Die gespeicherten Versionen und die Sicherungskopie folgen dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie erneut auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren für **.docx**-Dateien und für **.odt**-Dateien. Eine .odt-Datei wird so geschützt, wie es der OpenDocument-Standard beschreibt, sodass andere Programme, die den Standard unterstützen, sie ebenfalls mit dem Kennwort öffnen können. Eine reine Textdatei oder eine .rtf-Datei kann kein Kennwort speichern. JeffWrite speichert ein geschütztes Dokument deshalb nicht in diesen Formaten. Stattdessen erhalten Sie eine Meldung, anstatt ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

JeffWrite öffnet auch eine alte **.doc**-Datei, die ein Kennwort verlangt. Die Abfrage erfolgt auf dieselbe Weise. JeffWrite speichert niemals eine .doc-Datei. Daher fragt **Speichern unter** nach einem neuen Namen, und die .docx-Kopie behält dasselbe Kennwort.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument an jemand anderen senden. Die Prüfung listet auf, was die Datei enthält, das Sie vielleicht nicht weitergeben möchten, und wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung in den Text.
- **Dokumenteigenschaften und persönliche Informationen** sind der Autor, die Person, die das Dokument zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text oben und unten auf den Seiten.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme der Datei hinzugefügt haben. Auf der Seite sind sie nicht sichtbar.
- **Externe Verknüpfungen** sind Verweise auf andere Dateien, nicht Weblinks: ein Bild oder ein Objekt, das nur verknüpft ist (es zeigt eine Datei, die nicht im Dokument enthalten ist), sowie ein Feld, das seinen Text oder sein Bild aus einer anderen Datei bezieht (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE und DDEAUTO). **Alle entfernen** wandelt jedes Feld in den Text um, den es jetzt anzeigt, entfernt jedes verknüpfte Bild und jedes verknüpfte Objekt und lässt die Verknüpfung beim Speichern aus der Datei weg. Die anderen Dateien werden nicht verändert. Verknüpfungen in Kopf- und Fußzeilen, Fuß- und Endnoten werden ebenfalls gezählt und entfernt. Weblinks bleiben erhalten. Auch Verknüpfungen in Kommentaren, Textfeldern und den Teilen, die das Programm unverändert belässt (etwa der Text von Zeichnungen), werden gesucht. Ein verknüpftes Bild, das zusätzlich eine eigene Kopie des Bildes enthält, bleibt als Bild ohne Verknüpfung erhalten, und die Verknüpfung wird beim Speichern aus der Datei entfernt. Das Entfernen von Verknüpfungen in Kommentaren und anderen unverändert belassenen Teilen kann nicht rückgängig gemacht werden; die Textfelder sind ein Schritt von Rückgängig. Die Vorlage, an die das Dokument angehängt ist (ihr Pfad enthält oft einen Benutzernamen), zählt ebenfalls als externe Verknüpfung: **Alle entfernen** löst sie, und das Dokument behält seine eigenen Formatvorlagen. Andere Verknüpfungen zu Dateien werden nicht entfernt. Eine Zeile unter der Anzahl nennt sie: die Datenquelle des Seriendrucks, Rahmen, Teildokumente in anderen Teilen, externe Daten eines Diagramms, verknüpfte Filme und Töne sowie verknüpfte Formatvorlagendateien. Dieselbe Zeile zählt auch andere Verknüpfungen in einem Feld, einem Kommentar, einem Textfeld oder der Verknüpfungsliste der Datei, die im Dokument nicht verwendet werden, und sie sagt es auch, wenn die Adresse eines verknüpften Bildes nicht in der Datei steht oder ein Teil in einer Codierung vorliegt, die die Prüfung nicht lesen kann. Die Zeile lautet dann „Nicht vollständig geprüft“ statt „Nichts gefunden“. Ein Durchlauf von **Alle entfernen** entfernt höchstens 100.000 Verknüpfungen in beibehaltenen Teilen; bleiben weitere übrig, nennt die Zeile deren Anzahl, und Sie können **Alle entfernen** erneut ausführen.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu entfernen. Änderungen am Text machen Sie mit **Strg+Z** rückgängig. Verknüpfungen, die aus Kommentaren und anderen beibehaltenen Teilen entfernt wurden, sowie benutzerdefinierte XML-Daten lassen sich nicht rückgängig machen. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

**Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten sorgt dafür, dass dies bei jedem Speichern geschieht, für jede Datei, ohne das geöffnete Dokument zu ändern. Es ist dasselbe Kontrollkästchen wie in **Optionen** und in jeder JeffOffice-App.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen** und wählen Sie einen Dateityp, um zu sehen, was er nicht speichern kann. JeffWrite speichert **.docx**-, **.odt**-, **.rtf**- und **.txt**-Dateien. Eine .docx-Datei enthält alles aus Ihrem Dokument. Die anderen Typen können einige Dinge nicht speichern, und diese Liste zeigt, welche. Sie nennt auch die Einstellungen, die in der Datei selbst gespeichert werden, also Als final markieren und Immer schreibgeschützt öffnen, wenn sie aktiv sind, sowie das Kennwort für einen Typ, der keines speichern kann. Wird nichts aufgeführt, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht enthalten kann, **bevor** etwas geschrieben wird. Sie können **Trotzdem speichern**, **Als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen. Aktivieren Sie **Für diese Datei nicht erneut fragen**, und JeffWrite fragt bei dieser Datei nicht mehr nach, bis Sie das Programm schließen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Punkte zu öffnen, die für manche Menschen schwer zu lesen oder zu bedienen sein können, etwa ein Bild ohne Beschreibung. Es ist dieselbe Prüfung wie **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer aufzubewahren. Nichts wird irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in dem Zustand, in dem es in diesem Moment ist. Legen Sie vor einer größeren Änderung eine Version an, damit Sie zu ihr zurückkehren können. Das Dokument muss zuerst gespeichert sein, weil Versionen unter dem Namen der Datei abgelegt werden.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst wird dabei nicht verändert.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, wobei die Unterschiede als nachverfolgte Änderungen angezeigt werden. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument identisch mit der gewählten Version. JeffWrite sichert das bisherige Dokument zuvor als neue Version, sodass nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite behält bis zu 50 Versionen einer Datei und verwirft die älteste, wenn es mehr werden.
