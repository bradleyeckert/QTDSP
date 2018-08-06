## PDF generation from LaTeX

`/source` is LaTeX source files (content) without formatting.

`/format_*` is journal-specific formatting and machine-generated files.

If you edit the paper, please edit the corresponting `/source`. Then, rebuild the paper using one of the formats depending on your personal taste.

To minimize archive size, you may delete all `/format_*` files without
a .tex or .bbl using condense.bat. That does delete the PDFs. Before you zap everything, copy the desired PDF to this directory.

Figures are in Inkscape SVG format, exported as eps. See `/source`.

### Rebuilding the PDF

You'll need Latex. I used TexStudio. It's a front end for TexWorks. You'll need to download both.

