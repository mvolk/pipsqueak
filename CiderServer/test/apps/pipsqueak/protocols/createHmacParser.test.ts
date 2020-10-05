import { mocked } from 'ts-jest/utils';
import {
  STATUS_MASK_AUTHENTICITY_CHECK_FAILED,
  STATUS_MASK_TIMESTAMP_TOO_OLD,
} from '../../../../src/apps/pipsqueak/protocols/constants';
import createExcessParser from '../../../../src/apps/pipsqueak/protocols/createExcessParser';
import createHmacParser from '../../../../src/apps/pipsqueak/protocols/createHmacParser';
import {
  DeviceWithKey,
  PipsqueakMessageParser,
  PipsqueakSessionState,
} from '../../../../src/types';

jest.mock('../../../../src/apps/pipsqueak/protocols/createExcessParser');

describe('createHmacParser()', () => {
  describe('created parser', () => {
    const expectedSize = 33;
    let parser: PipsqueakMessageParser;
    let state: PipsqueakSessionState;
    const originalStatusCode = STATUS_MASK_TIMESTAMP_TOO_OLD;
    const mockExcessParser: PipsqueakMessageParser = jest.fn() as PipsqueakMessageParser;
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
    let device: DeviceWithKey;
    let authenticHmac: Buffer;

    beforeEach(() => {
      parser = createHmacParser(expectedSize);
      device = { key: testKey } as DeviceWithKey;
      state = {
        statusCode: originalStatusCode,
        device,
      } as PipsqueakSessionState;
      // @ts-ignore
      mocked(createExcessParser).mockReturnValue(mockExcessParser);
      authenticHmac = Buffer.from([
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
    });
    let mockResult: { parser: PipsqueakMessageParser; buffer: Buffer };

    describe('when invoked without enough data', () => {
      let buffer: Buffer;

      beforeEach(() => {
        buffer = Buffer.alloc(32);
        mockResult = { parser: mockExcessParser, buffer };
        mocked(mockExcessParser).mockReturnValue(mockResult);
      });

      test('returns synchronously', () => {
        expect(parser(buffer, state)).not.toBeInstanceOf(Promise);
      });

      test('returns a reference to itself', () => {
        const result = parser(buffer, state);
        if (!(result instanceof Promise)) {
          expect(result.parser).toBe(parser);
        }
      });

      test('returns the buffer it was passed', () => {
        const result = parser(buffer, state);
        if (!(result instanceof Promise)) {
          expect(result.buffer).toBe(buffer);
        }
      });
    });

    describe('when invoked with enough data', () => {
      describe('when the message is authentic', () => {
        let buffer: Buffer;

        beforeEach(() => {
          buffer = Buffer.alloc(33);
          buffer.writeUInt8(0x0f, 0);
          authenticHmac.copy(buffer, 1, 0, 32);
          mockResult = { parser: mockExcessParser, buffer };
          mocked(mockExcessParser).mockReturnValue(mockResult);
        });

        test('does not change the status code', () => {
          parser(buffer, state);
          expect(state.statusCode).toBe(originalStatusCode);
        });

        test('sets the "complete" flag', () => {
          parser(buffer, state);
          expect(state.complete).toBe(true);
        });

        test('returns synchronously', () => {
          expect(parser(buffer, state)).not.toBeInstanceOf(Promise);
        });

        test('correctly creates the excess parser', () => {
          parser(buffer, state);
          expect(mocked(createExcessParser).mock.calls[0][0]).toBe(
            expectedSize,
          );
        });

        test('correctly invokes the excess parser', () => {
          parser(buffer, state);
          expect(mocked(mockExcessParser).mock.calls[0][0]).toBe(buffer);
          expect(mocked(mockExcessParser).mock.calls[0][1]).toBe(state);
        });

        test('returns the result of invoking the excess parser', () => {
          expect(parser(buffer, state)).toBe(mockResult);
        });
      });

      describe('when the message is not authentic', () => {
        let buffer: Buffer;

        beforeEach(() => {
          buffer = Buffer.from([
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
        });

        test('sets the "STATUS_MASK_AUTHENTICITY_CHECK_FAILED" status', () => {
          parser(buffer, state);
          // tslint:disable-next-line:no-bitwise
          expect(state.statusCode & STATUS_MASK_AUTHENTICITY_CHECK_FAILED).toBe(
            STATUS_MASK_AUTHENTICITY_CHECK_FAILED,
          );
        });

        test('sets the "complete" flag', () => {
          parser(buffer, state);
          expect(state.complete).toBe(true);
        });

        test('does not overwrite exists status flags', () => {
          parser(buffer, state);
          // tslint:disable-next-line:no-bitwise
          expect(state.statusCode & originalStatusCode).toBe(
            originalStatusCode,
          );
        });

        test('returns synchronously', () => {
          expect(parser(buffer, state)).not.toBeInstanceOf(Promise);
        });

        test('correctly creates the excess parser', () => {
          parser(buffer, state);
          expect(mocked(createExcessParser).mock.calls[0][0]).toBe(
            expectedSize,
          );
        });

        test('correctly invokes the excess parser', () => {
          parser(buffer, state);
          expect(mocked(mockExcessParser).mock.calls[0][0]).toBe(buffer);
          expect(mocked(mockExcessParser).mock.calls[0][1]).toBe(state);
        });

        test('returns the result of invoking the excess parser', () => {
          expect(parser(buffer, state)).toBe(mockResult);
        });
      });

      describe('when the device is unknown', () => {
        let buffer: Buffer;

        beforeEach(() => {
          buffer = Buffer.alloc(33);
          buffer.writeUInt8(0x0f, 0);
          authenticHmac.copy(buffer, 1, 0, 32);
          state.device = undefined;
        });

        test('sets the "STATUS_MASK_AUTHENTICITY_CHECK_FAILED" status', () => {
          parser(buffer, state);
          // tslint:disable-next-line:no-bitwise
          expect(state.statusCode & STATUS_MASK_AUTHENTICITY_CHECK_FAILED).toBe(
            STATUS_MASK_AUTHENTICITY_CHECK_FAILED,
          );
        });

        test('sets the "complete" flag', () => {
          parser(buffer, state);
          expect(state.complete).toBe(true);
        });

        test('does not overwrite exists status flags', () => {
          parser(buffer, state);
          // tslint:disable-next-line:no-bitwise
          expect(state.statusCode & originalStatusCode).toBe(
            originalStatusCode,
          );
        });

        test('returns synchronously', () => {
          expect(parser(buffer, state)).not.toBeInstanceOf(Promise);
        });

        test('correctly creates the excess parser', () => {
          parser(buffer, state);
          expect(mocked(createExcessParser).mock.calls[0][0]).toBe(
            expectedSize,
          );
        });

        test('correctly invokes the excess parser', () => {
          parser(buffer, state);
          expect(mocked(mockExcessParser).mock.calls[0][0]).toBe(buffer);
          expect(mocked(mockExcessParser).mock.calls[0][1]).toBe(state);
        });

        test('returns the result of invoking the excess parser', () => {
          expect(parser(buffer, state)).toBe(mockResult);
        });
      });
    });
  });
});
