import { Socket } from 'net';
import { mocked } from 'ts-jest/utils';
import PipsqueakApp from '../../../src/apps/pipsqueak';
import type { PipsqueakProtocol, PipsqueakSession } from '../../../src/types';

describe('pipsqueakApp()', () => {
  let subject: PipsqueakApp;
  let socket: Socket;

  beforeEach(() => {
    subject = new PipsqueakApp();
    socket = new Socket();
  });

  describe('.createSession(Socket)', () => {
    describe('when the buffer is empty', () => {
      const buffer = Buffer.from([]);

      test('returns null', () => {
        expect(subject.createSession(socket, buffer)).toBeNull();
      });
    });

    describe('when the protocol is not supported', () => {
      const buffer = Buffer.from([1]);

      test('throws an Error', () => {
        expect(() => subject.createSession(socket, buffer)).toThrow(Error);
      });
    });

    describe('when the protocol is supported', () => {
      const protocol255: PipsqueakProtocol = {
        id: 255,
        createSession: jest.fn(),
      };
      const buffer = Buffer.from([255]);
      const session: PipsqueakSession = {} as PipsqueakSession;

      beforeEach(() => {
        subject.use(protocol255);
        mocked(protocol255.createSession).mockReturnValue(session);
      });

      test('the createSession method of the protocol is invoked with the socket', () => {
        subject.createSession(socket, buffer);
        expect(mocked(protocol255.createSession)).toHaveBeenCalledTimes(1);
        expect(mocked(protocol255.createSession)).toHaveBeenCalledWith(socket);
      });

      test('the protocol-specific session is returned', () => {
        expect(subject.createSession(socket, buffer)).toBe(session);
      });
    });
  });
});
