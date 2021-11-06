package main

/*
#cgo LDFLAGS: -lm -ldl
#define EMSCRIPTEN
#define CONFIG_VERSION "v0.0.0"

#include "quickjs.h"
*/
import "C"
import "fmt"

func main() {
	fmt.Println("hey")
}
