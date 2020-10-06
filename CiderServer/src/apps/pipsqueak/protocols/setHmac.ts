import computeHmac from '../security/computeHmac';

export default function setHmac(
  responseBuffer: Buffer,
  contentLengthExcludingHmac: number,
  key: Buffer | undefined,
) {
  if (key) {
    // for clarity... making the implicit explicit:
    const hmacOffset = contentLengthExcludingHmac;
    computeHmac(responseBuffer, 0, contentLengthExcludingHmac, key).copy(
      responseBuffer,
      hmacOffset,
    );
  }
}
