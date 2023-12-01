// comments are not in compiled binary
import * as uv from 'uv';
//"use strict";
//uv=await import('uv');


let ip=new uv.sockaddr6("::1",10000);
//let ip=new uv.sockaddr("127.0.0.1",10000);
let v= new uv.udp();
v.connect(ip);
v.try_send("a");
