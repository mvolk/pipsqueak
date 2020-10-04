import pipsqueak from './servers/pipsqueak';
import pino from 'pino';

const LOGGER = pino({ name: 'CiderServer' });

const pipsqueakServer = pipsqueak.createServer();
pipsqueakServer.listen(9001, () => {
  LOGGER.info('Listening for inbound pipsqueak messages');
});
