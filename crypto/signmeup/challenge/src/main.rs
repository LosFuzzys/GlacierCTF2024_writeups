use curve25519_dalek::edwards::CompressedEdwardsY;
use curve25519_dalek::Scalar;
use hex;
use rand::Rng;
use std::io::{BufRead, Write};
use std::{error::Error, io};
use std::fs;

use rand::rngs::OsRng;
use ed25519::PrivateKey;

mod ed25519;

fn generate_challenge(length: usize) -> String {
    const CHARSET: &[u8] = b"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    let mut rng = OsRng;
    let challenge: String = (0..length)
        .map(|_| {
            let idx = rng.gen_range(0..CHARSET.len());
            CHARSET[idx] as char
        })
        .collect();
    challenge
}


fn main() -> Result<(), Box<dyn Error>> {
    let flag = fs::read_to_string("flag.txt")?;
    let flag = flag.trim();

    let mut csprng = OsRng;

    let key_bytes: [u8; 32] = csprng.gen();
    let private_key = PrivateKey::new(key_bytes);

    println!("public key: {}", hex::encode(private_key.public_key.as_bytes()));

    loop {
        print!("msg> ");
        io::stdout().lock().flush()?;

        let mut line = String::new();
        std::io::stdin().lock().read_line(&mut line)?;
        let line = line.trim();

        if line.is_empty() {
            break;
        }

        let (r, s) = private_key.sign(line.as_bytes());

        println!("signature: {} {}", hex::encode(r.as_bytes()), hex::encode(s.as_bytes()));
    }

    let challenge = generate_challenge(32);
    println!("sign this: {}", challenge);
    print!("signature> ");
    io::stdout().lock().flush()?;

    let mut line = String::new();
    std::io::stdin().lock().read_line(&mut line)?;

    let (r_hex, s_hex) = line.trim().split_once(' ').ok_or("bad input")?;

    let r_bytes = hex::decode(r_hex)?;
    let s_bytes = hex::decode(s_hex)?;

    let r = CompressedEdwardsY::from_slice(&r_bytes)?;
    let s: Option<Scalar> = Scalar::from_canonical_bytes(s_bytes.try_into().map_err(|_| "invalid scalar length")?).into();
    let s = s.ok_or("scalar out of range")?;

    match private_key.public_key.verify(challenge.as_bytes(), (r, s)) {
        Ok(_) => println!("{}", flag),
        Err(_) => println!("Better luck next time"),
    }

    Ok(())
}
