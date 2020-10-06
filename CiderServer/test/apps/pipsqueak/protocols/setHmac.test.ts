import setHmac from '../../../../src/apps/pipsqueak/protocols/setHmac';
import { key as testKey } from '../../../fixtures/device';

describe('setHmac', () => {
  const messageText = 'The first 32 bytes of this message will be used';

  describe('with a key', () => {
    test('sets a valid hmac', () => {
      const message = Buffer.alloc(32, messageText, 'utf-8');
      const buffer = Buffer.alloc(64);
      message.copy(buffer, 0, 0);
      setHmac(buffer, 32, testKey);
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
      const message = Buffer.alloc(32, messageText, 'utf-8');
      const buffer = Buffer.alloc(64);
      message.copy(buffer, 0, 32);
      setHmac(buffer, 32, undefined);
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
