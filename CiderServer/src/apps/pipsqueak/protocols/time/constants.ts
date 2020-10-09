import { HMAC_LENGTH, HEADER_LENGTH } from '../constants';

export const PROTOCOL_ID = 0;
export const REQUEST_LENGTH = HEADER_LENGTH + HMAC_LENGTH;
export const RESPONSE_LENGTH = HEADER_LENGTH + HMAC_LENGTH;
