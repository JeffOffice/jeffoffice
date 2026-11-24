# Measures the stock fonts installed on a computer and writes numbers only (no font data):
#   widths.txt   lines in the format of resources/fontwidths.txt: family <tab> style <tab> widths
#   metrics.tsv  family, style, units per em, ascent, descent, line gap (design units)
# Usage (PowerShell, on a computer that has the fonts installed):
#   powershell -File measure_font_widths.ps1 -Families "Cavolini,Roboto" -CharsFrom fontwidths.txt -OutDir out
# -CharsFrom is a copy of resources/fontwidths.txt: its character list (the first line that is
# not a comment) is the list measured, so new lines match the file. Style: 0 regular, 1 bold,
# 2 italic, 3 bold italic. A style the family does not have is skipped (the file leaves it out too),
# and a character the font lacks is written as -1. Widths are 1/2048 em, rounded.
param([Parameter(Mandatory)][string]$Families, [Parameter(Mandatory)][string]$CharsFrom, [Parameter(Mandatory)][string]$OutDir)
Add-Type -AssemblyName PresentationCore, WindowsBase, System.Drawing
New-Item -ItemType Directory -Force $OutDir | Out-Null
$chars = ((Get-Content $CharsFrom | Where-Object { $_ -notmatch '^#' } | Select-Object -First 1) -split ',') | ForEach-Object { [int]$_ }
$widths = New-Object System.Collections.Generic.List[string]
$metrics = New-Object System.Collections.Generic.List[string]
$installed = (New-Object System.Drawing.Text.InstalledFontCollection).Families
foreach ($name in ($Families -split ',')) {
    $name = $name.Trim()
    $gdi = $installed | Where-Object { $_.Name -ieq $name } | Select-Object -First 1
    if (-not $gdi) { $metrics.Add("$name`tnot installed"); continue }
    foreach ($style in 0..3) {
        $weight = if ($style -band 1) { [System.Windows.FontWeights]::Bold } else { [System.Windows.FontWeights]::Normal }
        $slant = if ($style -band 2) { [System.Windows.FontStyles]::Italic } else { [System.Windows.FontStyles]::Normal }
        $gs = [System.Drawing.FontStyle]::Regular
        if ($style -band 1) { $gs = $gs -bor [System.Drawing.FontStyle]::Bold }
        if ($style -band 2) { $gs = $gs -bor [System.Drawing.FontStyle]::Italic }
        if (-not $gdi.IsStyleAvailable($gs)) { continue }
        $typeface = New-Object System.Windows.Media.Typeface((New-Object System.Windows.Media.FontFamily($gdi.Name)), $slant, $weight, [System.Windows.FontStretches]::Normal)
        $glyphs = $null
        if (-not $typeface.TryGetGlyphTypeface([ref]$glyphs)) { continue }
        # a face that is not really bold or italic is drawn by hand: leave it out, as the file does
        if (($style -band 1) -and $glyphs.Weight.ToOpenTypeWeight() -lt 600) { continue }
        if (($style -band 2) -and $glyphs.Style -eq [System.Windows.FontStyles]::Normal) { continue }
        $w = foreach ($c in $chars) {
            $g = 0
            if ($glyphs.CharacterToGlyphMap.TryGetValue($c, [ref]$g)) { [int][math]::Round($glyphs.AdvanceWidths[$g] * 2048) } else { -1 }
        }
        $widths.Add("$($gdi.Name)`t$style`t$($w -join ',')")
        $em = $gdi.GetEmHeight($gs); $asc = $gdi.GetCellAscent($gs); $desc = $gdi.GetCellDescent($gs)
        $metrics.Add("$($gdi.Name)`t$style`t$em`t$asc`t$desc`t$($gdi.GetLineSpacing($gs) - $asc - $desc)")
    }
}
[System.IO.File]::WriteAllText((Join-Path $OutDir 'widths.txt'), (($widths -join "`n") + "`n"))
[System.IO.File]::WriteAllText((Join-Path $OutDir 'metrics.tsv'), (($metrics -join "`n") + "`n"))
"measured $($widths.Count) faces"
