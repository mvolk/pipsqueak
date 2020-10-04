const mockPino = {
  error: jest.fn(),
  info: jest.fn(),
};

jest.mock('pino', () => {
  return jest.fn(() => mockPino);
});
