module.exports = {
  roots: ['<rootDir>/test/'],
  transform: {
    '^.+\\.tsx?$': 'ts-jest',
  },
  preset: 'ts-jest',
  testEnvironment: 'node',
  setupFiles: ['<rootDir>/test/setup/pino.ts'],
  clearMocks: true,
  restoreMocks: true,
};
