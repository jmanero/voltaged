#include <v8.h>
#include <node.h>
#include <cstdint>
#include <stdlib.h>
#include <errno.h>

#include "device.h"

using namespace std;
using namespace node;
using namespace v8;

struct SPIBaton {
  uv_work_t request;
  Persistent<Function> callback;

  int32_t error_code;
  string error_message;

  SPIDevice *device;
  uint8_t *data;
  uint32_t length;
};

void SPIOpenWorker(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);
  // ...
}

void SPITransferWorker(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);
  // ...
}

void SPICloseWorker(uv_work_t* req) {
  SPIBaton* baton = static_cast<SPIBaton*>(req->data);
  // ...
}

void SPIResult(uv_work_t* req) {
    HandleScope scope;

    SSPIBaton* baton = static_cast<SPIBaton*>(req->data);
    delete req;

    Handle<Value> argv[2];

    // XXX: Error handling
    argv[0] = Undefined();
    argv[1] = Undefined(); // TODO Create Buffer

    TryCatch try_catch;
    request->callback->Call(Context::GetCurrent()->Global(), 2, argv);
    if (try_catch.HasCaught()) FatalException(try_catch);

    request->callback.Dispose();
    delete request;
}

namespace SPIInterface {
  Handle<Value> Open(const Arguments& args) {
    HandleScope scope;

    return scope.Close(Undefined());
  }

  Handle<Value> Transfer(const Arguments& args) {
    HandleScope scope;

    if (!args[0]->IsBuffer()) return ThrowException(Exception::TypeError(
      String::New("First argumernt must be a Buffer")));
    // TODO Convert buffer to byte-array

    if (!args[1]->IsFunction()) return ThrowException(Exception::TypeError(
      String::New("Second argumernt must be a callback function")));
    Local<Function> callback = Local<Function>::Cast(args[1]);

    uv_work_t request;
    SPIBaton* baton = new SPIBaton();
    baton->callback = Persistent<Function>::New(callback);
    baton->request.data = baton;

    baton->device = NULL; // TODO Get handle
    baton->data = NULL;
    baton->length = 0;


    uv_queue_work(uv_default_loop(), &baton->request, SPITransferWorker, SPIResult);
    return scope.Close(Undefined());
  }

  Handle<Value> Close(const Arguments& args) {
    HandleScope scope;

    return scope.Close(Undefined());
  }

  Handle<Object> New(const Arguments& args) {
    HandleScope scope;

    const std::string device = args.Length() > 0 ?
      std::String(*(args[0]->toString())) : std::string("/dev/spidev0.0");
    uint8_t bits = args.Length() > 1 ? (uint8_t)(args[0]->Uint32Value()) : 8;
    uint32_t speed = args.Length() > 2 ? args[0]->Uint32Value() : 1000000;

    Handle<SPIDevice> _handle = new SPIDevice(device, SPI_MODE_0, bits, speed);
    args.This()->Set(String::NewSymbol("_handle"), _handle);

    return scope.Close(args.This());
  }

  Persistent<Function> Constructor;
}

void Init(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> device = FunctionTemplate::New(SPIInterface::New)
  device->SetClassName(String::NewSymbol("SPIDevice"));

  SetPrototypeMethod(device, "open", SPIInterface::Open);
  SetPrototypeMethod(device, "transfer", SPIInterface::Transfer);
  SetPrototypeMethod(device, "close", SPIInterface::Close);

  Constructor = Persistent<Function>::New(device->GetFunction());
  target->Set(String::NewSymbol("SPIDevice"), Constructor);
}

NODE_MODULE(SPIDevice, Init)
