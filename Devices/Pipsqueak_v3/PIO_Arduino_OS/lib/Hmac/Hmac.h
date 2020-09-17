/*
 * Calculates HMACs using SHA-256.
 *
 * A valid HMAC ensures that a message was generated using the (presumably)
 * private key shared by sender and receiver, and that the message has not
 * been modified.
 *
 * The sender and receiver must share a private key. If the key is leaked,
 * message authenticity cannot be assured.
 *
 * Adapted from https://github.com/budnail/Cryptosuite.
 */

#ifndef Hmac_h
#define Hmac_h

#include <Arduino.h>

// Lengths expressed in bytes
#define SHA_256_KEY_LENGTH 32
#define SHA_256_HASH_LENGTH 32
#define SHA_256_BLOCK_LENGTH 64
#define SHA_256_BUFFER_SIZE 64

// The values of ipad and opad are arbitrary, but must not be equal
#define SHA_256_HMAC_IPAD 0x36
#define SHA_256_HMAC_OPAD 0x5c

/**
 * Generates an HMAC.
 *
 * Not for use within ISRs.
 */
class Hmac {
  public:
    Hmac(const byte * secretKey, const byte * messageContent, size_t messageContentLength);

    /**
     * Writes the HMAC value to a provided buffer.
     * The buffer's size must be at least length() bytes.
     */
    void write(void * buffer);

    /**
     * Compares the value of this HMAC to another HMAC value.
     */
    bool equals(const byte * hmac);

    /** Returns the length of the HMAC in bytes. */
    size_t length();

  private:
    /** Initializes the SHA256 process, setting the counter and buffers. */
    void init(void);

    /** Pads the last block and reverses the byte order. */
    byte * padAndReverseByteOrder(void);

    /** Adds data to the buffer and increases the byteCount variable. */
    void write(byte);

    /**
     * Implement SHA-1 padding (fips180-2 5.1.1).
     * Pad with 0x80 followed by 0x00 until the end of the block
     */
    void pad();

    /** Adds data to the buffer. */
    void addUncounted(byte data);

    /** Hashes a single block of data. */
    void hashBlock();

    /** Rotate a 32-bit value left by the given number of bits. */
    uint32_t ror32(uint32_t number, uint8_t bits);

    union _buffer {
      uint8_t b[SHA_256_BLOCK_LENGTH];
      uint32_t w[SHA_256_BLOCK_LENGTH/4];
    };

    union _state {
      uint8_t b[SHA_256_HASH_LENGTH];
      uint32_t w[SHA_256_HASH_LENGTH/4];
    };

    _buffer buffer;
    uint8_t bufferOffset;
    _state state;
    uint32_t byteCount;
    byte keyBuffer[SHA_256_BLOCK_LENGTH];
    byte innerHash[SHA_256_HASH_LENGTH];
};

#endif // Hmac.h
