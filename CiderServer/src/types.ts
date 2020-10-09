import type { Socket } from 'net';

export type PipsqueakSession = {
  handleData(data: Buffer): void;
  end(): void;
  error(err: any): void;
  close(hadErrors: boolean): void;
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
  expectedRequestSize: number;
  request: Buffer;
  statusCode: number;
  deviceID?: number;
  timestamp?: number;
  challenge?: number;
  device?: DeviceWithKey;
  authentic?: boolean;
};
