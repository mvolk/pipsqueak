import loadDeviceWithKey from '../../../../src/apps/pipsqueak/protocols/loadDeviceWithKey';
import { getDeviceWithKey } from '../../../../src/dao';
import BadRequestError from '../../../../src/errors/BadRequestError';
import EntityNotFoundError from '../../../../src/errors/EntityNotFoundError';
import { DeviceWithKey, PipsqueakSessionState } from '../../../../src/types';
import { mocked } from 'ts-jest/utils';

jest.mock('../../../../src/dao', () => {
  return {
    getDeviceWithKey: jest.fn(),
  };
});

describe('loadDeviceWithKey', () => {
  let state: PipsqueakSessionState;

  describe('when deviceID is undefined', () => {
    beforeEach(() => {
      state = {} as PipsqueakSessionState;
    });

    test('does not invoke getDeviceWithKey', () => {
      function assertExpectations() {
        expect(getDeviceWithKey).not.toHaveBeenCalled();
      }
      return loadDeviceWithKey(state).then(
        assertExpectations,
        assertExpectations,
      );
    });

    test('return a rejected promise with a BadRequestError', () => {
      return expect(loadDeviceWithKey(state)).rejects.toBeInstanceOf(
        BadRequestError,
      );
    });
  });

  describe('when deviceID is zero', () => {
    beforeEach(() => {
      state = { deviceID: 0 } as PipsqueakSessionState;
    });

    test('does not invoke getDeviceWithKey', () => {
      function assertExpectations() {
        expect(getDeviceWithKey).not.toHaveBeenCalled();
      }
      return loadDeviceWithKey(state).then(
        assertExpectations,
        assertExpectations,
      );
    });

    test('throws a BadRequestError', () => {
      return expect(loadDeviceWithKey(state)).rejects.toBeInstanceOf(
        BadRequestError,
      );
    });
  });

  describe('when deviceID is a positive non-sero integer', () => {
    const deviceID = 1;
    let device: DeviceWithKey;

    beforeEach(() => {
      state = { deviceID } as PipsqueakSessionState;
    });

    test('invokes getDeviceWithKey with the deviceID', () => {
      function assertExpectations() {
        expect(getDeviceWithKey).toHaveBeenCalledTimes(1);
        expect(getDeviceWithKey).toHaveBeenCalledWith(deviceID);
      }
      return loadDeviceWithKey(state).then(
        assertExpectations,
        assertExpectations,
      );
    });

    describe('when the load succeeds', () => {
      beforeEach(() => {
        device = { id: deviceID } as DeviceWithKey;
        mocked(getDeviceWithKey).mockResolvedValue(device);
      });

      test('sets state.device equal to the result', () => {
        return loadDeviceWithKey(state).then(() => {
          expect(state.device).toBe(device);
        });
      });
    });

    describe('when the load fails with an exception', () => {
      beforeEach(() => {
        mocked(getDeviceWithKey).mockRejectedValue(
          new EntityNotFoundError('TestEntity', 1),
        );
      });

      test('returns a rejected promise', () => {
        expect(loadDeviceWithKey(state)).rejects.toBeInstanceOf(
          EntityNotFoundError,
        );
      });
    });
  });
});
