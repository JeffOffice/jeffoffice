# How a barcode is saved in other programs' files

Insert > Barcode is in JeffPub, JeffDeck, JeffWrite, and JeffCrunch, with the same dialog (`libs/shell/app/barcodedialog.cpp`): book ISBNs with an optional price add-on, EAN-13, UPC-A, EAN-8 (with 2- or 5-digit add-ons), Code 128, Code 39, and QR codes. The bars come from one encoder (`libs/base/core/barcode.cpp`), and `libs/draw/core/barcodeitem.cpp` makes the object from a set of settings.

A `.docx`, `.pptx`, or `.xlsx` has no barcode of its own, and neither does an OpenDocument file. So a barcode is saved twice over:

1. **As a picture every program shows.** The barcode is an SVG (the quiet zone and the bars with their digits, as outlines, so no font is needed), with a PNG of it beside it for programs that don't draw SVG. In a file of the Open XML kind the PNG is the picture's blip, and the SVG is linked in the blip's extension list (`asvg:svgBlip`, extension `{96DAC541-7B7A-43D3-8B79-37D633B846F1}`), the way other programs save an SVG picture. The picture has the barcode's alternative text (`Barcode: ISBN 978-1-64002-163-1`, `Barcode: QR code https://example.com`).
2. **With a marker that holds the settings.** JeffOffice opens a picture with the marker as a barcode again, which can be edited, and draws it as vectors. Every other program leaves the marker alone.

## The marker in an Open XML drawing

An extension of the picture's non-visual properties (`p:cNvPr` in a presentation, `xdr:cNvPr` in a workbook's drawing, `wp:docPr` in a document), after any other children:

```xml
<a:extLst>
  <a:ext uri="{5B6C1E0A-2F4D-4C7B-9A3E-8D1F6A2B7C40}">
    <jo:barcode xmlns:jo="https://jeffoffice.com/ns/barcode" version="1" settings="{&quot;type&quot;:6,&quot;data&quot;:&quot;https://example.com&quot;,...}"/>
  </a:ext>
</a:extLst>
```

`settings` is a JSON object, written compact:

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

A marker is passed over (the picture stays a picture) when its settings are longer than 16 KB, are not a JSON object, or are not a barcode the encoder draws (no data, a wrong check digit, more than a QR code at that level holds). The encoder's own limits are the only limits: a QR code holds at most 2,953 bytes of text at level L, 2,331 at M, 1,663 at Q, and 1,273 at H.

The picture's size in the file is the barcode's size on the page. Opening the file lays the barcode out from the settings again and fits it to that frame, so a barcode that was stretched or shrunk keeps the size it was given.

## In an OpenDocument file

The picture is the frame's `draw:image` (a PNG), and the settings are an attribute of the frame in JeffOffice's own namespace (the one its presentations already use for what the format has no place for):

```xml
<draw:frame ... xmlns:jo="urn:jeffoffice:names:extension:1.0" jo:barcode="{...}">
```

JeffDeck writes this in `.odp` files and JeffWrite in `.odt` files, and both read it back. JeffCrunch's `.ods` copy has no drawings yet, so a barcode is not in it.

## Where the code is

| What | Where |
|---|---|
| The marker, written and read | `libs/office/office/barcodemarker.cpp` |
| Presentations and workbooks (the shared DrawingML reader and writer) | `libs/office/office/drawingml_shape.cpp`, `drawingml_write.cpp` |
| Documents | `apps/write/core/pictureops.cpp` (`insertBarcode`, `changeBarcode`), `libs/office/wml/wmldrawing.cpp` (reading the marker) |
| Open document files | `apps/deck/io/odpexport_shape.cpp`, `odpimport_shape.cpp`; `libs/office/odt/odtwritebody.cpp`, `odtbody.cpp` |
| The workbook's object | `apps/crunch/core/barcodeclip.cpp` |
