import { getDeviceWithKey } from '../../src/dao';

describe('dao', () => {
  // getDeviceWithKey is currently just a stub
  describe('getDeviceWithKey', () => {
    test('returns a Promise that resolves to nothing', () => {
      return getDeviceWithKey(1).then((value) => {
        expect(value).toBeUndefined();
      });
    });
  });
});
