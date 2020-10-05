import computeHmac from '../security/computeHmac';
import type { PipsqueakSessionState } from '../../../types';

export default function setHmac(
  buffer: Buffer,
  contentLength: number,
  state: PipsqueakSessionState,
) {
  if (state.device) {
    // for clarity... making the implicit explicit:
    const hmacOffset = contentLength;
    computeHmac(buffer, 0, contentLength, state.device.key).copy(
      buffer,
      hmacOffset,
    );
  }
}
