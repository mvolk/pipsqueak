import { PipsqueakSessionState } from '../../../../../src/types';
import {
  validRequest,
  validRequestData,
  validResponse,
  validResponseData,
} from '../../../../fixtures/timeProtocol';
import buildResponse from '../../../../../src/apps/pipsqueak/protocols/time/buildResponse';

describe('buildResponse', () => {
  describe('without a device', () => {
    let realDateNow: () => number;
    const state: PipsqueakSessionState = {
      protocolID: validRequestData.protocolID,
      statusCode: 0,
      challenge: validRequestData.challenge,
      expectedRequestSize: validRequest.length,
      request: validRequest,
    };

    beforeEach(() => {
      realDateNow = Date.now;
      Date.now = () => validResponseData.timestamp * 1000;
    });

    afterEach(() => {
      Date.now = realDateNow;
    });

    test('degrades gracefully', () => {
      const result = buildResponse(state);
      expect(result.slice(0, 32)).toEqual(validResponse.slice(0, 32));
      expect(result.slice(32, 64)).toEqual(Buffer.alloc(32));
    });
  });
});
