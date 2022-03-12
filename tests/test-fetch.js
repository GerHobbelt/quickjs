import { exit, puts, open } from 'std';
import { fetch, setLog, logLevels, LLL_INFO, LLL_USER } from 'net';

function FetchNext(array) {
  return new Promise((resolve, reject) => {
    let url = array.shift();
    console.log(`fetching \x1b[1;33m${url}\x1b[0m`);
    let promise = fetch(url, {});

    console.log('FetchNext', promise);

    promise
      .then(response => {
        let buf = response.arrayBuffer();
        let text = response.text();
        console.log('response', { buf, text });

        if(array.length) FetchNext(array);
        else resolve();
      })
      .catch(error => reject(error));
  });
}

function main(...args) {
  if(args.length == 0) args = ['http://www.w3.org/'];

  let logfile = std.open('test-fetch.log', 'w+');

  setLog(-1, (level, msg) => {
    //if(level < LLL_INFO || level == LLL_USER)
    logfile.puts(logLevels[level].padEnd(10) + msg + '\n');
    logfile.flush();
  });

  import('console')
    .then(({ Console }) => {
      console.log('Console', Console);
      globalThis.console = new Console({
        inspectOptions: { compact: 0, depth: 0, maxArrayLength: 10, maxStringLength: 30 }
      });
      run();
    })
    .catch(() => {
      run();
    });

  function run() {
    let promise = FetchNext(args);
    console.log('promise', promise);

    promise
      .then(() => {
        console.log('SUCCEEDED');
      })
      .catch(err => {
        console.log('FAILED:', err);
      });
  }
}

try {
  main(...scriptArgs.slice(1));
} catch(error) {
  console.log(`FAIL: ${error && error.message}\n${error && error.stack}`);
  exit(1);
}
