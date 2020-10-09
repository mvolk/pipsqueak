import EntityNotFoundError from '../../src/errors/EntityNotFoundError';

describe('EntityNotFoundError', () => {
  test('is an instanceof Error', () => {
    expect(new EntityNotFoundError('TestEntity', 123)).toBeInstanceOf(Error);
  });

  test('builds its message from the constructor arguments', () => {
    expect(new EntityNotFoundError('TestEntity', 123).message).toEqual(
      'TestEntity 123 not found',
    );
  });
});
