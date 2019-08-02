## PDF generation from LaTeX

`/source` is LaTeX source files (content) without formatting.

`/format_*` is journal-specific formatting and machine-generated files.

If you edit the paper, please edit the corresponting `/source`.
Then, rebuild the paper using one of the formats depending on your personal taste.
I'm partial to `format_ieeecs` myself.
Open the `.tex` file there using a LaTeX app.

To minimize archive size, you may delete all `/format_*` files without
a .tex or .bbl filename extension by running `condense.bat`.
That also deletes the PDFs,
so before you zap everything copy the desired PDF to this directory
so that the paper is the latest version.
Use `qtdsp.pdf` as the filename to avoid breaking external links.

Figures are in Inkscape SVG format, exported as eps. See `/source`.

### Tools for Rebuilding the PDF

You'll need LaTeX. I use TexStudio. It's a front end for TexWorks,
which is itself a front end for TeX. You'll need to download all three.
Try to open `.tex` files in TexStudio. It seems better at resolving dependencies.

Inkscape did a fine job of creating SVG files, which can be exported as `eps`
(Encapsulated Postscript) for inclusion in LaTeX.
Vector formats are much better than bitmaps when it comes to publishing.
Please don't bloat the PDF if you can help it.
