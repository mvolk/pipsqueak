import type { Socket } from 'net';

export type PipsqueakSession = {
  handleData(data: Buffer): void;
  end(): void;
};

export interface PipsqueakProtocol {
  id: number;
  createSession(socket: Socket): PipsqueakSession;
}

export interface Device {
  id: number;
}

export interface DeviceWithKey extends Device {
  key: Buffer;
}

export type PipsqueakSessionState = {
  protocolID: number;
  deviceID?: number;
  device?: DeviceWithKey;
  timestamp?: number;
  challenge?: number;
  statusCode: number;
  complete: boolean;
};

export type PipsqueakMessageParser = (
  buffer: Buffer,
  message: PipsqueakSessionState,
) =>
  | { parser: PipsqueakMessageParser; buffer: Buffer }
  | Promise<{ parser: PipsqueakMessageParser; buffer: Buffer }>;
