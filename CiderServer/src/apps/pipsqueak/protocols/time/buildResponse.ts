import setHeader from '../setHeader';
import setHmac from '../setHmac';
import { HEADER_LENGTH } from '../constants';
import { RESPONSE_LENGTH } from './constants';
import type { PipsqueakSessionState } from '../../../../types';

export default function buildResponse(state: PipsqueakSessionState): Buffer {
  const response = Buffer.alloc(RESPONSE_LENGTH);
  setHeader(response, state);
  if (state.device) {
    setHmac(response, HEADER_LENGTH, state.device.key);
  }
  return response;
}
