import { LLL_ALL, LLL_NOTICE, LLL_USER, logLevels, createServer, setLog } from 'net';

import('console').then(({ Console }) => { globalThis.console = new Console({ inspectOptions: { compact: 0 } });
});

setLog(/*LLL_ALL |*/ (LLL_NOTICE - 1) | LLL_USER, (level, message) => console.log(logLevels[level].padEnd(10), message));

createServer(
  (globalThis.options = {
    host: '0.0.0.0',
    port: '8765',
    block: false,
    tls: true,
    protocol: 'http',
    mounts: {
      '/': ['/', '.', 'index.html'],
      '/404.html': function* (req, res) {
        console.log('/404.html', { req, res });
        yield '<html><head><meta charset=utf-8 http-equiv="Content-Language" content="en"/><link rel="stylesheet" type="text/css" href="/error.css"/></head><body><h1>403</h1></body></html>';
      },
      *generator(req, res) {
        console.log('/generator', { req, res });
        yield 'This';
        yield ' ';
        yield 'is';
        yield ' ';
        yield 'a';
        yield ' ';
        yield 'generated';
        yield ' ';
        yield 'response';
        yield '\n';
      }
    },
    onConnect(ws, req) {
      console.log('onConnect', { ws, req });
    },
    onClose(ws, status, reason) {
      console.log('onClose', { ws, status, reason });
      ws.close(status);
    },
    onError(ws, error) {
      console.log('onError', { ws, error });
    },
    onRequest(ws, req, resp) {
      console.log('onRequest', { req, resp });
      Object.assign(globalThis, { req, resp });
    },
    onMessage(ws, msg) {
      console.log('onMessage', ws.fd, msg);
      ws.send('ECHO: ' + msg);
    }
  })
);
