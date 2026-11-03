# Willkommen bei JeffCrunch
<!-- keywords: Willkommen, Erste Schritte, Einführung, Einstieg, Anfänger, Schnellstart, Überblick, Übersicht, Was ist JeffCrunch, Grundlagen, Beta, Tabellenkalkulation, Arbeitsmappe, Tabellenblatt, Blatt, Zellen, Was funktioniert, Noch nicht, Einschränkungen, Grenzen -->

JeffCrunch ist ein Tabellenkalkulationsprogramm. Es ist kostenlos und Open Source. Es fallen keine Gebühren an, es gibt kein Abonnement, und Sie müssen kein Konto einrichten. JeffCrunch gehört zu den JeffOffice-Apps und läuft unter Windows, macOS und Linux.

Die Datei, mit der Sie arbeiten, heißt **Arbeitsmappe**. Eine Arbeitsmappe enthält ein oder mehrere **Blätter**, und jedes Blatt ist ein Raster aus **Zellen**. JeffCrunch speichert Arbeitsmappen als .xlsx-Dateien. Es öffnet auch .xlsx- und .xlsm-Arbeitsmappen aus anderen Programmen sowie ältere .xls-Dateien, OpenDocument-Tabellen und Textdateien mit Werten. Was JeffCrunch noch nicht bearbeiten kann, bleibt erhalten.

## JeffCrunch ist eine Betaversion

Diese Version von JeffCrunch ist eine **Betaversion**. Sie eignet sich für die echte Arbeit, beherrscht aber noch nicht alles. Bewahren Sie von jeder wichtigen Arbeitsmappe eine Kopie auf.

**Was funktioniert**

- Öffnen von Arbeitsmappen: .xlsx und .xlsm, ältere .xls-Dateien, OpenDocument-Tabellen (.ods und .fods) und Textdateien mit Werten (.csv, .tsv und .txt). Speichern als .xlsx sowie von Kopien als .xls oder .ods. Siehe [Arbeitsmappen öffnen und speichern](open-save).
- Öffnen kennwortgeschützter Arbeitsmappen (.xlsx, .xls und .ods) und Schützen einer .xlsx-Arbeitsmappe mit einem Kennwort. Siehe [Kennwortgeschützte Arbeitsmappen öffnen](open-password).
- Eingeben von Zahlen, Text und Formeln in Zellen, mit Rückgängig und Wiederholen. Formeln werden schon während der Eingabe berechnet; mehr als 200 Funktionen stehen zur Verfügung. Während Sie eine Formel eingeben, können Sie Zellen anklicken oder mit den Pfeiltasten ansteuern, und im Dialogfeld **Funktionsargumente** geben Sie die Argumente einer Funktion ein und sehen sofort das Ergebnis. Siehe [In Zellen eingeben](edit-cells), [Wie Formeln angezeigt und berechnet werden](formulas) und [Eine Funktion mit dem Dialogfeld „Funktionsargumente“ eingeben](function-arguments).
- Formatieren von Zellen: Schriftarten, Farben, Rahmen, Zahlenformate, Zellenformatvorlagen und verbundene Zellen. Siehe [Zellen formatieren](format-cells).
- Ausschneiden, Kopieren und Einfügen, Ausfüllen von Zellen, Bilden von Summen, Suchen und Ersetzen, Sortieren, Filtern, Tabellen und Pivot-Tabellen. Siehe [Pivot-Tabellen](pivot-tables).
- Zeilen, Spalten und Blätter: Einfügen, Löschen, Ändern der Größe, Ausblenden und Verschieben.
- Diagramme, Bilder und Formen sowie Notizen und Links.
- Schützen eines Blatts oder der Arbeitsmappe.
- Einrichten der Seiten, Anzeigen einer Vorschau vor dem Drucken, Drucken und Erstellen einer PDF-Datei. Siehe [Drucken und PDF erstellen](printing).

**Was noch nicht funktioniert**

- Regeln für die bedingte Formatierung und für die Datenüberprüfung können Sie nicht erstellen, ändern oder löschen. JeffCrunch zeigt die vorhandenen Regeln einer Arbeitsmappe an und behält sie bei.
- Sie können eine Zellenformatvorlage nicht ändern und keine neue erstellen.
- Verwendet eine neue Formel eine Funktion, die JeffCrunch noch nicht kennt, erhält sie keinen Wert; eine Formel, die bereits in der Arbeitsmappe stand, behält das in der Datei gespeicherte Ergebnis.
- Makros bleiben erhalten und werden mitgespeichert, aber nie ausgeführt.
- Pivot-Tabellen haben einige Einschränkungen, die unter [Pivot-Tabellen](pivot-tables) aufgeführt sind.
- Eine aus einer .xls-, .ods- oder Textdatei geöffnete Arbeitsmappe wird als neue .xlsx-Datei gespeichert, sofern Sie keine Kopie als .xls oder .ods wählen. Eine solche Kopie kann nicht alles enthalten; JeffCrunch teilt Ihnen vor dem Speichern mit, was dabei wegfällt.
- Ein Kennwort kann eine .xlsx-Arbeitsmappe schützen, aber keine Kopie im .xls- oder .ods-Format.
- Eine Datentabelle (Was-wäre-wenn-Tabelle) kann berechnet, aber nicht erstellt werden.

## Die ersten fünf Schritte

1. **Arbeitsmappe öffnen.** Drücken Sie **{key:Open}**, oder klicken Sie auf **Datei** und dann auf **Öffnen**. Die ersten Zeilen erscheinen sofort, selbst bei einer sehr großen Arbeitsmappe, während JeffCrunch den Rest einliest.
2. **Umsehen.** Klicken Sie auf eine Zelle, um sie auszuwählen. Ihre Adresse steht im **Namenfeld** oben links, ihr Inhalt in der **Bearbeitungsleiste** daneben. Mit den Pfeiltasten, dem Mausrad und den Bildlaufleisten bewegen Sie sich im Blatt. Siehe [Zellen auswählen und navigieren](select-and-move).
3. **Zelle ändern.** Wählen Sie eine Zelle aus, und tippen Sie los. Drücken Sie die **Eingabetaste**, um die Eingabe zu übernehmen. Siehe [In Zellen eingeben](edit-cells).
4. **Zellen formatieren.** Wählen Sie Zellen aus, und ändern Sie auf der Registerkarte **Start** Schriftart, Farben, Rahmen und Zahlenformat. Siehe [Zellen formatieren](format-cells).
5. **Ansicht ändern.** Auf der Registerkarte **Ansicht** können Sie zoomen, Gitternetzlinien und Überschriften ein- oder ausblenden und die obersten Zeilen oder die ersten Spalten fixieren. Siehe [Ansicht ändern](view).

## Was Sie sehen

Eine Arbeitsmappe sieht so aus, wie sie gespeichert wurde: mit den Spaltenbreiten und Zeilenhöhen, den Schriftarten, Farben, Füllungen und Rahmen, verbundenen Zellen, fixierten Bereichen und den Zahlenformaten, die eine gespeicherte Zahl als Datum, Prozentwert oder Preis darstellen. Text, der zu lang für seine Zelle ist, läuft in die leeren Nachbarzellen hinein. Eine Zahl, die zu breit für ihre Zelle ist, erscheint als Reihe von #-Zeichen; verbreitern Sie die Spalte, oder wählen Sie die Zelle aus, um die Zahl in der Bearbeitungsleiste zu lesen. Zahlen werden so geschrieben, wie es die Sprache Ihres Computers vorsieht (siehe [Zahlenformate und Ihre Sprache](number-formats)), und die bedingten Formatierungen einer Arbeitsmappe, etwa Farbskalen, Datenbalken und Symbole, werden dargestellt (siehe [Bedingte Formatierung](conditional-formats)).

Speichern Sie Ihre Arbeit mit **{key:Save}**, um Ihre Änderungen in die geöffnete Arbeitsmappe zu schreiben, oder mit **Speichern unter**, um sie in eine neue Datei zu schreiben. Siehe [Arbeitsmappen öffnen und speichern](open-save).
