import { STATUS_MASK_TOO_BIG } from './constants';
import type { PipsqueakSessionState } from '../../../types';

export default function createExcessParser(expectedMessageSize: number) {
  return function parseExcess(buffer: Buffer, state: PipsqueakSessionState) {
    if (buffer.length > expectedMessageSize) {
      // tslint:disable-next-line:no-bitwise
      state.statusCode = state.statusCode | STATUS_MASK_TOO_BIG;
    }

    return { parser: parseExcess, buffer };
  };
}
