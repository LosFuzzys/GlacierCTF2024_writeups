# Writeup

This challenge is similar to last years pyjail _Avatar_.

In comparison to _Avatar_, _Korra_ has even fewer whitelisted characters.
The main difference is that there are no arithmetics operations allowed (except potentially `>`, right shift).

However, the challenge includes one more nested `eval`.

The goal of the challenge is to build a string that pops a shell when `eval`'ed in the outermost `eval`.
Thus, the goal for the first nested eval is to build a format string generating a standard `eval` sandbox escape.

The (unoptimized) exploit uses the following payload for the outermost eval:

```python
[ x.__init__.__globals__ for x in ''.__class__.__base__.__subclasses__() if 'wrapper' not in f'{x.__init__}' and 'builtins' in x.__init__.__globals__ ][0]['builtins'].__import__('os').system('/bin/sh')

# (source: https://book.hacktricks.xyz/generic-methodologies-and-resources/python/bypass-python-sandboxes)
```

This is a generic payload that recovers builtins, then executes `os.system('/bin/sh')`


In the innermost payload, we can use the following inputs to generate `1`'s and `0`'s:

```python
ONE = """{"c">"a":d}"""
ZERO = """{"a">"c":d}"""
```


Now that we have 1's and 0's, we can build a format string that combines them to single characters.
For example, `f"{0b1000001:c}"` results in the letter "A", when evaluated by the second nested `eval`.
This way, we can build arbitrary strings.
Now, the only difficulty is to make sure that the right f-strings are evaluated at the right time, which is a bit finnicky.

The final payload is 18k characters long, so we won't include it in the writeup.

Note: The intended solution only works on Python 3.12 or newer, since Python 3.11 doesn't allow for nested f-strings.