/**
 * @file ethcomm_test.c
 * @brief Bare-metal verification suite for EthComm MMIO loopback operations.
 */

#include <stdint.h>
#include <stdio.h>
#include "mmio.h"

#define ETHCOMM_BASE 0x30000000UL
#define ETHCOMM_SIZE 0x00100000UL // 1MB
#define CHUNK_SIZE   1500UL       // 1500 bytes per chunk

#define TOTAL_CHUNKS ((ETHCOMM_SIZE + CHUNK_SIZE - 1UL) / CHUNK_SIZE) // 700 chunks
#define NUM_HEAD_CHUNKS 3
#define NUM_RAND_CHUNKS 7
#define NUM_TAIL_CHUNKS 3
#define NUM_TEST_CHUNKS (NUM_HEAD_CHUNKS + NUM_RAND_CHUNKS + NUM_TAIL_CHUNKS) // 13 chunks

/**
 * @brief Main entry point executing MMIO verification cases.
 * @return 0 on success, non-zero error code on failure.
 */
int main(void) {
    volatile uint32_t *base32 = (volatile uint32_t *)ETHCOMM_BASE;
    volatile uint8_t  *base8  = (volatile uint8_t  *)ETHCOMM_BASE;

    printf("[INFO] Starting bare-metal MMIO verification suite...\n");

    // Test 1: 32-bit Word Read/Write Loopback
    uint32_t test_val32 = 0xDEADBEEF;
    base32[0] = test_val32;
    // Check if word readback matches expected value
    if (base32[0] != test_val32) {
        printf("[FAIL] Test 1 (32-bit Word Loopback): Read: 0x%08x, Expected: 0x%08x\n",
               (unsigned int)base32[0], (unsigned int)test_val32);
        // Return error code 1 indicating word loopback failure
        return 1;
    }
    printf("[PASS] Test 1: 32-bit Word Loopback verified (Head: [0x%02x 0x%02x], Tail: [0x%02x 0x%02x])\n",
           (unsigned int)base8[0], (unsigned int)base8[1],
           (unsigned int)base8[2], (unsigned int)base8[3]);

    // Test 2: Byte-Masked Write Verification
    base32[1] = 0x00000000;
    base8[4]  = 0xAA; // Byte access at offset 4 (Word 1, Byte 0)
    base8[5]  = 0xBB; // Byte access at offset 5 (Word 1, Byte 1)
    // Check if byte-mask logic properly combines into 32-bit word
    if (base32[1] != 0x0000BBAA) {
        printf("[FAIL] Test 2 (Byte-Masked Write Loopback): Read: 0x%08x, Expected: 0x0000BBAA\n",
               (unsigned int)base32[1]);
        // Return error code 2 indicating byte-mask logic failure
        return 2;
    }
    printf("[PASS] Test 2: Byte-Masked Write Loopback verified (Head: [0x%02x 0x%02x], Tail: [0x%02x 0x%02x])\n",
           (unsigned int)base8[4], (unsigned int)base8[5],
           (unsigned int)base8[6], (unsigned int)base8[7]);

    // Test 3: Upper Boundary Address Loopback (Offset: 1MB - 4 bytes)
    uint32_t last_idx = (ETHCOMM_SIZE / sizeof(uint32_t)) - 1;
    uint32_t last_byte_offset = ETHCOMM_SIZE - sizeof(uint32_t);
    uint32_t boundary_val = 0x12345678;
    base32[last_idx] = boundary_val;
    // Check if boundary word readback matches expected value
    if (base32[last_idx] != boundary_val) {
        printf("[FAIL] Test 3 (Upper Boundary Address Loopback): Index: %u, Read: 0x%08x, Expected: 0x%08x\n",
               (unsigned int)last_idx, (unsigned int)base32[last_idx], (unsigned int)boundary_val);
        // Return error code 3 indicating address decoding boundary failure
        return 3;
    }
    printf("[PASS] Test 3: Upper Boundary Address Loopback verified (Head: [0x%02x 0x%02x], Tail: [0x%02x 0x%02x])\n",
           (unsigned int)base8[last_byte_offset],
           (unsigned int)base8[last_byte_offset + 1],
           (unsigned int)base8[last_byte_offset + 2],
           (unsigned int)base8[last_byte_offset + 3]);

    // Test 4: Sampled 13 Chunks Loopback (3 Head, 7 Random Mid, 3 Tail)
    printf("[INFO] Running Test 4: Sampled chunk loopback (3 head, 7 mid-random, 3 tail)...\n");

    uint32_t target_chunks[NUM_TEST_CHUNKS];

    // Populate first 3 chunks: #0, #1, #2
    target_chunks[0] = 0;
    target_chunks[1] = 1;
    target_chunks[2] = 2;

    // Pseudo-random generation for 7 middle chunks in range [3, TOTAL_CHUNKS - 4]
    uint32_t lcg_state = 0x12345678;
    uint32_t mid_min = NUM_HEAD_CHUNKS;
    uint32_t mid_max = TOTAL_CHUNKS - NUM_TAIL_CHUNKS - 1; // 696
    uint32_t mid_range = mid_max - mid_min + 1;            // 694
    uint32_t generated_mid = 0;

    // Generate 7 unique pseudo-random chunk indices
    while (generated_mid < NUM_RAND_CHUNKS) {
        lcg_state = lcg_state * 1664525UL + 1013904223UL;
        uint32_t candidate = mid_min + (lcg_state % mid_range);
        uint8_t is_duplicate = 0;

        // Verify candidate chunk index is unique
        for (uint32_t k = 0; k < generated_mid; k++) {
            // Check for collision with previously selected middle chunk
            if (target_chunks[NUM_HEAD_CHUNKS + k] == candidate) {
                is_duplicate = 1;
                // Break out of uniqueness verification loop
                break;
            }
        }

        // Add candidate if no duplicate detected
        if (!is_duplicate) {
            target_chunks[NUM_HEAD_CHUNKS + generated_mid] = candidate;
            generated_mid++;
        }
    }

    // Populate last 3 chunks: #697, #698, #699
    target_chunks[10] = TOTAL_CHUNKS - 3;
    target_chunks[11] = TOTAL_CHUNKS - 2;
    target_chunks[12] = TOTAL_CHUNKS - 1;

    // Verify each designated chunk in target_chunks array
    for (uint32_t t = 0; t < NUM_TEST_CHUNKS; t++) {
        uint32_t chunk_idx = target_chunks[t];
        uint32_t offset = chunk_idx * CHUNK_SIZE;
        uint32_t current_chunk = CHUNK_SIZE;

        // Clip size if chunk exceeds boundary (applies to final chunk #699: 76 bytes)
        if (offset + current_chunk > ETHCOMM_SIZE) {
            current_chunk = ETHCOMM_SIZE - offset;
        }

        // Write deterministic byte pattern into MMIO region
        for (uint32_t i = 0; i < current_chunk; i++) {
            uint8_t pattern = (uint8_t)((offset + i) ^ 0xA5);
            base8[offset + i] = pattern;
        }

        // Read back and verify written byte pattern
        for (uint32_t i = 0; i < current_chunk; i++) {
            uint8_t expected = (uint8_t)((offset + i) ^ 0xA5);
            uint8_t actual = base8[offset + i];

            // Verify readback data integrity
            if (actual != expected) {
                printf("[FAIL] Test 4 (Sampled Chunked Loopback): Mismatch at offset 0x%08x (Chunk #%u). Read: 0x%02x, Expected: 0x%02x\n",
                       (unsigned int)(offset + i), (unsigned int)chunk_idx, actual, expected);
                // Return error code 4 indicating pattern mismatch
                return 4;
            }
        }

        printf("[INFO] Chunk #%03u passed: [0x%08x - 0x%08x] (%u bytes) | Head: [0x%02x 0x%02x], Tail: [0x%02x 0x%02x]\n",
               (unsigned int)chunk_idx,
               (unsigned int)offset,
               (unsigned int)(offset + current_chunk - 1),
               (unsigned int)current_chunk,
               (unsigned int)base8[offset],
               (unsigned int)base8[offset + 1],
               (unsigned int)base8[offset + current_chunk - 2],
               (unsigned int)base8[offset + current_chunk - 1]);
    }

    printf("[PASS] Test 4: Sampled 13 Chunks Loopback verified\n");

    printf("[INFO] All EthComm MMIO tests passed successfully!\n");
    // Return success status code
    return 0;
}