#!/usr/bin/env python3
import os
from typing import List
from Crypto.Util.Padding import pad, unpad
import aes

NORMAL_ROUNDS = 22
PREMIUM_ROUNDS = 24
PREMIUM_USER = b"premium"


def expand_key(key: bytes, rounds: int) -> List[List[int]]:
    round_keys = [[key[i:i + 4] for i in range(0, len(key), 4)]]
    while len(round_keys) < rounds + 1:
        base_key = b"".join(round_keys[-1])
        round_keys += aes.expand_key(base_key, 10)[1:]
    round_keys = [b"".join(k) for k in round_keys]
    round_keys = [aes.bytes2matrix(k) for k in round_keys]
    return round_keys[:rounds + 1]


def encrypt_block(pt: bytes, key: bytes, rounds: int) -> bytes:
    if len(pt) != 16 or len(key) != 16:
        raise ValueError("Invalid input length")

    subkeys = expand_key(key, rounds)
    assert len(subkeys) == rounds + 1

    block = aes.bytes2matrix(pt)
    aes.add_round_key(block, subkeys[0])

    for i in range(1, rounds+1):
        aes.sub_bytes(block)
        aes.shift_rows(block)
        aes.mix_columns(block)
        aes.add_round_key(block, subkeys[i])

    return aes.matrix2bytes(block)


def decrypt_block(ct: bytes, key: bytes, rounds: int) -> bytes:
    if len(ct) != 16 or len(key) != 16:
        raise ValueError("Invalid input length")

    subkeys = expand_key(key, rounds)[::-1]
    assert len(subkeys) == rounds + 1

    block = aes.bytes2matrix(ct)

    for i in range(rounds):
        aes.add_round_key(block, subkeys[i])
        aes.inv_mix_columns(block)
        aes.inv_shift_rows(block)
        aes.inv_sub_bytes(block)

    aes.add_round_key(block, subkeys[-1])

    return aes.matrix2bytes(block)


def encrypt_msg(pt: str, key: bytes, premium: bool) -> str:
    pt_bytes = pad(pt.encode(), 16)
    pt_blocks = [pt_bytes[i:i + 16] for i in range(0, len(pt_bytes), 16)]

    for block in pt_blocks[1:]:
        if block.startswith(PREMIUM_USER):
            raise ValueError("Invalid plaintext")

    if len(pt_blocks) > 3:
        raise ValueError("Message too long")

    rounds = PREMIUM_ROUNDS if premium else NORMAL_ROUNDS
    ct = b"".join([encrypt_block(block, key, rounds) for block in pt_blocks])
    return ct.hex()


def decrypt_msg(ct: str, key: bytes, premium: bool) -> str:
    ct_bytes = bytes.fromhex(ct)
    ct_blocks = [ct_bytes[i:i + 16] for i in range(0, len(ct_bytes), 16)]

    if len(ct_blocks) > 3:
        raise ValueError("Ciphertext too long")

    rounds = PREMIUM_ROUNDS if premium else NORMAL_ROUNDS
    pt = b"".join([decrypt_block(block, key, rounds) for block in ct_blocks])
    return unpad(pt, 16).decode()


def main():
    key = os.urandom(16)
    attempts = 0
    max_attempts = 3

    while attempts <= max_attempts:
        option = input(
            "Enter option (1: Encrypt, 2: Premium Encrypt, 3: Guess Key): ")

        if option == "1":
            pt = input("Enter plaintext: ")
            if pt == PREMIUM_USER.decode():
                print("Not so fast my friend.")
                return
            ct = encrypt_msg(pt, key, False)
            print(f"Ciphertext: {ct}")

        elif option == "2":
            ct = input("Enter ciphertext: ")
            try:
                if decrypt_msg(ct, key, False) != PREMIUM_USER.decode():
                    print("Sorry, you are not a premium user.")
                    return
            except Exception:
                print("Error")
                return

            pt = input("Enter plaintext: ")
            ct = encrypt_msg(pt, key, True)
            print(f"Ciphertext: {ct}")

        elif option == "3":
            key_guess = input("Enter key (hex): ")
            try:
                key_guess = bytes.fromhex(key_guess)
                if key_guess == key:
                    print("Correct key!")
                    with open('/app/flag.txt', 'r') as flag:
                        print(f"Flag: {flag.read().strip()}")
                    return
                else:
                    print("Incorrect key.")
            except ValueError:
                print("Invalid key format. Please enter a valid hex string.")

        else:
            print("Invalid option. Please choose 1, 2, or 3.")

        attempts += 1

    print("Maximum attempts reached. See you later!")


if __name__ == "__main__":
    main()
