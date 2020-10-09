import setStatusFlag from '../../../../src/apps/pipsqueak/protocols/setStatusFlag';
import {
  STATUS_MASK_BUSY,
  STATUS_MASK_DEVICE_NOT_REGISTERED,
} from '../../../../src/apps/pipsqueak/protocols/constants';

describe('setStatusFlag', () => {
  test('preserves existing flags', () => {
    // tslint:disable-next-line:no-bitwise
    const expectedStatus = STATUS_MASK_DEVICE_NOT_REGISTERED | STATUS_MASK_BUSY;
    expect(
      setStatusFlag(STATUS_MASK_DEVICE_NOT_REGISTERED, STATUS_MASK_BUSY),
    ).toBe(expectedStatus);
  });

  test('adds new flag', () => {
    expect(setStatusFlag(0, STATUS_MASK_BUSY)).toBe(STATUS_MASK_BUSY);
  });
});
