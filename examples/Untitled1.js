
function Foo() {

    function Bar(arg) {

        async function Baz(arg1, arg2) {
            function empty() {
            }

            function pro() {

                function zero() {
                    return 0;
                }

                function inner() {
                    function inn(a) {
                        return inn(empty);
                    }
                    return 0;
                }

                Object.defineProperty(zero, 1, { configurable: true, get: inner, set: inner });

                const apple = new Promise(()=>{});

                Object.defineProperty(arg2, "constructor", { configurable: true, value: zero });

                apple.finally(arg);
                return apple;
            }

            const banana = new Promise(pro);

            async function* asy_func(a) {
                return a;
            }
            const bear = asy_func(0);

            Object.defineProperty(bear, "g", { configurable: true, enumerable: true, get: empty });
            await banana;
            return banana;
        }

        Baz(Baz, Foo, arg);
        return arg;
    }

    new Promise(Bar);
}

function test() {
    try {
    	//test();
    }
    catch (e) {
    	console.log(e);
    }
    new Foo();
}

test();
