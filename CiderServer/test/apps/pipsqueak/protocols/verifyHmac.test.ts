import { mocked } from 'ts-jest/utils';
import verifyHmac from '../../../../src/apps/pipsqueak/protocols/verifyHmac';
import isValidHmac from '../../../../src/apps/pipsqueak/security/isValidHmac';
import { STATUS_MASK_AUTHENTICITY_CHECK_FAILED } from '../../../../src/apps/pipsqueak/protocols/constants';
import type {
  DeviceWithKey,
  PipsqueakSessionState,
} from '../../../../src/types';

jest.mock('../../../../src/apps/pipsqueak/security/isValidHmac');

describe('verifyHmac', () => {
  const expectedRequestSize = 42;
  const statusCode = 0x00;
  const request = Buffer.alloc(42);
  let state: PipsqueakSessionState;

  describe('when the device is undefined', () => {
    beforeEach(() => {
      state = {
        request,
        statusCode,
        expectedRequestSize,
      } as PipsqueakSessionState;
      mocked(isValidHmac).mockReturnValue(false);
      verifyHmac(state);
    });

    test('properly invokes "isValidHmac"', () => {
      expect(mocked(isValidHmac)).toHaveBeenCalledWith(
        request,
        expectedRequestSize,
        undefined,
      );
    });

    test('sets "authentic" field in session state to "false"', () => {
      expect(state.authentic).toBe(false);
    });

    test('adds the authenticity check failure status flag', () => {
      expect(state.statusCode).toBe(STATUS_MASK_AUTHENTICITY_CHECK_FAILED);
    });
  });

  describe('when the device is defined', () => {
    const key = Buffer.from('FakeKey');
    let device: DeviceWithKey;

    beforeEach(() => {
      device = { key } as DeviceWithKey;
      state = {
        request,
        statusCode,
        expectedRequestSize,
        device,
      } as PipsqueakSessionState;
    });

    describe('when the HMAC is valid', () => {
      beforeEach(() => {
        mocked(isValidHmac).mockReturnValue(true);
        verifyHmac(state);
      });

      test('properly invokes "isValidHmac"', () => {
        expect(mocked(isValidHmac)).toHaveBeenCalledWith(
          request,
          expectedRequestSize,
          key,
        );
      });

      test('sets "authentic" field in session state to "true"', () => {
        expect(state.authentic).toBe(true);
      });

      test('does not add the authenticity check failure status flag', () => {
        expect(state.statusCode).toBe(statusCode);
      });
    });

    describe('when the HMAC is invalid', () => {
      beforeEach(() => {
        mocked(isValidHmac).mockReturnValue(false);
        verifyHmac(state);
      });

      test('properly invokes "isValidHmac"', () => {
        expect(mocked(isValidHmac)).toHaveBeenCalledWith(
          request,
          expectedRequestSize,
          key,
        );
      });

      test('sets "authentic" field in session state to "false"', () => {
        expect(state.authentic).toBe(false);
      });

      test('adds the authenticity check failure status flag', () => {
        expect(state.statusCode).toBe(STATUS_MASK_AUTHENTICITY_CHECK_FAILED);
      });
    });
  });
});
