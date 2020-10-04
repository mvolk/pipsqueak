import net from 'net';
import pino from 'pino';
import PipsqueakApp from '../apps/pipsqueak';
import type { Socket, Server } from 'net';
import type { PipsqueakSession } from '../types';

export type Options = {
  maxConnections?: number;
  socketTimeoutMs?: number;
};

const DEFAULTS = {
  maxConnections: 5,
  socketTimeoutMs: 1000,
};

const LOGGER = pino({ name: 'PipsqueakServer' });

function createServer(options: Options = {}): Server {
  const config = { ...DEFAULTS, ...options };
  const app = new PipsqueakApp();

  function handleConnection(socket: Socket): void {
    socket.setTimeout(config.socketTimeoutMs);

    let session: PipsqueakSession | null = null;

    socket.on('data', (data: Buffer) => {
      try {
        if (!session) session = app.createSession(socket, data);
        if (session) session.handleData(data);
      } catch (err) {
        socket.destroy(err);
      }
    });

    socket.on('end', () => {
      if (session) {
        session.end();
      } else {
        socket.end();
      }
    });

    socket.on('error', (err) => {
      if (!session) LOGGER.error(err);
    });

    socket.on('timeout', () => {
      socket.destroy(new Error(`Socket timeout (${config.socketTimeoutMs}ms)`));
    });

    socket.on('close', (hadError: boolean) => {
      if (hadError && !session) LOGGER.error('Socket closed with error');
    });
  }

  const server = net.createServer(handleConnection);

  server.on('error', (err) => {
    LOGGER.error(err);
  });

  server.maxConnections = config.maxConnections;

  return server;
}

export default {
  createServer,
};
