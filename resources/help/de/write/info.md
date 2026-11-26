# Info: Eigenschaften, Kennwörter und Dokumentprüfung
<!-- keywords: Barrierefreiheit prüfen, Info, Eigenschaften, Titel, Autor, Betreff, Stichwörter, Kategorien, Kommentare, Firma, Manager, benutzerdefinierte Eigenschaften, Dokumenteigenschaft, als final markieren, final, schreibgeschützt, Kennwort, Passwort, verschlüsseln, schützen, Öffnungskennwort, Dokument prüfen, persönliche Informationen, ausgeblendeter Text, benutzerdefiniertes XML, Kompatibilität prüfen, konvertieren, Kompatibilitätsmodus, Versionsverlauf, Versionen, Version speichern, zurückkehren, wiederherstellen, Versionen vergleichen, im Ordner anzeigen, Pfad kopieren, Ordner, Pfad -->

Klicken Sie auf **Datei > Info**, um die Angaben zu Ihrem Dokument zu sehen und es vor dem Weitergeben zu schützen. Drücken Sie **Esc**, um zu Ihrem Dokument zurückzukehren.

## Speicherort der Datei

Oben auf der Seite wird angezeigt, wo die Datei gespeichert ist. Klicken Sie auf **Im Ordner anzeigen**, um diesen Ordner zu öffnen. Klicken Sie auf **Pfad kopieren**, um den Pfad zu kopieren, damit Sie ihn in eine E-Mail oder einen Chat einfügen können.

## Eigenschaften

Eigenschaften sind Angaben zum Dokument, etwa Titel und Verfasser. Die Seite enthält für jede ein Feld: **Titel**, **Autor**, **Betreff**, **Stichwörter**, **Kategorien**, **Kommentare**, **Firma** und **Manager**. Geben Sie etwas in ein Feld ein, und drücken Sie **Tab** oder **Eingabe**. JeffWrite übernimmt die Änderung und speichert sie beim nächsten Speichern des Dokuments.

Unter den Feldern zeigt die Seite an, was JeffWrite ermittelt: wann das Dokument erstellt, gespeichert und gedruckt wurde sowie wie viele Seiten, Wörter, Zeichen und Absätze es hat.

### Eigene Eigenschaften

Klicken Sie auf **Benutzerdefinierte Eigenschaften**, um eigene Eigenschaften anzulegen, etwa **Projekt** oder **Kunde**. Klicken Sie auf **Hinzufügen**, geben Sie einen Namen ein, wählen Sie die Art des Werts (Text, eine ganze Zahl, eine Zahl, Ja oder Nein oder ein Datum) und geben Sie den Wert ein.

Ein Feld kann eine benutzerdefinierte Eigenschaft anzeigen. Klicken Sie auf **Einfügen > Feld**, und wählen Sie ein Feld aus. Ändern Sie den Code in `DOCPROPERTY "Kunde"`, und klicken Sie auf **OK**. Der Text zeigt dann den Wert an. Nach einer Änderung der Eigenschaft drücken Sie **F9**, um das Feld zu aktualisieren. Siehe [Felder](fields).

## Als final markieren

Klicken Sie auf **Als final markieren**, um den Lesern mitzuteilen, dass das Dokument fertig ist. JeffWrite speichert das Dokument und öffnet es danach nur noch zum Lesen. Wer es öffnet, sieht in der Titelleiste, dass es schreibgeschützt ist, und ein Hinweis teilt mit, dass es als final markiert wurde. Das ist eine freundliche Bitte, das Dokument nicht zu ändern. Es ist kein Schloss, denn jeder kann **Als final markiert** erneut auswählen, um es zu bearbeiten.

## Immer schreibgeschützt öffnen

Klicken Sie auf **Immer schreibgeschützt öffnen**, damit das Dokument jedes Mal nur zum Lesen geöffnet wird und niemand es versehentlich ändert. Die Auswahl wird beim Speichern mit der Datei gespeichert. Wenn die Datei das nächste Mal geöffnet wird, zeigt die Statusleiste an, dass sie nur zum Lesen geöffnet ist. Zum Bearbeiten öffnen Sie **Datei > Info** und klicken auf **Wird immer schreibgeschützt geöffnet**. Das ist mit einem Klick erledigt, die Option ist also eine Erinnerung und keine Sperre. Andere Programme lesen diese Auswahl nicht, und eine .odt-Datei kann sie nicht speichern. Speichern Sie das Dokument deshalb zuerst als .docx-Datei. Die Kompatibilitätsprüfung listet dies für eine .odt-Datei auf.

## Mit Kennwort verschlüsseln

Klicken Sie auf **Mit Kennwort verschlüsseln**, damit beim Öffnen der Datei ein Kennwort verlangt wird. Geben Sie das Kennwort zweimal ein, und klicken Sie auf **Schützen**. Die Datei wird beim Speichern verschlüsselt, sodass niemand sie ohne Kennwort lesen kann. JeffWrite verwendet die starke Schutzmethode, die auch andere Textverarbeitungen nutzen, sodass diese die Datei ebenfalls mit dem Kennwort öffnen können.

- Ein Kennwort **lässt sich nicht wiederherstellen**, wenn Sie es vergessen. Bewahren Sie es sicher auf.
- Die gespeicherten Versionen und die Sicherungskopie folgen dem Kennwort: Sie sind damit geschützt und ändern sich, wenn Sie das Kennwort ändern oder entfernen.
- Um das Kennwort zu ändern, klicken Sie erneut auf dieselbe Karte. Klicken Sie auf **Kennwort entfernen**, um es zu löschen. Die Änderung wird beim nächsten Speichern übernommen.
- Kennwörter funktionieren für **.docx**-Dateien und für **.odt**-Dateien. Eine .odt-Datei wird so geschützt, wie es der OpenDocument-Standard beschreibt, sodass andere Programme, die diesem Standard folgen, sie ebenfalls mit dem Kennwort öffnen können. Eine reine Textdatei oder eine .rtf-Datei kann kein Kennwort speichern. JeffWrite speichert ein geschütztes Dokument daher nicht in diesem Format, sondern weist Sie darauf hin, statt ohne Kennwort zu speichern.

Um eine geschützte Datei zu öffnen, öffnen Sie sie wie gewohnt. JeffWrite fragt nach dem Kennwort. Ist es nicht richtig, können Sie es erneut versuchen oder auf **Abbrechen** klicken.

JeffWrite öffnet auch eine alte **.doc**-Datei, die ein Kennwort verlangt. Die Abfrage erfolgt auf dieselbe Weise. JeffWrite speichert nie eine .doc-Datei. Daher fragt **Speichern unter** nach einem neuen Namen, und die .docx-Kopie behält dasselbe Kennwort.

## Dokument prüfen

Klicken Sie auf **Dokument prüfen**, bevor Sie ein Dokument an jemand anderen senden. Es werden die Inhalte aufgelistet, die die Datei enthält und die Sie vielleicht nicht weitergeben möchten, mit der jeweiligen Anzahl:

- **Kommentare** sind Notizen, die Personen am Rand hinterlassen haben.
- **Nachverfolgte Änderungen** sind Bearbeitungen, die noch nicht angenommen oder abgelehnt wurden. **Alle annehmen** übernimmt jede Änderung in den Text.
- **Dokumenteigenschaften und persönliche Informationen** sind Autor, die Person, die zuletzt gespeichert hat, Firma und Manager.
- **Ausgeblendeter Text** ist Text, der als ausgeblendet formatiert ist. Er wird weder angezeigt noch gedruckt, ist aber in der Datei enthalten.
- **Kopf- und Fußzeilen** sind der Text am oberen und unteren Seitenrand.
- **Benutzerdefinierte XML-Daten** sind Daten, die andere Programme in die Datei eingefügt haben. Auf einer Seite sind sie nicht zu sehen.
- **Externe Verknüpfungen** sind Verweise auf andere Dateien, nicht Weblinks: ein Bild oder Objekt, das nur verknüpft ist (es zeigt eine Datei, die nicht im Dokument enthalten ist), sowie ein Feld, das seinen Text oder sein Bild aus einer anderen Datei bezieht (INCLUDEPICTURE, INCLUDETEXT, LINK, DDE und DDEAUTO). **Alle entfernen** wandelt jedes Feld in den Text um, den es jetzt anzeigt, entfernt jedes verknüpfte Bild und Objekt und lässt die Verknüpfung beim Speichern aus der Datei. Die anderen Dateien bleiben unverändert. Verknüpfungen in Kopf- und Fußzeilen, Fuß- und Endnoten werden ebenfalls gezählt und entfernt. Weblinks bleiben erhalten. Verknüpfungen in Kommentaren, Textfeldern und in Teilen, die das Programm unverändert lässt (etwa der Text von Zeichnungen), werden ebenfalls gesucht. Ein verknüpftes Bild, das zusätzlich eine eigene Kopie des Bildes enthält, bleibt als Bild ohne Verknüpfung erhalten, und die Verknüpfung wird beim Speichern aus der Datei entfernt. Das Entfernen von Verknüpfungen in Kommentaren und anderen unverändert belassenen Teilen kann nicht rückgängig gemacht werden; die Textfelder sind ein Schritt von Rückgängig. Die Vorlage, an die das Dokument angehängt ist (ihr Pfad enthält oft einen Benutzernamen), zählt ebenfalls als externe Verknüpfung: **Alle entfernen** löst sie, und das Dokument behält seine eigenen Formatvorlagen. Andere Verknüpfungen zu Dateien werden nicht entfernt. Eine Zeile unter der Anzahl nennt sie: die Datenquelle des Seriendrucks, Rahmen, Unterdokumente in anderen Teilen, externe Daten eines Diagramms, verknüpfte Filme und Töne sowie verknüpfte Formatvorlagendateien. Dieselbe Zeile zählt auch andere Verknüpfungen in einem Feld, einem Kommentar, einem Textfeld oder in der Liste der Verknüpfungen der Datei, die im Dokument nicht verwendet werden, und meldet, wenn die Adresse eines verknüpften Bildes nicht in der Datei steht oder ein Teil in einer Codierung vorliegt, die die Prüfung nicht lesen kann. Die Zeile lautet dann „Nicht vollständig geprüft“ statt „Keine gefunden“. Ein Lauf von **Alle entfernen** entfernt höchstens 100.000 Verknüpfungen in unverändert belassenen Teilen; bleiben mehr übrig, nennt die Zeile die Anzahl, und Sie können **Alle entfernen** erneut ausführen.
- **Originalbilder unter Bildeffekten** sind die Bilder, aus denen ein Effekt wie Weichzeichnen oder Mosaik erzeugt wurde. Die Datei bewahrt das Original neben dem fertigen Bild auf, und jeder, der die Datei erhält, kann es herausholen. **Originale entfernen** behält nur das fertige Bild, danach kann **Bild zurücksetzen** das Original nicht mehr wiederherstellen.
- **Zugeschnittene Bildteile** sind die Teile, die Sie außerhalb des sichtbaren Bereichs beschnitten haben. Sie sind weiterhin im Bild enthalten, und jeder, der die Datei erhält, kann sie sehen. **Abgeschnittene Teile entfernen** löscht sie, und die Seite sieht genauso aus wie zuvor. Ein Bild mit Effekt wird vorher zu seinem fertigen Bild.

Klicken Sie neben einer Art auf **Alle entfernen**, um sie zu entfernen. Änderungen am Text können Sie mit **Strg+Z** rückgängig machen. Entfernte Verknüpfungen in Kommentaren und anderen unverändert belassenen Teilen sowie benutzerdefinierte XML-Daten lassen sich nicht rückgängig machen. Speichern Sie das Dokument danach, um die bereinigte Fassung zu behalten.

**Persönliche Informationen beim Speichern aus dieser Datei entfernen** unter den Karten bewirkt dies bei jedem Speichern, für jede Datei, ohne das geöffnete Dokument zu verändern. Es ist dasselbe Kontrollkästchen wie in **Optionen** und in allen JeffOffice-Apps.

## Konvertieren

Eine .docx-Datei, die angibt, mit der Arbeitsweise eines älteren Programms erstellt worden zu sein, zeigt **[Kompatibilitätsmodus]** nach ihrem Namen in der Titelleiste. Das bedeutet, dass das Dokument nach den Regeln des älteren Programms gesetzt ist. Ein neues Dokument, eine Datei ohne Angabe sowie eine .odt-, .rtf-, .doc- oder Textdatei zeigen nichts an. Öffnen Sie **Datei > Info** und klicken Sie auf **Konvertieren**, um stattdessen die neuesten Regeln zu verwenden. Vorher wird nachgefragt, und die Aktion lässt sich nicht mit Strg+Z rückgängig machen. Die Seiten sehen sofort so aus, wie sie nach dem Speichern und erneuten Öffnen der Datei aussehen. Konvertieren ändert diese Dinge:

- **Tabellen.** In der älteren Arbeitsweise wird der Einzug einer Tabelle bis zum Text in ihrer ersten Zelle gemessen, sodass die linke Tabellenkante etwas links davon liegt. In der neuesten Arbeitsweise wird der Einzug bis zur Tabellenkante gemessen, sodass Tabellen mit Einzug um die Breite des Zellenrands nach rechts rücken.
- **Seitenwechsel.** In der älteren Arbeitsweise lässt ein Absatz, der mit einem Seitenwechsel endet, den nächsten Absatz auf der neuen Seite beginnen. In der neuesten Arbeitsweise wandert das Ende dieses Absatzes auf die neue Seite und lässt oben eine leere Zeile.
- **Zeilenhöhen.** Eine Datei, die in der ältesten Weise erstellt wurde, kann Zeilen mit ganzen Punkthöhen verlangen. Nach dem Konvertieren haben Zeilen ihre natürliche Höhe, daher können Zeilen und Seiten an anderen Stellen umbrechen.

Die nächste Speicherung der Datei kennzeichnet sie als im neuesten Modus, und das Kennzeichen verschwindet aus der Titelleiste. Einstellungen, die die Datei einzeln festlegt, etwa Silbentrennung und Doppelseiten, bleiben erhalten. Ein Dokument, das bereits im neuesten Modus ist, hat keine Schaltfläche **Konvertieren**.

## Kompatibilität prüfen

Klicken Sie auf **Kompatibilität prüfen** und wählen Sie dann einen Dateityp, um zu sehen, was er nicht speichern kann. JeffWrite speichert **.docx**-, **.odt**-, **.rtf**- und **.txt**-Dateien. Eine .docx-Datei enthält alles aus Ihrem Dokument. Die anderen Typen können manche Dinge nicht speichern, und diese Liste nennt sie. Sie nennt außerdem die Einstellungen, die in der Datei selbst gespeichert werden, also Als final markieren und Immer schreibgeschützt öffnen, wenn sie aktiv sind, und das Kennwort für einen Typ, der keines speichern kann. Wird nichts aufgelistet, geht nichts verloren.

Wenn Sie als .odt-Datei speichern, listet JeffWrite, was die Datei nicht enthalten kann, **bevor** etwas geschrieben wird. Sie können **Trotzdem speichern**, **Als .docx speichern** (die .docx-Datei wird daneben abgelegt, und nichts geht verloren) oder **Abbrechen** wählen. Aktivieren Sie **Für diese Datei nicht erneut fragen**, dann fragt JeffWrite bei dieser Datei nicht mehr nach, bis Sie das Programm schließen.

## Barrierefreiheit prüfen

Klicken Sie auf **Barrierefreiheit prüfen**, um die Liste der Dinge zu öffnen, die manche Menschen schwer lesen oder nutzen können, etwa ein Bild ohne Beschreibung. Es ist dieselbe Prüfung wie **Überprüfen > Barrierefreiheit prüfen**. Siehe [Wörter zählen, Sprache festlegen, Barrierefreiheit prüfen und Wörter nachschlagen](proofing-tools).

## Versionsverlauf

Klicken Sie auf **Versionsverlauf**, um benannte Kopien des Dokuments auf Ihrem Computer aufzubewahren. Nichts wird irgendwohin gesendet. Eine Version ist eine vollständige Kopie des Dokuments in genau diesem Moment. Legen Sie vor einer größeren Änderung eine Version an, damit Sie darauf zurückgreifen können. Das Dokument muss zuerst gespeichert sein, weil Versionen unter dem Dateinamen abgelegt werden.

- **Version speichern** fragt nach einem Namen, etwa „Vor der Überarbeitung“, und legt eine Kopie an.
- **Kopie öffnen** öffnet die gewählte Version in einem neuen Fenster. Die Version selbst ändert sich dabei nicht.
- **Vergleichen** stellt das Dokument und die gewählte Version nebeneinander als ein Dokument dar, und die Unterschiede werden als nachverfolgte Änderungen angezeigt. Siehe [Änderungen nachverfolgen](track-changes).
- **Hierzu zurückkehren** macht das Dokument identisch mit der gewählten Version. JeffWrite legt vorher den bisherigen Stand als neue Version ab, damit nichts verloren geht. Speichern Sie das Dokument, um die Änderung zu behalten.
- **Löschen** entfernt die gewählte Version.

JeffWrite bewahrt bis zu 50 Versionen einer Datei auf und verwirft die ältesten, wenn es mehr sind.
