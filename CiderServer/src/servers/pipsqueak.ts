import net from 'net';
import pino from 'pino';
import type PipsqueakApp from '../apps/pipsqueak';
import type { Socket, Server } from 'net';
import type { PipsqueakSession } from '../types';

export type Options = {
  maxConnections?: number;
  socketTimeoutMs?: number;
};

const CONFIG = {
  maxConnections: 5,
  socketTimeoutMs: 1000,
};

const LOGGER = pino({ name: 'PipsqueakServer' });

function createServer(app: PipsqueakApp): Server {
  function handleConnection(socket: Socket): void {
    socket.setTimeout(CONFIG.socketTimeoutMs);

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
        LOGGER.error('Received FIN before any data was received');
        socket.end();
      }
    });

    socket.on('error', (err) => {
      if (session) {
        session.error(err);
      } else {
        LOGGER.error(err);
      }
    });

    socket.on('timeout', () => {
      socket.destroy(new Error(`Socket timeout (${CONFIG.socketTimeoutMs}ms)`));
    });

    socket.on('close', (hadError: boolean) => {
      if (session) {
        session.close(hadError);
      } else if (hadError) {
        LOGGER.error('Socket closed with error');
      }
    });
  }

  const server = net.createServer({ allowHalfOpen: true }, handleConnection);

  server.on('error', (err) => {
    LOGGER.error(err);
  });

  server.maxConnections = CONFIG.maxConnections;

  return server;
}

export default {
  createServer,
};
