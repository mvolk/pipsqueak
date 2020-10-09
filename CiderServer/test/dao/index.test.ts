import EntityNotFoundError from '../../src/errors/EntityNotFoundError';
import { getDeviceWithKey } from '../../src/dao';

describe('dao', () => {
  // getDeviceWithKey is currently just a stub
  describe('getDeviceWithKey', () => {
    test('returns a rejected Promise', () => {
      return expect(getDeviceWithKey(1)).rejects.toBeInstanceOf(
        EntityNotFoundError,
      );
    });
  });
});
