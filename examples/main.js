// comments are not in compiled binary
import * as uv from 'uv';
//"use strict";
//uv=await import('uv');

let v= new uv.udp();
v.bind(new uv.sockaddr("0.0.0.0",10000));


function onOpen(err){
    if(!err){
        fp.read(-1,10,(err,data)=>{
            console.log("read: ",err,new String(new Int8Array(data)));
        });
    }else{
        console.log(err);
    }
}


let fp=new uv.File('examples/PI.js','rw',onOpen);



function onrecv(ip,val){
    console.log("Recived from ",ip,val);
    const view = new Int8Array(val);
    if(view[0]==6)
        uv.sim(0x27);
    else if(view[0]==4)
        uv.sim(0x25);
    else if(view[0]==2)
        uv.sim(0x26);
    else if(view[0]==8)
        uv.sim(0x28);
    else if(view[0]==5)
        v.stopRecv();
}

function event(){
    uv.sim(0x2);
}
//setTimeout(event,2000);
v.startRecv(onrecv);


