import struct


def lagrange_interpolate(shares: list[tuple[int, int]]) -> int:
    reconstructed_share = 0

    for i, (xi, yi) in enumerate(shares):
        prod = 1
        for j, (xj, yj) in enumerate(shares):
            if i == j:
                continue

            prod *= (-xj)/(xi - xj)
        reconstructed_share += prod * int(yi[0])

    return int(reconstructed_share)


if __name__ == '__main__':
    with open("replica_1_challenge.bin", "rb") as replica_1:
        replica_1_data = replica_1.read()

    with open("replica_2_challenge.bin", "rb") as replica_2:
        replica_2_data = replica_2.read()

    with open("replica_3_challenge.bin", "rb") as replica_3:
        replica_3_data = replica_3.read()

    assert len(replica_1_data) == len(replica_2_data) == len(replica_3_data)

    reconstructed_bytes = []
    for i in range(0, len(replica_1_data), 8):
        reconstructed_bytes.append(lagrange_interpolate([
            (1, struct.unpack("<Q", replica_1_data[i:i+8])),
            (2, struct.unpack("<Q", replica_2_data[i:i+8])),
            (3, struct.unpack("<Q", replica_3_data[i:i+8])),
        ]))

    with open("reconstructed.bin", "wb") as reconstructed:
        reconstructed.write(bytes(reconstructed_bytes))