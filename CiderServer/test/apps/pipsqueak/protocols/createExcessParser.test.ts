import {
  STATUS_MASK_AUTHENTICITY_CHECK_FAILED,
  STATUS_MASK_TOO_BIG,
} from '../../../../src/apps/pipsqueak/protocols/constants';
import createExcessParser from '../../../../src/apps/pipsqueak/protocols/createExcessParser';
import {
  PipsqueakMessageParser,
  PipsqueakSessionState,
} from '../../../../src/types';

describe('createExcessParser()', () => {
  describe('created parser', () => {
    const expectedSize = 1;
    let parser: PipsqueakMessageParser;

    beforeEach(() => {
      parser = createExcessParser(expectedSize);
    });

    describe('when invoked with a right-sized buffer', () => {
      let state: PipsqueakSessionState;
      const originalStatusCode = STATUS_MASK_AUTHENTICITY_CHECK_FAILED;
      let buffer: Buffer;

      beforeEach(() => {
        state = { statusCode: originalStatusCode } as PipsqueakSessionState;
        buffer = Buffer.from([0x00]);
      });

      it('does not change the status code', () => {
        parser(buffer, state);
        expect(state.statusCode).toBe(originalStatusCode);
      });

      it('returns synchronously', () => {
        expect(parser(buffer, state)).not.toBeInstanceOf(Promise);
      });

      it('returns a reference to itself', () => {
        const result = parser(buffer, state);
        if (!(result instanceof Promise)) {
          expect(result.parser).toBe(parser);
        }
      });

      it('returns the buffer it was passed', () => {
        const result = parser(buffer, state);
        if (!(result instanceof Promise)) {
          expect(result.buffer).toBe(buffer);
        }
      });
    });

    describe('when invoked with an over-sized buffer', () => {
      let state: PipsqueakSessionState;
      const originalStatusCode = STATUS_MASK_AUTHENTICITY_CHECK_FAILED;
      let buffer: Buffer;

      beforeEach(() => {
        state = { statusCode: originalStatusCode } as PipsqueakSessionState;
        buffer = Buffer.from([0x00, 0x01]);
      });

      it('sets the "TOO_BIG" status', () => {
        parser(buffer, state);
        // tslint:disable-next-line:no-bitwise
        expect(state.statusCode & STATUS_MASK_TOO_BIG).toBe(
          STATUS_MASK_TOO_BIG,
        );
      });

      it('does not overwrite exists status flags', () => {
        parser(buffer, state);
        // tslint:disable-next-line:no-bitwise
        expect(state.statusCode & originalStatusCode).toBe(originalStatusCode);
      });

      it('returns synchronously', () => {
        expect(parser(buffer, state)).not.toBeInstanceOf(Promise);
      });

      it('returns a reference to itself', () => {
        const result = parser(buffer, state);
        if (!(result instanceof Promise)) {
          expect(result.parser).toBe(parser);
        }
      });

      it('returns the buffer it was passed', () => {
        const result = parser(buffer, state);
        if (!(result instanceof Promise)) {
          expect(result.buffer).toBe(buffer);
        }
      });
    });
  });
});
