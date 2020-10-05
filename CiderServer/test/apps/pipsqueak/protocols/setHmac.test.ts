import setHmac from '../../../../src/apps/pipsqueak/protocols/setHmac';
import type { PipsqueakSessionState } from '../../../../src/types';

describe('setHmac', () => {
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
  const messageText = 'The first 32 bytes of this message will be used';

  describe('with a device', () => {
    test('computes and sets the hmac', () => {
      const state = { device: { key: testKey } } as PipsqueakSessionState;
      expect(messageText.length).toBeGreaterThan(32);
      const message = Buffer.alloc(32, messageText, 'utf-8');
      const buffer = Buffer.alloc(64);
      message.copy(buffer, 0, 0);
      setHmac(buffer, 32, state);
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
      expect(buffer.slice(32, 64)).toEqual(expectedHmac);
    });
  });

  describe('without a device', () => {
    test('leaves the buffer unchanged', () => {
      const state = {} as PipsqueakSessionState;
      expect(messageText.length).toBeGreaterThan(32);
      const message = Buffer.alloc(32, messageText, 'utf-8');
      const buffer = Buffer.alloc(64);
      message.copy(buffer, 0, 32);
      setHmac(buffer, 32, state);
      const expectedHmac = Buffer.from([
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
      ]);
      expect(buffer.slice(32, 64)).toEqual(expectedHmac);
    });
  });
});
