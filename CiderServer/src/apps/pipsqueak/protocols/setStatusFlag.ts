export default function setStatusFlag(statusCode: number, flagMask: number) {
  // tslint:disable-next-line:no-bitwise
  return statusCode | flagMask;
}
