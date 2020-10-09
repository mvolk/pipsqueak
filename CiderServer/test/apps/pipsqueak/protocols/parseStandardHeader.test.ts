import parseStandardHeader from '../../../../src/apps/pipsqueak/protocols/parseStandardHeader';
import type { PipsqueakSessionState } from '../../../../src/types';

describe('parseStandardHeader()', () => {
  let state: PipsqueakSessionState;

  beforeEach(() => {
    state = {} as PipsqueakSessionState;
  });

  describe('with insufficient data', () => {
    test('returns without modifying the state', () => {
      const request = Buffer.from([0x00]);
      state.request = request;
      parseStandardHeader(state);
      expect(state).toEqual({ request });
    });
  });

  describe('with sufficient data', () => {
    const expectedTimestamp = 123456;
    const expectedDeviceID = 1;
    const expectedChallenge = 42;

    beforeEach(() => {
      state.request = Buffer.alloc(32);
      state.request.writeUInt8(1, 0);
      state.request.writeUInt32LE(expectedDeviceID, 1);
      state.request.writeUInt32LE(expectedTimestamp, 5);
      state.request.writeUInt32LE(expectedChallenge, 10);
      parseStandardHeader(state);
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
