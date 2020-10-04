import { Socket } from 'net';
import PipsqueakApp from '../../../src/apps/pipsqueak';

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
  });
});
