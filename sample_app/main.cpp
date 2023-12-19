#include <iostream>

#include <quickjspp.hpp>
#include <quickjs-libc.h>

#include "monolithic_examples.h"


class ChartXY
{
private:
    double x = 0.0, y = 0.0;
    double width = 100.0, height = 100.0;
public:
    ChartXY()
    { }
    
    ChartXY(double w, double h): width(w), height(h) 
    { }

    void show() const 
    {
      std::cout << " [ChartXY Object] x = " << x << " ; y = " << y 
                << " ; width = " << width << " height = " << height 
                << '\n';
    }

    void set_width(double width) 
    {  
        this->width = width; 
        std::fprintf(stdout, " [ChartXY] Width set to %f \n", width);
        
    }
    
    void set_height(double height)
    { 
        this->height = height; 
        std::fprintf(stdout, " [ChartXY] Height set to %f \n", height);        
    }

    double get_height() const { return this->height; }
    double get_width () const { return this->width; }

    void plot_points(std::vector<double> const& points)
    {
        std::cout << " [ChartXY] Plotting points =>> ";
        for(auto p : points) { std::cout << " " << p; }
        std::cout << "\n";
    }
};

qjs::Value
try_eval_module(
                qjs::Context& context
              , qjs::Runtime& runtime
              , std::string const& code)
{
      try
      {
          return context.eval(code, "<eval>", JS_EVAL_TYPE_MODULE);
      } catch( const qjs::exception& ex)
      {
            //js_std_dump_error(ctx);
            auto exc = context.getException();
            std::cerr << (exc.isError() ? "Error: " : "Throw: ") << exc.toJSON() << std::endl;
            if((bool)exc["stack"])
                std::cerr << (std::string)exc["stack"] << std::endl;

            js_std_free_handlers(runtime.rt);
            return context.newObject();
      }

}

#if defined(BUILD_MONOLITHIC)
#define main		qjs_sample_app_main
#endif

int main(int argc, const char** argv)
{
    std::cout << " [INFO] Started Ok" << std::endl; 
    
    using namespace qjs;

	qjs_clear_dump_flags();
	qjs_set_dump_flags(QJS_DUMP_FLAG_LEAKS);
	qjs_set_dump_flags(QJS_DUMP_FLAG_GC_FREE);

    Runtime runtime;
    //JSRuntime* rt = runtime.rt;

    Context context(runtime);
    //JSContext* ctx = context.ctx;

    js_std_init_handlers(runtime.rt);
    
    /* loader for ES6 modules */
    JS_SetModuleLoaderFunc(runtime.rt, nullptr, js_module_loader, nullptr);
    
    js_std_add_helpers(context.ctx, argc - 1, argv + 1);

    /* system modules */
    js_init_module_std(context.ctx, "std");
    js_init_module_os(context.ctx, "os");

    std::fprintf(stderr, " [TRACE] Before loading code. \n");

    const char* str = R"(
            /*
            import * as std from 'std';
            import * as os from 'os';
            globalThis.std = std;
            globalThis.os = os;
            */

            console.log(" [QUICKJS] => =>> Script loaded. Ok. \n");

            for(n = 1; n <= 5; n++){
                console.log(` [QUICKJS-TRACE] n = ${n}/5 `);
            }

            // ----- Define user variables here ----

            asset_path = "/Users/mydir-macosx/data/blackjack.txt";
            game_score = 0.25156;

            let x = 10.352;
            datapoints = [ 0.251, 19.2363, 9.262, 100.125 ];

            console.log(`\n  [QUICKJS] asset_path = ${asset_path}` );
            console.log(`   [QUICKJS] score = ${100.0 * game_score} (in percent) \n`);
            console.log(`   [QUICKJS] data points = ${datapoints} `)
      )";

    try
    {
         context.eval(str); //, "", JS_EVAL_TYPE_MODULE);
    } catch( const qjs::exception& ex)
    {
          //js_std_dump_error(ctx);
          auto exc = context.getException();
          std::cerr << (exc.isError() ? "Error: " : "Throw: ") << exc.toJSON() << std::endl;
          if((bool)exc["stack"])
              std::cerr << (std::string)exc["stack"] << std::endl;

          js_std_free_handlers(runtime.rt);
          return 1;
    }

    std::fprintf(stderr, " [TRACE] After loading code. \n");


    int number = (int) context.eval(" 10 * (3 + 1 + 10 ) - 1000 * 2");                               
    std::cout << " [RESULT] number = " << number << '\n';
        
    std::puts("\n [*] ===== Read configuration variables defined in the js code. ====\n");    
    {
        auto var_asset_path = context.global()["asset_path"].as<std::string>();
        std::cout << "    =>> asset_path = " << var_asset_path << '\n';

        auto score = context.global()["game_score"].as<double>();
        std::cout << "    =>> game_score (%) = " << 100.0 * score << '\n';

        auto points = context.global()["datapoints"].as<std::vector<double>>();
        std::cout << "    ==>> datapoints = [" << points.size() << "]( ";
        for(auto p : points) {  std::cout << p << ' '; }
        std::cout << " ) \n";
    }

    std::puts("\n [*] ===== Define variables in C++-side  ====\n");    
    { 
        
        context.global()["user_name"]   = context.newValue("Gaius Julius Caesar");
        context.global()["user_points"] = context.newValue(101235);

        auto data = std::vector<std::string>{ "ADA", "RUST", "C++11", "C++17", "C++20"
                                            , "Dlang", "OCaml", "C#(Csharp)" };

        context.global()["user_data"] = context.newValue(data);         

        // Note: This code should be within an exception handler. 
        context.eval(R"(
            console.log(` [STEP 2] user_name = ${user_name} ; points = ${user_points} `);
            console.log(` [STEP 2] user_data = ${user_data} ; type = ${ typeof(user_data) } `);
            console.log(` [STEP 2] user_data[5] = ${ user_data[5] } `)

            // Iterate over the array 
            for(let x in user_data){ console.log(user_data[x]); }
        )");          

    }

    std::puts("\n [*] ===== Register class ChartXY   ====\n");    

    auto& module = context.addModule("chart");
    module.class_<ChartXY>("ChartXY")
      .constructor() 
      .constructor<double, double>()
      .fun<&ChartXY::show>("show")
      .fun<&ChartXY::set_height>("set_height")
      .fun<&ChartXY::set_width>("set_width")
      .fun<&ChartXY::plot_points>("plot_points")
      .property<&ChartXY::get_width,  &ChartXY::set_width>("width")      
      .property<&ChartXY::get_height, &ChartXY::set_height>("height")      
      ;  

    module.add("user_path", js_traits<const char *>::wrap(context.ctx, "/Users/data/assets/game/score/marks"));
    module.add("user_points", js_traits<int>::wrap(context.ctx, 1023523));

    module.function("myfunc", [](double x, double y){ return 4.61 * x + 10 * y * y; });

    const char* module_code = R"(
        import { ChartXY } from "chart";

        import * as chart from "chart"

        console.log(` [SCRIPT] chart.user_path = ${chart.user_path} \n\n`);
        console.log(` [SCRIPT] chart.user_points = ${chart.user_points} \n\n`);

        console.log(` [SCRIPT] Result = ${ chart.myfunc(5.61, 9.821) } \n`);

        let ch = new ChartXY(200, 600);
        ch.show();

        ch.set_width(800.0);
        ch.set_height(700.0)
        ch.show();

        console.log("   [QUICKJS] Change chart dimensions using properties ");
        ch.width = 500;
        ch.height = 660;

        console.log(`\n   <QUICKJS> Chart width = ${ch.width} ; Chart height = ${ch.height} \n`);

        ch.plot_points( [ 10.522, 8.261, -100.24, 7.2532, 56.123, 89.23 ] );
    )";

    try_eval_module(context, runtime, module_code);

    js_std_loop(context.ctx);
    // ----- Shutdown virtual machine ---------------// 
    js_std_free_handlers(runtime.rt);

    return 0;
}
