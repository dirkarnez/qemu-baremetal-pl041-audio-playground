
#include <stdint.h>

// --- PL041 Register Definitions (Base: 0x10004000) ---
#define PL041_BASE       0x10004000
#define AACI_CR          (*(volatile uint32_t *)(PL041_BASE + 0x00))
#define AACI_DR1         (*(volatile uint32_t *)(PL041_BASE + 0x14))
#define AACI_TXCR1       (*(volatile uint32_t *)(PL041_BASE + 0x1C))
#define AACI_SR1         (*(volatile uint32_t *)(PL041_BASE + 0x2C))

// --- Register Bit Definitions ---
#define AACI_CR_EN       (1 << 0)   // Enable Advanced Audio Component Interface
#define TXCR_TXEN        (1 << 0)   // Transmit Enable
#define TXCR_COMP_16BIT  (1 << 15)  // 16-bit audio format
#define SR_TXHE          (1 << 2)   // Transmit FIFO Half Empty flag

// --- Audio Constants ---
#define SINE_TABLE_SIZE  32

// Simple 32-step pre-computed 16-bit signed sine wave table (Peak amplitude ~16000)
// This avoids needing math.h or floating point emulation in your bare-metal setup
const int16_t sine_wave[SINE_TABLE_SIZE] = {
    0, 3118, 6119, 8900, 11362, 13416, 14986, 16012,
    16462, 16321, 15598, 14324, 12550, 10343, 7784, 4967,
    1966, -1071, -4107, -6994, -9616, -11867, -13658, -14917,
    -15600, -15682, -15160, -14053, -12398, -10243, -7701, -4907
};

void init_audio(void) {
    // 1. Reset and Enable the primary AACI Controller
    AACI_CR = AACI_CR_EN;
    
    // 2. Configure Channel 1 for 16-bit mode and Enable TX
    // (Note: In a true HW environment you must also configure the AC97 codec registers,
    // but QEMU's PL041 emulation bypasses codec negotiation and accepts straight writes)
    AACI_TXCR1 = TXCR_COMP_16BIT | TXCR_TXEN;
}

void play_sine(void) {
    uint32_t table_index = 0;
    
    init_audio();

    while (1) {
        // Wait until the Transmit FIFO is Half Empty
        while (!(AACI_SR1 & SR_TXHE)) {
            // Busy loop waiting for QEMU / Windows host to consume audio data
        }

        // The PL041 FIFO depth handles multiple entries. 
        // When half-empty, we can safely write a small batch of samples.
        for (int i = 0; i < 8; i++) {
            int16_t sample = sine_wave[table_index];
            
            // For Stereo configuration: pack the same 16-bit sample into both Left and Right
            // Left channel = lower 16 bits, Right channel = upper 16 bits
            uint32_t stereo_sample = ((uint32_t)sample & 0xFFFF) | ((uint32_t)sample << 16);
            
            // Feed the register
            AACI_DR1 = stereo_sample;

            // Increment index and cycle the loop
            table_index = (table_index + 1) % SINE_TABLE_SIZE;
        }
    }
}
