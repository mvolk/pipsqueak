import type { Socket } from 'net';
import type { PipsqueakProtocol } from '../../types';

export default class PipsqueakApp {
  private protocolByID;

  constructor() {
    this.protocolByID = new Map<number, PipsqueakProtocol>();
  }

  use(protocol: PipsqueakProtocol) {
    this.protocolByID.set(protocol.id, protocol);
  }

  createSession(socket: Socket, data: Buffer) {
    if (!data.length) return null;
    const protocolID = data.readUInt8(0);
    const protocol = this.protocolByID.get(protocolID);
    if (!protocol) {
      throw new Error(`Protocol ${protocolID} is not supported`);
    }
    return protocol.createSession(socket);
  }
}
