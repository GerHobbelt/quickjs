import { LLL_CLIENT } from 'net.so';
import { LLL_DEBUG } from 'net.so';
import { LLL_ERR } from 'net.so';
import { LLL_EXT } from 'net.so';
import { LLL_HEADER } from 'net.so';
import { LLL_INFO } from 'net.so';
import { LLL_LATENCY } from 'net.so';
import { LLL_NOTICE } from 'net.so';
import { LLL_PARSER } from 'net.so';
import { LLL_THREAD } from 'net.so';
import { LLL_USER } from 'net.so';
import { LLL_WARN } from 'net.so';
import { setLog } from 'net.so';
import { err } from 'std';
let logName;

export const Levels = (() => {
  const llObj = {
    LLL_ERR,
    LLL_WARN,
    LLL_NOTICE,
    LLL_INFO,
    LLL_DEBUG,
    LLL_PARSER,
    LLL_HEADER,
    LLL_EXT,
    LLL_CLIENT,
    LLL_LATENCY,
    LLL_USER,
    LLL_THREAD
  };
  return Object.keys(llObj).reduce((acc, n) => {
    let v = Math.log2(llObj[n]);
    if(Math.floor(v) === v) acc[llObj[n]] = n.substring(4);
    return acc;
  }, {});
})();

export const isDebug = () => scriptArgs.some(a => /^-[dx]/.test(a));

export const DebugCallback = fn => (isDebug() ? fn : () => {});

export const DefaultLevels =
  LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_CLIENT | LLL_LATENCY | LLL_USER | LLL_THREAD;

export const Init = (name, mask = LLL_USER | ((LLL_CLIENT << 1) - 1)) => {
  if(typeof name == 'string') logName = name;
  setLog(
    mask,
    DebugCallback((level, msg) => {
      let l = Levels[level];
      /*  if(!(level & mask)) return;
      if(level >= LLL_NOTICE && level <= LLL_EXT) return;*/
      if(l == 'USER') l = name ?? l;
      err.puts(`${l.padEnd(10)} ${msg}\n`);
    })
  );
};

export const SetLog = (name, maxLevel = LLL_CLIENT) => {
  if(typeof name == 'string') logName = name;

  setLog(LLL_USER | ((maxLevel << 1) - 1), (level, msg) => {
    let l = Levels[level] ?? 'UNKNOWN';
    if(l == 'USER') l = name ?? l;
    err.puts(('X', l).padEnd(9) + msg.replace(/\r/g, '\\r').replace(/\n/g, '\\n'));
  });
};

export const log = (() => {
  logName ??= scriptArgs[0].replace(/.*\//g, '');
  let cons = console;
  import('console').then(({ Console }) => {
    globalThis.console = cons = new Console(
      /*err, */ {
        inspectOptions: { compact: 1, depth: 10, customInspect: true, maxStringLength: 1000, colors: true }
      }
    );
  });
  return (...args) => cons.log(logName + ':', ...args);
})();