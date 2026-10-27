#!/usr/bin/env python3
"""Makes the workbooks that `jcrunchtool edit-struct-bench` measures structural edits on.

Usage: crunch_struct_bench_make.py OUT.xlsx ROWS NUMERIC_COLS [--cross] [--cf] [--dv] [--table] [--chart] [--names] [--all] [--plain]

The sheet has ROWS rows of NUMERIC_COLS numbers and, in the next column, a formula that adds the row up (written as one shared formula
unless --plain). Then, by flag:
  --cross  a column that reads the row above (=U2-U1), and a few formulas that read whole columns (totals and a lookup)
  --cf     three conditional formats over the data (a color scale, a cell-is rule, and an expression)
  --dv     two data validations (a list on one column, a whole-number rule on another)
  --table  a table of 1,000 rows by 3 columns to the right of the data
  --chart  a drawing with a column chart whose series read 200 rows of the data, and another that reads all of a column
  --names  fifty defined names over cells, rows, and the whole data
"""
import sys
import zipfile

NS = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
REL = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'


def col(n):
    s = ''
    n += 1
    while n:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return s


def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    flags = {a for a in sys.argv[1:] if a.startswith('--')}
    if '--all' in flags:
        flags |= {'--cross', '--cf', '--dv', '--table', '--chart', '--names'}
    out, rows, cols = args[0], int(args[1]), int(args[2])
    plain = '--plain' in flags
    sumc = cols  # the column of the row sums
    lastdata = col(cols - 1)
    sumcol = col(sumc)
    cross = '--cross' in flags
    crossc = sumc + 1 if cross else None
    tablec = sumc + 4  # leaves a gap
    tcol = [col(tablec + i) for i in range(3)]
    parts = []
    w = parts.append
    w('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<worksheet xmlns="%s" xmlns:r="%s"><sheetData>' % (NS, REL))
    for r in range(1, rows + 1):
        row = ['<row r="%d">' % r]
        for c in range(cols):
            row.append('<c r="%s%d"><v>%d</v></c>' % (col(c), r, (r * 7 + c * 13) % 1000))
        if r == 1 or plain:
            f = 'SUM(A%d:%s%d)' % (r, lastdata, r)
            if plain:
                row.append('<c r="%s%d"><f>%s</f><v>0</v></c>' % (sumcol, r, f))
            else:
                row.append('<c r="%s%d"><f t="shared" ref="%s1:%s%d" si="0">%s</f><v>0</v></c>' % (sumcol, r, sumcol, sumcol, rows, f))
        else:
            row.append('<c r="%s%d"><f t="shared" si="0"/><v>0</v></c>' % (sumcol, r))
        if cross:
            cc = col(crossc)
            if r == 1:
                row.append('<c r="%s1"><f>%s1</f><v>0</v></c>' % (cc, sumcol))
            else:
                row.append('<c r="%s%d"><f>%s%d-%s%d</f><v>0</v></c>' % (cc, r, sumcol, r, sumcol, r - 1))
            if r <= 6:
                tot = col(crossc + 1)
                fs = ['SUM(%s$1:%s$%d)' % (sumcol, sumcol, rows), 'SUM(A$1:A$%d)' % rows, 'INDEX(A$1:A$%d,%d)' % (rows, min(rows, 50 + r)),
                      'AVERAGE(%s$1:%s$%d)' % (sumcol, sumcol, rows), 'MAX(B$1:B$%d)' % rows, 'COUNT(A:A)']
                row.append('<c r="%s%d"><f>%s</f><v>0</v></c>' % (tot, r, fs[r - 1]))
        if '--table' in flags and r <= 1001:
            if r == 1:
                for i in range(3):
                    row.append('<c r="%s1" t="inlineStr"><is><t>H%d</t></is></c>' % (tcol[i], i))
            else:
                for i in range(3):
                    row.append('<c r="%s%d"><v>%d</v></c>' % (tcol[i], r, r + i))
        row.append('</row>')
        w(''.join(row))
    w('</sheetData>')
    if '--cf' in flags:
        w('<conditionalFormatting sqref="A1:%s%d"><cfRule type="colorScale" priority="1"><colorScale><cfvo type="min"/><cfvo type="max"/><color rgb="FFFFFFFF"/><color rgb="FFFF0000"/></colorScale></cfRule></conditionalFormatting>' % (lastdata, rows))
        w('<conditionalFormatting sqref="B2:B%d"><cfRule type="cellIs" dxfId="0" priority="2" operator="greaterThan"><formula>$A$1</formula></cfRule></conditionalFormatting>' % rows)
        w('<conditionalFormatting sqref="C2:D%d"><cfRule type="expression" dxfId="0" priority="3"><formula>C2&gt;C1</formula></cfRule></conditionalFormatting>' % rows)
    if '--dv' in flags:
        w('<dataValidations count="2"><dataValidation type="list" allowBlank="1" showErrorMessage="1" sqref="A2:A%d"><formula1>"1,2,3"</formula1></dataValidation>'
          '<dataValidation type="whole" operator="between" allowBlank="1" showErrorMessage="1" sqref="B2:B%d"><formula1>0</formula1><formula2>$A$1</formula2></dataValidation></dataValidations>' % (rows, rows))
    if '--chart' in flags:
        w('<drawing r:id="rId1"/>')
    if '--table' in flags:
        w('<tableParts count="1"><tablePart r:id="rId2"/></tableParts>')
    w('</worksheet>')
    sheet = ''.join(parts)

    ct = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
          '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
          '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
          '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
          '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>']
    srels = []
    if '--chart' in flags:
        ct.append('<Override PartName="/xl/drawings/drawing1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawing+xml"/>')
        ct.append('<Override PartName="/xl/charts/chart1.xml" ContentType="application/vnd.openxmlformats-officedocument.drawingml.chart+xml"/>')
        srels.append('<Relationship Id="rId1" Type="%s/drawing" Target="../drawings/drawing1.xml"/>' % REL)
    if '--table' in flags:
        ct.append('<Override PartName="/xl/tables/table1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.table+xml"/>')
        srels.append('<Relationship Id="rId2" Type="%s/table" Target="../tables/table1.xml"/>' % REL)
    ct.append('</Types>')

    names = ''
    if '--names' in flags:
        items = ['<definedName name="Data">Sheet1!$A$1:$%s$%d</definedName>' % (lastdata, rows),
                 '<definedName name="Totals">Sheet1!$%s$1:$%s$%d</definedName>' % (sumcol, sumcol, rows),
                 '<definedName name="FirstRow">Sheet1!$1:$1</definedName>', '<definedName name="WholeA">Sheet1!$A:$A</definedName>']
        for i in range(46):
            items.append('<definedName name="N%d">Sheet1!$%s$%d</definedName>' % (i, col(i % cols), 1 + (i * rows) // 46))
        names = '<definedNames>' + ''.join(items) + '</definedNames>'
    workbook = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<workbook xmlns="%s" xmlns:r="%s"><sheets><sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets>%s</workbook>' % (NS, REL, names)
    wbrels = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
              '<Relationship Id="rId1" Type="%s/worksheet" Target="worksheets/sheet1.xml"/><Relationship Id="rId2" Type="%s/styles" Target="styles.xml"/></Relationships>' % (REL, REL))
    styles = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<styleSheet xmlns="%s"><fonts count="1"><font><sz val="11"/><name val="Calibri"/></font></fonts>'
              '<fills count="2"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill></fills><borders count="1"><border><left/><right/><top/><bottom/><diagonal/></border></borders>'
              '<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs><cellXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/></cellXfs>'
              '<dxfs count="1"><dxf><font><b/></font></dxf></dxfs></styleSheet>' % NS)
    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED, compresslevel=1) as z:
        z.writestr('[Content_Types].xml', ''.join(ct))
        z.writestr('_rels/.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="%s/officeDocument" Target="xl/workbook.xml"/></Relationships>' % REL)
        z.writestr('xl/workbook.xml', workbook)
        z.writestr('xl/_rels/workbook.xml.rels', wbrels)
        z.writestr('xl/styles.xml', styles)
        z.writestr('xl/worksheets/sheet1.xml', sheet)
        if srels:
            z.writestr('xl/worksheets/_rels/sheet1.xml.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">%s</Relationships>' % ''.join(srels))
        if '--table' in flags:
            t = ('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<table xmlns="%s" id="1" name="Table1" displayName="Table1" ref="%s1:%s1001"><autoFilter ref="%s1:%s1001"/>'
                 '<tableColumns count="3">%s</tableColumns><tableStyleInfo name="TableStyleMedium2" showFirstColumn="0" showLastColumn="0" showRowStripes="1" showColumnStripes="0"/></table>'
                 % (NS, tcol[0], tcol[2], tcol[0], tcol[2], ''.join('<tableColumn id="%d" name="H%d"/>' % (i + 1, i) for i in range(3))))
            z.writestr('xl/tables/table1.xml', t)
        if '--chart' in flags:
            drel = 'http://schemas.openxmlformats.org/drawingml/2006/chart'
            z.writestr('xl/drawings/drawing1.xml',
                       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<xdr:wsDr xmlns:xdr="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing" xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
                       '<xdr:twoCellAnchor><xdr:from><xdr:col>%d</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>2</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:from><xdr:to><xdr:col>%d</xdr:col><xdr:colOff>0</xdr:colOff><xdr:row>20</xdr:row><xdr:rowOff>0</xdr:rowOff></xdr:to>'
                       '<xdr:graphicFrame macro=""><xdr:nvGraphicFramePr><xdr:cNvPr id="2" name="Chart 1"/><xdr:cNvGraphicFramePr/></xdr:nvGraphicFramePr><xdr:xfrm><a:off x="0" y="0"/><a:ext cx="0" cy="0"/></xdr:xfrm>'
                       '<a:graphic><a:graphicData uri="%s"><c:chart xmlns:c="%s" xmlns:r="%s" r:id="rId1"/></a:graphicData></a:graphic></xdr:graphicFrame><xdr:clientData/></xdr:twoCellAnchor></xdr:wsDr>'
                       % (tablec + 4, tablec + 12, drel, drel, REL))
            z.writestr('xl/drawings/_rels/drawing1.xml.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                       '<Relationship Id="rId1" Type="%s/chart" Target="../charts/chart1.xml"/></Relationships>' % REL)

            def ser(i, ref, cat):
                return ('<c:ser><c:idx val="%d"/><c:order val="%d"/><c:cat><c:numRef><c:f>%s</c:f></c:numRef></c:cat><c:val><c:numRef><c:f>%s</c:f></c:numRef></c:val></c:ser>' % (i, i, cat, ref))
            z.writestr('xl/charts/chart1.xml',
                       '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<c:chartSpace xmlns:c="http://schemas.openxmlformats.org/drawingml/2006/chart" xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"><c:chart><c:plotArea><c:barChart><c:barDir val="col"/><c:grouping val="clustered"/>'
                       + ser(0, 'Sheet1!$B$2:$B$201', 'Sheet1!$A$2:$A$201') + ser(1, 'Sheet1!$C$1:$C$%d' % rows, 'Sheet1!$A$1:$A$%d' % rows)
                       + '<c:axId val="1"/><c:axId val="2"/></c:barChart><c:catAx><c:axId val="1"/><c:scaling><c:orientation val="minMax"/></c:scaling><c:delete val="0"/><c:axPos val="b"/><c:crossAx val="2"/></c:catAx>'
                       '<c:valAx><c:axId val="2"/><c:scaling><c:orientation val="minMax"/></c:scaling><c:delete val="0"/><c:axPos val="l"/><c:crossAx val="1"/></c:valAx></c:plotArea></c:chart></c:chartSpace>')


main()
