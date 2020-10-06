import computeHmac from './computeHmac';
import { HMAC_LENGTH } from './constants';

export default function isHmacValid(
  requestBuffer: Buffer,
  expectedRequestSize: number,
  key: Buffer | undefined,
): boolean {
  if (requestBuffer.length < expectedRequestSize) return false;
  if (!key) return false;
  const hmac = computeHmac(
    requestBuffer,
    0,
    expectedRequestSize - HMAC_LENGTH,
    key,
  );
  return (
    hmac.compare(
      requestBuffer,
      expectedRequestSize - HMAC_LENGTH,
      expectedRequestSize,
    ) === 0
  );
}
