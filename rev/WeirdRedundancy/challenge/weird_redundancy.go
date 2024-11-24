package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"math"
	"math/rand"
	"os"
	"strconv"
	"strings"
)

func evaluate_polynomial(a0 uint8, a_n []uint8, x uint32) (y uint64) {
	y = uint64(a0)

	for i, a_i := range a_n {
		y += uint64(x) * uint64(math.Pow(float64(a_i), float64(i+1)))
	}

	return
}

func main() {

	var num_replicas int
	k := 2

	reader := bufio.NewReader(os.Stdin)
	fmt.Print("File to compute redundancy: ")
	input_file, _ := reader.ReadString('\n')
	input_file = strings.Trim(input_file, "\n")

	fmt.Print("Number of replicas: ")
	fmt.Scan(&num_replicas)

	fmt.Sprintf("Creating replicas for %s (%d replicas)", input_file, num_replicas)

	var replica_files []os.File
	var replica_buffers = make([][]uint32, num_replicas)
	for i := 1; i <= num_replicas; i++ {
		replica_file, err := os.Create("replica_" + strconv.Itoa(i) + "_" + input_file)

		if err != nil {
			panic(err)
		}

		replica_files = append(replica_files, *replica_file)
		replica_buffers[i-1] = make([]uint32, 1024)
	}

	file, err := os.Open(input_file)
	if err != nil {
		panic(err)
	}
	defer file.Close()

	secret_buffer := make([]byte, 1024)
	random_values := make([]uint8, k)

	var share uint64
	share_binary := make([]byte, 8)
	for {
		n, err := file.Read(secret_buffer)
		if err != nil && err != io.EOF {
			fmt.Printf("Error reading file: %v\n", err)
			break
		}
		if n == 0 {
			break
		}

		for bit := 0; bit < n; bit++ {
			// For every bit in the secret buffer, we'll need to get K random values
			for i := 0; i < k; i++ {
				random_values[i] = uint8(rand.Uint32() >> 24)
			}

			// Create a share for all files
			for file_i, file := range replica_files {
				// Write each share-buffer into the according file
				share = evaluate_polynomial(secret_buffer[bit], random_values, uint32(file_i)+1)
				binary.LittleEndian.PutUint64(share_binary, share)
				file.Write(share_binary)
			}
		}
	}
}
