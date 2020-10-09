import isValidHmac from '../security/isValidHmac';
import setStatusFlag from './setStatusFlag';
import { STATUS_MASK_AUTHENTICITY_CHECK_FAILED } from './constants';
import type { PipsqueakSessionState } from '../../../types';

export default function verifyHmac(state: PipsqueakSessionState) {
  const { request, expectedRequestSize, device } = state;
  state.authentic = isValidHmac(request, expectedRequestSize, device?.key);
  if (!state.authentic) {
    state.statusCode = setStatusFlag(
      state.statusCode,
      STATUS_MASK_AUTHENTICITY_CHECK_FAILED,
    );
  }
}
