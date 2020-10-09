import {
  HEADER_LENGTH,
  REQUEST_DEVICE_ID_OFFSET,
  REQUEST_TIMESTAMP_OFFSET,
  REQUEST_CHALLENGE_OFFSET,
} from './constants';
import type { PipsqueakSessionState } from '../../../types';

/*
| Start | End | Length | Type   | Content
| ----- | --- | ------ | ------ | ----------------------------
| 0     | 0   | 1      | uint8  | Protocol ID
| 1     | 4   | 4      | uint32 | Device ID
| 5     | 8   | 4      | uint32 | Unix timestamp of request
| 9     | 9   | 1      | ------ | use varies by protocol
| 10    | 13  | 4      | uint32 | challenge
| 14    | 31  | 18     | ------ | use varies by protocol
*/
export default function parseStandardHeader(state: PipsqueakSessionState) {
  const { request } = state;
  if (request.length >= HEADER_LENGTH) {
    state.deviceID = request.readUInt32LE(REQUEST_DEVICE_ID_OFFSET);
    state.timestamp = request.readUInt32LE(REQUEST_TIMESTAMP_OFFSET);
    state.challenge = request.readUInt32LE(REQUEST_CHALLENGE_OFFSET);
  }
}
