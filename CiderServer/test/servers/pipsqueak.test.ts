import net from 'net';
import pino from 'pino';
import pipsqueak from '../../src/servers/pipsqueak';
import { mocked } from 'ts-jest/utils';
import type { PipsqueakSession } from '../../src/types';
import PipsqueakApp from '../../src/apps/pipsqueak';

type ConnectionEventListener = (socket: net.Socket) => void;
type ErrorEventListener = (err: Error) => void;
type DataEventListener = (data: Buffer) => void;
type VoidEventListener = () => void;
type CloseEventListener = (hadError: boolean) => void;

jest.mock('net');
jest.mock('../../src/apps/pipsqueak');

function findListener(
  eventEmitter: jest.Mocked<net.Socket> | jest.Mocked<net.Server>,
  type: string,
): unknown {
  // @ts-ignore implicit any
  function match(args) {
    return args[0] && args[0] == type;
  }
  // @ts-ignore overloaded function
  const call = eventEmitter.on.mock.calls.find(match);
  return call && call[1];
}

describe('pipsqueak', () => {
  const mockNet = net as jest.Mocked<typeof net>;
  let mockServer: jest.Mocked<net.Server>;
  let mockSocket: jest.Mocked<net.Socket>;
  let mockSession: PipsqueakSession = {
    handleData: jest.fn(),
    end: jest.fn(),
  };
  let mockPipsqueakApp: jest.Mocked<PipsqueakApp>;

  beforeEach(() => {
    mockServer = new net.Server() as jest.Mocked<net.Server>;
    mockNet.createServer.mockReturnValue(mockServer);
    mockSocket = new net.Socket() as jest.Mocked<net.Socket>;
    mockPipsqueakApp = new PipsqueakApp() as jest.Mocked<PipsqueakApp>;
  });

  describe('.createServer(pipsqueakApp)', () => {
    test('sets max connections to 5', () => {
      expect(pipsqueak.createServer(mockPipsqueakApp).maxConnections).toBe(5);
    });

    describe('connectionListener passed to net.Server', () => {
      let connectionListener: ConnectionEventListener;

      beforeEach(() => {
        pipsqueak.createServer(mockPipsqueakApp);
        // @ts-ignore overloaded function + possibly undefined
        connectionListener = mockNet.createServer.mock.calls[0][0];
      });

      test('sets socket timeout to 1000 ms', () => {
        connectionListener(mockSocket);
        expect(mockSocket.setTimeout.mock.calls[0][0]).toBe(1000);
      });

      describe('socket event listeners', () => {
        let dataListener: DataEventListener;
        let endListener: VoidEventListener;
        let errorListener: ErrorEventListener;
        let timeoutListener: VoidEventListener;
        let closeListener: CloseEventListener;

        beforeEach(() => {
          connectionListener(mockSocket);
          dataListener = findListener(mockSocket, 'data') as DataEventListener;
          endListener = findListener(mockSocket, 'end') as VoidEventListener;
          errorListener = findListener(
            mockSocket,
            'error',
          ) as ErrorEventListener;
          timeoutListener = findListener(
            mockSocket,
            'timeout',
          ) as VoidEventListener;
          closeListener = findListener(
            mockSocket,
            'close',
          ) as CloseEventListener;
        });

        describe('with an established session', () => {
          beforeEach(() => {
            mocked(mockPipsqueakApp.createSession).mockReturnValue(mockSession);
            dataListener(Buffer.from([]));
            mocked(mockSession.handleData).mockReset();
            mocked(mockPipsqueakApp.createSession).mockReset();
          });

          describe('socket data event listener', () => {
            test('does not invoke the PipsqueakApp.createSession method again', () => {
              dataListener(Buffer.from([]));
              expect(mockPipsqueakApp.createSession).not.toHaveBeenCalled();
            });

            test('delegates to the session', () => {
              const buffer = Buffer.from([]);
              dataListener(buffer);
              expect(mockSession.handleData).toHaveBeenCalledTimes(1);
              expect(mockSession.handleData).toHaveBeenCalledWith(buffer);
            });

            describe('when an error is thrown', () => {
              test('destroys the socket', () => {
                const err = new Error();
                mocked(mockSession.handleData).mockImplementation(() => {
                  throw err;
                });
                dataListener(Buffer.from([]));
                expect(mockSocket.destroy).toHaveBeenCalledTimes(1);
                expect(mockSocket.destroy).toHaveBeenCalledWith(err);
              });
            });
          });

          describe('socket end event listener', () => {
            test('calls session.end()', () => {
              expect(endListener).toBeDefined();
              endListener();
              expect(mockSession.end).toHaveBeenCalledTimes(1);
            });
          });

          describe('socket error event listener', () => {
            test('does not emit a log message', () => {
              const err = new Error();
              expect(errorListener).toBeDefined();
              errorListener(err);
              expect(mocked(pino().error)).not.toHaveBeenCalled();
            });
          });

          describe('socket timeout event listener', () => {
            test('calls socket.destroy with an error', () => {
              timeoutListener();
              expect(mockSocket.destroy).toHaveBeenCalledTimes(1);
              expect(mockSocket.destroy.mock.calls[0][0] instanceof Error).toBe(
                true,
              );
            });
          });

          describe('socket close event listener', () => {
            test('is defined', () => {
              expect(closeListener).toBeDefined();
            });

            describe('with hadError true', () => {
              test('does not emit a log message', () => {
                closeListener(true);
                expect(mocked(pino().error)).not.toHaveBeenCalled();
              });
            });

            describe('with hadError false', () => {
              test('does not emit a log message', () => {
                closeListener(false);
                expect(mocked(pino().error)).not.toHaveBeenCalled();
              });
            });
          });
        });

        describe('without an established session', () => {
          describe('socket data event listener', () => {
            test('invokes the PipsqueakApp.createSession method', () => {
              mocked(mockPipsqueakApp.createSession).mockReturnValue(
                mockSession,
              );
              const buffer = Buffer.from([]);
              dataListener(buffer);
              expect(mockPipsqueakApp.createSession).toHaveBeenCalledTimes(1);
              expect(mockPipsqueakApp.createSession).toHaveBeenCalledWith(
                mockSocket,
                buffer,
              );
            });

            test('delegates to the session', () => {
              mocked(mockPipsqueakApp.createSession).mockReturnValue(
                mockSession,
              );
              const buffer = Buffer.from([]);
              dataListener(buffer);
              expect(mockSession.handleData).toHaveBeenCalledTimes(1);
              expect(mockSession.handleData).toHaveBeenCalledWith(buffer);
            });

            describe('when an error is thrown', () => {
              test('destroys the socket', () => {
                const err = new Error();
                mocked(mockPipsqueakApp.createSession).mockImplementation(
                  () => {
                    throw err;
                  },
                );
                dataListener(Buffer.from([]));
                expect(mockSocket.destroy).toHaveBeenCalledTimes(1);
                expect(mockSocket.destroy).toHaveBeenCalledWith(err);
              });
            });
          });

          describe('socket end event listener', () => {
            test('calls socket.end()', () => {
              expect(endListener).toBeDefined();
              endListener();
              expect(mockSocket.end).toHaveBeenCalledTimes(1);
            });
          });

          describe('socket error event listener', () => {
            test('logs the error', () => {
              const err = new Error();
              expect(errorListener).toBeDefined();
              errorListener(err);
              expect(mocked(pino().error)).toHaveBeenCalledTimes(1);
              expect(mocked(pino().error)).toHaveBeenCalledWith(err);
            });
          });

          describe('socket timeout event listener', () => {
            test('calls socket.destroy with an error', () => {
              timeoutListener();
              expect(mockSocket.destroy).toHaveBeenCalledTimes(1);
              expect(mockSocket.destroy.mock.calls[0][0] instanceof Error).toBe(
                true,
              );
            });
          });

          describe('socket close event listener', () => {
            test('is defined', () => {
              expect(closeListener).toBeDefined();
            });

            describe('with hadError true', () => {
              test('emit a log message', () => {
                closeListener(true);
                expect(mocked(pino().error)).toHaveBeenCalledTimes(1);
                expect(mocked(pino().error)).toHaveBeenCalledWith(
                  'Socket closed with error',
                );
              });
            });

            describe('with hadError false', () => {
              test('does not emit a log message', () => {
                closeListener(false);
                expect(mocked(pino().error)).not.toHaveBeenCalled();
              });
            });
          });
        });
      });
    });
  });

  describe('server error event listener', () => {
    let errorListener: ErrorEventListener;

    beforeEach(() => {
      pipsqueak.createServer(mockPipsqueakApp);
      errorListener = findListener(mockServer, 'error') as ErrorEventListener;
    });

    test('logs the error', () => {
      const err = new Error();
      expect(errorListener).toBeDefined();
      errorListener(err);
      expect(mocked(pino().error)).toHaveBeenCalledTimes(1);
      expect(mocked(pino().error)).toHaveBeenCalledWith(err);
    });
  });
});
