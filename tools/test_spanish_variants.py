#!/usr/bin/env python3
"""python3 -I tools/test_spanish_variants.py : checks the Spanish (Spain) word list and that the committed files are current."""
import collections
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "translate"))
import spanish_variants as v


def es(text):
    return v.respell_text(text, "UI")


class Words(unittest.TestCase):
    def test_file_and_the_others(self):
        self.assertEqual(es("Abra el archivo y cierre los archivos."), "Abra el fichero y cierre los ficheros.")
        self.assertEqual(es("El mouse y los mouses"), "El ratón y los ratones")
        self.assertEqual(es("Su celular o sus celulares"), "Su móvil o sus móviles")
        self.assertEqual(es("Presione Ctrl+S y presiona Entrar; está presionada."), "Pulse Ctrl+S y pulsa Entrar; está pulsada.")

    def test_capitals_are_kept(self):
        self.assertEqual(es("Archivos recientes"), "Ficheros recientes")
        self.assertEqual(es("ARCHIVOS RECIENTES"), "FICHEROS RECIENTES")
        self.assertEqual(es("Archivo vinculado: %1"), "Fichero vinculado: %1")

    def test_whole_words_only(self):
        self.assertEqual(es("archivado, archivar, archivista, presión, presiones"), "archivado, archivar, archivista, presión, presiones")

    def test_the_file_tab_stays(self):
        self.assertEqual(es("Archivo"), "Archivo")
        self.assertEqual(es("&Archivo"), "&Archivo")
        self.assertEqual(es("Abre la página Archivo: nuevo, abrir."), "Abre la página Archivo: nuevo, abrir.")
        self.assertEqual(es("Abrir en una página de Archivo."), "Abrir en una página de Archivo.")
        self.assertEqual(es("Elija Archivo > Guardar como."), "Elija Archivo > Guardar como.")
        self.assertEqual(es("Haga clic en **Archivo > Abrir**."), "Haga clic en **Archivo > Abrir**.")
        self.assertEqual(es("Haga clic en el botón **Archivo**."), "Haga clic en el botón **Archivo**.")
        self.assertEqual(es("Cambie esto en Archivo &gt; Opciones."), "Cambie esto en Archivo &gt; Opciones.")
        self.assertEqual(v.respell_text("[Archivo", "Help"), "[Archivo")
        # ...and a file named by the same word is a fichero, even at the start of a sentence.
        self.assertEqual(es("Archivo guardado."), "Fichero guardado.")
        self.assertEqual(es("Guarde el archivo en Archivo > Guardar."), "Guarde el fichero en Archivo > Guardar.")

    def test_marks_are_kept(self):
        self.assertEqual(es("Abrir %1 archivos (%n)"), "Abrir %1 ficheros (%n)")
        self.assertEqual(es("<b>archivo</b> y <a href=\"archivo\">archivo</a>"), "<b>fichero</b> y <a href=\"archivo\">fichero</a>")
        self.assertEqual(es("Vea archivo.png y mouse_speed y a/archivo"), "Vea archivo.png y mouse_speed y a/archivo")
        self.assertEqual(es("A&rchivos"), "Fiche&ros")

    def test_gender_follows_the_noun(self):
        self.assertEqual(es("Use la computadora."), "Use el ordenador.")
        self.assertEqual(es("Una computadora o unas computadoras"), "Un ordenador o unos ordenadores")
        self.assertEqual(es("Guárdelo en la computadora de su oficina; sale de la computadora."), "Guárdelo en el ordenador de su oficina; sale del ordenador.")
        self.assertEqual(es("Vaya a la computadora"), "Vaya al ordenador")
        self.assertEqual(es("Esta computadora local"), "Este ordenador local")
        self.assertEqual(es("El computador"), "El ordenador")

    def test_an_adjective_we_cannot_change_leaves_the_word(self):
        left = []
        self.assertEqual(v.respell_text("Su nueva computadora", "UI", None, left), "Su nueva computadora")
        self.assertEqual(v.respell_text("La computadora lenta", "UI", None, left), "La computadora lenta")
        self.assertEqual(len(left), 2)

    def test_lists_do_not_overlap(self):
        self.assertFalse(set(v.WORDS) & v.SAME)
        self.assertFalse(set(v.FEMININE) & v.SAME)
        for w in ("carpeta", "pantalla", "guardar"):
            self.assertIn(w, v.SAME)
            self.assertEqual(es(w), w)
            self.assertEqual(es(w.capitalize()), w.capitalize())


class Files(unittest.TestCase):
    def test_the_files_are_current(self):
        stale = []
        text, _ = v.ui_file()
        path = os.path.join(v.ROOT, "translations", "jeffoffice_es_ES.ts")
        with open(path, encoding="utf-8", newline="") as f:
            if f.read() != text:
                stale.append(path)
        pages = v.help_files()
        folder = os.path.join(v.ROOT, "resources", "help", "es_ES")
        for rel, page in pages.items():
            with open(os.path.join(folder, rel), encoding="utf-8", newline="") as f:
                if f.read() != page:
                    stale.append(rel)
        self.assertEqual(stale, [])

    def test_no_shortcut_letter_is_shared_by_the_swap(self):
        self.assertEqual(v.shortcut_clashes(), [])

    def test_the_word_list_is_in_use(self):
        changed = collections.Counter()
        v.ui_file(changed)
        v.help_files(changed)
        words = {old.lower() for (_, old, new) in changed}
        for w in ("archivo", "archivos", "mouse", "presione"):
            self.assertIn(w, words)


if __name__ == "__main__":
    unittest.main()
