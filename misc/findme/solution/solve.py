import pymupdf as UmPDF

CHALL_PDF = 'chall.pdf'
SOLVE_IMG = 'solve.png'

with UmPDF.open(CHALL_PDF) as file:
    with open(SOLVE_IMG, 'wb') as image_f:
        chunks = []
        xreflen = file.xref_length()  # length of objects table
        for xref in range(20, xreflen):
            if stream := file.xref_stream(xref):
                chunks.append(stream)

        for chunk in chunks:
            image_f.write(chunk)

