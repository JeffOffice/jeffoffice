#!/usr/bin/env python3
"""Makes a workbook for timing a pivot table refresh: a sheet of `rows` rows by `cols` columns (the first two text, the rest numbers), and a pivot table on it
(Region down the side, Month across, the sum of the third column) with its cache and records.

  crunch_pivot_bench_make.py <out.xlsx> [rows=100000] [cols=20]
"""
import sys
import zipfile


def main():
    out = sys.argv[1]
    rows = int(sys.argv[2]) if len(sys.argv) > 2 else 100000
    cols = int(sys.argv[3]) if len(sys.argv) > 3 else 20
    regions = ['Region %d' % i for i in range(40)]
    months = ['M%d' % i for i in range(12)]
    strings = ['F%d' % (c + 1) for c in range(cols)] + regions + months
    base_region, base_month = cols, cols + len(regions)

    def col(c):
        s = ''
        c += 1
        while c:
            c, r = divmod(c - 1, 26)
            s = chr(65 + r) + s
        return s

    def num(r, c):
        return (r * 7 + c) % 1000

    with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
                   '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>'
                   '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>'
                   '<Override PartName="/xl/worksheets/sheet1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
                   '<Override PartName="/xl/sharedStrings.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sharedStrings+xml"/>'
                   '<Override PartName="/xl/pivotCache/pivotCacheDefinition1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheDefinition+xml"/>'
                   '<Override PartName="/xl/pivotCache/pivotCacheRecords1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotCacheRecords+xml"/>'
                   '<Override PartName="/xl/pivotTables/pivotTable1.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.pivotTable+xml"/></Types>')
        z.writestr('_rels/.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        z.writestr('xl/workbook.xml', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
                   'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"><sheets><sheet name="Sheet1" sheetId="1" r:id="rId1"/></sheets>'
                   '<pivotCaches><pivotCache cacheId="1" r:id="rId3"/></pivotCaches></workbook>')
        z.writestr('xl/_rels/workbook.xml.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet1.xml"/>'
                   '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings" Target="sharedStrings.xml"/>'
                   '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="pivotCache/pivotCacheDefinition1.xml"/></Relationships>')
        z.writestr('xl/sharedStrings.xml', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><sst xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="%d" uniqueCount="%d">%s</sst>'
                   % (len(strings), len(strings), ''.join('<si><t>%s</t></si>' % s for s in strings)))
        z.writestr('xl/worksheets/_rels/sheet1.xml.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotTable" Target="../pivotTables/pivotTable1.xml"/></Relationships>')
        with z.open('xl/worksheets/sheet1.xml', 'w', force_zip64=True) as f:
            f.write(b'<?xml version="1.0" encoding="UTF-8" standalone="yes"?><worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main"><sheetData>')
            chunk = []
            for r in range(rows + 1):
                cells = []
                for c in range(cols):
                    ref = '%s%d' % (col(c), r + 1)
                    if r == 0:
                        cells.append('<c r="%s" t="s"><v>%d</v></c>' % (ref, c))
                    elif c == 0:
                        cells.append('<c r="%s" t="s"><v>%d</v></c>' % (ref, base_region + r % 40))
                    elif c == 1:
                        cells.append('<c r="%s" t="s"><v>%d</v></c>' % (ref, base_month + r % 12))
                    else:
                        cells.append('<c r="%s"><v>%d</v></c>' % (ref, num(r, c)))
                chunk.append('<row r="%d">%s</row>' % (r + 1, ''.join(cells)))
                if len(chunk) >= 2000:
                    f.write(''.join(chunk).encode())
                    chunk = []
            f.write(''.join(chunk).encode())
            f.write(b'</sheetData></worksheet>')
        shared = lambda items: ''.join('<s v="%s"/>' % i for i in items)
        fields = ['<cacheField name="F1" numFmtId="0"><sharedItems count="%d">%s</sharedItems></cacheField>' % (len(regions), shared(regions)),
                  '<cacheField name="F2" numFmtId="0"><sharedItems count="%d">%s</sharedItems></cacheField>' % (len(months), shared(months))]
        for c in range(2, cols):
            fields.append('<cacheField name="F%d" numFmtId="0"><sharedItems containsSemiMixedTypes="0" containsString="0" containsNumber="1" containsInteger="1" minValue="0" maxValue="999"/></cacheField>' % (c + 1))
        z.writestr('xl/pivotCache/pivotCacheDefinition1.xml', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><pivotCacheDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
                   'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" r:id="rId1" refreshedBy="x" refreshedDate="45000" createdVersion="6" refreshedVersion="6" recordCount="%d">'
                   '<cacheSource type="worksheet"><worksheetSource ref="A1:%s%d" sheet="Sheet1"/></cacheSource><cacheFields count="%d">%s</cacheFields></pivotCacheDefinition>'
                   % (rows, col(cols - 1), rows + 1, cols, ''.join(fields)))
        z.writestr('xl/pivotCache/_rels/pivotCacheDefinition1.xml.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheRecords" Target="pivotCacheRecords1.xml"/></Relationships>')
        with z.open('xl/pivotCache/pivotCacheRecords1.xml', 'w', force_zip64=True) as f:
            f.write(('<?xml version="1.0" encoding="UTF-8" standalone="yes"?><pivotCacheRecords xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" count="%d">' % rows).encode())
            chunk = []
            for r in range(1, rows + 1):
                chunk.append('<r><x v="%d"/><x v="%d"/>%s</r>' % (r % 40, r % 12, ''.join('<n v="%d"/>' % num(r, c) for c in range(2, cols))))
                if len(chunk) >= 2000:
                    f.write(''.join(chunk).encode())
                    chunk = []
            f.write(''.join(chunk).encode())
            f.write(b'</pivotCacheRecords>')
        fields = ['<pivotField axis="axisRow" compact="0" outline="0" showAll="0"><items count="%d">%s<item t="default"/></items></pivotField>' % (len(regions) + 1, ''.join('<item x="%d"/>' % i for i in range(len(regions)))),
                  '<pivotField axis="axisCol" compact="0" outline="0" showAll="0"><items count="%d">%s<item t="default"/></items></pivotField>' % (len(months) + 1, ''.join('<item x="%d"/>' % i for i in range(len(months)))),
                  '<pivotField dataField="1" compact="0" outline="0" showAll="0"/>'] + ['<pivotField compact="0" outline="0" showAll="0"/>'] * (cols - 3)
        z.writestr('xl/pivotTables/pivotTable1.xml', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><pivotTableDefinition xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" name="PivotTable1" cacheId="1" '
                   'dataCaption="Values" compact="0" compactData="0"><location ref="%s3:%s%d" firstHeaderRow="1" firstDataRow="2" firstDataCol="1"/><pivotFields count="%d">%s</pivotFields>'
                   '<rowFields count="1"><field x="0"/></rowFields><colFields count="1"><field x="1"/></colFields><dataFields count="1"><dataField name="Sum of F3" fld="2" baseField="0" baseItem="0"/></dataFields></pivotTableDefinition>'
                   % (col(cols + 1), col(cols + 14), 4 + 40, cols, ''.join(fields)))
        z.writestr('xl/pivotTables/_rels/pivotTable1.xml.rels', '<?xml version="1.0" encoding="UTF-8" standalone="yes"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
                   '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/pivotCacheDefinition" Target="../pivotCache/pivotCacheDefinition1.xml"/></Relationships>')


if __name__ == '__main__':
    main()
