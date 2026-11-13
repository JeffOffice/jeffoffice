# How a barcode is saved in other programs' files

Insert > Barcode is in JeffPub, JeffDeck, JeffWrite, and JeffCrunch, with the same dialog (`libs/shell/app/barcodedialog.cpp`): book ISBNs with an optional price add-on, EAN-13, UPC-A, EAN-8 (with 2- or 5-digit add-ons), Code 128, Code 39, and QR codes. The bars come from one encoder (`libs/base/core/barcode.cpp`), and `libs/draw/core/barcodeitem.cpp` makes the object from a set of settings.

A `.docx`, `.pptx`, or `.xlsx` has no barcode of its own, and neither does an OpenDocument file. So a barcode is saved twice over:

1. **As a picture every program shows.** The barcode is an SVG (the quiet zone and the bars with their digits, as outlines, so no font is needed), with a PNG of it beside it for programs that don't draw SVG. In a file of the Open XML kind the PNG is the picture's blip, and the SVG is linked in the blip's extension list (`asvg:svgBlip`, extension `{96DAC541-7B7A-43D3-8B79-37D633B846F1}`), the way other programs save an SVG picture. The picture has the barcode's alternative text (`Barcode: ISBN 978-1-64002-163-1`, `Barcode: QR code https://example.com`).
2. **With a marker that holds the settings.** JeffOffice opens a picture with the marker as a barcode again, which can be edited, and draws it as vectors. Other programs may drop the marker when they save the file again, and then the barcode is a plain picture, which still shows and prints but is no longer edited as a barcode.

## The marker in an Open XML drawing

An extension of the picture's non-visual properties (`p:cNvPr` in a presentation, `xdr:cNvPr` in a workbook's drawing, `wp:docPr` in a document), after any other children:

```xml
<a:extLst>
  <a:ext uri="{5B6C1E0A-2F4D-4C7B-9A3E-8D1F6A2B7C40}">
    <jo:barcode xmlns:jo="https://jeffoffice.com/ns/barcode" version="1" settings="{&quot;type&quot;:6,&quot;data&quot;:&quot;https://example.com&quot;,...}"
                png-sha256="..." svg-sha256="..."/>
  </a:ext>
</a:extLst>
```

`settings` is a JSON object, written compact. `png-sha256` and `svg-sha256` are the SHA-256 (lowercase hex) of the bytes of the PNG and of the SVG that were written with it; a file with no SVG has no `svg-sha256`.

**The marker is honored only where the picture is the one that was written.** When a file is read, the bytes of its PNG are hashed and must be `png-sha256`; if the picture has an SVG, its bytes must be `svg-sha256`. A picture another program put in the barcode's place (its "Change Picture"), or changed, is shown as the plain picture it is, whatever the marker says.

| Key | Meaning |
|---|---|
| `type` | 0 book (ISBN), 1 EAN-13, 2 UPC-A, 3 EAN-8, 4 Code 128, 5 Code 39, 6 QR code |
| `data` | The ISBN, the digits, or the text |
| `addOn` | EAN and UPC: none, or 2 or 5 digits |
| `priceMode`, `currency`, `price` | A book's price add-on: 0 none, 1 the price in `price` (`currency` 0 to 4: US, Canadian, pounds, Australian, New Zealand), 2 no suggested price (90000) |
| `magnification`, `height` | Percent of the standard size (50 to 300) and of the standard bar height (40 to 100) |
| `text` | Print the digits |
| `white` | The white quiet zone behind the bars |
| `check39` | Code 39: add the check character |
| `qrLevel` | QR: error correction, 0 L, 1 M, 2 Q, 3 H |
| `cell` | JeffCrunch: the cell the barcode's text is read from, as a reference (`Sheet1!$A$2`, the sheet's name quoted where it needs it). It follows rows and columns inserted or deleted, and the picture is made again from the cell's text when the cell changes and when the file is saved. Other programs pass it over |

The settings are held to the dialog's ranges when read: magnification 50% to 300%, bar height 40% to 100%, a price up to 99.99, the error correction levels 0 to 3, and so on; a number outside them is brought to the nearest. A marker is passed over (the picture stays a picture) when its settings are longer than 16 KB, are not a JSON object, or are not a barcode the encoder draws (no data, a wrong check digit, more than a QR code at that level holds). The encoder's own limits are the only limits: a QR code holds at most 2,953 bytes of text at level L, 2,331 at M, 1,663 at Q, and 1,273 at H.

At most 2,000 barcodes of a document are honored; the rest stay plain pictures. A barcode is laid out once however many copies there are.

The PNG has whole pixels to each module: at least 4 for a QR code and 3 for the others (about 1,200 across for a small code), and at most 4,000 pixels on a side; a code that would need more is held to 4,000, and its SVG stays exact.

A barcode that is turned or flipped is written as the unturned picture with the file's own turn and flips on it (`rot` and `flipH` in a drawing, the frame's transform and mirror in an open document), and is turned again when read. The marker is written on a picture marked as decorative too: it goes in the same extension list. When JeffWrite changes a barcode, it replaces its extension inside the extension list the picture's properties already have, and keeps any other extensions there.

The picture's size in the file is the barcode's size on the page. Opening the file lays the barcode out from the settings again and fits it to that frame, so a barcode that was stretched or shrunk keeps the size it was given.

## In an OpenDocument file

The picture is the frame's `draw:image` (a PNG), and the settings are an attribute of the frame in JeffOffice's own namespace (the one its presentations already use for what the format has no place for):

```xml
<draw:frame ... xmlns:jo="urn:jeffoffice:names:extension:1.0" jo:barcode="{...}">
```

`jo:barcode-sha256` holds the PNG's hash. JeffDeck writes this in `.odp` files and JeffWrite in `.odt` files, and both read it back. JeffCrunch's `.ods` copy has no drawings yet, so a barcode is not in it.

## Where the code is

| What | Where |
|---|---|
| The marker, written and read | `libs/office/office/barcodemarker.cpp` |
| Presentations and workbooks (the shared DrawingML reader and writer) | `libs/office/office/drawingml_shape.cpp`, `drawingml_write.cpp` |
| Documents | `apps/write/core/pictureops.cpp` (`insertBarcode`, `changeBarcode`), `libs/office/wml/wmldrawing.cpp` (reading the marker) |
| Open document files | `apps/deck/io/odpexport_shape.cpp`, `odpimport_shape.cpp`; `libs/office/odt/odtwritebody.cpp`, `odtbody.cpp` |
| The workbook's object | `apps/crunch/core/barcodeclip.cpp` |
