import {
  RESPONSE_PROTOCOL_ID_OFFSET,
  RESPONSE_STATUS_CODE_OFFSET,
  RESPONSE_CHALLENGE_OFFSET,
  RESPONSE_TIMESTAMP_OFFSET,
} from './constants';

/*
| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | -----------------------------------------------------------------
| 0     | 0   | 1      | uint8  | protocol ID
| 1     | 4   | 4      | uint32 | message timestamp (seconds since Jan 1 1970)
| 5     | 8   | 4      | ------ | reserved for protocol-specific use
| 9     | 9   | 1      | uint8  | status code
| 10    | 13  | 4      | uint32 | challenge
| 14    | 31  | 18     | ------ | reserved for protocol-specific use
*/
export default function setHeader(
  responseBuffer: Buffer,
  responseData: {
    protocolID: number;
    statusCode: number;
    challenge?: number;
  },
) {
  responseBuffer.writeUInt8(
    responseData.protocolID,
    RESPONSE_PROTOCOL_ID_OFFSET,
  );
  responseBuffer.writeUInt32LE(
    Math.round(Date.now() / 1000),
    RESPONSE_TIMESTAMP_OFFSET,
  );
  responseBuffer.writeUInt8(
    responseData.statusCode,
    RESPONSE_STATUS_CODE_OFFSET,
  );
  responseBuffer.writeUInt32LE(
    responseData.challenge || 0,
    RESPONSE_CHALLENGE_OFFSET,
  );
}
