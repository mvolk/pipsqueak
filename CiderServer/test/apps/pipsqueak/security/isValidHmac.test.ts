import { key as testKey } from '../../../fixtures/device';
import isValidHmac from '../../../../src/apps/pipsqueak/security/isValidHmac';

describe('isValidHmac', () => {
  const expectedRequestSize = 33;

  describe('with enough data', () => {
    describe('with a valid hmac', () => {
      const authenticRequest = Buffer.from([
        0x0f,
        0x6d,
        0xde,
        0x54,
        0x1a,
        0x14,
        0xa6,
        0x9b,
        0xc1,
        0x00,
        0x44,
        0xab,
        0xfc,
        0xd2,
        0xef,
        0x76,
        0x84,
        0xcf,
        0x16,
        0x59,
        0x0e,
        0xe0,
        0xe5,
        0xf8,
        0x19,
        0xd9,
        0xfc,
        0x50,
        0x48,
        0x67,
        0xcf,
        0x3e,
        0xa4,
      ]);

      describe('with a key', () => {
        test('returns true', () => {
          expect(authenticRequest.length).toBe(expectedRequestSize);
          expect(
            isValidHmac(authenticRequest, expectedRequestSize, testKey),
          ).toBe(true);
        });
      });

      describe('without a key', () => {
        test('returns false', () => {
          expect(authenticRequest.length).toBe(expectedRequestSize);
          expect(
            isValidHmac(authenticRequest, expectedRequestSize, undefined),
          ).toBe(false);
        });
      });
    });

    describe('with an invalid hmac', () => {
      test('returns false', () => {
        const invalidRequest = Buffer.from([
          0x0f,
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
        expect(invalidRequest.length).toBe(expectedRequestSize);
        expect(isValidHmac(invalidRequest, expectedRequestSize, testKey)).toBe(
          false,
        );
      });
    });
  });

  describe('without enough data', () => {
    test('returns false', () => {
      const incompleteRequest = Buffer.from([0x6d, 0xde, 0x54, 0x1a, 0x14]);
      expect(incompleteRequest.length).toBeLessThan(expectedRequestSize);
      expect(isValidHmac(incompleteRequest, expectedRequestSize, testKey)).toBe(
        false,
      );
    });
  });
});
