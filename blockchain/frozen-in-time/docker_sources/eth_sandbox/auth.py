import os

def get_shared_secret():
    return os.getenv(
        "SHARED_SECRET",
        "bonehead_implosive_gift_among_abacus_envelope_king_bonding"
    )
