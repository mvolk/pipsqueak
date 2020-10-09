import net from 'net';
import pino from 'pino';
import { mocked } from 'ts-jest/utils';
import { deviceWithKey } from '../../../../fixtures/device';
import {
  validRequest,
  validRequestData,
  validResponse,
  validResponseData,
} from '../../../../fixtures/timeProtocol';
import { getDeviceWithKey } from '../../../../../src/dao';
import timeProtocol from '../../../../../src/apps/pipsqueak/protocols/time';
import BadRequestError from '../../../../../src/errors/BadRequestError';
import EntityNotFoundError from '../../../../../src/errors/EntityNotFoundError';
import type { PipsqueakSession } from '../../../../../src/types';

jest.mock('net');
jest.mock('../../../../../src/dao', () => ({
  getDeviceWithKey: jest.fn(),
}));

describe('time protocol', () => {
  let realDateNow: () => number;
  const mockSocket: jest.Mocked<net.Socket> = mocked(new net.Socket());

  beforeEach(() => {
    realDateNow = Date.now;
  });

  afterEach(() => {
    Date.now = realDateNow;
  });

  describe('id', () => {
    test('is 0', () => {
      expect(timeProtocol.id).toBe(0);
    });
  });

  describe('createSession', () => {
    describe('handleData', () => {
      describe('happy path', () => {
        beforeEach(() => {
          Date.now = () => validResponseData.timestamp * 1000 + 328;
          mocked(getDeviceWithKey).mockResolvedValue(deviceWithKey);
        });

        test('returns the expected response', () => {
          return new Promise((resolve) => {
            function onCompletion() {
              expect(mockSocket.end.mock.calls[0][0]).toEqual(validResponse);
              resolve();
            }
            const session = timeProtocol.createSession(
              mockSocket,
              onCompletion,
            );
            session.handleData(validRequest.slice(0, 24));
            session.handleData(validRequest.slice(24));
          });
        });
      });

      describe('when the device is not registered', () => {
        const err = new EntityNotFoundError(
          'Device',
          validRequestData.deviceID,
        );

        beforeEach(() => {
          Date.now = () => validResponseData.timestamp * 1000 + 328;
          mocked(getDeviceWithKey).mockRejectedValue(err);
        });

        test('calls socket.end and socket.destroy', () => {
          return new Promise((resolve) => {
            function onCompletion() {
              expect(mockSocket.end).toHaveBeenCalledTimes(1);
              expect(mockSocket.end).toHaveBeenCalledWith();
              expect(mockSocket.destroy).toHaveBeenCalledTimes(1);
              expect(mockSocket.destroy).toHaveBeenCalledWith(err);
              resolve();
            }
            const session = timeProtocol.createSession(
              mockSocket,
              onCompletion,
            );
            session.handleData(validRequest);
          });
        });
      });

      describe('when the datastore fails', () => {
        const err = new Error('Unexpected stuff happened');

        beforeEach(() => {
          Date.now = () => validResponseData.timestamp * 1000 + 328;
          mocked(getDeviceWithKey).mockRejectedValue(err);
        });

        test('calls socket.end and socket.destroy', () => {
          return new Promise((resolve) => {
            function onCompletion() {
              expect(mockSocket.end).toHaveBeenCalledTimes(1);
              expect(mockSocket.end).toHaveBeenCalledWith();
              expect(mockSocket.destroy).toHaveBeenCalledTimes(1);
              expect(mockSocket.destroy).toHaveBeenCalledWith(err);
              resolve();
            }
            const session = timeProtocol.createSession(
              mockSocket,
              onCompletion,
            );
            session.handleData(validRequest);
          });
        });
      });

      describe('when too much data is received', () => {
        let fatRequest: Buffer;

        beforeEach(() => {
          fatRequest = Buffer.alloc(128);
        });

        test('calls socket.destroy with a BadRequestError', () => {
          const session = timeProtocol.createSession(mockSocket);
          session.handleData(fatRequest);
          expect(mockSocket.destroy.mock.calls[0][0]).toBeInstanceOf(
            BadRequestError,
          );
        });

        test('does not call socket.end', () => {
          const session = timeProtocol.createSession(mockSocket);
          session.handleData(fatRequest);
          expect(mockSocket.end).not.toHaveBeenCalled();
        });
      });

      describe('when the socket is prematurely destroyed', () => {
        beforeEach(() => {
          Date.now = () => validResponseData.timestamp * 1000 + 328;
          mocked(getDeviceWithKey).mockResolvedValue(deviceWithKey);
          Object.defineProperty(mockSocket, 'destroyed', {
            value: true,
            configurable: true,
          });
        });

        afterEach(() => {
          Object.defineProperty(mockSocket, 'destroyed', {
            value: false,
            configurable: true,
          });
        });

        test('does not send a response', () => {
          return new Promise((resolve) => {
            function onCompletion() {
              expect(mockSocket.end).not.toHaveBeenCalled();
              resolve();
            }
            const session = timeProtocol.createSession(
              mockSocket,
              onCompletion,
            );

            session.handleData(validRequest);
          });
        });
      });
    });

    describe('end', () => {
      describe('when the request is fully received', () => {
        beforeEach(() => {
          Date.now = () => validResponseData.timestamp * 1000 + 328;
          mocked(getDeviceWithKey).mockResolvedValue(deviceWithKey);
        });

        test('does not call socket.end', () => {
          return new Promise((resolve) => {
            let session: PipsqueakSession;
            function onCompletion() {
              expect(mockSocket.end.mock.calls[0][0]).toEqual(validResponse);
              mockSocket.end.mockReset();
              session.end();
              expect(mockSocket.end).not.toHaveBeenCalled();
              resolve();
            }
            session = timeProtocol.createSession(mockSocket, onCompletion);
            session.handleData(validRequest);
          });
        });
      });

      describe('when the request is not fully received', () => {
        test('calls socket.end with no arguments', () => {
          const session = timeProtocol.createSession(mockSocket);
          session.handleData(validRequest.slice(0, 10));
          expect(mockSocket.end).not.toHaveBeenCalled();
          session.end();
          expect(mockSocket.end).toHaveBeenCalledTimes(1);
          expect(mockSocket.end).toHaveBeenCalledWith();
        });
      });
    });

    describe('error', () => {
      test('logs the error', () => {
        const session = timeProtocol.createSession(mockSocket);
        const error = new Error('Test');
        session.error(error);
        expect(mocked(pino().error)).toHaveBeenCalledWith(error);
      });
    });

    describe('close', () => {
      describe('with errors', () => {
        test('logs', () => {
          timeProtocol.createSession(mockSocket).close(true);
          expect(mocked(pino().error)).toHaveBeenCalledTimes(1);
        });

        test('invokes the test callback', () => {
          return new Promise((resolve) => {
            timeProtocol.createSession(mockSocket, resolve).close(true);
          });
        });
      });

      describe('without errors', () => {
        test('invokes the test callback', () => {
          return new Promise((resolve) => {
            timeProtocol.createSession(mockSocket, resolve).close(false);
          });
        });

        test('does not log', () => {
          timeProtocol.createSession(mockSocket).close(false);
          expect(mocked(pino().error)).not.toHaveBeenCalled();
        });
      });
    });
  });
});
