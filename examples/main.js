"use strict";
//import * as uv from 'uv';


function onOpen(err){
    if(!err){
        fp.read(-1,10,(err,data)=>{
            console.log("read: ",err,new String(new Int8Array(data)));
        });
    }else{
        console.log(err);
    }
}
//let fp=new uv.File('examples/PI.js','rw',onOpen);





function onrecv(val,ip,port){
    console.log("Recived from ",ip,port);
    const view = new Int8Array(val);
    if(view[0]==6)
        uv.sim(0x27);
    else if(view[0]==4)
        uv.sim(0x25);
    else if(view[0]==2)
        uv.sim(0x26);
    else if(view[0]==8)
        uv.sim(0x28);
    else if(view[0]==0x41)
        v.stop_recv();
    else
        console.log("Unknown: ",view[0]);
}
function eve(){
    v.stop_recv();
}



async function main(){
    try{
    //let uv=await import('uv');
    let uv=await import('/usr/local/lib/libmod_uv.so');
    let sip=new uv.sockaddr("0.0.0.0",10000);
    let v = new uv.udp();
    v.bind(sip);
    //let sqlite=await import('sqlite');
    let sqlite=await import('/usr/local/lib/libmod_sqlite.so');
    v.start_recv(onrecv);
    let db=new sqlite.sqlite3("test.db");
    //db.exec("CREATE TABLE BTC_1m(time INTEGER PRIMARY KEY, open INTEGER, close INTEGER);",()=>{
    db.exec("SELECT * FROM BTC_1m",()=>{});
    //console.log(sqlite);
    }catch(e){
        console.log(e);
    }
}
main();



//    v= new uv.udp();
//    v.bind(ip);
//    v.startRecv(onrecv);
//    clearTimeout(this);
//    setInterval(eve2, 1000);

//setTimeout(eve,2000);
//setInterval(eve, 1000);

//setTimeout(async () => {
//    console.log("CALLED!");
//    let uv = await import('uv');
//    console.log("CALLED!");
//    uv.openFile('./scripts/main.js','rw',(err)=>{
//        if(!err){
//            fp.read(-1,10,(err,data)=>{
//                console.log("read: ",err,data);
//            })
//        }else{
//            console.log(err);
//        }
//    });
//    console.log("CALLED!");
//}, 1);
