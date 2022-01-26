import { exit } from 'std';
import { kill, SIGTERM, sleep, WNOHANG } from 'os';
import { assert, randStr } from './common.js';
import { spawn, wait4 } from './spawn.js';
import Client from './client.js';

function TestClient(url) {
  const message = randStr(100);

  return Client(url, {
    onConnect(ws, req) {
      ws.send(message);
    },
    onClose(ws, reason) {
      console.log('onClose', { ws, reason });
      exit(0);
    },
    onError(ws, error) {
      console.log('onError', { ws, error });
      exit(1);
    },
    onMessage(ws, msg) {
      console.log('onMessage', { ws, msg });
      const exitCode = +!(`ECHO: ${message}` == msg);
      ws.close(1000 + exitCode);
    }
  });
}

function main(...args) {
  let pid = spawn('server.js', 'localhost', 30000);
  let status = [];

  sleep(50);

  TestClient('ws://localhost:30000/ws');

  kill(pid, SIGTERM);
  wait4(pid, status, WNOHANG);
  console.log('status', status);
}

try {
  main(...scriptArgs.slice(1));
} catch(error) {
  console.log(`FAIL: ${error && error.message}\n${error && error.stack}`);
  exit(1);
}
