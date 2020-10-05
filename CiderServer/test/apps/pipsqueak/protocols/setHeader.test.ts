import setHeader from '../../../../src/apps/pipsqueak/protocols/setHeader';
import type { PipsqueakSessionState } from '../../../../src/types';

describe('setHeader', () => {
  let realDateNow: () => number;
  const mockNowUnix = 1601874303;
  const mockNow = mockNowUnix * 1000 + 453;
  const statusCode = 0x4b;

  beforeEach(() => {
    realDateNow = Date.now;
    Date.now = () => mockNow;
  });

  afterEach(() => {
    Date.now = realDateNow;
  });

  describe('with a challenge value', () => {
    test('sets header values appropriately', () => {
      const state = {
        protocolID: 42,
        statusCode,
        challenge: 19485294,
      } as PipsqueakSessionState;

      const header = Buffer.alloc(32);
      setHeader(header, state);

      expect(header.readUInt8(0)).toBe(state.protocolID);
      expect(header.readUInt32LE(1)).toBe(1601874303);
      expect(header.readUInt8(9)).toBe(state.statusCode);
      expect(header.readUInt32LE(10)).toBe(state.challenge);
    });
  });

  describe('without a challenge value', () => {
    test('sets header values appropriately', () => {
      const state = {
        protocolID: 42,
        statusCode,
      } as PipsqueakSessionState;

      const header = Buffer.alloc(32);
      setHeader(header, state);

      expect(header.readUInt8(0)).toBe(state.protocolID);
      expect(header.readUInt32LE(1)).toBe(1601874303);
      expect(header.readUInt8(9)).toBe(state.statusCode);
      expect(header.readUInt32LE(10)).toBe(0);
    });
  });
});
