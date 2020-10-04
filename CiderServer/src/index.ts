import pipsqueak from './servers/pipsqueak';
import pino from 'pino';
import PipsqueakApp from './apps/pipsqueak';

const LOGGER = pino({ name: 'CiderServer' });

const pipsqueakApp = new PipsqueakApp();

const pipsqueakServer = pipsqueak.createServer(pipsqueakApp);
pipsqueakServer.listen(9001, () => {
  LOGGER.info('Listening for inbound pipsqueak messages');
});
