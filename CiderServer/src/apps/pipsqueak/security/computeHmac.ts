import crypto from 'crypto';

export default function computeHmac(
  buffer: Buffer,
  offset: number,
  len: number,
  key: Buffer,
): Buffer {
  const hmac = crypto.createHmac('sha256', key);
  hmac.update(buffer.slice(offset, len));
  return hmac.digest();
}
