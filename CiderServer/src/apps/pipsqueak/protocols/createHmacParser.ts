import computeHmac from '../security/computeHmac';
import createExcessParser from './createExcessParser';
import { HMAC_LENGTH } from '../security/constants';
import { STATUS_MASK_AUTHENTICITY_CHECK_FAILED } from './constants';
import type { PipsqueakSessionState } from '../../../types';

function isAuthentic(
  buffer: Buffer,
  state: PipsqueakSessionState,
  expectedMessageSize: number,
) {
  if (!state.device) return false;
  const hmac = computeHmac(
    buffer,
    0,
    expectedMessageSize - HMAC_LENGTH,
    state.device.key,
  );
  return (
    hmac.compare(
      buffer,
      expectedMessageSize - HMAC_LENGTH,
      expectedMessageSize,
    ) === 0
  );
}

export default function createHmacParser(expectedMessageSize: number) {
  return function parseHmac(buffer: Buffer, state: PipsqueakSessionState) {
    if (buffer.length < expectedMessageSize) {
      return { parser: parseHmac, buffer };
    }

    if (!isAuthentic(buffer, state, expectedMessageSize)) {
      state.statusCode =
        // tslint:disable-next-line:no-bitwise
        state.statusCode | STATUS_MASK_AUTHENTICITY_CHECK_FAILED;
    }

    state.complete = true;

    return createExcessParser(expectedMessageSize)(buffer, state);
  };
}
