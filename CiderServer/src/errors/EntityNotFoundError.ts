export default class EntityNotFoundError extends Error {
  constructor(entityType: string, entityID: number) {
    super(`${entityType} ${entityID} not found`);
  }
}
