import EntityNotFoundError from '../errors/EntityNotFoundError';
import type { DeviceWithKey } from '../types';

// stub
export function getDeviceWithKey(deviceID: number): Promise<DeviceWithKey> {
  return Promise.reject(new EntityNotFoundError('Device', deviceID));
}
