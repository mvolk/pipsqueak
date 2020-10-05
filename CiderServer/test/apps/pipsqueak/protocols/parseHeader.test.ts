import pino from 'pino';
import { mocked } from 'ts-jest/utils';
import * as dao from '../../../../src/dao';
import parseHeader from '../../../../src/apps/pipsqueak/protocols/parseHeader';
import {
  STATUS_MASK_BUSY,
  STATUS_MASK_DEVICE_NOT_REGISTERED,
} from '../../../../src/apps/pipsqueak/protocols/constants';
import type {
  PipsqueakSessionState,
  DeviceWithKey,
} from '../../../../src/types';

jest.mock('../../../../src/dao');

describe('parseHeader()', () => {
  let state: PipsqueakSessionState;

  beforeEach(() => {
    state = {} as PipsqueakSessionState;
  });

  describe('with insufficient data', () => {
    test('returns a rejected promise', () => {
      const buffer = Buffer.from([0x00]);
      return expect(parseHeader(buffer, state, pino())).rejects.toBeInstanceOf(
        Error,
      );
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
    });

    describe('when device lookup does not throw', () => {
      beforeEach(() => {
        mocked(dao).getDeviceWithKey.mockReturnValue(Promise.resolve());
      });

      test('sets "deviceID" in the session state', () => {
        const asyncResult = parseHeader(buffer, state, pino());
        expect(state.deviceID).toBe(expectedDeviceID);
        return asyncResult;
      });

      test('sets "timestamp" in the session state', () => {
        const asyncResult = parseHeader(buffer, state, pino());
        expect(state.timestamp).toBe(expectedTimestamp);
        return asyncResult;
      });

      test('sets "challenge" in the session state', () => {
        const asyncResult = parseHeader(buffer, state, pino());
        expect(state.challenge).toBe(expectedChallenge);
        return asyncResult;
      });
    });

    describe('when device lookup throws', () => {
      const err = new Error('test');

      beforeEach(() => {
        mocked(dao).getDeviceWithKey.mockReturnValue(Promise.reject(err));
      });

      test('returns a resolved promise', () => {
        return expect(parseHeader(buffer, state, pino())).rejects.toBe(err);
      });

      test('logs the error', () => {
        return parseHeader(buffer, state, pino()).then(
          () => fail(),
          () => {
            expect(mocked(pino().error).mock.calls[0][0]).toBe(err);
          },
        );
      });
    });

    describe('when the device is registered', () => {
      const expectedDevice: DeviceWithKey = {} as DeviceWithKey;

      beforeEach(() => {
        mocked(dao).getDeviceWithKey.mockReturnValue(
          Promise.resolve(expectedDevice),
        );
      });

      test('sets "device" in the session state', () => {
        return parseHeader(buffer, state, pino()).then(() => {
          expect(state.device).toBe(expectedDevice);
        });
      });

      test('returns a promise that resolves', () => {
        return expect(
          parseHeader(buffer, state, pino()),
        ).resolves.toBeUndefined();
      });
    });

    describe('when the device is not registered', () => {
      beforeEach(() => {
        mocked(dao).getDeviceWithKey.mockReturnValue(
          Promise.resolve(undefined),
        );
      });

      test('set the device not registered status flag', () => {
        return parseHeader(buffer, state, pino()).then(() => {
          // tslint:disable-next-line:no-bitwise
          expect(state.statusCode & STATUS_MASK_DEVICE_NOT_REGISTERED).toBe(
            STATUS_MASK_DEVICE_NOT_REGISTERED,
          );
        });
      });

      test('preserves pre-existing status flags', () => {
        state.statusCode = STATUS_MASK_BUSY;
        return parseHeader(buffer, state, pino()).then(() => {
          // tslint:disable-next-line:no-bitwise
          expect(state.statusCode & STATUS_MASK_BUSY).toBe(STATUS_MASK_BUSY);
        });
      });

      test('logs an error', () => {
        return parseHeader(buffer, state, pino()).then(() => {
          expect(mocked(pino().error).mock.calls[0][0]).toEqual(
            'Device 1 is not registered',
          );
        });
      });

      test('returns a promise that resolves', () => {
        return expect(
          parseHeader(buffer, state, pino()),
        ).resolves.toBeUndefined();
      });
    });
  });
});
