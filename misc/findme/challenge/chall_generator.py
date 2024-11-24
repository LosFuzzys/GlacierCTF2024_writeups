import pymupdf as UmPDF
from PIL import Image, ImageDraw, ImageFont

FLAG_TXT = 'flag.txt'
IMAGE_BASE = 'base.png'
FINAL_IMAGE = 'image.png'
PDF_BASE = 'base.pdf'
FINAL_PDF = 'chall.pdf'
FONT_FILE = 'TibetanMachineUni.ttf'

with open(FLAG_TXT) as f_file:
    img = Image.open(IMAGE_BASE)
    draw = ImageDraw.Draw(img)
    font = ImageFont.truetype(FONT_FILE, 50)
    flag = f_file.readline()
    draw.text((100, 1000), flag, (180,82,82), font=font)
    img.save(FINAL_IMAGE)

with UmPDF.open(PDF_BASE) as file:
    with open(FINAL_IMAGE, 'rb') as image_f:
        image = image_f.read()
        max_length = 100
        chunks = [
            image[i:i + max_length]
            for i in range(0, len(image), max_length)
        ]
        for chunk in chunks:
            xref = file.get_new_xref()
            file.update_object(xref, "<<>>")
            file.update_stream(xref, chunk, compress=False)
        file.save(FINAL_PDF)
