import type { Socket } from 'net';

export type PipsqueakSession = {
  handleData(data: Buffer): void;
  end(): void;
};

export interface PipsqueakProtocol {
  id: number;
  createSession(socket: Socket): PipsqueakSession;
}
