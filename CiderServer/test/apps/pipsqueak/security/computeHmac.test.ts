import computeHmac from '../../../../src/apps/pipsqueak/security/computeHmac';

describe('computeHmac()', () => {
  const testKey = Buffer.from([
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0a,
    0x0b,
    0x0c,
    0x0d,
    0x0e,
    0x0f,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1a,
    0x1b,
    0x1c,
    0x1d,
    0x1e,
    0x1f,
  ]);

  test('correctly computes hmac for 32-byte aligned message', () => {
    const messageText = 'The first 32 bytes of this message will be used';
    expect(messageText.length).toBeGreaterThan(32);
    const message = Buffer.alloc(32, messageText, 'utf-8');
    const actualHmac = computeHmac(message, 0, 32, testKey);
    const expectedHmac = Buffer.from([
      0xa0,
      0x6d,
      0xff,
      0x11,
      0xe6,
      0x8d,
      0xf4,
      0xff,
      0x8f,
      0x2a,
      0x2e,
      0xdd,
      0x89,
      0x4a,
      0x15,
      0x8d,
      0x3b,
      0x88,
      0x0a,
      0xc4,
      0x83,
      0xe4,
      0xd7,
      0x97,
      0x42,
      0xeb,
      0x8d,
      0xb6,
      0xd0,
      0x6f,
      0x40,
      0x3c,
    ]);
    expect(actualHmac).toEqual(expectedHmac);
  });

  test('correctly computes hmac for non-32-byte aligned message', () => {
    const messageText =
      'The first 48 bytes of this message will be used for hmac';
    expect(messageText.length).toBeGreaterThan(48);
    const message = Buffer.alloc(48, messageText, 'utf-8');
    const actualHmac = computeHmac(message, 0, 32, testKey);
    const expectedHmac = Buffer.from([
      0xd6,
      0x62,
      0x23,
      0x99,
      0x6b,
      0x3a,
      0x37,
      0xed,
      0xf7,
      0x42,
      0x49,
      0x53,
      0x32,
      0x2d,
      0x83,
      0x80,
      0x43,
      0xcc,
      0xac,
      0x03,
      0x36,
      0xcb,
      0xa2,
      0xbd,
      0x2c,
      0x5e,
      0xc5,
      0xdd,
      0x33,
      0x3d,
      0x02,
      0x53,
    ]);
    expect(actualHmac).toEqual(expectedHmac);
  });
});
