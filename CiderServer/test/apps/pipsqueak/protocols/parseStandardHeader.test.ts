import parseStandardHeader from '../../../../src/apps/pipsqueak/protocols/parseStandardHeader';
import type { PipsqueakSessionState } from '../../../../src/types';

describe('parseStandardHeader()', () => {
  let state: PipsqueakSessionState;

  beforeEach(() => {
    state = {} as PipsqueakSessionState;
  });

  describe('with insufficient data', () => {
    test('returns without modifying the state', () => {
      const buffer = Buffer.from([0x00]);
      parseStandardHeader(buffer, state);
      expect(state).toEqual({});
    });
  });

  describe('with sufficient data', () => {
    let buffer: Buffer;
    const expectedTimestamp = 123456;
    const expectedDeviceID = 1;
    const expectedChallenge = 42;

    beforeEach(() => {
      buffer = Buffer.alloc(32);
      buffer.writeUInt8(1, 0);
      buffer.writeUInt32LE(expectedDeviceID, 1);
      buffer.writeUInt32LE(expectedTimestamp, 5);
      buffer.writeUInt32LE(expectedChallenge, 10);
      parseStandardHeader(buffer, state);
    });

    test('sets "deviceID" in the session state', () => {
      expect(state.deviceID).toBe(expectedDeviceID);
    });

    test('sets "timestamp" in the session state', () => {
      expect(state.timestamp).toBe(expectedTimestamp);
    });

    test('sets "challenge" in the session state', () => {
      expect(state.challenge).toBe(expectedChallenge);
    });
  });
});
