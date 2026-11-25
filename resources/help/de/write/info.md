# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Betreff, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, Dokumenteigenschaft, als final markieren, final, schreibgeschützt, Kennwort, Passwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, Versionsverlauf, Versionen, Version speichern, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um die Angaben zu Ihrem Dokument zu sehen und es vor der Weitergabe zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Wo die Datei gespeichert ist

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, zum Beispiel sein Titel und wer es geschrieben hat. Die Seite enthält für jede Eigenschaft ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Tippen Sie in ein Feld und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es hat.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, zum Beispiel **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, ganze Zahl, Zahl, Ja oder Nein oder Datum) und tragen Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld** und wählen Sie ein Feld. Ändern Sie den Code in `DOCPROPERTY "Kunde"` und klicken Sie auf **OK**. Der Text zeigt den Wert an. Drücken Sie **F9**, um ihn nach einer Änderung der Eigenschaft zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wer es öffnet, sieht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, nichts zu ändern. Es ist kein Schloss, denn jeder kann **Als final markiert** erneut wählen, um das Dokument wieder zu bearbeiten.

## Immer schreibgeschützt öffnen

Klicken Sie auf **Immer schreibgeschützt öffnen**, damit das Dokument jedes Mal nur zum Lesen geöffnet wird und niemand es versehentlich ändert. Die Einstellung wird beim Speichern mit der Datei gespeichert. Beim nächsten Öffnen der Datei zeigt die Statusleiste an, dass sie nur zum Lesen geöffnet ist. Um sie zu bearbeiten, öffnen Sie **Datei > Info** und klicken Sie auf **Immer schreibgeschützt öffnen**. Das geht mit einem Klick; die Einstellung ist also eine Erinnerung und kein Schutz. Andere Programme lesen diese Einstellung nicht, und eine .odt-Datei kann sie nicht speichern. Speichern Sie das Dokument deshalb zuerst als .docx-Datei. **Kompatibilität prüfen** führt sie für eine .odt-Datei auf.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei jedes Mal ein Kennwort abgefragt wird. Geben Sie das Kennwort zweimal ein und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet den starken Schutz, den auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es deshalb an einem sicheren Ort auf.
- Die gespeicherten Versionen und die Sicherungskopie folgen dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie wieder auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren für **.docx**-Dateien und für **.odt**-Dateien. Eine .odt-Datei wird so geschützt, wie der OpenDocument-Standard es beschreibt. Andere Programme, die diesem Standard folgen, können sie deshalb ebenfalls mit dem Kennwort öffnen. Eine Textdatei oder eine .rtf-Datei kann kein Kennwort speichern. JeffWrite speichert ein geschütztes Dokument daher nicht in diesem Format, sondern zeigt einen Hinweis an, statt es ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

JeffWrite öffnet auch eine alte **.doc**-Datei, die ein Kennwort verlangt. Die Abfrage erfolgt auf dieselbe Weise. JeffWrite speichert nie eine .doc-Datei. Deshalb fragt **Speichern unter** nach einem neuen Namen, und die .docx-Kopie behält dasselbe Kennwort.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument an jemand anderen senden. Die Prüfung listet auf, was die Datei enthält, das Sie möglicherweise nicht weitergeben möchten, und wie viele Einträge jeder Art gefunden wurden:

- **Kommentare** sind Notizen, die Personen am Seitenrand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung in den Text.
- **Dokumenteigenschaften und persönliche Informationen** sind Autor, die Person, die es zuletzt gespeichert hat, die Firma und der Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text am oberen und unteren Seitenrand.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei geschrieben haben. Auf einer Seite sind sie nicht sichtbar.
- **Externe Verknüpfungen** sind Verknüpfungen zu anderen Dateien, nicht Weblinks: ein Bild oder ein Objekt, das nur verknüpft ist (es zeigt eine Datei, die nicht im Dokument enthalten ist), sowie ein Feld, das seinen Text oder sein Bild aus einer anderen Datei bezieht (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE und DDEAUTO). **Alle entfernen** wandelt jedes Feld in den Text um, den es jetzt anzeigt, entfernt jedes verknüpfte Bild oder Objekt und lässt die Verknüpfung beim Speichern aus der Datei heraus. Die anderen Dateien bleiben unverändert. Verknüpfungen in Kopf- und Fußzeilen, Fußnoten und Endnoten werden ebenfalls gezählt und entfernt. Weblinks bleiben erhalten. Auch Verknüpfungen in Kommentaren, Textfeldern und in Teilen, die das Programm unverändert beibehält (etwa der Text von Zeichnungen), werden gesucht. Ein verknüpftes Bild, das zusätzlich eine eigene Kopie des Bildes enthält, bleibt als Bild ohne Verknüpfung erhalten; die Verknüpfung wird beim Speichern aus der Datei entfernt. Das Entfernen von Verknüpfungen in Kommentaren und anderen unverändert beibehaltenen Teilen kann nicht rückgängig gemacht werden. Die Textfelder sind ein Schritt von **Rückgängig**. Die Vorlage, an die das Dokument angehängt ist (ihr Pfad enthält oft einen Benutzernamen), zählt ebenfalls als externe Verknüpfung: **Alle entfernen** löst sie, und das Dokument behält seine eigenen Formatvorlagen. Andere Verknüpfungen zu Dateien werden nicht entfernt. Eine Zeile unter der Anzahl nennt sie: die Datenquelle des Seriendrucks, Rahmen, Teildokumente in anderen Teilen, externe Daten eines Diagramms, verknüpfte Filme und Töne sowie verknüpfte Formatvorlagendateien. Dieselbe Zeile zählt auch jede andere Verknüpfung in einem Feld, einem Kommentar, einem Textfeld oder in der Liste der Verknüpfungen der Datei, die im Dokument nicht verwendet wird, und sagt dies ausdrücklich. Ebenso wird angegeben, wenn die Adresse eines verknüpften Bildes nicht in der Datei steht oder ein Teil in einer Codierung vorliegt, die die Prüfung nicht lesen kann. Die Zeile lautet dann „Nicht vollständig geprüft“ statt „Keine gefunden“. Ein Durchlauf von **Alle entfernen** entfernt höchstens 100.000 Verknüpfungen in beibehaltenen Teilen. Bleiben weitere übrig, nennt die Zeile deren Anzahl, und Sie können **Alle entfernen** erneut ausführen.
- **Originalbilder unter Bildeffekten** sind die Bilder, aus denen ein Effekt wie Weichzeichnen oder Mosaik erzeugt wurde. Die Datei bewahrt das Original neben dem fertigen Bild auf, und jeder, der die Datei erhält, kann es herausholen. **Originale entfernen** behält nur das fertige Bild. Danach kann **Bild zurücksetzen** das Original nicht mehr wiederherstellen.
- **Abgeschnittene Bildteile** sind die Teile, die Sie beim Zuschneiden ausgeblendet haben. Sie sind weiterhin im Bild enthalten, und jeder, der die Datei erhält, kann sie sehen. **Abgeschnittene Teile entfernen** löscht sie, und die Seite sieht genauso aus wie vorher. Ein Bild mit Effekt wird zuvor zu seinem fertigen Bild.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu entfernen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Entfernte Verknüpfungen in Kommentaren und anderen beibehaltenen Teilen sowie benutzerdefinierte XML-Daten lassen sich nicht rückgängig machen. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

Die Option **Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten tut dies bei jedem Speichern, für jede Datei, ohne das geöffnete Dokument zu verändern. Es ist dasselbe Kontrollkästchen wie unter **Optionen** und in jeder JeffOffice-App.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen** und wählen Sie danach eine Dateiart aus, um zu sehen, was sie nicht speichern kann. JeffWrite speichert Dateien als **.docx**, **.odt**, **.rtf** und **.txt**. Eine .docx-Datei enthält alles aus Ihrem Dokument. Die anderen Arten können manches nicht speichern, und diese Liste zeigt, was genau. Außerdem nennt sie die Einstellungen, die in der Datei selbst gespeichert werden, also **Als final markieren** und **Immer schreibgeschützt öffnen**, sofern sie aktiv sind, sowie das Kennwort bei einer Art, die keines speichern kann. Wird nichts aufgeführt, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite **vor** dem Schreiben auf, was die Datei nicht enthalten kann. Sie können **Trotzdem speichern**, **Als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen. Wenn Sie **Diese Datei nicht erneut fragen** aktivieren, fragt JeffWrite bei dieser Datei nicht mehr, bis Sie das Programm schließen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Punkte zu öffnen, die für manche Menschen schwer zu lesen oder zu bedienen sein können, etwa ein Bild ohne Beschreibung. Es ist dieselbe Prüfung wie unter **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer aufzubewahren. Nichts wird irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in genau diesem Zustand. Legen Sie vor einer größeren Änderung eine Version an, damit Sie später dorthin zurückkehren können. Das Dokument muss zuerst gespeichert sein, weil Versionen unter dem Dateinamen abgelegt werden.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst bleibt unverändert.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, wobei die Unterschiede als nachverfolgte Änderungen angezeigt werden. Siehe [Änderungen nachverfolgen](track-changes).
- **Zu dieser Version zurückkehren** macht das Dokument identisch mit der gewählten Version. JeffWrite speichert das bisherige Dokument zuvor als neue Version, damit nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite behält bis zu 50 Versionen einer Datei und entfernt die älteste, wenn es mehr werden.
