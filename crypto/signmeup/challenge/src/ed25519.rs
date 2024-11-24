use std::error::Error;

use curve25519_dalek::{edwards::CompressedEdwardsY, EdwardsPoint, Scalar};
use ed25519_dalek::{hazmat::ExpandedSecretKey, SigningKey};
use digest::Digest;

use sha1::Sha1;

type HashType = Sha1;
const HASH_LEN: usize = 20;

pub struct PrivateKey {
    hash_prefix: [u8; 32],
    secret_scalar: Scalar,
    pub public_key: PublicKey,
}

pub struct PublicKey{
    compressed: CompressedEdwardsY,
    point: EdwardsPoint,
}

impl PrivateKey {
    pub fn new(secret_seed: [u8; 32]) -> Self {
        let key_pair: SigningKey = SigningKey::from_bytes(&secret_seed);
        let expanded_key: ExpandedSecretKey = (&secret_seed).into();
        let public_key = PublicKey::from_bytes(key_pair.verifying_key().as_bytes()).unwrap();
        Self {
            hash_prefix: expanded_key.hash_prefix,
            secret_scalar: expanded_key.scalar,
            public_key,
        }
    }

    pub fn sign(
        &self,
        message: &[u8],
    ) -> (CompressedEdwardsY, Scalar)
    {
        let mut h = HashType::new();
        h.update(&self.hash_prefix);
        h.update(message);

        let mut hash_val = [0u8; 64];
        hash_val[0..HASH_LEN].copy_from_slice(h.finalize().as_slice());

        let r_scalar = Scalar::from_bytes_mod_order_wide(&hash_val);
        let r: CompressedEdwardsY = EdwardsPoint::mul_base(&r_scalar).compress();

        let mut h = HashType::new();
        h.update(r.as_bytes());
        h.update(self.public_key.compressed.as_bytes());
        h.update(message);

        let mut hash_val = [0u8; 64];
        hash_val[0..HASH_LEN].copy_from_slice(h.finalize().as_slice());

        let h_scalar = Scalar::from_bytes_mod_order_wide(&hash_val);
        let s: Scalar = (h_scalar * self.secret_scalar) + r_scalar;

        (r, s)
    }
}

impl PublicKey {
    pub fn from_bytes(bytes: &[u8]) -> Result<Self, Box<dyn Error>> {
        let compressed = CompressedEdwardsY::from_slice(bytes)?;
        let point = compressed.decompress().ok_or("decompression failed")?;
        Ok(Self{compressed, point})
    }

    pub fn verify(&self, msg: &[u8], signature: (CompressedEdwardsY, Scalar)) -> Result<(), ()>
    {
        let (r, s) = signature;
        let mut h = HashType::new();
        h.update(r.as_bytes());
        h.update(self.compressed.as_bytes());
        h.update(msg);

        let mut hash_val = [0u8; 64];
        hash_val[0..HASH_LEN].copy_from_slice(h.finalize().as_slice());

        let c = Scalar::from_bytes_mod_order_wide(&hash_val);
        let minus_a = -self.point;

        let expected_r = EdwardsPoint::vartime_double_scalar_mul_basepoint(&c, &minus_a, &s);
        let expected_r = expected_r.compress();

        match r == expected_r {
            true => Ok(()),
            false => Err(()),
        }
    }

    pub fn as_bytes(&self) -> &[u8] {
        return self.compressed.as_bytes();
    }
}
