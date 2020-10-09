import pino from 'pino';
import buildResponse from './buildResponse';
import parseStandardHeader from '../parseStandardHeader';
import loadDeviceWithKey from '../loadDeviceWithKey';
import verifyHmac from '../verifyHmac';
import EntityNotFoundError from '../../../../errors/EntityNotFoundError';
import type { PipsqueakSessionState } from '../../../../types';
import type { Socket } from 'net';

const logger = pino({ name: 'PipsqueakTimeProtocol::handleRequest' });

export default async function handleRequest(
  state: PipsqueakSessionState,
  socket: Socket,
  callback?: () => void,
) {
  try {
    parseStandardHeader(state);
    await loadDeviceWithKey(state);
    verifyHmac(state);
    const response = buildResponse(state);
    if (!socket.destroyed) socket.end(response);
  } catch (err) {
    if (err instanceof EntityNotFoundError) {
      logger.error(err, `Time request from unregistered device`);
    } else {
      logger.error(err, `Time request failed unexpectedly`);
    }
    socket.end();
    socket.destroy(err);
  } finally {
    if (callback) callback();
  }
}
