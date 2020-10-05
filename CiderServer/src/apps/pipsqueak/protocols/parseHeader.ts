import {
  HEADER_LENGTH,
  REQUEST_DEVICE_ID_OFFSET,
  REQUEST_TIMESTAMP_OFFSET,
  REQUEST_CHALLENGE_OFFSET,
  STATUS_MASK_DEVICE_NOT_REGISTERED,
} from './constants';
import { getDeviceWithKey } from '../../../dao';
import type { Logger } from 'pino';
import type { PipsqueakSessionState } from '../../../types';

/*
| Start | End | Length | Type     | Content
| ----- | --- | ------ | -------- | -----------------------------------------------------------------
| 0     | 0   | 1      | uint8    | Protocol ID
| 1     | 4   | 4      | uint32   | Device ID
| 5     | 8   | 4      | uint32   | Unix timestamp of request
| 9     | 9   | 1      | ------   | use varies by protocol
| 10    | 13  | 4      | uint32   | challenge
| 14    | 31  | 18     | -------- | use varies by protocol
*/
export default function parseHeader(
  buffer: Buffer,
  state: PipsqueakSessionState,
  logger: Logger,
) {
  if (buffer.length < HEADER_LENGTH) {
    return Promise.reject(
      new Error(
        'parseHeader must not be invoked until the full header is received',
      ),
    );
  }

  state.deviceID = buffer.readUInt32LE(REQUEST_DEVICE_ID_OFFSET);
  state.timestamp = buffer.readUInt32LE(REQUEST_TIMESTAMP_OFFSET);
  state.challenge = buffer.readUInt32LE(REQUEST_CHALLENGE_OFFSET);

  return getDeviceWithKey(state.deviceID)
    .then((device) => {
      if (!device) {
        logger.error(`Device ${state.deviceID} is not registered`);
        // tslint:disable-next-line:no-bitwise
        state.statusCode = state.statusCode | STATUS_MASK_DEVICE_NOT_REGISTERED;
      } else {
        state.device = device;
      }
    })
    .catch((e: any) => {
      logger.error(e);
      throw e;
    });
}
