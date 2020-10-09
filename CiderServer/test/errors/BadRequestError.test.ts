import BadRequestError from '../../src/errors/BadRequestError';

describe('BadRequestError', () => {
  test('is an instanceof Error', () => {
    expect(new BadRequestError('foo')).toBeInstanceOf(Error);
  });

  test('sources its message directly from the constructor argument', () => {
    expect(new BadRequestError('foo').message).toEqual('foo');
  });
});
