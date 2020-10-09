import pino from 'pino';
import handleRequest from './handleRequest';
import { PROTOCOL_ID, REQUEST_LENGTH } from './constants';
import { STATUS_OK } from '../constants';
import type { Socket } from 'net';
import type {
  PipsqueakSessionState,
  PipsqueakSession,
} from '../../../../types';
import BadRequestError from '../../../../errors/BadRequestError';

// Protocol 0, the time protocol, is used to synchronize a device's clock to the server's clock

const logger = pino({ name: 'PipsqueakTimeProtocol' });

function createSession(
  socket: Socket,
  callback?: () => void,
): PipsqueakSession {
  const state: PipsqueakSessionState = {
    protocolID: PROTOCOL_ID,
    expectedRequestSize: REQUEST_LENGTH,
    request: Buffer.alloc(0),
    statusCode: STATUS_OK,
  };

  function handleData(data: Buffer) {
    const bytesRemaining = state.expectedRequestSize - state.request.length;
    if (data.length > bytesRemaining) {
      socket.destroy(new BadRequestError('Too much data received'));
    } else {
      state.request = Buffer.concat([state.request, data]);

      if (state.request.length === state.expectedRequestSize) {
        handleRequest(state, socket, callback);
      }
    }
  }

  function end() {
    if (state.request.length !== state.expectedRequestSize) {
      logger.error('Received an unexpected FIN signal');
      socket.end();
    }
  }

  function error(err: any) {
    logger.error(err);
  }

  function close(hadErrors: boolean) {
    if (hadErrors) logger.error('Socket closed with error');
    if (callback) callback();
  }

  return { handleData, end, error, close };
}

export default {
  id: PROTOCOL_ID,
  createSession,
};
