// comments are not in compiled binary
//import * as uv from 'uv';
"use strict";

function test(){ console.log("from CB"); }

async function main(){ try{
  //const math = await import('math');
  // let X=[4.8,1.21,2.385];
  // let Y=new Array();
  // let Z=new Array();
  // let sum=0;
  // for (let i of X)
  //   Y.push(Math.pow(Math.E,i));
  // console.log(Y);
  // for (let i of Y)
  //   sum+=i;
  // for (let i of Y)
  //   Z.push(i/sum);
  // console.log(Z);
  // sum=0;
  // for (let i of Z)
  //   sum+=i;
  // console.log(sum);
  const fs = await import('uv');
  let fp=fs.openFile('/scripts/main.js','rw',(err)=>{
    if(!err){
      fp.read(-1,10,(err,data)=>{
        console.log("read: ",err,data);
      })
    }else{
      console.log(err);
    }
  });



}catch(e){console.log(e);}}
//main();

//	const m = await import('scripts/module.js');
//	const fs = await import('uv');
//	m.print("from module");
//	setTimeout(test,0);
//	let fp=fs.createFileStream();
//	fp.open("test.txt","w+",(err)=>{
//        console.log("returned ",err);
//        fp.write("Hello!",-1,function(err){console.log("write: ",err);});
//        fp.read(-1,10,function (err,data){
//            console.log("read: ",err,data);
//        });
//	});
//    fp.open("scripts/main.js","r",(err)=>{console.log("returned ",err);});
//    fp.open("scripts/fs.mjs","r",(err)=>{console.log("returned ",err);});

let tmp1=0.0, tmp2=0.0, acc=0.0, den=0.0, num=0.0

function extract_Digit(nth){
  tmp1 = num * nth;
  tmp2 = tmp1 + acc;
  tmp1 = BigFloat.floor(tmp2 / den);
  return tmp1
}

function eliminate_Digit(d) {
  acc = acc - den * d;
  acc = acc * 10;
  num = num * 10;
}

function next_Term(k) {
  let k2=k*2+1;
  acc = acc + num * 2;
  acc = acc * k2;
  den = den * k2;
  num = num * k;
}

function main1() {
    let n=BigFloat(100.0);
    tmp1 = BigFloat(0.0) ;
    tmp2 = BigFloat(0.0);
    acc = BigFloat(0.0);
    den = BigFloat(1.0);
    num = BigFloat(1.0);
    let i=BigFloat(0.0);
    let k=BigFloat(0.0);
    for (; i < n; i++) {
      k+=1;
      next_Term(k);
      if (num > acc)continue;
      let d=extract_Digit(3);
      console.log(d);
      if (d!=extract_Digit(4)) continue;
      i+=1;
      eliminate_Digit(d);
    }
}
//main1();
let a=["5","4"];
let b=["5","4"]
let c = a==b;
console.log(c);

