rem - You will need to run BibTex and then LaTeX to
rem - rebuild the PDF and other files after this.

rem - This batch file deletes all unnecessary files
rem - to minimize archive space.

del .\format_aps\*.blg
del .\format_aps\*.log
del .\format_aps\*.pdf
del .\format_aps\*.gz
del .\format_aps\*.bib
del .\format_aps\*.log
del .\format_aps\*.spl
del .\format_aps\*.out

del .\format_ieeecs\*.blg
del .\format_ieeecs\*.log
del .\format_ieeecs\*.pdf
del .\format_ieeecs\*.gz
del .\format_ieeecs\*.bib
del .\format_ieeecs\*.log
del .\format_ieeecs\*.spl
del .\format_ieeecs\*.out

del .\format_elsevier\*.blg
del .\format_elsevier\*.log
del .\format_elsevier\*.pdf
del .\format_elsevier\*.gz
del .\format_elsevier\*.bib
del .\format_elsevier\*.log
del .\format_elsevier\*.spl
del .\format_elsevier\*.out

del .\format_ebook\*.blg
del .\format_ebook\*.log
del .\format_ebook\*.pdf
del .\format_ebook\*.gz
del .\format_ebook\*.bib
del .\format_ebook\*.log
del .\format_ebook\*.spl
del .\format_ebook\*.out
