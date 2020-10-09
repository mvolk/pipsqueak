import { getDeviceWithKey } from '../../../dao';
import BadRequestError from '../../../errors/BadRequestError';
import type { PipsqueakSessionState } from '../../../types';

export default async function loadDeviceWithKey(state: PipsqueakSessionState) {
  const { deviceID } = state;

  if (!deviceID) {
    // zero is not a valid deviceID
    throw new BadRequestError(`DeviceID not specified in time request`);
  }

  state.device = await getDeviceWithKey(deviceID);
}
