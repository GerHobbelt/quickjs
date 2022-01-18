import * as std from 'std';
import * as os from 'os';
import inspect from 'inspect';
import * as net from 'net';
import { Console } from 'console';

const connections = new Set();

function main(...args) {
  const base = scriptArgs[0].replace(/.*\//g, '').replace(/\.[a-z]*$/, '');
  globalThis.console = new Console({ inspectOptions: { compact: 2, customInspect: true } });

  const sslCert = 'localhost.crt',
    sslPrivateKey = 'localhost.key';

  if(args.length == 0) args.push('https://github.com/rsenn?tab=repositories');

  function createWS(url, callbacks, listen = 0) {
    let matches = [...url.matchAll(/([:\\/]+|[^:\\/]+)/g)].map(a => a[0]);
    let [protocol, , host] = matches.splice(0, 3);
    let port;
    if((matches[0] == ':' && (matches.shift(), true)) || !isNaN(+matches[0])) port = +matches.shift();
    else port = { https: 443, http: 80 }[protocol];

    console.log('matches', { matches });
    let path = matches.join('');
    let [location, query] = path.split(/\?/);
    console.log('matches', { location, query });

    //    const path = location.reduce((acc, part) => acc + '/' + part, '');
    net.setLog(/* net.LLL_USER |*/ (net.LLL_WARN << 1) - 1, (level, msg) => {
      const l = ['ERR', 'WARN', 'NOTICE', 'INFO', 'DEBUG', 'PARSER', 'HEADER', 'EXT', 'CLIENT', 'LATENCY', 'MINNET', 'THREAD'][level && Math.log2(level)] ?? level + '';
      if(l == 'MINNET') console.log(('X', l).padEnd(8), msg.replace(/\r/g, '\\r').replace(/\n/g, '\\n'));
    });

    console.log('createWS', { protocol, host, port, location, listen });

    const fn = [net.client, net.server][+listen];
    return fn({
      block: false,
      sslCert,
      sslPrivateKey,
      protocol,
      ssl: protocol == 'wss',
      host,
      port,
      path,
      ...callbacks,
      onConnect(ws, req) {
        connections.add(ws);
        //console.log('onConnect', ws, req);
        try {
          console.log(`Connected to ${protocol}://${host}:${port}${path}`, true);
        } catch(err) {
          console.log('error:', err.message);
        }
      },
      onClose(ws, status, reason, error) {
        connections.delete(ws);
        console.log('onClose', { ws, status, reason, error });
        std.exit(status != 1000 ? 1 : 0);
      },
      onHttp(req, rsp) {
        const { url, method, headers } = req;
        console.log('\x1b[38;5;82monHttp\x1b[0m(\n\t', Object.setPrototypeOf({ url, method, headers }, Object.getPrototypeOf(req)), ',\n\t', rsp, '\n)');
        return rsp;
      },
      onFd(fd, rd, wr) {
        console.log('onFd', fd, rd, wr);
        os.setReadHandler(fd, rd);
        os.setWriteHandler(fd, wr);
      },
      onMessage(ws, msg) {
        console.log('onMessage', { ws, msg });

        std.puts(msg);
      },
      onError(ws, error) {
        console.log('onError', ws, error);
      }
    });
  }
  Object.assign(globalThis, {
    get connections() {
      return [...connections];
    }
  });

  for(let arg of args) {
    createWS(arg, {}, false);
  }

  function quit(why) {
    console.log(`quit('${why}')`);
    std.exit(0);
  }
}
try {
  main(...scriptArgs.slice(1));
} catch(error) {
  console.log(`FAIL: ${error && error.message}\n${error && error.stack}`);
  std.exit(1);
} finally {
  //console.log('SUCCESS');
}
