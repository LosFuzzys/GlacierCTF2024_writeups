# Writeup

Sophie-Germain-Primes are prime numbers of the form q = 2p + 1 where p is also a prime.
In this challenge, sophie-germain-primes are used as prime numbers for RSA.
The problem is, that the primes `q`, `r` and `s` can be computed from `p` which makes this usage insecure.

Since we know that $$q=2p+1$$, $$r=2q+1=4p+3$$ and $$s=2r+1=2*(4p+3)=8p+7$$, we can rewrite
$$N = p*q*r*s = 64*p^4 + 136*p^3 + 94*p^2 + 21*p$$

This can be solved for `p`, we compute `q`, `r`, `s` and with these values we compute $$\phi(N) = (p-1)*(q-1)*(r-1)*(s-1)$$
Calculating the private exponent is according to textbook-rsa $$d \equiv e^{-1} \pmod{\phi(N)}$$
And decrypting the message is done $$PT \equiv CT^{d} \pmod{N}$$