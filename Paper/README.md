## PDF generation from LaTeX

`/source` is LaTeX source files (content) without formatting.

`/format_*` is journal-specific formatting and machine-generated files.

If you edit the paper, please edit the corresponting `/source`. Then, rebuild the paper using one of the formats depending on your personal taste. I'm partial to `format_ieeecs` myself. Open the `.tex` file there and you're in business.

To minimize archive size, you may delete all `/format_*` files without
a .tex or .bbl using condense.bat. That does delete the PDFs. Before you zap everything, copy the desired PDF to this directory. Use `etime.pdf` to avoid breaking links.

Figures are in Inkscape SVG format, exported as eps. See `/source`.

### Tools for Rebuilding the PDF

You'll need Latex. I use TexStudio. It's a front end for TexWorks. You'll need to download both. Try to open `.tex` files in TexStudio. It seems better at resolving dependencies.

Inkscape did a fine job of creating SVG files, which can be exported as `eps` (Encapsulated Postscript) for inclusion in LaTeX. Vector formats are much better than bitmaps when it comes to publishing. Please don't bloat the PDF if you can help it.
